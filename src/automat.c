/*
 * This file is part of Raspberry Pi steuert Modelleisenbahn
 * Copyright(C)  2015 Kurt Zerzawy www.zerzawy.ch
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
 * automats
 * \author
 * Kurt Zerzawy
 */

/*
 * $Author: kurt $
 * $Date: 2015-05-14 20:52:29 +0200 (Don, 14. Mai 2015) $
 * $Revision: 2451 $
 */

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "automat.h"
#include "line.h"


/*@only@*/ /*@null@*/ static struct sAutomat * m_pAutomats = NULL;	/**< Pointer to the first automat	*/
static unsigned short 		m_nrAutomats = 0;			/**< Number of automats	*/
static _Bool			m_automatsTodo = true;			/**< something to do for automats	*/

void automatsProcess(void);


/**
 * Constructor for the automat
 * @param nr		number of automats to be created
 * @exception       	Calls exit if no RAM available
 */
void automats(const unsigned short nr) 
{
	LOG_INF("started");
	
	m_nrAutomats = nr;
	m_pAutomats = malloc(m_nrAutomats * sizeof(struct sAutomat));
	if(NULL == m_pAutomats) 
	{
		/* error allocating */
		LOG_ERR("cannot allocate RAM for automats");
		exit(EXIT_FAILURE);
	}
	automatsInit();

	LOG_INF("ended");
}

/**
 * Destructor for the automats
 */
void automatsDestroy(void) 
{
	LOG_INF("started");
	if(NULL == m_pAutomats) 
	{
		LOG_INF("zero pointer, beende");
		return;
	}
	LOG_INF("release RAM of automats");
	free(m_pAutomats);
	m_pAutomats = NULL;
	LOG_INF("ended");
}

void automatsInit(void)
{
	struct sAutomat *	pAutomat = m_pAutomats;
	unsigned short  	i, j;

	LOG_INF("started");

	for(i = 0; i < m_nrAutomats; i++)
	{
		pAutomat->name[0]	= '\0';
		pAutomat->state	= AutomatNeutral;
		pAutomat->pAutomatedRoute	= NULL;

		pAutomat->nrSections	= 0;
		for(j = 0; j < AUTOMATNRSECTIONS; j++)
		{
			pAutomat->pSection[j] = NULL;
		}

		pAutomat->nrRoutes	= 0;
		for(j = 0; j < AUTOMATNRROUTES; j++)
		{
			pAutomat->pRoute[j] = NULL;
		}

		pAutomat->switchOn	= false;
		pAutomat->switchOff	= false;
		pAutomat->Todo	= true;
		pAutomat++;
	}		
	m_automatsTodo = true;
	LOG_INF("ended");
}

/**
 * Function to find the pointer of a automat by giving the name
 * @param name		name of the automat to be locked for
 * @return		pointer of the found automat
 * @exception		a parser assert is made if not found
 */
struct sAutomat * automatFind(const char * const name) 
{
        unsigned short		i = 0;
	struct sAutomat *	pAutomats = NULL;
	char			temp[128];

	while(NULL != (pAutomats = automatGet(i)))
	{ 
		if(0 == (strcmp(pAutomats->name, name))) 
		{
			return pAutomats;
		}
		i++;
        }
        sprintf(temp, "Automat %s nicht gefunden", name);
        parserExit(temp);
        return NULL;
}

/**
 * Function to get the pointer by giving the number of the automat
 * @param i		index, starting with 0
 * @return		pointer to this automat, NULL if not existing
 */ 
struct sAutomat * automatGet(const unsigned short i)
{
	if(i >= m_nrAutomats)
	{
		return NULL;
	}
	else
	{
		return m_pAutomats + i;
	}
}

/**
 * Function to get the number of automats
 * @return		Number of automats
 */
unsigned short automatsGetNr(void)
{
	return m_nrAutomats;
}

/**
 * Function to check if the automat is switched on or off
 * @param pAutomat	pointer to the automat
 * @return		true, if automat is switched on
 * @exception		assert on NULL pointer
 */ 
_Bool automatGetOn(const struct sAutomat * const pAutomat)
{
	assert(NULL != pAutomat);

	if(AutomatOn == pAutomat->state)
	{
		return true;
	}
	/* else	*/
	return false;
}

/**
 * Function to switch the the automat on or off
 * @param pAutomat	pointer to the automat
 * @param on		true, if automat shall be switched on
 * @exception		assert on NULL pointer
 */ 
void automatSetOn(struct sAutomat * const pAutomat, _Bool on)
{
	assert(NULL != pAutomat);

	if(on)
	{
		pAutomat->switchOn = true;
	}
	else
	{
		pAutomat->switchOff = true;
	}
	pAutomat->Todo = true;
}

/**
 * Function to get the name of a automat
 * @param pAutomat	pointer to the automat
 * @param name		pointer to the string filled out with the name
 * 			of the automat. Must be at least NAMELEN long
 * @return		true, if successful
 * @exception		assert on NULL pointer
 */ 
