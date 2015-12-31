/*
 * This file is part of Raspberry Pi steuert Modelleisenbahn
 * Copyright(C)  2008 - 2015 Kurt Zerzawy www.zerzawy.ch
 * 
 * Raspberry Pi steuert Modelleisenbahn is free software:
 * you can redistribute it and/or modify it under the terms of the
 * GNU General Public Licence as published by the Free Software Foundation,
 * either version 3 of the Licence, or (at your option) any later version.
 *
 * Raspberry Pi steuert Modelleisenbahn is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
 * PURPOSE. See GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Raspberry Pi steuert Modelleisenbahn. 
 * If not, see <http://www.gnu.org/licenses/>.
 *
 * Spreitenbacher Eisenbahn Amateur Klub SPEAK
 * www.speak.li
 */

/**
 * \file
 * TCP/IP communication implementation
 * \author
 * Kurt Zerzawy
 */

/*
 * $Author: Kurt $
 * $Date: 2015-08-05 08:06:23 +0200 (Mi, 05 Aug 2015) $
 * $Revision: 2537 $
 */

#include <stdbool.h>
#include <stdio.h>
#include <sys/un.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "hw.h"		/* for _Bool	*/
#include "registers.h"
#include "outshift.h"
#include "inshift.h"
#include "vars.h"
#include "assert.h"
#include "log.h"
#include "parser/parser.h"	/* for parserGetStation name	*/
#include "comm.h"

struct ssock 
{
	char *   outString;
	char *   inString;
	uint16_t port;
};

static struct ssock     * sock;
static  pthread_t         sockThread;
static _Bool              sockRunning = false;
static _Bool              sockKill = false;
static _Bool              forcing = false;
static struct sRegister * pRegister = NULL;
static unsigned short     outLen = 0;
static unsigned short     inLen  = 0;
static unsigned short     varLen = 0;
static unsigned short     commTime = 0;
static char             * pOutString = NULL;
static char             * pInString  = NULL;

#define REC_STRING_LEN 64

/* 
 * note that the values send and receive are relative to the
 * TCP/IP socket as seen from the UNC20.
 */

static const char inEnabled[]   	= "inshift enabled";
static const char inDisabled[]  	= "inshift disabled";
static const char outEnabled[]  	= "outshift enabled";
static const char outDisabled[] 	= "outshift disabled";
static const char outForced[]   	= "outshift forced";
static const char outReleased[] 	= "outshift released";
static const char myname[]      	= "Stellwerk Bad Waldsee Version " __VERSION__ 
																	 " vom " __DATE__ " " __TIME__ ;
static const char bye[]         	= "good bye!";
static const char getap216[]    	= "GET AP216"; 
static const char getep16[]	    	= "GET EP16"; 
static const char putap216[]    	= "PUT AP216 "; 
static const char putep16[]	    	= "PUT EP16 "; 
static const char quit[]	    	= "QUIT"; 
static const char inshiftenable[] 	= "INSHIFT ENABLE";
static const char outshiftenable[]	= "OUTSHIFT ENABLE";
static const char inshiftdisable[]	= "INSHIFT DISABLE";
static const char outshiftdisable[]	= "OUTSHIFT DISABLE";
static const char outshiftforce[] 	= "OUTSHIFT FORCE";
static const char outshiftrelease[]	= "OUTSHIFT RELEASE";
static const char station[]			= "STATION";
static const char who[]				= "WHO";

void * sockAccept(void * ptr);

/**
 * socket function used as a thread
 * @param ptr		is struct ssock * 
 */
