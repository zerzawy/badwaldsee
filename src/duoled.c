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
 * duo color LEDs 
 * \author
 * Kurt Zerzawy
 */

/*
 * $Author: kurt $
 * $Date: 2015-05-18 22:31:02 +0200 (Mon, 18. Mai 2015) $
 * $Revision: 2464 $
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "constants.h"
#include "log.h"
#include "hw.h"
#include "duoled.h"
#include "parser/parser.h"	/* for parserExit	*/

/*@null@*/ static struct sDuoled * 	m_pDuoled = NULL;	/**< Pointer to first duoled	*/
static unsigned short			m_nrDuoled = 0;		/**< Number of duoleds	*/
static unsigned short 			m_DuoledAddPosition = 0;	/**< Position to add additional duoleds	*/

/**
 * Constructor for the duo colour LED. 
 *
 * With the constructor the data structur is generated. Use this function as 
 * soon as you know the number of the duo LEDs.
 * @param nrDuoleds		number of the duoleds to be allocated
 * @exception           Calls exit if no RAM available
 */
/*@maynotreturn@*/
void duoled(/*@in@*/ const unsigned short nrDuoleds) 
{
	LOG_INF("gestartert");

	assert(NULL == m_pDuoled);

	m_nrDuoled = nrDuoleds;
	m_pDuoled = malloc(m_nrDuoled * sizeof(struct sDuoled));
	if(NULL == m_pDuoled) 
	{
		LOG_ERR("cannot allocate RAM");
		exit(EXIT_FAILURE);
	}

	duoledInit();
	LOG_INF("ended");
}

/** 
 * Destructor. 
 *
 * To free the allocated RAM, call this function before terminating the program.
 */
void duoledDestroy(void) 
{
	LOG_INF("started");
	free(m_pDuoled);
	m_pDuoled = NULL;
	m_nrDuoled = 0;
	m_DuoledAddPosition = 0;
	LOG_INF("ended");
}

/**
 * Function to set the duoleds in a defined state.
 */
void duoledInit(void) 
{
	/*@null@*/ struct sDuoled * ptr =	m_pDuoled;
	int i;

	LOG_INF("started");
	assert(NULL != m_pDuoled);

	for(i = 0; i < m_nrDuoled; i++)
	{
		ptr->mode = ledOff;
		ptr->occupied = false;
		ptr++;
	}

	m_DuoledAddPosition = 0;
	LOG_INF("ended");
}

/**
 * Function to add a pair of LEDs to the duoled table.
 * @param pYellow	pointer to the yellow LED
 * @param pRed		pointer to the red LED
 * @return         	Pointer to the newly made duoledstructur
 * @exception		Assert is made if too many duoleds are added
 */
struct sDuoled * duoledAdd(struct sLed * const pYellow, struct sLed * const pRed)
{
	struct sDuoled * ptr = m_pDuoled;

	assert(m_nrDuoled > m_DuoledAddPosition);
	ptr += m_DuoledAddPosition;

	ptr->pYellow = pYellow;
	ptr->pRed  = pRed;
	ptr->mode = ledOff;
	ptr->occupied = false;

	m_DuoledAddPosition += 1;
	return ptr;
}

static void duoledSet(const struct sDuoled * ptr);
static void duoledSet(const struct sDuoled * ptr)
{
	assert(NULL != ptr);

	if(ptr->occupied) 
	{
		switch(ptr->mode)
		{
			case ledOn:		ledSet(ptr->pRed, ledOn); break;
			case ledOff:		ledSet(ptr->pRed, ledOn); break;
			case ledAllOff:		ledSet(ptr->pRed, ledAllOff); break;
			case ledBlinking:	ledSet(ptr->pRed, ledBlinking); break;
			case ledBlankable:	ledSet(ptr->pRed, ledOn); break;
			default:
				fprintf(stderr, "falscher Default %d\n", ptr->mode);		
				assert(false);
		};
		ledSet(ptr->pYellow, ledOff);
	}
	else
	{
		ledSet(ptr->pYellow, ptr->mode);
		ledSet(ptr->pRed, ledOff);
	}
}

/**
 * Function to set a duoled to an occupied or free state.
 * @param ptr		pointer to the duoledstructure
 * @param occupied	true for occupied
 */
void duoledSetOccupied(struct sDuoled * const ptr,
						const _Bool occupied)
{
	assert(NULL != ptr);

	ptr->occupied = occupied;
	duoledSet(ptr);
}

