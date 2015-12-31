/*
 * This file is part of Raspberry Pi steuert Modelleisenbahn
 * Copyright(C)  2009 - 2015 Kurt Zerzawy www.zerzawy.ch
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
 * several LEDs combined on one occupancy detector 
 * \author
 * Kurt Zerzawy
 */

/*
 * $Author: Kurt $
 * $Date: 2015-08-05 08:06:23 +0200 (Mi, 05 Aug 2015) $
 * $Revision: 2537 $
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "registers.h"
#include "button.h"
#include "log.h"
#include "duoled.h"
#include "section.h"

/*@only@*/ /*@null@*/ static struct sSection * m_pSection = NULL;	/**< Pointer to the first section	*/
static unsigned short m_nrSection = 0;	/**< Number of sections	*/


/**
 * Constructor for the sections
 * @param nr		number of sections to be created
 * @exception       Calls exit if no RAM available
 */
void sections(const unsigned short nr) 
{
	unsigned short	i, j;
	struct sSection *	ptr;

	LOG_INF("started");
	
	m_nrSection = nr;
	m_pSection = malloc(m_nrSection * sizeof(struct sSection));
	if(NULL == m_pSection) 
	{
		/* error allocating */
		LOG_ERR("cannot allocate RAM of sections");
		exit(EXIT_FAILURE);
	}
	ptr = m_pSection;
	for(i = 0; i < m_nrSection; i++)
	{
		ptr->name[0]	= '\0';
		ptr->BI_occDetector	= NONEVALUE;
		ptr->DuoledAddPosition	= 0;
		ptr->line	= neutral;
		ptr->linePrev	= neutral;
		ptr->free	= false;
		ptr->wasFree	= false;
		ptr->route	= false;
		ptr->wasRoute	= false;
		for(j = 0; j < NRBLOCKPL; j++)
		{
			ptr->pDuoled[j] = NULL;
		}
		ptr++;
	}		
	LOG_INF("ended");
}

/**
 * Destructor for the sections
 */
void sectionsDestroy(void) 
{
	LOG_INF("started");
	if(NULL == m_pSection) 
	{
		LOG_INF("zero pointer, terminate");
		return;
	}
	LOG_INF("release RAM of sections");
	free(m_pSection);
	m_pSection = NULL;
	LOG_INF("ended");
}

/**
 * function to request and set the line for the section
 * @param ptr	pointer to the section to be asked for
 * @param	requestedLine line which shall be set and asked for
 * @return	true if the line could be set
 */
_Bool sectionLine(struct sSection * const ptr, const enum eLine requestedLine)
{
	enum eLine line;

	assert(NULL != ptr);

	line = ptr->line | ptr->linePrev;

	if(neutral == requestedLine)
	{
		return true;
	}
	if(neutral == line)
	{
		/* now it is possible to set the direction	*/
		ptr->line = requestedLine;
		return true;
	}
	/* now a direction is stored already, sectionLine is only
	 *  true if the direction is same
	 */
	if((LINEDIRMASK & requestedLine) == (LINEDIRMASK & line))
	{
		ptr->line = requestedLine;
		return true;
	}
	return false;
}

/**
 * Read function for the sections 
 *
 * Reads the hardware inputs of the sections and fills up the variables with 
 * the correct signals. 
 *
 * If something changed, the function returns true to allow overlaying functions
 * to detect if there is something to do.
 * @return		true if the state of the inputs changed
 */
_Bool sectionsReadHW(void) 
{
	struct sSection * ptr = m_pSection;  
	unsigned short i;
	_Bool todo = false;
	_Bool free = false;
	
	assert(NULL != ptr);

	for (i = 0; i < m_nrSection; i++) 
	{
		/* for each section the bit of the occupancy detectors must
		 * be read. 
		 * then it is checked if the state was changing. If so, 
		 * a text is given out and the todo bit is set
		 */
		free = ! registerBitGet(ptr->BI_occDetector); 
		if(free)
		{
			/* the section is free */
			if(! ptr->wasFree) 
			{
				LOG_STW("Section %s wird frei", ptr->name);
			   	todo = true;
			}
		}
		else 
		{
			/* the section is occupied */
			if(ptr->wasFree) 
			{
			   	LOG_STW("Section %s wird belegt", ptr->name);
				todo = true;
			}
		} 
		ptr->free = free;
		ptr++;
	}
	return todo;
}

