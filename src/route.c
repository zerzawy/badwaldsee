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
 * Train routes (Zugfahrstrassen)
 * \author
 * Kurt Zerzawy
 */

/*
 * $Author: kurt $
 * $Date: 2015-06-07 17:35:47 +0200 (Son, 07. Jun 2015) $
 * $Revision: 2484 $
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


/*@only@*/ /*@null@*/ static struct sRoute * m_pRoute = NULL;	/**< Pointer to the first train route	*/
static unsigned short m_nrRoute = 0;	/**< Number of train routes	*/

/**
 * Constructor for the train routes
 * @param n		number of train routes to be created
 * @exception		Calls exit if no RAM available
 */
void routes(const unsigned short nr)
{
	unsigned short		i, j;
	struct sRoute *		pRoute;

	LOG_INF("started");
	
	m_nrRoute = nr;
	m_pRoute = malloc(m_nrRoute * sizeof(struct sRoute));
	if(NULL == m_pRoute) 
	{
		/* error allocating */
		LOG_ERR("cannot allocate RAM of train routes");
		exit(EXIT_FAILURE);
	}
	pRoute = m_pRoute;
	for(i = 0; i < m_nrRoute; i++)
	{
		pRoute->name[0]	= '\0';
		pRoute->toRight	= false;
		pRoute->start	= NULL;
		pRoute->dest	= NULL;
		pRoute->nrDistants	= 0;
		pRoute->eState		= RouteInit;
		pRoute->destButton	= false;
		pRoute->twoButtons	= false;
		pRoute->dissolve	= false;
		pRoute->toHalt		= false;
		pRoute->partsAddPosition	= 0;
		pRoute->headPosition	= -2;
		pRoute->tailPosition	= -1;
		pRoute->dissolveConditions = false;
		pRoute->dissolveCondition1 = false;
		pRoute->dissolveCondition2 = false;
		pRoute->dissolveCondition3 = false;
		pRoute->eRequestedState	= ShuntrouteNone;

		for(j = 0; j < NRSHUNTPARTS; j++)
		{
			pRoute->pPart[j].type		= RoutePartNone;
			pRoute->pPart[j].argument	= 0;
			pRoute->pPart[j].ptr		= NULL;
			pRoute->pPart[j].isOccupied	= false;
			pRoute->pPart[j].wasOccupied	= false;
		}
		pRoute->resetSection	= 0;
		pRoute->dissolveSection	= 0;
		
		pRoute++;
	}

	LOG_INF("ended");
}

/**
 * Destructor for the train routes
 */
void routesDestroy(void) 
{
	LOG_INF("started");
	if(NULL == m_pRoute) 
	{
		LOG_INF("zero pointer, terminate");
		return;
	}
	LOG_INF("release RAM of train routes");
	free(m_pRoute);
	m_pRoute = NULL;
	LOG_INF("ended");
}

/**
 * Init function for the train routes
 *
 * Here, all train routes are initialised 
 */
void routesInit(void) 
{
}

/**
 * Function to get the number of train routes 
 * @return		Number of train routes
 */
unsigned short routesGetNr(void)
{
	return m_nrRoute;
}

/**
 * Function to get the pointer by giving the number of the route
 * @param i		index, starting with 0
 * @return		pointer to this route, NULL if not existing
 */ 
struct sRoute * routeGet(const unsigned short index)
{
	if(index >= m_nrRoute)
	{
		return NULL;
	}
	else
	{
		return m_pRoute + index;
	}
}

/**
 * Function to find the pointer of a route by giving the name
 * @param name		name of the route to be locked for
 * @return		pointer of the found route
 * @exception		a parser assert is made if not found
 */
struct sRoute * routeFind(const char * const name) 
{
        unsigned short	i = 0;
	struct sRoute *	pRoute = NULL;
        char		temp[128];

	i = 0;
	while(NULL != (pRoute = routeGet(i)))
	{ 
		if(0 == (strcmp(pRoute->name, name))) 
		{
			return pRoute;
		}
		i++;
        }
	sprintf(temp, "Fahrstrasse >%s< nicht gefunden", name);
        parserExit(temp);
        return NULL;
}

/**
 * Helper function to update the registers containing the occupancy data
 * @param pRoute		pointer to the shuntroute to be updated
 */
static void routeUpdateOccupancy(struct sRoute * const pRoute);
static void routeUpdateOccupancy(struct sRoute * const pRoute)
{
	unsigned short	j = 0;

	assert(NULL != pRoute);

	for(j = 0; j < NRSHUNTPARTS; j++)
	{
		switch(pRoute->pPart[j].type) 
		{
		case RoutePartNone:
			break;

		case RoutePartTurnout:
			pRoute->pPart[j].isOccupied = ! turnoutIsFree(pRoute->pPart[j].ptr);
			break;

		case RoutePartShuntroute:
			pRoute->pPart[j].isOccupied = ! shuntrouteIsFree(pRoute->pPart[j].ptr);
			break;

		case RoutePartCondShuntroute:
			break;

		case RoutePartCondOccupied:
			pRoute->pPart[j].isOccupied = ! sectionIsFree(pRoute->pPart[j].ptr);
			break;

		case RoutePartIllegalRoute:
			break;

		case RoutePartCondRoute:
			break;

		case RoutePartLine:
			/* TODO Line is not implemented yet	*/
			break;

		case RoutePartSection:
			pRoute->pPart[j].isOccupied = ! sectionIsFree(pRoute->pPart[j].ptr);
			break;

		default:
			printf("strange default with j=%d %d\n", j, pRoute->pPart[j].type);

			assert(0);
			break;
		}
	}
}

/**
 * Helper function to find out if a change in the state (or keeping a state )
 *  is possible.  This function is separated from the state machine since it is
 *  used by the state machine and by the request function form the 
 *  combi train route.
 * 
 * @param pRoute	pointer to the route to be checked
 * @param requested	requested state
 * @parem combiRoute	to be set true if request from combiroute
 * @return		true if possible
 * @exception		assert on NULL pointer
 */
static _Bool routeTransitionPossible(struct sRoute * const pRoute,
					const enum eRouteState requested,
					const _Bool combiRoute);