_Bool automatGetName(const struct sAutomat * const pAutomat, /*@out@*/ char * const name)
{
	assert(NULL != pAutomat);

	name[NAMELEN - 1] = '\0';
	memmove(name, pAutomat->name, NAMELEN - 1);
	return true;
}

/**
 * Function to set the name of a automat
 * @param pAutomat	pointer to the automat
 * @param name		pointer to the string with the name 
 * @exception		assert on NULL pointer
 */ 
void automatSetName(struct sAutomat * const pAutomat, const char * const name)
{
	assert(NULL != pAutomat);

	memmove(pAutomat->name, name, NAMELEN);
}

/**
 * Function to get the route to be set if the conditions apply
 * @param pAutomat	pointer to the automat
 * @return		pointer to the route
 * @exception		assert on NULL pointer
 */ 
struct sRoute * automatGetAutomatedRoute(const struct sAutomat * const pAutomat)
{
	assert(NULL != pAutomat);

	return pAutomat->pAutomatedRoute;
}

/**
 * Function to set the route to be set if the conditions apply
 * @param pAutomat	pointer to the automat
 * @param pRoute	pointer to the route
 * @exception		assert on NULL pointer
 */
void automatSetAutomatedRoute(struct sAutomat * const pAutomat, struct sRoute * const pRoute)
{
	assert(NULL != pAutomat);

	pAutomat->pAutomatedRoute = pRoute;
}

#ifdef DOMONO55
	/**
	 * Function to get the button used to switch on the automat
	 * @param pAutomat	pointer to the automat
	 * @return		pointer to the 'On' button
	 * @exception		assert on NULL pointer
	 */
	const struct sButton * automatGetOnButton(const struct sAutomat * const pAutomat)
	{
		assert(NULL != pAutomat);

		return pAutomat->pOnButton;
	}

	/**
	 * Function to set the button used to switch on the automat
	 * @param pAutomat	pointer to the automat
	 * @param pButton 	pointer to the 'On' button
	 * @exception		assert on NULL pointer
	 */
	void automatSetOnButton(struct sAutomat * const pAutomat, struct sButton * const pButton)
	{
		assert(NULL != pAutomat);

		pAutomat->pOnButton = pButton;
	}

	/**
	 * Function to get the button used to switch off the automat
	 * @param pAutomat	pointer to the automat
	 * @return		pointer to the 'Off' button
	 * @exception		assert on NULL pointer
	 */
	const struct sButton * automatGetOffButton(const struct sAutomat * const pAutomat)
	{
		assert(NULL != pAutomat);

		return pAutomat->pOffButton;
	}

	/**
	 * Function to set the button used to switch off the automat
	 * @param pAutomat	pointer to the automat
	 * @param pButton 	pointer to the 'Off' button
	 * @exception		assert on NULL pointer
	 */
	void automatSetOffButton(struct sAutomat * const pAutomat, struct sButton * const pButton)
	{
		assert(NULL != pAutomat);

		pAutomat->pOffButton = pButton;
	}
#endif

/**
 * Function to get the handle of the desk LED showing on state of the automat
 * @param pAutomat	pointer to the automat
 * @return 		handle of the desk led
 * @exception		assert on NULL pointer
 */
const unsigned short automatGet_BO_DeskLED(const struct sAutomat * const pAutomat)
{
	assert(NULL != pAutomat);

	return pAutomat->BO_DeskLED;
}

/**
 * Function to set the handle of the red LED showing on state of the automat
 * @param pAutomat	pointer to the automat
 * @param pMain		handle of the desk led
 * @exception		assert on NULL pointer
 */
void automatSet_BO_DeskLED(struct sAutomat * const pAutomat, const unsigned short handle)
{
	assert(NULL != pAutomat);

	pAutomat->BO_DeskLED = handle;
}

/**
 * Function to add a section to the automat. This section is starting the automat when getting
 *  occupied
 * @param pAutomat	pointer to the automat
 * @param pSection	pointer to the section
 * @return		index, where the section has been inserted
 * @exception		Assertion on NULL pointer or too many sections
 */
unsigned short automatAddSection(struct sAutomat * const pAutomat, struct sSection * const pSection)
{
	assert(NULL != pAutomat);
	assert(LINENRSECTIONS > pAutomat->nrSections);

	pAutomat->pSection[pAutomat->nrSections] = pSection;
	pAutomat->nrSections++;
	return pAutomat->nrSections - 1;
}

/**
 * Function to get the indexed section of the automat. This section is starting the automat when getting
 *  occupied
 * @param pAutomat	pointer to the automat
 * @param nr		number (index) of the section
 * @return		pointer of the section, if index is too big, NULL is returned
 * @exception		Assertion on NULL pointer
 */
struct sSection * automatGetSection(const struct sAutomat * const pAutomat, const unsigned short index)
{
	assert(NULL != pAutomat);
	if(AUTOMATNRSECTIONS <= index)
	{
		return NULL;
	}
	return pAutomat->pSection[index];
}