void * sockFunction(void * ptr) 
{
	struct ssock     * psock;
	int                serverSock = 0;
	int                newSock = 0;
	char               recString[REC_STRING_LEN];
	struct             sockaddr_in server;
	int                errCode;
	int                errno;
	int                strLen;
	char             * answer;
	char             * cpos = NULL;
	
	LOG_INF("started");

	sockRunning = true;
	psock = (struct ssock *) ptr;
	
	/* make a server socket with given address	*/
	if(0 > (serverSock = socket(AF_INET, SOCK_STREAM, 0))) 
	{
		LOG_ERR("error making socket");
		sockRunning = false;
		pthread_exit(NULL);
	}
	
	/* set nonblock flag in order not to block
	 * other important applications
	 */
	if(0 > fcntl(serverSock, F_SETFL, O_NONBLOCK)) 
	{
		LOG_ERR("setting control flags"); 
		close(serverSock);
		sockRunning = false;
		pthread_exit(NULL);	
	}
	
	/* 
	 * any IP address is ok
	 * port address is 50500
	 */
	server.sin_family = AF_INET;         /* internet domain */
	server.sin_addr.s_addr = INADDR_ANY; /* anyone allowed to connect */
	server.sin_port = htons(psock->port); /* attach port */

	LOG_INF("attach IP address");

	/*
	 * bind server socket, only possible if port not in use.
	 * Otherwise error will occur
	 */
	if(0 > bind(serverSock, 
				(struct sockaddr *) &server, 
				(socklen_t) sizeof(struct sockaddr_in))) 
	{
		/* cannot be bound, socket may be in use	*/
		LOG_ERR("binding to server socket");
		close(serverSock);
		sockRunning = false;
		pthread_exit(NULL);	
	}
	
	/*
	 * tell that the server socket is ready to accept
	 * connections
	 */
	if(0 > listen(serverSock, 1)) 
	{
		LOG_ERR("listening to server socket");
		close(serverSock);
		sockRunning = false;
		pthread_exit(NULL);	
	}
	
	while(true) 
	{
		/*
		 * now accept incoming connections. 
		 * non blocking so the function returns if there is
		 * no client who want to connect
		 * note that errno is set to EWOULDBLOCK if there is
		 * no client.
		 */
		
		do 
		{
			if(sockKill) 
			{
			   	/* ask if communication shall be killed */
			   	close(serverSock); 
				sockRunning = false; pthread_exit(NULL);
			}

			errno = 0;
			newSock = accept(serverSock, 0, 0);
		} 
		while(EWOULDBLOCK == errno);
		
		/* would not block any more or some error	*/
		if(0 > newSock) 
		{
			LOG_ERR("accepting socket");
			close(serverSock);
			break;
		}
		else 
		{
			/* 
			 * client available. Set the flags not to make
			 * the subroutine block the rest of the program
			 */

			if(0 > fcntl(newSock, F_SETFL, O_NONBLOCK)) { 
				LOG_ERR("setting control flags");
			   	close(newSock);
			   	break;
			}
		}

		/*
		 * loop for sending and receiving
		 */
		while(true) 
		{
			/* 
			 * prepare the clients answer
			 */
			psock->inString[0] = '\0';
			
			/*
			 * receive part. We listen to the client.
			 * errCode is set to the number of received
			 * characters, if any. 
			 * If errCode is -1, then there are no datas at
			 *  the moment
			 * If errCode is less than -2, there is an error
			 */
			do 
			{
			   	errCode = recv(newSock, recString, REC_STRING_LEN, 0);
	
				if(-1 > errCode) 
				{
				   	LOG_ERR("while receiving from TCP socket");
				   	close(newSock);
				   	break;
			   	}
	
				if(0 < errCode) 
				{
				   	/* data available, note: errCode is length of received String	*/
				   	strncat(psock->inString, recString, errCode);
				   	/* TODO prevent overflow of inString -*/
			   	}
	
				if(sockKill) 
				{
					/* kill signalled, shut down socket */
					close(newSock);
					close(serverSock);
					sockRunning = false;
					pthread_exit(NULL);
				}
			} 
			while(NULL == (cpos = strchr(psock->inString, '\n')));
			
			/* now a line is received, cpos is on the position of LF */
			* cpos = '\0';   /* terminate line well */
			
			cpos = strchr(psock->inString, '\r');
			if(NULL != cpos) 
			{
				/* if CR found, terminate line there */
				* cpos = '\0'; 
			}
			
			/*
			 * a complete string has been read.
			 */ 
			if(0 == strcmp(psock->inString, getap216)) 
			{
				/* client wants to get the values of the AP216 */
				strncpy(psock->outString, "ap216 ", 6);
				registerGetString(psock->outString + 6, 0, outLen);
			}
			else if(0 == strcmp(psock->inString, getep16)) 
			{
				/* client wants to get the values of the EP16 */
				strncpy(psock->outString, "ep16 ", 5);
				registerGetString(psock->outString + 5, outLen, inLen);
			}
			else if(0 == strncmp(psock->inString, putap216, 10)) 
			{
				/* client wants to write to AP216 */
				answer = registerSetString(psock->inString + 9, 0, outLen);
				/* 
				 * real String to outshiftSetString starts with the
				 * first space after AP216
				 */
				if(NULL == answer) 
				{
					/* outshiftSetString returned correctly */
					strcpy(psock->outString, "ok");
				}
				else 
				{
					/* String was not understood. Send it back  */
					* answer = '\0';       /* terminate string  */
					psock->outString = psock->inString;
				}
			}
			else if( 0 == strncmp(psock->inString, putep16, 9)) 
			{
				/* client wants to write to EP16	*/
				answer = registerSetString(psock->inString + 8, outLen, inLen);
				if(NULL == answer) 
				{
					strcpy(psock->outString, "ok");
				}
				else {
					/* String was not understood. Send it back   */
					* answer = '\0'; /* terminate string         */
					psock->outString = psock->inString;
				}
			}
			else if(0 == strcmp(psock->inString, quit)) 
			{
				/* client wants to quit. */
				LOG_INF("quitting");
				close(newSock);
				break;
			}
			else if(0 == strcmp(psock->inString, inshiftenable)) 
			{
				inshiftEnable(true);
				strcpy(psock->outString, inEnabled);
			}
			else if(0 == strcmp(psock->inString, inshiftdisable)) 
			{
				inshiftEnable(false);
				strcpy(psock->outString, inDisabled);
			}
			else if(0 == strcmp(psock->inString, outshiftenable)) 
			{
				outshiftEnable(true);
				strcpy(psock->outString, outEnabled);
			}
			else if(0 == strcmp(psock->inString, outshiftdisable)) 
			{
				outshiftEnable(false);
				strcpy(psock->outString, outDisabled);
			}
			else if(0 == strcmp(psock->inString, outshiftforce)) 
			{
				forcing = true;
				strcpy(psock->outString, outForced);
			}
			else if(0 == strcmp(psock->inString, outshiftrelease)) 
			{
				forcing = false;
				strcpy(psock->outString, outReleased);
			}
			else if(0 == strcmp(psock->inString, who)) 
			{
				strcpy(psock->outString, myname);
			}
			else if(0 == strcmp(psock->inString, station)) 
			{
				strcpy(psock->outString, parserGetStation());
			}
			else 
			{
				sprintf(psock->outString, "please write one of the follwing \n %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s	\n",
					   	getap216, getep16, putap216, putep16, quit, inshiftenable,
					   	outshiftenable, inshiftdisable, outshiftdisable, outshiftforce,
					   	outshiftrelease, station, who);
			}
			
			/* 
			 * send routine. Here we try to send something to the
			 * client.
			 */
			
			psock->outString = strcat(psock->outString, "\r\n");
			strLen = strlen (psock->outString);
			if(0 > send (newSock, (void *) psock->outString, strLen, 0)) 
			{
				LOG_ERR("sending to TCP-Socket");
				close(newSock);
				break;
			}
		}
	}
	pthread_exit(NULL);
}