static _Bool routeTransitionPossible(struct sRoute * const pRoute,
					const enum eRouteState requested,
					const _Bool combiRoute)
{
	enum eLine			eSectionLine = neutral;
	enum eShuntrouteState		eShuntrouteState;	/**< state of the conditional shuntroute	*/
	enum eRouteState		eRouteState;		/**< state of the conditional or illegal route	*/
	unsigned short			condition;		/**< calculated condition for this element	*/
	unsigned short			j;

	assert(NULL != pRoute);

	/* update the occupancies, they are needed later	*/
	routeUpdateOccupancy(pRoute);

	/* now prepare the masks for turnouts, protective turnouts and lines
	 * they depend on the requested state
	 * the turnout is supposed to be Minus state in the beginning
	 * the direction is supposed to be left to right
	 */

	switch(requested)
	{
	case RouteInit:
		return true;
		break;

	case RouteNone:
		return true;
		break;

	case RouteDestButton:
		return true;
		break;

	case RouteRegistered:
		return true;
		break;

	case RouteReserve:
		eSectionLine = LINERESERVE;
		eShuntrouteState = ShuntrouteReserve;
		break;

	case RouteSwitch:
		eSectionLine = LINESWITCH;
		eShuntrouteState = ShuntrouteSwitch;
		break;

	case RouteEstablished:
		eSectionLine = LINEBLOCK;
		eShuntrouteState = ShuntrouteEstablished;
		break;

	case RouteBlocked:
		eSectionLine = LINEINTERLOCK;
		eShuntrouteState = ShuntrouteBlocked;
		break;

	default:
		break;
	}

	/* handle only the elements in front of the tails
	 * the other elements could be dissolved already
	 * without influencing the route
	 */

	for(j = 0; j < NRROUTEPARTS; j++)
	{
		/* read the condition of this element
		 * the condition must only contain the direction LR / RL 
		 * and the Plus/Minus of the turnout, if any
		 * the other conditions are made by eSectionLine
		 */
		condition = TURNOUTDIRMASK & pRoute->pPart[j].argument;
		condition |= eSectionLine;

		/* now handle each element depending on its type	*/
		switch(pRoute->pPart[j].type) 
		{
		case RoutePartNone:
			break;

		case RoutePartTurnout:
			if(!turnoutLine(pRoute->pPart[j].ptr, condition))
			{
				return false;
			}

			/* now check occupancy for trainroutes	*/
			if(!turnoutIsFree(pRoute->pPart[j].ptr))
			{
				return false;
			}
			break;

		case RoutePartShuntroute:
			if(!shuntrouteRequestState(pRoute->pPart[j].ptr, eShuntrouteState))
			{
				return false;
			}
			break;

		case RoutePartCondShuntroute:

			/* special case: The condition shuntroute is only valid for reserve
			 * and switch
			 */
			if((RouteReserve == requested) || (RouteSwitch == requested))
			{
				eShuntrouteState = shuntrouteGetState(pRoute->pPart[j].ptr);
				if((ShuntrouteEstablished != eShuntrouteState)
				&& (ShuntrouteBlocked != eShuntrouteState))
				{
					/* now the conditional shuntroute is not active	*/
					return false;
				}
			}
			break;

		case RoutePartCondOccupied:
			/* special case: The condition occupied is only valid for reserve
			 * and switch
			 */
			if((RouteReserve == requested) || (RouteSwitch == requested))
			{
				if(! pRoute->pPart[j].isOccupied)
				{
					return false;
				}
			}

		case RoutePartIllegalRoute:
			/* now check if this illegal route is in some way set up or 
			 * on the way to be set up
			 */
			if(RouteNone != routeGetState(pRoute->pPart[j].ptr))
			{
				return false;
			}

		case RoutePartCondRoute:
			/* special case: The condition route is only valid for reserve
			 * and switch
			 */
			if((RouteReserve == requested) || (RouteSwitch == requested))
			{
				eRouteState = routeGetState(pRoute->pPart[j].ptr);
				if((RouteEstablished != eRouteState)
				&& (RouteBlocked != eRouteState))
				{
					/* now the conditional route is not active	*/
					return false;
				}
			}
			break;

		case RoutePartLine:
			/* TODO
			 * here check, if the line is not occupied
			 * and if it is possible to set it to correct direction
			 */
			 break;

		case RoutePartSection:
			if(pRoute->pPart[j].isOccupied)
			{
				return false;
			}
			break;

		default:
			printf("strange default with j=%d %d\n", j, pRoute->pPart[j].type);

			assert(0);
			break;
		}
	}
	return true;
}

/**
 * Helper Function to find the head of the train movement
 * @param pRoute	pointer to the route to be observed
 * @return		head of the train, -2 if not found
 * @exception		assert on NULL Pointer
 */
static short routeTrainHead(const struct sRoute * const pRoute);
static short routeTrainHead(const struct sRoute * const pRoute)
{
	short	tmpHead		= -2;	/**< must be set to -2 so a free element 0 does not fulfill condition for head	*/
	short	lastElement	= -2;	/**< last real element passed by the train. Used to calculate situations
					 *    where every real element is occupied	
					 */
	short	i;
	_Bool	occupied = false;

	assert(NULL != pRoute);

	/* first find out where is the head of the train	*/

	for(i = 0; i < NRSHUNTPARTS; i++)
	{
		switch(pRoute->pPart[i].type)
		{
		case RoutePartTurnout:
		case RoutePartShuntroute:
		case RoutePartLine:
		case RoutePartSection:
			occupied = pRoute->pPart[i].isOccupied;
			lastElement = i;
			break;

		default:
			break;
		}

		if(occupied)
		{
			/* it is occupied, remember this place as possible head	*/
			tmpHead = i;
		}
		else
		{
			/* it is free. check if the temporary head was set to 
			 * the last element. If so, then the head is found
			 */
			if(i == tmpHead + 1)
			{
				/* head found	*/
				return tmpHead;
			}
		}
	}
	/* now two cases are possible:
	 * case 1: it was never occupied, return -2
	 * case 2: last real element was occupied and that is why the 
	            head could not be found. Remember: all other elements
		    do not change the state of occupied
		    So, the last element is the head of the train.
         */
	if(tmpHead == -2)
	{
		/* case number 1	*/	
		return -2;
	}
	else
	{
		return lastElement;
	}
}


/**
 * Helper Function to find the tail of the train movement
 * @param pRoute	pointer to the route to be observed
 * @param tmpHead	temporary head of train. MUST really be the head.
 * @return		tail of the train
 * @exception		assert on NULL Pointer
 */
static short routeTrainTail(const struct sRoute * const pRoute, short tmpHead);
static short routeTrainTail(const struct sRoute * const pRoute, short tmpHead)
{
	short 	tmpTail = tmpHead;
	short	i;
	_Bool	occupied = true;

	assert(NULL != pRoute);

	if(-2 == tmpHead)
	{
		return -2;
	}

	for(i = tmpHead - 1; i >= 0; i--)
	{
		switch(pRoute->pPart[i].type)
		{
		case RoutePartTurnout:
		case RoutePartSection:
			occupied = pRoute->pPart[i].isOccupied;
			break;

		default:
			break;
		}

		if(occupied)
		{
			/* it is occupied, remember this place as possible tail	*/
			tmpTail = i;
		}
		else
		{
			/* it is free. So the tail must be the next element.
			 * note that we searched from the occupied head
			 */
			return i + 1;
		}
	}
	return tmpTail;
} 

/**
 * state machine function
 * provides the function when being inside the state given in pRoute->eState
 * @param pRoute	pointer to the train route leaving the state
 */
static enum eRouteState routeStateBehaviour(struct sRoute * const pRoute);
static enum eRouteState routeStateBehaviour(struct sRoute * const pRoute)
{
	short	tmpHead		= -2;
	short	tmpTail		= -1;

	routeUpdateOccupancy(pRoute);