/** 
 * Function to ask a section if it is free
 * @param ptr		pointer to the section
 * @return			true if free
 * @exception		Assert on NULL pointer
 */
_Bool sectionIsFree(const struct sSection * const ptr) 
{
	assert (NULL != ptr);
	return ptr->free;
}

/**
 * Init function for the sections
 *
 * Here, all sections are initialised according to the state of the occupancy
 * detectors.
 */
void sectionsInit(void) 
{
	struct sSection *		ptr = m_pSection;  
	struct sDuoled *	pDuoled;
	unsigned short 		i;
	unsigned short 		j;
	LOG_INF("started");
	
	/* Note: all vars are initialised, also the ones which are not
	 * used anymore, e.g. slave sections
	 */
	assert(NULL != ptr);
	for (i = 0; i < m_nrSection; i++) 
	{
		ptr->route         = false;
		ptr->wasRoute      = false;
		ptr->line          = neutral;
		ptr->linePrev      = neutral;
		j = 0;
		while(NULL != (pDuoled = sectionGetDuoled(ptr, j)))
		{
			duoledSetMode(pDuoled, ledOff);
			j++;
		}
		/* check if there are occupied sections and set the color on the 
		 * desk accordingly
		 */
		if(registerBitGet(ptr->BI_occDetector)) 
		{
			ptr->free = false;
			ptr->wasFree = false;
			j = 0;
			while(NULL != (pDuoled = sectionGetDuoled(ptr, j)))
			{
				duoledSetOccupied(pDuoled, true);
				j++;
			}
		}
		else 
		{
			ptr->free = true;
			ptr->wasFree = true;
			j = 0;
			while(NULL != (pDuoled = sectionGetDuoled(ptr, j)))
			{
				duoledSetOccupied(pDuoled, false);
				j++;
			}
		}
		ptr++;
	}
	LOG_INF("ended"); 
}

/**
 * Function to process the sections 
 *
 * To be called once a cycle in case the inputs changed as checked by sectionReadHW.
 *
 * The function is reading the input signals and checks for new conditions. 
 * if there are any, a text is given for log.
 */
void sectionsProcess(void) 
{
	struct sSection * 	ptr = m_pSection;
	struct sDuoled *	pDuoled;
	unsigned short		i;
	unsigned short		j;

	for(i = 0; i < m_nrSection; i++) 
	{
		assert(NULL != ptr);
		if((ptr->free) && (! ptr->wasFree)) 
		{
			/* section changed to free */
			j = 0;
			while(NULL != (pDuoled = sectionGetDuoled(ptr, j)))
			{
				duoledSetOccupied(pDuoled, false);
				j++;
			}
		}
		if((ptr->wasFree) && (! ptr->free)) 
		{
			/* section changed to occupied */
			j = 0;
			while(NULL != (pDuoled = sectionGetDuoled(ptr, j)))
			{
				duoledSetOccupied(pDuoled, true);
				j++;
			}
		}
	
		if(ptr->line != ptr->linePrev)
		{
			if(neutral == (LINEMASK & ptr->line))
			{ 
				LOG_STW("Section %s hat keine Fahrstrassenspur mehr", ptr->name);

				j = 0;
				while(NULL != (pDuoled = sectionGetDuoled(ptr, j)))
				{
					duoledSetMode(pDuoled, ledOff);
					j++;
				}

			}
			else
			{
				LOG_STW("Section %s hat eine Fahrstrassenspur", ptr->name);

				j = 0;
				while(NULL != (pDuoled = sectionGetDuoled(ptr, j)))
				{
					duoledSetMode(pDuoled, ledOn);
					j++;
				}
			}
		}

		ptr->wasFree = ptr->free;
		ptr->linePrev = ptr->line;
		ptr->line = neutral;
		ptr++;
	}
}