/**
 * Function to add a route to the automat. This route is starting the automat when getting active
 * @param pAutomat	pointer to the automat
 * @param pSection	pointer to the route
 * @return		index, where the route has been inserted
 * @exception		Assertion on NULL pointer or too many sections
 */
unsigned short automatAddRoute(struct sAutomat * const pAutomat, struct sRoute * const pRoute)
{
	assert(NULL != pAutomat);
	assert(AUTOMATNRROUTES > pAutomat->nrRoutes);

	pAutomat->pRoute[pAutomat->nrRoutes] = pRoute;
	pAutomat->nrRoutes++;
	return pAutomat->nrRoutes - 1;
}

/**
 * Function to get the indexed route of the automat. This route is starting the automat when getting active
 * @param pAutomat	pointer to the automat
 * @param nr		number (index) of the route
 * @return		pointer of the route, if index is too big, NULL is returned
 * @exception		Assertion on NULL pointer
 */
struct sRoute * automatGetRoute(const struct sAutomat * const pAutomat, const unsigned short index)
{
	assert(NULL != pAutomat);
	if(AUTOMATNRROUTES <= index)
	{
		return NULL;
	}
	return pAutomat->pRoute[index];
}

/**
 * state machine function
 * provides the function when being inside the state given in pAutomat->eState
 * @param pAutomat		pointer to the automat leaving the state
 */
static enum eAutomatState automatStateBehaviour(struct sAutomat * const pAutomat);
static enum eAutomatState automatStateBehaviour(struct sAutomat * const pAutomat)
{	
	unsigned short		i;
	struct sSection *	pSection = NULL;
	struct sRoute *		pRoute = NULL;
	_Bool			toSet = false;
	enum eAutomatState	automatState;

	assert(NULL != pAutomat);

	if(pAutomat->Todo)
	{
		switch(pAutomat->state)
		{
		case AutomatNeutral:
			/* go to Off state	*/
			automatState = AutomatOff;
			break;

		case AutomatOff:
			/* this state is left when the automat shall be switched on	*/

			if(pAutomat->switchOn)
			{
				automatState = AutomatOn;
			}
			break;

		case AutomatOn:
			/* this state is left when the automat shall be switched off	*/

			if(pAutomat->switchOff)
			{
				automatState = AutomatOff;
				break;
			}
			/* else	*/
			/* now check if a route shall be set.	*/

			toSet = false;
			i = 0;
			while(NULL != (pSection = automatGetSection(pAutomat, i)))
			{
				/* check if any of the sections is occupied, if so, Route is to set	*/
				if(! sectionIsFree(pSection))
				{
					toSet = true;
					break;
				}
			}

			i = 0;
			while(NULL != (pRoute = automatGetRoute(pAutomat, i)))
			{
				/* check if any of the routes is active, if so, Route is to set	*/
				if((RouteEstablished == routeGetState(pRoute))
				|| (RouteBlocked == routeGetState(pRoute)))
				{
					toSet = true;
					break;
				}
			}

			if(toSet)
			{
				routeRequestState(automatGetAutomatedRoute(pAutomat), RouteBlocked);
			}
			break;

		default:
			automatState = AutomatNeutral;
			break;
		}
		pAutomat->switchOn  = false;
		pAutomat->switchOff = false;
		pAutomat->Todo	= false;
		return automatState;
	}
}

/**
 * state machine entry function
 * provides the function when entering the state given in pAutomat->eState
 * @param pAutomat	pointer to the automat leaving the state
 */
static void automatStateEntry(struct sAutomat * const pAutomat);
static void automatStateEntry(struct sAutomat * const pAutomat)
{
	assert(NULL != pAutomat);

	switch(pAutomat->state)
	{
	case AutomatNeutral:
		LOG_STW("Automat %s ist neu undefiniert", pAutomat->name);
		break;

	case AutomatOff:
		LOG_STW("Automat %s ist ausgeschaltet", pAutomat->name);
		break;

	case AutomatOn:
		LOG_STW("Automat %s ist eingeschaltet", pAutomat->name);
		break;

	default:
		break;
	}
}

/**
 * state machine exit function
 * provides the function when leaving the state given in pAutomat->eState
 * @param pAutomat	pointer to the automat leaving the state
 */
static void automatStateExit(struct sAutomat * const pAutomat);
static void automatStateExit(struct sAutomat * const pAutomat)
{
	assert(NULL != pAutomat);
}

/**
* Function to let all automats switch to the desired state
*/
void automatsProcess(void)
{
	struct sAutomat *	pAutomat = m_pAutomats;
	enum eAutomatState	resultState;
	int 			i;

	assert(NULL != pAutomat);

	for(i = 0; i < m_nrAutomats; i++)
	{
		resultState = automatStateBehaviour(pAutomat);
		if(pAutomat->state != resultState)
		{
			automatStateExit(pAutomat);
			pAutomat->state = resultState;
			automatStateEntry(pAutomat);
			pAutomat++;		
		}	
	}
}