	switch(pRoute->eState)
	{
	case RouteInit: /* just go to none      */
		return RouteNone; 
		break;

	case RouteNone:
		/* init values needed later	*/
		pRoute->dissolve = false;

		if(RouteNone != pRoute->eRequestedState)
		{
			/* now the combi route want to set the route	*/
			return pRoute->eRequestedState;
		}
		/* else */

		/* if two buttons are pushed, route shall be registered	*/
		if(pRoute->twoButtons)
		{
			routeUpdateOccupancy(pRoute);
			return RouteRegistered;
		}

		/* keep state	*/
		break;

	case RouteDestButton:
		switch(pRoute->eRequestedState)
		{
		case RouteInit:
			break;

		case RouteNone:
			break;

		case RouteDestButton:
			break;

		case RouteRegistered:
			return RouteRegistered;
			break;

		case RouteReserve:
			return RouteReserve;
			break;

		case RouteSwitch:
			return RouteSwitch;
			break;

		case RouteEstablished:
			return RouteEstablished;
			break;

		default:
			break;
		}

		/*
		 * check if the destination button still is pushed. If so, nothing to do
		 *  so far, otherwise it must be checked in which case the destination
		 *  was released
		 */
		routeUpdateOccupancy(pRoute);

		if((!pRoute->destButton) && (!pRoute->dissolve)) 
		{
			/* normal release of both buttons, destination button last	*/
			return RouteReserve;
		}

		if((!pRoute->destButton) && (pRoute->dissolve))
		{
			/* dissolve was pushed and the destination button released	*/
			return RouteNone;
		}

		/* now the cases with the destination button or both buttons pushed	*/
		if(pRoute->twoButtons)
		{
			/* two buttons pushed	*/
			pRoute->twoButtons = false;
			return RouteRegistered;
		}
		/* else */
		/* only destination button pushed, keep state	*/
		break;
	
	case RouteRegistered:
		switch(pRoute->eRequestedState)
		{
		case RouteInit:
			break;

		case RouteNone:
			break;

		case RouteDestButton:
			break;

		case RouteRegistered:
			break;

		case RouteReserve:
			return RouteReserve;
			break;

		case RouteSwitch:
			return RouteSwitch;
			break;
	
		case RouteEstablished:
			return RouteEstablished;
			break;

		default:
			break;
		}

		if(pRoute->destButton)
		{
			/* destination button pushed	*/
			pRoute->destButton = false;
			return RouteDestButton;	
		}
		/* else */
		if(! pRoute->twoButtons)
		{
			/* no more button pushed	*/
			return RouteReserve;
		}
		break;

	case RouteReserve:
		/* a train route shall be reserved to be switched and established later
		 * so set all lines to reserve and if this succeeds, then switch them.
		 */
		switch(pRoute->eRequestedState)
		{
		case RouteInit:
			break;

		case RouteNone:
			break;

		case RouteDestButton:
			break;

		case RouteRegistered:
			break;

		case RouteReserve:
			break;

		case RouteSwitch:
			return RouteSwitch;
			break;

		case RouteEstablished:
			return RouteEstablished;
			break;

		default:
			break;
		}

		/* check if a higher state is possible	*/
		if(routeTransitionPossible(pRoute, RouteSwitch, false));
		{
			return RouteSwitch;
		}

		/* check if state reserve can be kept	*/
		if(!routeTransitionPossible(pRoute, RouteReserve, false))
		{
			/* even reserve is not possible	*/
			return RouteNone;
		}
		/* possible, keep state	*/
		break;
	
	case RouteSwitch:
		/* a train route shall be switched to be established later
		 * so set all lines to switch and if this succeeds, then establish them.
		 */
		routeUpdateOccupancy(pRoute);

		switch(pRoute->eRequestedState)
		{
		case RouteInit:
			break;

		case RouteNone:
			break;

		case RouteDestButton:
			break;

		case RouteRegistered:
			break;

		case RouteReserve:
			break;

		case RouteSwitch:
			return RouteSwitch;
			break;

		case RouteEstablished:
			return RouteEstablished;
			break;

		default:
			break;
		}

		if(routeTransitionPossible(pRoute, RouteEstablished, false))
		{
			return RouteEstablished;
		}
		break;
	
	case RouteEstablished:
	case RouteBlocked:
		/* now a train route is established.
		 * continue checking for things happening 
		 * which make the train route dissolve.
		 */
		routeUpdateOccupancy(pRoute);

		/* TODO calculation if all conditions for keeping the
		 * route are still met. If one disappears, the mains shall
		 * be set to halt
		 */
		if(pRoute->dissolve)
		{
			/* now the route shall be dissolved
			 * this is only possible if the main is not clear
			 */
			#ifdef SPDR60
				if(SIG_MN_HP0 == mainGetAspect(pRoute->start))
				{
					return RouteNone;
				}
			#endif

			#ifdef DOMINO55
				if(SIG_MN_HALT == mainGetAspect(pRoute->start))
				{
					return RouteNone;
				}
			#endif
		}

		/*--------------------------------------------------------------*/
		/* HALT calculation of the mains                                */
		/* the mains is set to HP0 / Halt if the train is entering the  */
		/*  mains set halt section 'Haltefallenabschnitt'		*/
		/*--------------------------------------------------------------*/

		 
		assert(NULL != pRoute->resetSection);

		if(! sectionIsFree(pRoute->resetSection))
		{
			#ifdef SPDR60
				mainSetAspect(pRoute->start, SIG_MN_HP0);
			#endif

			#ifdef DOMINO55
				mainSetAspect(pRoute->start, SIG_MN_HALT);
			#endif
		}


		/*-------------------------------------------------------------*/
		/* Calculation of the dissolve by a train passing              */
		/*-------------------------------------------------------------*/
		/* the check for dissolve is done separately to prevent checking things like 
		 * protective switches or protective sections 
		 */

		/* first find out where the head and the tail of the train is	*/
		tmpHead = routeTrainHead(pRoute);
		tmpTail = routeTrainTail(pRoute, tmpHead);

		/* Now calculate if one of the were broken by the new 
		 *  values of head and tail
		 *--------------------------------------------------------*/

		if(tmpHead < pRoute->headPosition)
		{
			/* now the head returned somehow. This is not allowed for trains
			 *  (only for shunting)
			 */
			pRoute->dissolveConditions = false;
		}

		if((tmpTail > pRoute->headPosition) && (0 <= pRoute->headPosition))
		{
			/* now a section got occupied without the last section still occupied (jumping)
			 * this is not allowed. Filtering out cases where the headPosition is not
			 * really a position inside the elements
			 */
			pRoute->dissolveCondition2 = false;
		}

		/* now calculate if the dissolve element was left. This is only
		 * possible if tmpTail is bigger than the dissolve Section
		 */
		if(tmpTail > pRoute->dissolveSection)
		{
			pRoute->dissolveCondition3 = true;
		}

		/* check if the route is dissolved by passing it correctly	*/
		if(pRoute->dissolveConditions
		&& pRoute->dissolveCondition1
		&& pRoute->dissolveCondition2
		&& pRoute->dissolveCondition3
		&& pRoute->toHalt)
		{
			/* all is done, reset the complete route	*/
			return RouteNone;
		}
		/* else train is still driving inside route	*/
		pRoute->headPosition	= tmpHead;
		pRoute->tailPosition	= tmpTail;

		/* now adapt the state of the route	*/
		if(RouteEstablished == pRoute->eState)
		{
			return RouteBlocked;
		}
		break;
	default:
		assert(0);
		break;
	}
	pRoute->destButton = false;
	pRoute->twoButtons = false;
	pRoute->dissolve = false;