/** 
 * Function to set the mode of a duoled.
 * @param ptr     	pointer to the duoled
 * @param mode		telling the behaviour of the duoled
 * @exception		Assert on NULL pointer
 */
void duoledSetMode(struct sDuoled * const ptr, const enum eLedMode mode)
{
	assert(NULL != ptr);

	ptr->mode = mode;
	duoledSet(ptr);
}

/**
 * Function to get the mode of a duoled.
 * @param ptr     	pointer to the duoled
 * @return		Behaviour of the duoled
 * @exception		Assert on NULL pointer
 */
enum eLedMode duoledGetMode(const struct sDuoled * const ptr)
{
	assert(NULL != ptr);
	return ptr->mode;
}

/**
 * Function to find the pointer of a duoled out of the name of it.
 * @param name		pointer to the name to be looked for
 * @return		Pointer to the duoled found
 * @exception		A parser assert is made if not found
 */
struct sDuoled * duoledFind(const char * const name)
{
	unsigned short 		i = 0;
	struct sDuoled *	ptr = m_pDuoled;
	char			str[NAMELEN];
	char			temp[128];

	assert(NULL != ptr);
	for(i = 0; i < m_nrDuoled; i++)
	{
		if(duoledGetName(ptr, str))
		{
			if(0 == strncmp(name, str, NAMELEN))
			{
				return ptr;
			}
		}
		ptr++;
	}
	sprintf(temp, "Duoled %s nicht gefunden", name);
	parserExit(temp);
	return NULL;
}

/**
 * Function to check if a LED is a duoled.
 * @param ptr	pointer to the LED to be checked
 * @return		true if the pointer points to a duoled
 */
_Bool duoledLedIsDuoled(const struct sLed * const ptr)
{
	char name[NAMELEN];

	assert(NULL != ptr);
	if(! ledIsLed(ptr))
		return false;
	ledGetName(ptr, name);
	if(0 == strncmp("D_", name, 2))
		return true;
	else
		return false;
}

/**
 * Function to check for duoled name.
 *
 * This function checks if a given name could be a duoled, which
 * is the case when a name starts with D_.
 * @param name	name to be checked
 * @return		true if it could be a duoled
 */
_Bool duoledIsDuoledName(const char * const name)
{	
	return(0 == strncmp("D_", name, 2));
}

_Bool duoledGetName(struct sDuoled * const ptr, char * name)
{
	assert(NULL != ptr);
	assert(NULL != ptr->pYellow);

	if(!duoledLedIsDuoled(ptr->pYellow))
	{
		*name = '\0';
		return false;
	}

	assert(NULL != ptr->pRed);
	ledGetName(ptr->pRed, name);
	memmove(name, name+2, NAMELEN-2);
	name[strlen(name)-4] = '\0';
	return true;
}

/**
 * Function to return the number of allocated duoleds.
 * @return	Number of allocated duoleds
 */
unsigned short duoledGetNr(void)
{
	unsigned short nr = 0;
	
	while(NULL != ledGet(nr))
		nr++;

	return nr;
}

/**
 * Function to return the pointer to the yellow / white LED.
 *
 * In this function, the name of the duoled is given without trailing
 * D_ and the function searches for the yellow LED of this duoled.
 *
 * If the name cannot be fount, the program exits with an error message.
 *
 * @param name	name of the duoled
 * @return 	Pointer to the yellow LED
 * @exception 	Assert if the name would be too long
 */
struct sLed * duoledGetYellow(const char * const name)
{
	char str[NAMELEN];

	assert(NAMELEN - 8 > strlen(name));
	strcpy(str, "D_");
	strcpy(str+2, name);
	strcpy(str+strlen(name)+2, "_gelb");
	return ledFind(str);
}

/**
 * Function to return the pointer to the red LED.
 *
 * For details, see duoledGetYellow
 */
struct sLed * duoledGetRed(const char * const name)
{
	char str[NAMELEN];

	assert(NAMELEN - 7 > strlen(name));
	strcpy(str, "D_");
	strcpy(str+2, name);
	strcpy(str+strlen(name)+2, "_rot");
	return ledFind(str);
}

/**
 * Helper function for the compare of the end of a string.
 * @param string	string to be checked
 * @param pattern	string to be searched for
 * @return		true if the string ends with pattern
 */
_Bool duoledCompareEnd(const char * const string,
			const char * const pattern)
{
	assert(strlen(pattern) < strlen(string));
	return (0 == strncmp(pattern, 
		& string[strlen(string)-strlen(pattern)],
		strlen(pattern)));
}