/**
 * Function to get the pointer by giving the number of the section
 * @param i		index, starting with 0
 * @return		Pointer to this section, NULL if not existing
 */ 
struct sSection * sectionGet(const unsigned short i)
{
	if(i >= m_nrSection)
	{
		return NULL;
	}
	else
	{
		return m_pSection + i;
	}
}

/**
 * Function to get the number of sections 
 * @return		Number of sections
 */
unsigned short sectionsGetNr(void)
{
	return m_nrSection;
}

/**
 * Function to get the name of a section
 * @param ptr		pointer to the section
 * @param name		pointer to the string filled out with the name
 * 					of the section. Must be at least NAMELEN long
 * @return			true if successful
 * @exception		Assert on NULL pointer
 */ 
_Bool sectionGetName(const struct sSection * const ptr, char * const name)
{
	assert(NULL != ptr);

	name[NAMELEN - 1] = '\0';
	memmove(name, ptr->name, NAMELEN - 1);
	return true;
}

/**
 * Function to set the name of a section
 * @param ptr		pointer to the section
 * @param name		pointer to the string with the name 
 * @exception		Assert on NULL pointer
 */ 
void sectionSetName(struct sSection * const ptr, const char * const name)
{
	assert(NULL != ptr);

	memmove(ptr->name, name, NAMELEN);
}

/**
 * Function to get the indexed duoled of a section
 * @param ptr		pointer to the section
 * @param nr		number of the duoled of this section
 * @return			Pointer to the duoled, NULL if none assigned
 * @exception		Assertion on NULL pointer 
 */
struct sDuoled * sectionGetDuoled(const struct sSection * ptr, const unsigned short i)
{
	assert(NULL != ptr);
	if(NRBLOCKPL <= i)
		return NULL;

	return ptr->pDuoled[i];		
}

/**
 * Function to add a duoled to a section
 * @param ptr		pointer to the section
 * @param pDuoled	Pointer to the duoled
 * @exception		Assertion on NULL pointer or too many duoleds
 */
void sectionAddDuoled(struct sSection * const ptr, struct sDuoled * const pDuoled)
{
	assert(NULL != ptr);
	assert(NRBLOCKPL > ptr->DuoledAddPosition);

	ptr->pDuoled[ptr->DuoledAddPosition++] = pDuoled;		
}

/**
 * Function to find the pointer of a section by giving the name
 * @param name		name of the section to be locked for
 * @return		Pointer of the found section
 * @exception		A parser assert is made if not found
 */
struct sSection * sectionFind(const char * const name) 
{
	unsigned short		i;
	struct sSection *		ptr;
	char				temp[128];

	i = 0;
	while(NULL != (ptr = sectionGet(i)))
	{
		if(0 == (strcmp(ptr->name, name)))
			return ptr;
		i++;
	}
	sprintf(temp, "Abschnitt %s nicht gefunden", name);
	parserExit(temp);
	return NULL;
}

/**
 * Function to get the register of the occupancy detector
 * @param ptr		pointer to the section
 * @return			Register handle of the occupancy detector
 * @exception		Assert on NULL pointer
 */
unsigned short sectionGetBI_occDetector(const struct sSection * const ptr)
{
	assert(NULL != ptr);

	return ptr->BI_occDetector;
}

/**
 * Function to set the register of the occupancy detector
 * @param ptr		pointer to the section
 * @param handle	register handle of the occupancy detector
 * @exception		Assert on NULL pointer
 */
void sectionSetBI_occDetector(struct sSection * const ptr, const unsigned short handle)
{
	assert(NULL != ptr);

	ptr->BI_occDetector = handle;
}