	return pRoute->eState;
}

/**
 * state machine exit function
 * provides the function when entering the state given in pRoute->eState
 * @param pRoute		pointer to the train route leaving the state
 */
static void routeStateEntry(struct sRoute * const pRoute);
static void routeStateEntry(struct sRoute * const pRoute)
{
	switch(pRoute->eState)
	{
        case RouteInit:
		break;

	case RouteNone:
		pRoute->dissolve = false;
		pRoute->destButton = false;
		pRoute->toHalt = false;
		#ifdef SPDR60
			mainSetAspect(pRoute->start, SIG_MN_HP0);
		#endif

		#ifdef DOMINO55
			mainSetAspect(pRoute->start, SIG_MN_HALT);
		#endif
		LOG_STW("Zugfahrstrasse %s ist passiv", pRoute->name);
		break;
	
	case RouteDestButton:
		LOG_STW("Zugfahrstrasse %s Zieltaste ist gedrueckt", pRoute->name);
		break; 
	
	case RouteRegistered:
		LOG_STW("Zugfahrstrasse %s ist registriert", pRoute->name);
		break;
	
	case RouteReserve:
		LOG_STW("Zugfahrstrasse %s ist reserviert", pRoute->name);
		break;
	
	case RouteSwitch:
		LOG_STW("Zugfahrstrasse %s schaltet", pRoute->name);
		break;
	
	case RouteEstablished:
	case RouteBlocked:
		/* initializing the state. For the dissolve conditions I suppose that
		 * everything goes well for condition 1 and 2, it is set to false if
		 * something goes wrong.
		 * condition3 must be proven first
		 */
		pRoute->occupiedPosition = -1;
		pRoute->dissolveCondition1 = true;
		pRoute->dissolveCondition2 = true;
		pRoute->dissolveCondition3 = false;

		/* now set the train Head and Tail correct. This is
		 * necessary because the passing of the train is dissolving
		 * single parts of the route, thus the function routeTransitionPossible
		 * must know if the element is still necessary to check
		 * in other words: if the tail was passing a part, it can change
		 *  the desired state
		 */
		pRoute->headPosition	= -2;
		pRoute->tailPosition	= -1;

		if(RouteEstablished == pRoute->eState)
		{
			LOG_STW("Zugfahrstrasse %s ist eingefahren", pRoute->name);
		}
		else
		{
			LOG_STW("Zugfahrstrasse %s ist verschlossen", pRoute->name);
			mainSetAspect(pRoute->start, pRoute->mainAspect);
		}
		break;
	default: break;
	}
}

/**
 * state machine exit function
 * provides the function when leaving the state given in pRoute->eState
 * @param pRoute		pointer to the main signal leaving the state
 */
static void routeStateExit(struct sRoute * const pRoute);
static void routeStateExit(struct sRoute * const pRoute)
{
	return;	/* temporary switched off this comments	*/
	switch(pRoute->eState)
	{
        case RouteInit:
		LOG_STW("Zugfahrstrasse %s ist nicht mehr init", pRoute->name);
		break;

	case RouteNone:
		LOG_STW("Zugfahrstrasse %s ist init", pRoute->name);
		break;
	
	case RouteDestButton:
		LOG_STW("Zugfahrstrasse %s Zieltaste ist nicht mehr gedrueckt", pRoute->name);
		break; 
	
	case RouteRegistered:
		LOG_STW("Zugfahrstrasse %s ist nicht mehr registriert", pRoute->name);
		break;
	
	case RouteReserve:
		LOG_STW("Zugfahrstrasse %s ist nicht mehr reserviert", pRoute->name);
		break;
	
	case RouteSwitch:
		LOG_STW("Zugfahrstrasse %s ist nicht mehr geschaltet", pRoute->name);
		break;
	
	case RouteEstablished:
		LOG_STW("Zugfahrstrasse %s ist nicht mehr eingefahren", pRoute->name);
		break;

	case RouteBlocked:
		LOG_STW("Zugfahrstrasse %s ist nicht mehr verschlossen", pRoute->name);
		break;

	default:
		break;
	}
}

/**
 * Function to let all train routes switch to the desired state
 */
void routesProcess(void)
{
	struct sRoute *	pRoute = m_pRoute;
	int 	i;
	enum eRouteState resultState;

	assert(NULL != pRoute);

	for(i = 0; i < m_nrRoute; i++)
	{
		/**
		 * now the lines of the sections, mainss, mains and turnouts are activated. Since
		 *  these signals are static nature, they always must be applied
		 * you can compare these signals more to the voltage applied to a relais than 
		 *  a signal like in signal-slot combination of Qt
		 */
	
		resultState = routeStateBehaviour(pRoute);
		if(pRoute->eState != resultState)
		{
			routeStateExit(pRoute);
			pRoute->eState = resultState;
			routeStateEntry(pRoute);
		}
		pRoute++;		
	}	
}

/**
 * Function for cancelling a Route either before it is established or
 * if it is not part of a route
 */
void routeCancel(const struct sMain * const pMain)
{
	unsigned short		i;
	struct sRoute *	pRoute;

	LOG_INF("started");

	pRoute = m_pRoute;
	for(i = 0; i < m_nrRoute; i++)
	{
		/* TODO find out if it really was this train route
		 *      make sure the train route may be cancelled, only allowed if no
		 *	 train route is active over this train route
		 */
		if(pMain == pRoute->dest)
		{
			pRoute->dissolve = true;
		}
		pRoute++;
	}
	LOG_INF("ended");
}

#ifdef DOMINO55
	/**
	 * Function called when only one mains button is pushed. This is important
	 *  for storing the train route. Remember: as soon as the destination is not
	 *  pushed any more, the train route is stored
	 *
	 * on SpDr60, this function is not implemented.
	 *
	 * @param *pMain	pointer to the button of the mains which button is pushed
	 * @exception		exception on NULL pointer
	 */
	void routeOneButton(const struct sMain const * pMain)
	{
		unsigned short		i = 0;
		struct sRoute *		pRoute = m_pRoute;

		assert(NULL != pRoute);

		for(i = 0; i < m_nrRoute; i++)
		{
			if(pMain == pRoute->dest)
			{
				/* the train route was found
				 * note that we use the state as filter, since many
				 *  train routes can have the same destination but
				 *  only one registed before shall be used
				 */
				pRoute->destButton = true;
			}
			pRoute++;
		}
	}	
#endif

/**
 * Function called when only two mains buttons are pushed. 
 * @param pMain1	pointer to the button of the mains which button is pushed
 * @param pMain2	pointer to the second mains which button is pushed
 */
