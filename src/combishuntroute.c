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
 * shunting routes combined from several single shunting routes
 * \author
 * Kurt Zerzawy
 */

/*
 * $Author: Kurt $
 * $Date: 2015-11-14 23:10:12 +0100 (Sa, 14 Nov 2015) $
 * $Revision: 2672 $
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
#include "shuntroute.h"
#include "combishuntroute.h"


/*@only@*/ /*@null@*/ static struct sCombiShuntroute * m_pCombiShuntroute = NULL;	/**< Pointer to the first combi shuntroute	*/
static unsigned short m_nrCombiShuntroute = 0;			/**< Number of combi shuntroutes	*/
static _Bool		m_combishuntrouteTodo = false;		/**< something to do for combi shuntroutes	*/

/**
 * Function to get the indexed pointer of parts of a combi shuntroute
 * @param ptr       pointer to the combi shuntroute
 * @param nr        number of the shuntroute part
 * @return          pointer of this part, if index is too big, NULL is returned
 * @exception       Assertion on NULL pointer
 */
static void * shuntrouteGetPartPtrByIndex(const struct sShuntroute * const ptr, const unsigned short i);

/**
 * function to add parts of the shunting route to the list
 * @param ptr		pointer to the shuntroute
 * @param type		type of the element to be added
 * @param pPart		void pointer to the element
 * @param argument	argument for this element
 * @return			index, where this part has been inserted
 * @exception		assert on NULL pointer or if list is full
 */
static unsigned short shuntrouteAddPart(struct sShuntroute * const ptr, const enum eShuntroutePartType type, void * pPart, const unsigned short argument);


/**
 * Constructor for the combi shuntroutes
 * @param nr		number of combi shuntroutes to be created
 * @exception		Calls exit if no RAM available
 */
void combishuntroute(const unsigned short nr)
{
	unsigned short			i, j;
	struct sCombiShuntroute *	ptr;

	LOG_INF("started");
	
	assert(0 == nr);	/* TODO function not completely implemented yet	*/

	m_nrCombiShuntroute = nr;
	m_pCombiShuntroute = malloc(m_nrCombiShuntroute * sizeof(struct sCombiShuntroute));
	if(NULL == m_pCombiShuntroute) 
	{
		/* error allocating */
		LOG_ERR("cannot allocate RAM of combi shuntroutes");
		exit(EXIT_FAILURE);
	}
	ptr = m_pCombiShuntroute;
	LOG_INF("ended");
}

/**
 * Destructor for the combi shuntroutes
 */
void combishuntrouteDestroy(void) 
{
	LOG_INF("started");
	if(NULL == m_pCombiShuntroute) 
	{
		LOG_INF("zero pointer, terminate");
		return;
	}
	LOG_INF("release RAM of shuntroutes");
	free(m_pCombiShuntroute);
	m_pCombiShuntroute = NULL;
	LOG_INF("ended");
}

/**
 * Init function for the combi shuntroutes
 *
 * Here, all shuntroutes are initialised 
 */
void combishuntrouteInit(void) 
{
}

/**
 * Function to get the number of combi shuntroutes 
 * @return		Number of combi shuntroutes
 */
unsigned short combishuntrouteGetNr(void)
{
	return m_nrCombiShuntroute;
}

/**
 * Function to get the pointer by giving the number of the combi shuntroute
 * @param i		index, starting with 0
 * @return		Pointer to this combi shuntroute, NULL if not existing
 */ 
struct sCombiShuntroute * combishuntrouteGet(const unsigned short i)
{
	if(i >= m_nrCombiShuntroute)
	{
		return NULL;
	}
	else
	{
		return m_pCombiShuntroute + i;
	}
}

/**
 * function to set the name of a combi shuntroute
 * @param ptr		pointer to the combi shuntroute
 * @param name		pointer to the string with the name 
 * @exception		assert on NULL pointer
 */ 
void combishuntrouteSetName(struct sCombiShuntroute * const ptr, const char * const name)
{
	assert(NULL != ptr);
	
	memmove(ptr->name, name, NAMELEN);
}

/**
 * function to get the name of a combi shuntroute
 * @param ptr		pointer to the combi shuntroute
 * @param name		pointer to the string filled out with the name
 * 			of the combi shuntroute. Must be at least NAMELEN long
 * @return		true, if successful
 * @exception		assert on NULL pointer
 */ 
_Bool combishuntrouteGetName(const struct sCombiShuntroute * const ptr, /*@out@*/ char * const name)
{
	assert(NULL != ptr);

	name[NAMELEN - 1] = '\0';
	memmove(name, ptr->name, NAMELEN - 1);
	return true;
}

void combishuntrouteProcess(void) 
{
	;
}
