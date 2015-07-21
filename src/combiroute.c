/*
 * This file is part of Raspberry Pi steuert Modelleisenbahn
 * Copyright(C)  2014 - 2015 Kurt Zerzawy www.zerzawy.ch
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
 * Combined train combiroutes (Kombi-Zugfahrstrassen)
 * \author
 * Kurt Zerzawy
 */

/*
 * $Author: kurt $
 * $Date: 2015-06-07 15:52:39 +0200 (Son, 07. Jun 2015) $
 * $Revision: 2483 $
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "log.h"
#include "registers.h"
#include "duoled.h"
#include "section.h"
#include "button.h"
#include "turnout.h"
#include "mains.h"
#include "route.h"
#include "combiroute.h"


/*@only@*/ /*@null@*/ static struct sCombiroute * m_pCombiroutes = NULL;	/**< Pointer to the first combi train route	*/
static unsigned short m_nrCombiroutes	= 0;	/**< Number of combi train routes	*/

/**
 * Constructor for the combi combiroutes
 * @param n		number of combi combiroutes to be created
 * @exception		Calls exit if no RAM available
 */
void combiroutes(const unsigned short nr)
{
	unsigned short		i, j;
	struct sCombiroute *	pCombiroute;

	LOG_INF("started");
	
	m_nrCombiroutes = nr;
	m_pCombiroutes = malloc(m_nrCombiroutes * sizeof(struct sCombiroute));
	if(NULL == m_pCombiroutes) 
	{
		/* error allocating */
		LOG_ERR("cannot allocate RAM of train combiroutes");
		exit(EXIT_FAILURE);
	}
	pCombiroute = m_pCombiroutes;
	for(i = 0; i < m_nrCombiroutes; i++)
	{
		pCombiroute->name[0]	= '\0';
		pCombiroute->start	= NULL;
		pCombiroute->dest	= NULL;
		pCombiroute->partsAddPosition	= 0;

		for(j = 0; j < NRCOMBIPARTS; j++)
		{
			pCombiroute->pRoute[j]	= NULL;
		}

		pCombiroute++;
	}

	LOG_INF("ended");
}

/**
 * Destructor for the train combiroutes
 */
void combiroutesDestroy(void) 
{
	LOG_INF("started");
	if(NULL == m_pCombiroutes) 
	{
		LOG_INF("zero pointer, terminate");
		return;
	}
	LOG_INF("release RAM of train combiroutes");
	free(m_pCombiroutes);
	m_pCombiroutes = NULL;
	LOG_INF("ended");
}

/**
 * Init function for the train combiroutes
 *
 * Here, all train combiroutes are initialised 
 */
void combiroutesInit(void) 
{
}

/**
 * Function to get the number of train combiroutes 
 * @return		Number of train combiroutes
 */
unsigned short combiroutesGetNr(void)
{
	return m_nrCombiroutes;
}

/**
 * Function to get the pointer by giving the number of the route
 * @param i		index, starting with 0
 * @return		pointer to this route, NULL if not existing
 */ 
struct sCombiroute * combirouteGet(const unsigned short index)
{
	if(index >= m_nrCombiroutes)
	{
		return NULL;
	}
	else
	{
		return m_pCombiroutes + index;
	}
}

/**
 * Function to find the pointer of a route by giving the name
 * @param name		name of the route to be locked for
 * @return		pointer of the found route
 * @exception		a parser assert is made if not found
 */
struct sCombiroute * combirouteFind(const char * const name) 
{
        unsigned short	i = 0;
	struct sCombiroute *	pCombiroute = NULL;
        char		temp[128];

	i = 0;
	while(NULL != (pCombiroute = combirouteGet(i)))
	{ 
		if(0 == (strcmp(pCombiroute->name, name))) 
		{
			return pCombiroute;
		}
		i++;
        }
	sprintf(temp, "Kombifahrstrasse >%s< nicht gefunden", name);
        parserExit(temp);
        return NULL;
}