void routeTwoButtons(const struct sMain const * pMain1, const struct sMain const * pMain2)
{
	unsigned short		i;
	struct sRoute *	pRoute;

	pRoute = m_pRoute;
	assert(NULL != pRoute);

	if(pMain1 == pMain2)
	{
		/* not really two buttons.	*/
		return;
	}

	for(i = 0; i < m_nrRoute; i++)
	{
		if(((pMain1 == pRoute->start) && (pMain2 == pRoute->dest))
		|| ((pMain1 == pRoute->dest)  && (pMain2 == pRoute->start)))
		{
			/* register this train route	*/
			pRoute->twoButtons = true;
			return;
		}
		pRoute++;
	}
}

/**
 * Function to get the name of a train route
 * @param pRoute	pointer to the train route
 * @param name	pointer to the string filled out with the name
 * 		of the turnout. Must be at least NAMELEN long
 * @return	true, if successful
 * @exception	assert on NULL pointer
 */ 
_Bool routeGetName(const struct sRoute * const pRoute, /*@out@*/ char * const name)
{
	assert(NULL != pRoute);

	name[NAMELEN - 1] = '\0';
	memmove(name, pRoute->name, NAMELEN - 1);
	return true;
}

/**
 * Function to set the name of a train route
 * @param pRoute	pointer to the train route
 * @param name	pointer to the string with the name 
 * @exception	assert on NULL pointer
 */ 
void routeSetName(struct sRoute * const pRoute, const char * const name)
{
	assert(NULL != pRoute);
	
	memmove(pRoute->name, name, NAMELEN);
}

/**
 * Function to set the direction to the right
 * @param pRoute		pointer to the train route
 * @param toright	true if shunt route goes from left to right
 * @exception		assert on NULL pointer
 */
void routeSetToRight(struct sRoute * const pRoute, const _Bool toright)
{
	assert(NULL != pRoute);

	pRoute->toRight = toright;
}

/**
 * Function to get the direction of the train route
 * @param pRoute		pointer to the train route
 * @return		true if shunt route goes from left to right
 * @exception		assert on NULL pointer
 */
_Bool routeGetToRight(const struct sRoute * const pRoute)
{
	assert(NULL != pRoute);

	return pRoute->toRight;
}

/**
 * Function to get the start mains
 * @param pRoute		pointer to the train route
 * @return			pointer to the start mains
 * @exception		assert on NULL pointer
 */
struct sMain * routeGetStart(const struct sRoute * pRoute)
{
	assert(NULL != pRoute);

	return pRoute->start;
}

/**
 * Function to set the start mains
 * @param pRoute		pointer to the train route
 * @param start		pointer to the start mains
 * @exception		assert on NULL pointer
 */
void routeSetStart(struct sRoute * const pRoute, struct sMain * start)
{
	assert(NULL != pRoute);

	pRoute->start = start;
}

/**
 * Function to get the destination mains
 * @param pRoute	pointer to the train route
 * @return	pointer to the destination mains
 * @exception	assert on NULL pointer
 */
struct sMain * routeGetDest(const struct sRoute * pRoute)
{
	assert(NULL != pRoute);

	return pRoute->dest;
}

/**
* Function to set the destination mains
* @param pRoute		pointer to the train route
* @param dest		pointer to the destination mains
* @exception		assert on NULL pointer
*/
void routeSetDest(struct sRoute * const pRoute, struct sMain * dest)
{
	assert(NULL != pRoute);

	pRoute->dest = dest;
}

/**
* Function to get the aspect of the main signal when this route is set
* @param pRoute	pointer to the train route
* @return		mains signals aspect when this route is set
* @exception		assert on NULL pointer
*/
enum eMainAspect routeGetMainAspect(const struct sRoute * const pRoute)
{
	assert(NULL != pRoute);

	return pRoute->mainAspect;
}

/**
 * Function to set the aspect to be shown on the main signal wehen this route is set
 * @param pRoute	pointer to the train route
 * @param aspect	aspect to be shown
 * @exception		assert on NULL pointer or invalid aspect
 */
void routeSetMainAspect(struct sRoute * const pRoute, enum eMainAspect aspect)
{
	assert(NULL != pRoute);

#ifdef SPDR60
	switch(aspect)
	{
	case SIG_MN_UNDEF:	pRoute->mainAspect = SIG_MN_UNDEF;	break;
	case SIG_MN_HP0:	pRoute->mainAspect = SIG_MN_HP0;	break;
	case SIG_MN_HP1:	pRoute->mainAspect = SIG_MN_HP1;	break;
	case SIG_MN_HP1_ZS3:	pRoute->mainAspect = SIG_MN_HP1_ZS3;break;
	case SIG_MN_HP2:	pRoute->mainAspect = SIG_MN_HP2;	break;
	case SIG_MN_HP2_ZS3:	pRoute->mainAspect = SIG_MN_HP2_ZS3;break;
	default: assert(false); break;
	}
#endif

#ifdef DOMINO55
	switch(aspect)
	{
	case SIG_MN_UNDEF:	pRoute->mainAspect = SIG_MN_UNDEF;	break;
	case SIG_MN_HALT:	pRoute->mainAspect = SIG_MN_HALT;	break;
	case SIG_MN_1:		pRoute->mainAspect = SIG_MN_1;	break;
	case SIG_MN_2:		pRoute->mainAspect = SIG_MN_2;	break;
	case SIG_MN_3:		pRoute->mainAspect = SIG_MN_3;	break;
	case SIG_MN_5:		pRoute->mainAspect = SIG_MN_5;	break;
	case SIG_MN_6:		pRoute->mainAspect = SIG_MN_6;	break;
	default: assert(false); break;
	}
#endif
}

/**
 * Function to get the section that is setting the main signal to 
 *  halt aspect
 * @param pRoute	pointer to the route
 * @return 		pointer to the section setting main signal to halt
 * @exception		assert on NULL pointer
 */
struct sSection * routeGetResetSection(const struct sRoute * const pRoute)
{
	assert(NULL != pRoute);

	return pRoute->resetSection;
}

/**
 * Function to set the section that is setting the main signal to 
 *  halt aspect
 * @param pRoute	pointer to the route
 * @param pSection	pointer to the section setting the main signal to halt
 * @exception		assert on NULL pointer
 */
void routeSetResetSection(struct sRoute * const pRoute, struct sSection * const pSection)
{
	assert(NULL != pRoute);

	pRoute->resetSection = pSection;
}

/**
 * Function to get the number of distant signals used by this route
 * @param pRoute	pointer to the route
 * @return 		number of distant signals used by this route
 * @exception		assert on NULL pointer
 */
unsigned short routeGetNrDistants(const struct sRoute * const pRoute)
{
	assert(NULL != pRoute);

	return pRoute->nrDistants;
}

/**
 * Function to set the number of distant signals used by this route.
 * @param pRoute	pointer to the route
 * @param nrDistants	number of distant signals
 * @exception		assert on NULL pointer
 */
void routeSetNrDistants(struct sRoute * const pRoute, const unsigned short nrDistants)
{
	assert(NULL != pRoute);

	pRoute->nrDistants = nrDistants;
}