void comm(struct sRegister * const pReg,
		const unsigned short outshiftLen,
		const unsigned short inshiftLen,
		const unsigned short tempLen) 
{

	pRegister = pReg;
	outLen    = outshiftLen;
	inLen     = inshiftLen;
	varLen    = tempLen;

	LOG_INF("started");
	pOutString = malloc((10 + outLen + inLen + varLen) * 5 * sizeof(char));
	if(NULL == pOutString) 
	{
		fprintf(stderr, "ERROR Comm:commInit cannot allocate RAM\n");
		exit(EXIT_FAILURE);
	}
	pInString = malloc((10 + outLen + inLen + varLen) * 5 * sizeof(char));
	if(NULL == pInString) 
	{
		fprintf(stderr, "ERROR Comm:commInit cannot allocate RAM\n");
		free(pOutString);
		exit(EXIT_FAILURE);
	}

#ifdef __TEST__
	fprintf(stderr, "Message: Comm:commInit started\n");
#endif
	
	sock = malloc(sizeof(struct ssock));
	if(NULL == sock) 
	{
		LOG_ERR("cannot allocate RAM");
		free(pOutString);
		free(pInString);
		exit(EXIT_FAILURE);
	}

	/* set the values to be given over to sockFunction */
	sock->inString  = pInString;
	sock->outString = pOutString;
	sock->port      = 50500;
	commTime = 0;
	LOG_INF("ended");
}

void commDestroy(void) 
{
	void * ptr;
	
	LOG_INF("started");
	sockKill = true;
	pthread_join(sockThread, & ptr);
	free(pOutString);
	free(pInString);
	LOG_INF("ended");
}


void commInit(void) 
{
	LOG_INF("started");
	sockKill = false;
	pthread_create(& sockThread, NULL, sockFunction, (void *) sock);

	LOG_INF("ended");
}

void commTask(unsigned short timeout) 
{
	if(! commRunning()) 
	{
		/* only when thread died */
		commTime++;
		if(timeout < commTime)  
		{
			/* if timeout reached */
			LOG_INF("restarting");
			commTime = 0;
			if(NULL != sock) 
			{
				/* if initialized respawn the function */
				sockKill = false;
				pthread_create(& sockThread, NULL, sockFunction, (void *) sock);
				commTime = 0;
				return;
			}
		}
	}
	else 
	{
		commTime = 0;
	}
}

_Bool commForcing(void) 
{
	return forcing;
}

_Bool commRunning() 
{
	return sockRunning;
}

