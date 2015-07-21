/**
 * \mainpage Stellwerk Bad Waldsee
 *
 * This is the program for the switchboard Bad Waldsee for the 
 * 'Spreitenbacher Eisenbahn Amateur Klub'.<br>
 *
 *  The description is given in the book Raspberry Pi steuert Modelleisenbahn
 *  written by Kurt Zerzawy
 *
 *  Copyright(C)  2008 - 2015 Kurt Zerzawy <http://www.zerzawy.ch><br>
 *  Raspberry Pi steuert Modelleisenbahn is free software: 
 *  you can redistribute it and/or modify it under the terms of the GNU General 
 *  Public License as published by the Free Software Foundation, either 
 *  version 3 of the License, or (at your option) any later version.<br>
 *
 *  Raspberry Pi steuert Modelleisenbahn is distributed in the hope that it 
 *  will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 *  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.<br>
 *
 *  You should have received a copy of the GNU General Public License along 
 *  with Raspberry Pi steuert Modelleisenbahn. 
 *  If not, see <http://www.gnu.org/licenses/>.<br>
 * <hr>
 * Spreitenbacher Eisenbahn Amateur Klub SPEAK<br>
 * <http://www.speak.li>
 */

/**
 * \file
 * main program
 * \author
 * Kurt Zerzawy
 */

/*
 * $Author: kurt $
 * $Date: 2015-05-27 14:21:39 +0200 (Mit, 27. Mai 2015) $
 * $Revision: 2471 $
 */

#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sched.h>
#include "log.h"
#include "hw.h"
#include "registers.h"
#include "inshift.h"
#include "outshift.h"
#include "vars.h"
#include "led.h"
#include "duoled.h"
#include "block.h"
#include "button.h"
#include "line.h"
#include "turnout.h"
#include "dwarf.h"
#include "comm.h"
#include "parser/parser.h"

_Bool blink;
void atExit(void);

/**
 * main program. 
 *
 * @param	name of the configuration file
 * @return	only in case of error or when key is pushed
 */
int main(int argc, char * argv[]) 
{
#define STRLEN 255
	int test = 0;
	struct timespec interval;
	void atExit(void);
	char str[STRLEN];
	char str2[STRLEN];
	int handle;
	_Bool neuerEingang;
	LOG_INF("started");

	/* register a function called when program terminates
	 */
	assert(0 == atexit(atExit));

	if(1 == argc) 
	{
		/* if no filename given, use standard configuration file */
		parser("/home/pi/stw.conf");
		LOG_STW("Konfigurationsfile ist /home/pi/stw.conf");
	}
	else 
	{
		parser(argv[1] /*pConfFileName*/);
		LOG_STW("Konfigurationsfile ist %s", argv[1]);
	}
	
	parserReadConf();

	handle = registerFindHandle("SIG_A_LED3_gruen");

	comm(registerGetPtr(),
			 outshiftGetRegLen(),
			 inshiftGetRegLen(),
			 varsGetRegLen());
	
	ports_init();
	registersInit();
	
			 
//	commInit();
	blocksInit();
	turnoutInit();
	dwarfInit();
	//	ClockInit();
	
	LOG_INF("Stellwerk startet");
	inshiftEnable(true);
	outshiftEnable(true);  

	tickTimeStart();
	while(1) 
	{
		/* eternal loop */
		
		tickTimeWait(100000);

		neuerEingang = inshiftRegGet();
		
		/* logic part, only called if no hw test running */
		if(! commForcing()) 
		{
			if(neuerEingang) 
			{
				LOG_STW("neue Eingabedaten\n");
				buttonReadHW();
				turnoutReadHW();
				sectionsReadHW();
			}
			buttonProcess();
			// rafaProcess();
			// zufasProcess();
			sectionsProcess();
			// hauptsignaleProcess();
			// VerarbeiteVorsignale();
//			zwergsignalProcess();
		
			turnoutProcess();
			dwarfProcess();

			//registerGetString(TestString, NR_IN_REGISTERS, NR_OUT_REGISTERS);
		}
		commTask(50);
		outshiftRegSend(); 
		
		if(filteredKey()) 
		{
			/* end by keypress requested, terminate */
			LOG_INF("Ende durch Tastendruck");
			nanosleep(& interval, & interval);
			break;
		}

		port_set_watchdog(false);
		if(test % 10 == 5)
		{
			ledSetBlinkClock(true);
		}
		if(test %10 == 0)
		{
			ledSetBlinkClock(false);
		}
		if(test % 95 == 45) 
		{

			registerBitSet(handle);
/*			for(i=0; i<outshiftGetRegLen()*16-1; i+=2)
			{ 
				registerBitSet(i);
				registerBitClr(i+1);
			}
*/	
			registerGetString(str, 
							outshiftGetRegLen(), 
							inshiftGetRegLen()); /* input */
			registerGetString(str2, 0, outshiftGetRegLen()); /* output  */
			LOG_INF("95=45");
			printf("test == 45 %s : %s\n", str, str2);
		}
		if(test % 95 == 0) 
		{
			registerBitClr(handle);
/*			for(i=0; i<outshiftGetRegLen()*16-1; i+=2)
			{ 
				registerBitClr(i);
				registerBitSet(i+1);
			}
*/
			registerGetString(str, 
							outshiftGetRegLen(), 
							inshiftGetRegLen()); /* input */
			registerGetString(str2, 0, outshiftGetRegLen()); /* output  */
			LOG_INF("95=0");
			printf("test = 95 %s : %s\n", str, str2);
		}
		test++;
		port_set_watchdog(true);
		
		/* now calculate how much time was used	*/
		printf("time left = %lld\n", tickTimeUsed(100000));	
	}

	/* End of program, clean up please */
	exit(EXIT_SUCCESS);
}

/**
 * Function called at exit of program
 *
 * This function is used to clean up the complete memory from 
 * allocated ressources.
 */
void atExit(void) 
{
	LOG_INF("Anfang des Herunterfahrens");
	commDestroy();
	parserDestroy();
	registersDestroy();
	turnoutDestroy();
//	blockDestroy();
//	zwergsignalDestroy();
	
	LOG_INF("Ende des Programms");
	LOG_STW("Stellwerkprogramm ended");
}