/**
 * Function to get the number of parts of the train route
 * @param pRoute	pointer to the train route
 * @return		number of parts the train route consists of
 * @exception		assert on NULL pointer
 */
unsigned short routeGetParts(const struct sRoute * const pRoute)
{
	assert(NULL != pRoute);

	return pRoute->partsAddPosition;
}

/**
 * Function to get the indexed pointer of parts of a train route
 * @param pRoute	pointer to the train route
 * @param nr		number of the train route part
 * @return		the argument of this part
 * @exception		Assertion on NULL pointer or if index is too big
 */
unsigned short routeGetPartArgumentByIndex(const struct sRoute * const pRoute, const unsigned short index);
unsigned short routeGetPartArgumentByIndex(const struct sRoute * const pRoute, const unsigned short index)
{
	assert(NULL != pRoute);
	assert(NRROUTEPARTS > index);

	return pRoute->pPart[index].argument;
}

/**
 * Function to add a turnout to a train route
 * @param pRoute	pointer to the train route
 * @param pTurnout	pointer to the turnout to be inserted
 * @param line		line the turnout has to be set to
 * @return		index, where the turnout has been inserted
 * @exception		Assertion on NULL pointer or too many turnout parts
 */
unsigned short routeAddTurnout(struct sRoute * const pRoute, struct sTurnout * const pTurnout, const enum eTurnoutLine line)
{
	assert(NULL != pRoute);
	assert(NRROUTEPARTS > pRoute->partsAddPosition);

	pRoute->pPart[pRoute->partsAddPosition].type = RoutePartTurnout;
	pRoute->pPart[pRoute->partsAddPosition].ptr = (void *) pTurnout;
	pRoute->pPart[pRoute->partsAddPosition].argument = line;
	pRoute->partsAddPosition++;
	return pRoute->partsAddPosition - 1;
}

/**
 * Function to get the indexed pointer of a turnout as a part of a train route
 * @param pRoute	pointer to the train route
 * @param nr		number of the train route part
 * @return		pointer of the turnout, if index is too big or no turnout on this place, NULL is returned
 * @exception		Assertion on NULL pointer
 */
struct sTurnout * routeGetTurnout(const struct sRoute * const pRoute, const unsigned short index)
{
	assert(NULL != pRoute);
	if(NRROUTEPARTS <= index)
	{
		return RoutePartNone;
	}

	if(RoutePartTurnout != pRoute->pPart[index].type)
	{
		return NULL;
	}

	return (struct sTurnout *) pRoute->pPart[index].ptr;
}
	
/**
 * Function to get the parameter of a turnout as a part of a train route
 * @param pRoute	pointer to the train route
 * @param nr		number of the train route part
 * @return		argument
 * @exception		Assertion on NULL pointer, if it is not a turnout or if index is too big
 */
enum eTurnoutLine routeGetTurnoutArgument(const struct sRoute * const pRoute, const unsigned short index)
{
	assert(NULL != pRoute);
	assert(NRROUTEPARTS > index);
	assert(RoutePartTurnout == pRoute->pPart[index].type);

	return (enum eTurnoutLine) routeGetPartArgumentByIndex(pRoute, index);
}

/**
 * Function to add a shuntroute to a train route
 * @param pRoute	pointer to the train route
 * @param pShuntroute	pointer to the shuntroute
 * @return		index, where the shuntroute has been inserted
 * @exception		Assertion on NULL pointer or too many route parts
 */
unsigned short routeAddShuntroute(struct sRoute * const pRoute, struct sShuntroute * const pShuntroute)
{
	assert(NULL != pRoute);
	assert(NRROUTEPARTS > pRoute->partsAddPosition);

	pRoute->pPart[pRoute->partsAddPosition].type = RoutePartShuntroute;
	pRoute->pPart[pRoute->partsAddPosition].ptr = (void *) pShuntroute;
	pRoute->pPart[pRoute->partsAddPosition].argument = 0;
	pRoute->partsAddPosition++;
	return pRoute->partsAddPosition - 1;
}

/**
 * Function to get the indexed pointer of a shuntroute as part of a train route
 * @param pRoute	pointer to the train route
 * @param nr		number of the train route part
 * @return		pointer of the shuntroute, if index is too big or no shuntroute on this place, NULL is returned
 * @exception		Assertion on NULL pointer
 */
struct sShuntroute * routeGetShuntroute(const struct sRoute * const pRoute, const unsigned short index)
{
	assert(NULL != pRoute);
	if(NRROUTEPARTS <= index)
	{
		return RoutePartNone;
	}

	if(RoutePartShuntroute != pRoute->pPart[index].type)
	{
		return NULL;
	}

	return (struct sShuntroute *) pRoute->pPart[index].ptr;
}

/**
 * Function to add a shuntroute which must be set before setting the train route
 * @param pRoute	pointer to the train route
 * @param pShuntroute	pointer to the condition shuntroute
 * @return		index, where the condition shuntroute has been inserted
 * @exception		Assertion on NULL pointer or too many route parts
 */
unsigned short routeAddCondShuntroute(struct sRoute * const pRoute, struct sShuntroute * const pShuntroute)
{
	assert(NULL != pRoute);
	assert(NRROUTEPARTS > pRoute->partsAddPosition);

	pRoute->pPart[pRoute->partsAddPosition].type	= RoutePartCondShuntroute;
	pRoute->pPart[pRoute->partsAddPosition].ptr	= (void *) pShuntroute;
	pRoute->partsAddPosition++;
	return pRoute->partsAddPosition - 1;
}


/**
 * Function to get the indexed pointer of a shuntroute which must be set before setting the train route
 * @param pRoute	pointer to the train route
 * @param nr		number of the train route part
 * @return		pointer of the condition shuntroute, if index is too big or no condition shuntroute on this place, NULL is returned
 * @exception		Assertion on NULL pointer
 */
struct sShuntroute * routeGetCondShuntroute(const struct sRoute * const pRoute, const unsigned short index)
{
	assert(NULL != pRoute);
	if(NRROUTEPARTS <= index)
	{
		return RoutePartNone;
	}

	if(RoutePartCondShuntroute != pRoute->pPart[index].type)
	{
		return NULL;
	}

	return (struct sShuntroute *) pRoute->pPart[index].ptr;
}

/**
 * Function to add an occupied section which must be set before setting the train route
 * @param pRoute		pointer to the train route
 * @param pShuntroute	pointer to the condition section
 * @return		index, where the condition section has been inserted
 * @exception		Assertion on NULL pointer or too many route parts
 */
unsigned short routeAddCondOccupied(struct sRoute * const pRoute, struct sSection * const pSection)
{
	assert(NULL != pRoute);
	assert(NRROUTEPARTS > pRoute->partsAddPosition);

	pRoute->pPart[pRoute->partsAddPosition].type	= RoutePartCondOccupied;
	pRoute->pPart[pRoute->partsAddPosition].ptr	= (void *) pSection;
	pRoute->partsAddPosition++;
	return pRoute->partsAddPosition - 1;
}