/**
 * Function to get the name of a combined train combiroute
 * @param pCcombiroute	pointer to the combined train route
 * @param name	pointer to the string filled out with the name
 * 		of the turnout. Must be at least NAMELEN long
 * @return	true, if successful
 * @exception	assert on NULL pointer
 */ 
_Bool combirouteGetName(const struct sCombiroute * const pCombiroute, /*@out@*/ char * const name)
{
	assert(NULL != pCombiroute);

	name[NAMELEN - 1] = '\0';
	memmove(name, pCombiroute->name, NAMELEN - 1);
	return true;
}

/**
 * Function to set the name of a train combiroute
 * @param pCombiroute	pointer to the train route
 * @param name	pointer to the string with the name 
 * @exception	assert on NULL pointer
 */ 
void combirouteSetName(struct sCombiroute * const pCombiroute, const char * const name)
{
	assert(NULL != pCombiroute);

	memmove(pCombiroute->name, name, NAMELEN);
}

/**
 * Functionet the start mains
 * @param pCombiroute	pointer to the train route
 * @return		pointer to the start mains
 * @exception		assert on NULL pointer
 */
struct sMain * combirouteGetStart(const struct sCombiroute * pCombiroute)
{
	assert(NULL != pCombiroute);

	return pCombiroute->start;
}

/**
 * Function to set the start mains
 * @param pCombiroute	pointer to the train route
 * @param start		pointer to the start mains
 * @exception		assert on NULL pointer
 */
void combirouteSetStart(struct sCombiroute * const pCombiroute, struct sMain * start)
{
	assert(NULL != pCombiroute);

	pCombiroute->start = start;
}

/**
 * Function to get the destination mains
 * @param pCombiroute	pointer to the train route
 * @return		pointer to the destination mains
 * @exception		assert on NULL pointer
 */
struct sMain * combirouteGetDest(const struct sCombiroute * pCombiroute)
{
	assert(NULL != pCombiroute);

	return pCombiroute->dest;
}

/**
 * Function to set the destination mains
 * @param pCombiroute	pointer to the train route
 * @param dest		pointer to the destination mains
 * @exception		assert on NULL pointer
 */
void combirouteSetDest(struct sCombiroute * const pCombiroute, struct sMain * dest)
{
	assert(NULL != pCombiroute);

	pCombiroute->dest = dest;
}

/**
 * Function to get the number of parts of the combi train route
 * @param pCombiroute	pointer to the combi train route
 * @return		number of parts the combi train route consists of
 * @exception		assert on NULL pointer
 */
unsigned short combirouteGetNrRoutes(const struct sCombiroute * const pCombiroute)
{
	assert(NULL != pCombiroute);

	return pCombiroute->partsAddPosition;
}

/**
 * Function to add a route to a combi train route
 * @param pCombiroute	pointer to the combi train route
 * @param pRoute	pointer to the train route to be added
 * @return		index, where the turnout has been inserted
 * @exception		Assertion on NULL pointer or too many turnout parts
 */
unsigned short combirouteAddRoute(struct sCombiroute * const pCombiroute, struct sRoute * const pRoute)
{
	assert(NULL != pCombiroute);
	assert(NRCOMBIPARTS > pCombiroute->partsAddPosition);

	pCombiroute->pRoute[pCombiroute->partsAddPosition] = pRoute;
	pCombiroute->partsAddPosition++;
	return pCombiroute->partsAddPosition - 1;
}

/**
 * Function to get the indexed pointer of a route as part of a combi train route
 * @param pRoute	pointer to the combi train route
 * @param nr		number of the combi train route part
 * @return		pointer of the route, if index is too big 
 * @exception		Assertion on NULL pointer
 */
struct sRoute * combirouteGetRoute(const struct sCombiroute * const pCombiroute, const unsigned short index)
{
	assert(NULL != pCombiroute);
	if(NRCOMBIPARTS <= index)
	{
		return NULL;
	}

	return pCombiroute->pRoute[index];
}