/**
 * Function to get the indexed pointer of a section which must be occupied before setting the train route
 * @param pRoute	pointer to the train route
 * @param nr		number of the train route part
 * @return		pointer of the condition section, if index is too big or no condition shuntroute on this place, NULL is returned
 * @exception		Assertion on NULL pointer
 */
struct sSection * routeGetCondOccupied(const struct sRoute * const pRoute, const unsigned short index)
{
	assert(NULL != pRoute);
	if(NRROUTEPARTS <= index)
	{
		return RoutePartNone;
	}

	if(RoutePartCondOccupied != pRoute->pPart[index].type)
	{
		return NULL;
	}

	return (struct sSection *) pRoute->pPart[index].ptr;
}

/**
 * Function to add an route that must NOT be set in order to set the train route
 *  Note that in this function, the name must be given since the pointer of routes not 
 *   defined yet is unknown.
 * @param pRoute	pointer to the train route
 * @param name		name of the illegal route
 * @return		index, where the illegal route has been inserted
 * @exception		Assertion on NULL pointer or too many route parts
 */
unsigned short routeAddIllegalRoute(struct sRoute * const pRoute, const char * const name)
{
	assert(NULL != pRoute);
	assert(NRROUTEPARTS > pRoute->partsAddPosition);

	pRoute->pPart[pRoute->partsAddPosition].type	= RoutePartIllegalRoute;
	memmove(pRoute->pPart[pRoute->partsAddPosition].name, name, NAMELEN);
	pRoute->partsAddPosition++;
	return pRoute->partsAddPosition - 1;
}

/**
 * Function to get the name of a route that must NOT be set in order to set the train route
 *  Note that in this function, the name must be given since the pointer of routes not 
 *   defined yet is unknown.
 * @param pRoute	pointer to the train route
 * @param name		name of the illegal route
 * @return		true, if index and part type were given correctly
 * @exception		Assertion on NULL pointer
 */
_Bool routeGetIllegalRouteName(const struct sRoute * const pRoute, /*@out@*/char * const name, const unsigned short index)
{
	assert(NULL != pRoute);
	if(NRROUTEPARTS <= index)
	{
		return false;
	}

	if(RoutePartIllegalRoute != pRoute->pPart[index].type)
	{
		return false;
	}

	memmove(name, pRoute->pPart[pRoute->partsAddPosition].name, NAMELEN);
	return true;
}

/**
 * Function to get the indexed pointer of a route that must NOT be set in order to set the train route
 * @param pRoute	pointer to the train route
 * @param nr		number of the train route part
 * @return		pointer of the condition section, if index is too big or no condition shuntroute on this place, NULL is returned
 * @exception		Assertion on NULL pointer
 */
struct sRoute * routeGetIllegalRoute(const struct sRoute * const pRoute, const unsigned short index)
{
	assert(NULL != pRoute);
	if(NRROUTEPARTS <= index)
	{
		return RoutePartNone;
	}

	if(RoutePartIllegalRoute != pRoute->pPart[index].type)
	{
		return NULL;
	}

	return (struct sRoute *) pRoute->pPart[index].ptr;
}

/**
 * Function to add the pointer of the illegal route after resolving it from the name
 * @param pRoute	pointer to the train route
 * @param pIllegalRoute	pointer to the illegal route
 * @param index		index, where the illegal route will be set
 * @exception		Assertion on NULL pointer or too many route parts
 */
void routeSetIllegalRoute(struct sRoute * const pRoute, struct sRoute * const pIllegalRoute, const unsigned short index)
{
	assert(NULL != pRoute);
	assert(NRROUTEPARTS > index);

	if(RoutePartIllegalRoute != pRoute->pPart[index].type)
	{
		return;
	}

	pRoute->pPart[index].ptr = pIllegalRoute;
}

/**
 * Function to add an route that MUST be set in order to set the train route
 *  Note that in this function, the name must be given since the pointer of routes not 
 *   defined yet is unknown.
 * @param pRoute	pointer to the train route
 * @param name		name of the illegal route
 * @return		index, where the illegal route has been inserted
 * @exception		Assertion on NULL pointer or too many route parts
 */
unsigned short routeAddCondRoute(struct sRoute * const pRoute, const char * const name)
{
	assert(NULL != pRoute);
	assert(NRROUTEPARTS > pRoute->partsAddPosition);

	pRoute->pPart[pRoute->partsAddPosition].type = RoutePartCondRoute;
	memmove(pRoute->pPart[pRoute->partsAddPosition].name, name, NAMELEN);
	pRoute->partsAddPosition++;
	return pRoute->partsAddPosition - 1;
}


/**
 * Function to get the name of a route that MUST be set in order to set the train route
 *  Note that in this function, the name must be given since the pointer of routes not 
 *   defined yet is unknown.
 * @param pRoute	pointer to the train route
 * @param name		name of the illegal route
 * @return		true, if index and part type were given correctly
 * @exception		Assertion on NULL pointer
 */
_Bool routeGetCondRouteName(const struct sRoute * const pRoute, /*@out@*/char * const name, const unsigned short index)
{
	assert(NULL != pRoute);
	if(NRROUTEPARTS <= index)
	{
		return false;
	}

	if(RoutePartCondRoute != pRoute->pPart[index].type)
	{
		return false;
	}

	memmove(name, pRoute->pPart[pRoute->partsAddPosition].name, NAMELEN);
	return true;
}

/**
 * Function to get the indexed pointer of a route that MUST be set in order to set the train route
 * @param pRoute	pointer to the train route
 * @param nr		number of the train route part
 * @return		pointer of the condition section, if index is too big or no condition shuntroute on this place, NULL is returned
 * @exception		Assertion on NULL pointer
 */
struct sRoute * routeGetCondRoute(const struct sRoute * const pRoute, const unsigned short index)
{
	assert(NULL != pRoute);
	if(NRROUTEPARTS <= index)
	{
		return RoutePartNone;
	}

	if(RoutePartCondRoute != pRoute->pPart[index].type)
	{
		return NULL;
	}

	return (struct sRoute *) pRoute->pPart[index].ptr;
}

/**
 * Function to set the pointer of the illegal route after resolving it from the name
 * @param pRoute	pointer to the train route
 * @param pCondRoute	pointer to the illegal route
 * @param index		index, where the illegal route will be set
 * @exception		Assertion on NULL pointer or too many route parts
 */
void routeSetCondRoute(struct sRoute * const pRoute, struct sRoute * const pCondRoute, const unsigned short index)
{
	assert(NULL != pRoute);
	assert(NRROUTEPARTS > index);

	if(RoutePartCondRoute != pRoute->pPart[index].type)
	{
		return;
	}

	pRoute->pPart[index].ptr = pCondRoute;
}

/**
 * Function to add a line to a train route
 * @param pRoute	pointer to the train route
 * @param name		name of the block
 * @param ptr2		second pointer used for this line
 * @return		index, where the line has been inserted
 * @exception		Assertion on NULL pointer or too many line parts
 */
unsigned short routeAddLine(struct sRoute * const pRoute, const char  * const name, void * ptr2)
{
	assert(NULL != pRoute);
	assert(NRROUTEPARTS > pRoute->partsAddPosition);

	pRoute->pPart[pRoute->partsAddPosition].type = RoutePartLine;
	memmove(pRoute->pPart[pRoute->partsAddPosition].name, name, NAMELEN);
	pRoute->pPart[pRoute->partsAddPosition].ptr2	= ptr2;
	pRoute->partsAddPosition++;
	return pRoute->partsAddPosition - 1;
}

/**
 * Function to get the indexed pointer of a line as a part of a train route
 * @param pRoute	pointer to the train route
 * @param name		pointer to string filled with the name. Must be at least NAMELEN long 			
 * @param nr		number of the train route part
 * @return		true, if index and part type were given correctly
 * @exception		Assertion on NULL pointer
 */
_Bool routeGetLineName(const struct sRoute * const pRoute, /*@out@*/char * const name, const unsigned short index)
{
	assert(NULL != pRoute);
	if(NRROUTEPARTS <= index)
	{
		return RoutePartNone;
	}

	if(RoutePartLine != pRoute->pPart[index].type)
	{
		return NULL;
	}

	memmove(name, pRoute->pPart[index].name, NAMELEN);
	return true;
}

/**
 * Function to get the indexed pointer of a line as a part of a train route
 * @param pRoute	pointer to the train route
 * @param nr		number of the train route part
 * @return		pointer of the line, if index is too big or no line on this place, NULL is returned
 * @exception		Assertion on NULL pointer
 */
struct sLine * routeGetLine(const struct sRoute * const pRoute, const unsigned short index)
{
	assert(NULL != pRoute);
	if(NRROUTEPARTS <= index)
	{
		return RoutePartNone;
	}

	if(RoutePartTurnout != pRoute->pPart[index].type)
	{
		return NULL;
	}

	return (struct sLine *) pRoute->pPart[index].ptr;
}

#ifdef DOMINO55
	/**
	 * Function to get the second indexed pointer of a line as a part of a train route
	 *  this pointer will keep the pointer to the dwarf on DOMINO55 switchboards
	 * @param pRoute	pointer to the train route
	 * @param nr		number of the train route part
	 * @return		pointer to the 2nd pointer stored in this index.
	 * @exception		Assertion on NULL pointer
	 */
	void * routeGetLinePtr2(const struct sRoute * const pRoute, const unsigned short index)
	{
		assert(NULL != pRoute);
		if(NRROUTEPARTS <= index)
		{
			return NULL;
		}

		if(RoutePartTurnout != pRoute->pPart[index].type)
		{
			return NULL;
		}

	return (void *) pRoute->pPart[index].ptr2;
	}
#endif

/**
 * Function to set the pointer of the line after resolving it from the name
 * @param pRoute	pointer to the train route
 * @param pCondRoute	pointer to the illegal route
 * @param index		index, where the illegal route will be set
 * @exception		Assertion on NULL pointer or too many route parts
 */
void routeSetLine(struct sRoute * const pRoute, struct sLine * const pLine, const unsigned short index)
{
	assert(NULL != pRoute);
	assert(NRROUTEPARTS > index);

	pRoute->pPart[index].ptr = pLine;
}

/**
/**
 * Function to add a section to a train route. Note that the section is not written explicitely
 *  in some config file. It is calculated by the section on the end of a shuntroute that is
 *  part of the route
 * @param pRoute	pointer to the train route
 * @param pSection	pointer to the section
 * @return		index, where the section has been inserted
 * @exception		Assertion on NULL pointer or too many route parts
 */
unsigned short routeAddSection(struct sRoute * const pRoute, struct sSection * const pSection)
{
	assert(NULL != pRoute);
	assert(NRROUTEPARTS > pRoute->partsAddPosition);

	pRoute->pPart[pRoute->partsAddPosition].type	= RoutePartSection;
	pRoute->pPart[pRoute->partsAddPosition].ptr	= (void *) pSection;
	pRoute->pPart[pRoute->partsAddPosition].argument = 0;
	pRoute->partsAddPosition++;
	return pRoute->partsAddPosition - 1;
}

/**
 * Function to get the indexed pointer of a section passed by the train. Note that the section 
 *  is not written explicitely in some config file. It is calculated by the section on the 
 *  end of a shuntroute that is part of the route
 * @param pRoute	pointer to the train route
 * @param nr		number of the train route part
 * @return		pointer of the section, if index is too big or no condition shuntroute on this place, NULL is returned
 * @exception		Assertion on NULL pointer
 */
struct sSection * routeGetSection(const struct sRoute * const pRoute, const unsigned short index)
{
	assert(NULL != pRoute);
	if(NRROUTEPARTS <= index)
	{
		return RoutePartNone;
	}

	if(RoutePartSection != pRoute->pPart[index].type)
	{
		return NULL;
	}

	return (struct sSection *) pRoute->pPart[index].ptr;
}

/**
 * Function to get the indexed type of parts of a route
 * @param pRoute	pointer to the route
 * @param nr		number of the route part
 * @return		type of this part, if index is too big, RoutePartNone is returned
 * @exception		Assertion on NULL pointer
 */
enum eRoutePartType routeGetPartType(const struct sRoute * const pRoute, const unsigned short index)
{
	assert(NULL != pRoute);
	if(NRROUTEPARTS <= index)
		return RoutePartNone;

	return pRoute->pPart[index].type;
}

/**
 * Function to calculate the important elements the train is passing
 * @param pRoute	pointer to the route
 * @exception		assert on NULL pointer
 */
void routeCalcRoute(struct sRoute * const pRoute)
{
	short		i;

	assert(NULL != pRoute);

	pRoute->dissolveSection	= -1;

	for(i = 0; i < NRROUTEPARTS; i++)
	{
		if(RoutePartTurnout == pRoute->pPart[i].type)
		{
			pRoute->dissolveSection = i;
		}
	}
}

/**
 * Function to get the number of the route part dissolving 
 *  the route when leaving
 * @param pRoute	pointer to the route
 * @return		number of the route part
 * @exception		assert on NULL pointer
 */
unsigned short routeGetDissolveSection(const struct sRoute * const pRoute)
{
	assert(NULL != pRoute);

	return pRoute->dissolveSection;
}

/**
 * Function to get the state of a route
 * @param pRoute	pointer to the route
 * @return		state of the route
 * @exception		assert on NULL pointer
 */
enum eRouteState routeGetState(const struct sRoute * const pRoute)
{
	assert(NULL != pRoute);

	return pRoute->eState;
}

/**
 * Function to request a state for the route
 * @param pRoute	pointer to the route
 * @param state		state requested
 * @return		true if the request was made and could be fulfilled
 * @exception		assert on NULL pointer
 */
_Bool routeRequestState(struct sRoute * const pRoute, const enum eRouteState state)
{
	assert(NULL != pRoute);

	if(routeTransitionPossible(pRoute, state, true))
	{
		pRoute->eRequestedState = state;
		return true;
	}
	/* else */
	return false;
}

	
