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
 * Shunting routes (Rangierfahrstrassen)
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
#include "constants.h"


/*@only@*/ /*@null@*/ static struct sShuntroute * m_pShuntroute = NULL;	/**< Pointer to the first shuntroute	*/
static unsigned short m_nrShuntroute = 0;	/**< Number of shuntroutes	*/

/**
 * Function to get the indexed pointer of parts of a shuntroute
 * @param pShuntroute       pointer to the shuntroute
 * @param nr        number of the shuntroute part
 * @return          pointer of this part, if index is too big, NULL is returned
 * @exception       Assertion on NULL pointer
 */
static void * shuntrouteGetPart(const struct sShuntroute * const pShuntroute, const unsigned short i);

/**
 * Function to find the shuntroute by giving two dwarfs
 * @param pDwarf1	1st dwarf
 * @param pDwarf2	2nd dwarf
 * @param pShuntroute	pointer to the shuntroute to be asked
 * @return		true if pShuntroute has these dwarfs
 */
_Bool shuntrouteFindByDwarfs(const struct sDwarf const * pDwarf1, 
				const struct sDwarf const * pDwarf2,
				const struct sShuntroute const * pShuntroute);

/**
 * Constructor for the shuntroutes
 * @param nr		number of shuntroutes to be created
 * @exception		Calls exit if no RAM available
 */
void shuntroute(const unsigned short nr)
{
	unsigned short			i, j;
	struct sShuntroute *	pShuntroute;

	LOG_INF("started");
	
	m_nrShuntroute = nr;
	m_pShuntroute = malloc(m_nrShuntroute * sizeof(struct sShuntroute));
	if(NULL == m_pShuntroute) 
	{
		/* error allocating */
		LOG_ERR("cannot allocate RAM of shuntroutes");
		exit(EXIT_FAILURE);
	}
	pShuntroute = m_pShuntroute;
	for(i = 0; i < m_nrShuntroute; i++)
	{
		pShuntroute->name[0]	= '\0';
		pShuntroute->toRight	= false;
		pShuntroute->start	= NULL;
		pShuntroute->dest	= NULL;
		pShuntroute->eState	= ShuntrouteInit;
		pShuntroute->destButton = false;
		pShuntroute->twoButtons = false;
		pShuntroute->dissolve	= false;
		pShuntroute->toHalt	= false;
		pShuntroute->PartsAddPosition	= 0;
		pShuntroute->headPosition	= -2;
		pShuntroute->tailPosition	= -1;
		pShuntroute->dissolveConditions = false;
		pShuntroute->dissolveCondition1 = false;
		pShuntroute->dissolveCondition2 = false;
		pShuntroute->dissolveCondition3 = false;
		pShuntroute->eRequestedState	= ShuntrouteNone;

		for(j = 0; j < NRSHUNTPARTS; j++)
		{
			pShuntroute->pPart[j].type		= ShuntroutePartNone;
			pShuntroute->pPart[j].argument		= 0;
			pShuntroute->pPart[j].ptr		= NULL;
			pShuntroute->pPart[j].isOccupied	= false;
			pShuntroute->pPart[j].wasOccupied	= false;
		}
		pShuntroute->resetSection	= 0;
		pShuntroute->dissolveSection	= 0;
		pShuntroute->lastSection	= 0;
		
		pShuntroute++;
	}

	LOG_INF("ended");
}

/**
 * Destructor for the shuntroutes
 */
void shuntrouteDestroy(void) 
{
	LOG_INF("started");
	if(NULL == m_pShuntroute) 
	{
		LOG_INF("zero pointer, terminate");
		return;
	}
	LOG_INF("release RAM of shuntroutes");
	free(m_pShuntroute);
	m_pShuntroute = NULL;
	LOG_INF("ended");
}

/**
 * Init function for the shuntroutes
 *
 * Here, all shuntroutes are initialised 
 */
void shuntrouteInit(void) 
{
}

/**
 * Function to find the pointer of a shuntroute by giving the name
 * @param name		name of the shuntroute to be locked for
 * @return		pointer of the found shuntroute
 * @exception		a parser assert is made if not found
 */
struct sShuntroute * shuntrouteFind(const char * const name) 
{
        unsigned short		i = 0;
	struct sShuntroute *	pShuntroute = NULL;
        char			temp[128];

	i = 0;
	while(NULL != (pShuntroute = shuntrouteGet(i)))
	{ 
		if(0 == (strcmp(pShuntroute->name, name))) 
			return pShuntroute;
		i++;
        }
        sprintf(temp, "Rangierfahrstrasse %s nicht gefunden", name);
        parserExit(temp);
        return NULL;
}

/**
 * Function to get the pointer by giving the number of the shuntroute
 * @param i		index, starting with 0
 * @return		pointer to this shuntroute, NULL if not existing
 */ 
struct sShuntroute * shuntrouteGet(const unsigned short i)
{
	if(i >= m_nrShuntroute)
	{
		return NULL;
	}
	else
	{
		return m_pShuntroute + i;
	}
}

/**
 * Function to get the number of shuntroutes 
 * @return		Number of shuntroutes
 */
unsigned short shuntrouteGetNr(void)
{
	return m_nrShuntroute;
}

static _Bool shuntrouteUpdateOccupancy(struct sShuntroute * const pShuntroute);

/**
 * Function returning true if all elements passed by the train are free
 * @param pShuntroute	pointer to the shuntroute
 * @return		true if all elements passed are not occupied
 * @exception		assert on NULL pointer
 */
_Bool shuntrouteIsFree(struct sShuntroute * const pShuntroute)
{
	assert(NULL != pShuntroute);

	return shuntrouteUpdateOccupancy(pShuntroute);
}

/**
 * Helper function to update the registers containing the occupancy data
 * @param pShuntroute	pointer to the shuntroute to be updated
 * @return		true if all parts the train is passing are free
 * @exception		assert on NULL pointer
 */
static _Bool shuntrouteUpdateOccupancy(struct sShuntroute * const pShuntroute)
{
	unsigned short	j = 0;
	_Bool		free = true;	/**< flag assuming free	*/

	assert(NULL != pShuntroute);

	for(j = 0; j < NRSHUNTPARTS; j++)
	{
		switch(pShuntroute->pPart[j].type) 
		{
		case ShuntroutePartNone:
			break;

		case ShuntroutePartTurnout:
			pShuntroute->pPart[j].isOccupied = ! turnoutIsFree(pShuntroute->pPart[j].ptr);
			if(pShuntroute->pPart[j].isOccupied)
			{
				free = false;
			}
			break;

		case ShuntroutePartProtectiveTurnout:
			break;

		case ShuntroutePartSection:
			pShuntroute->pPart[j].isOccupied = ! sectionIsFree(pShuntroute->pPart[j].ptr);
			if(pShuntroute->pPart[j].isOccupied)
			{
				free = false;
			}
			break;

		case ShuntroutePartProtectiveSection:
			pShuntroute->pPart[j].isOccupied = ! sectionIsFree(pShuntroute->pPart[j].ptr);
			break;

		case ShuntroutePartProtectiveDwarf:
			break;

		default:
			printf("strange default with j=%d %d\n", j, pShuntroute->pPart[j].type);

			assert(0);
			break;
		}
	}
	return free;
}

/**
 * Helper function to find out if a change in the state is possible. 
 *  This function is separated from the state machine since it is
 *  used by the state machine and by the request function form the 
 *  train route.
 * 
 * @param pShuntroute	pointer to the shuntroute to be checked
 * @param requested	requested state
 * @param trainroute	to be set true if it is part of a trainroute
 *			 needed because sections must be free for trainroutes
 * @return		true if possible
 * @exception		assert on NULL pointer
 */
static _Bool shuntrouteTransitionPossible(const struct sShuntroute * const pShuntroute,
					const enum eShuntrouteState requested,
					const _Bool trainroute);
static _Bool shuntrouteTransitionPossible(const struct sShuntroute * const pShuntroute,
					const enum eShuntrouteState requested,
					const _Bool trainroute)
{
	enum eLine			eSectionLine = neutral;
	unsigned short			condition;		/**< calculated condition for this element	*/
	unsigned short			j;

	assert(NULL != pShuntroute);

	/* now prepare the masks for turnouts, protective turnouts and lines
	 * they depend on the requested state
	 * the turnout is supposed to be Minus state in the beginning
	 * the direction is supposed to be left to right
	 */

	switch(requested)
	{
	case ShuntrouteInit:
		return true;
		break;

	case ShuntrouteNone:
		return true;
		break;

	case ShuntrouteDestButton:
		return true;
		break;

	case ShuntrouteRegistered:
		return true;
		break;

	case ShuntrouteReserve:
		eSectionLine = LINERESERVE;
		break;

	case ShuntrouteSwitch:
		eSectionLine = LINESWITCH;
		break;

	case ShuntrouteEstablished:
		eSectionLine = LINEBLOCK;
		break;

	case ShuntrouteBlocked:
		eSectionLine = LINEINTERLOCK;
		break;

	default:
		break;
	}

	for(j = 0; j < NRSHUNTPARTS; j++)
	{
		/* read the condition of this element
		 * the condition must only contain the direction LR / RL 
		 * and the Plus/Minus of the turnout, if any
		 * the other conditions are made by eSectionLine
		 */
		condition = TURNOUTDIRMASK & pShuntroute->pPart[j].argument;
		condition |= eSectionLine;

		/* now handle each element depending on its type	*/
		switch(pShuntroute->pPart[j].type) 
		{
		case ShuntroutePartNone:
			break;

		case ShuntroutePartTurnout:
			if(!turnoutLine(pShuntroute->pPart[j].ptr, condition))
			{
				return false;
			}

			if(trainroute)
			{
				/* now check occupancy for trainroutes	*/
				if(!turnoutIsFree(pShuntroute->pPart[j].ptr))
				{
					return false;
				}
			}
			break;

		case ShuntroutePartProtectiveTurnout:
			if(!turnoutProtectiveLine(pShuntroute->pPart[j].ptr, condition))
			{
				return false;
			}
			break;

		case ShuntroutePartSection:
			if(!sectionLine(pShuntroute->pPart[j].ptr, condition))
			{
				return false;
			}
			if(trainroute)
			{
				/* checking occupancy for trainroute	*/
				if(!sectionIsFree(pShuntroute->pPart[j].ptr))
				{
					return false;
				}
			}
			break;

		case ShuntroutePartProtectiveSection:
			if(!sectionIsFree(pShuntroute->pPart[j].ptr))
			{
				return false;
			}
			break;

		case ShuntroutePartProtectiveDwarf:
			/* TODO	*/
			break;

		default:
			printf("strange default with j=%d %d\n", j, pShuntroute->pPart[j].type);

			assert(0);
			break;
		}
	}
	return true;

}

/**
 * Helper Function to find the head of the train movement
 * @param pShuntroute	pointer to the shuntroute to be observed
 * @return		head of the train, -2 if not found
 * @exception		assert on NULL Pointer
 */
static short shuntrouteTrainHead(const struct sShuntroute * const pShuntroute);
static short shuntrouteTrainHead(const struct sShuntroute * const pShuntroute)
{
	short	tmpHead		= -2;	/**< must be set to -2 so a free element 0 does not fulfill condition for head	*/
	short	lastElement	= -2;	/**< last real element passed by the train. Used to calculate situations
					 *    where every real element is occupied	
					 */
	short	i;
	_Bool	occupied = false;

	assert(NULL != pShuntroute);

	/* first find out where is the head of the train	*/

	for(i = 0; i < NRSHUNTPARTS; i++)
	{
		switch(pShuntroute->pPart[i].type)
		{
		case ShuntroutePartTurnout:
		case ShuntroutePartSection:
			occupied = pShuntroute->pPart[i].isOccupied;
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
 * @param pShuntroute	pointer to the shuntroute to be observed
 * @param tmpHead	temporary head of train. MUST really be the head.
 * @return		tail of the train
 * @exception		assert on NULL Pointer
 */
static short shuntrouteTrainTail(const struct sShuntroute * const pShuntroute, short tmpHead);
static short shuntrouteTrainTail(const struct sShuntroute * const pShuntroute, short tmpHead)
{
	short 	tmpTail = tmpHead;
	short	i;
	_Bool	occupied = true;

	assert(NULL != pShuntroute);

	if(-2 == tmpHead)
	{
		return -2;
	}

	for(i = tmpHead - 1; i >= 0; i--)
	{
		switch(pShuntroute->pPart[i].type)
		{
		case ShuntroutePartTurnout:
		case ShuntroutePartSection:
			occupied = pShuntroute->pPart[i].isOccupied;
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
 * provides the function when being inside the state given in pShuntroute->eState
 * @param pShuntroute	pointer to the shuntroute leaving the state
 */
static enum eShuntrouteState shuntrouteStateBehaviour(struct sShuntroute * const pShuntroute);
static enum eShuntrouteState shuntrouteStateBehaviour(struct sShuntroute * const pShuntroute)
{
	short	tmpHead		= -2;
	short	tmpTail		= -1;

	switch(pShuntroute->eState)
	{
	case ShuntrouteInit: /* just go to none      */
		return ShuntrouteNone; 
		break;

	case ShuntrouteNone:
		/* init values needed later	*/
		pShuntroute->dissolve = false;

		if(ShuntrouteNone != pShuntroute->eRequestedState)
		{
			/* now the train route want to set the shuntroute	*/
			return pShuntroute->eRequestedState;
		}
		/* else */

		/* if two buttons are pushed, shuntroute shall be registered	*/
		if(pShuntroute->twoButtons)
		{
			shuntrouteUpdateOccupancy(pShuntroute);
			return ShuntrouteRegistered;
		}

		/* keep state	*/
		break;

	case ShuntrouteDestButton:
		/*
		 * dest button must be handled here to prevent dissolving a single
		 *  shuntroute that is part of a train route
		 */
		switch(pShuntroute->eRequestedState)
		{
		case ShuntrouteInit:
			break;

		case ShuntrouteNone:
			break;

		case ShuntrouteDestButton:
			break;

		case ShuntrouteRegistered:
			return ShuntrouteRegistered;
			break;

		case ShuntrouteReserve:
			return ShuntrouteReserve;
			break;

		case ShuntrouteSwitch:
			return ShuntrouteSwitch;
			break;

		case ShuntrouteEstablished:
			return ShuntrouteEstablished;
			break;

		default:
			break;
		}

		/*
		 * check if the destination button still is pushed. If so, nothing to do
		 *  so far, otherwise it must be checked in which case the destination
		 *  was released
		 */
		shuntrouteUpdateOccupancy(pShuntroute);

		if((!pShuntroute->destButton) && (!pShuntroute->dissolve))
		{
			/* normal release of both buttons, destination button last	*/
			return ShuntrouteReserve;
		}

		if((!pShuntroute->destButton) && (pShuntroute->dissolve))
		{
			/* dissolve was pushed and the destination button released	*/
			return ShuntrouteNone;
		}

		/* now the cases with the destination button or both buttons pushed	*/
		if(pShuntroute->twoButtons)
		{
			/* two buttons pushed	*/
			pShuntroute->twoButtons = false;
			return ShuntrouteRegistered;
		}
		/* else */
		/* only destination button pushed, keep state	*/
		break;

	case ShuntrouteRegistered:
		shuntrouteUpdateOccupancy(pShuntroute);
		/*
		 * shuntroute registered must be handled here if part of a train route
		 */
		switch(pShuntroute->eRequestedState)
		{
		case ShuntrouteInit:
			break;

		case ShuntrouteNone:
			break;

		case ShuntrouteDestButton:
			break;

		case ShuntrouteRegistered:
			break;

		case ShuntrouteReserve:
			return ShuntrouteReserve;
			break;

		case ShuntrouteSwitch:
			return ShuntrouteSwitch;
			break;
	
		case ShuntrouteEstablished:
			return ShuntrouteEstablished;
			break;

		default:
			break;
		}

		/* now the cases by single setting of the shuntroute	*/
		if(pShuntroute->destButton)
		{
			/* destination button pushed	*/
			pShuntroute->destButton = false;
			return ShuntrouteDestButton;	
		}
		/* else */
		if(! pShuntroute->twoButtons)
		{
			/* no more button pushed	*/
			return ShuntrouteReserve;
		}
		break;

	case ShuntrouteReserve:
		/* a shuntroute shall be reserved to be switched and established later
		 * so set all lines to reserve and if this succeeds, then switch them.
		 */
		shuntrouteUpdateOccupancy(pShuntroute);
		switch(pShuntroute->eRequestedState)
		{
		case ShuntrouteInit:
			break;

		case ShuntrouteNone:
			break;

		case ShuntrouteDestButton:
			break;

		case ShuntrouteRegistered:
			break;

		case ShuntrouteReserve:
			break;

		case ShuntrouteSwitch:
			return ShuntrouteSwitch;
			break;
	
		case ShuntrouteEstablished:
			return ShuntrouteEstablished;
			break;

		default:
			break;
		}

		/* check if a higher state is possible	*/
		if(shuntrouteTransitionPossible(pShuntroute, ShuntrouteSwitch, false))
		{
			return ShuntrouteSwitch;
		}
		
		/* check if state reserve can be kept	*/
		if(!shuntrouteTransitionPossible(pShuntroute, ShuntrouteReserve, false))
		{
			/* even reserve is not possible	*/
			return ShuntrouteNone;
		}
		/* possible, keep state	*/
		break;
	
	case ShuntrouteSwitch:
		/* a shuntroute shall be switched to be established later
		 * so set all lines to switch and if this succeeds, then establish them.
		 */
		shuntrouteUpdateOccupancy(pShuntroute);

		switch(pShuntroute->eRequestedState)
		{
		case ShuntrouteInit:
			break;

		case ShuntrouteNone:
			break;

		case ShuntrouteDestButton:
			break;

		case ShuntrouteRegistered:
			break;

		case ShuntrouteReserve:
			break;

		case ShuntrouteSwitch:
			return ShuntrouteSwitch;
			break;
	
		case ShuntrouteEstablished:
			return ShuntrouteEstablished;
			break;

		default:
			break;
		}

		if(shuntrouteTransitionPossible(pShuntroute, ShuntrouteEstablished, false))
		{
			return ShuntrouteEstablished;
		}
		break;

	case ShuntrouteEstablished:
	case ShuntrouteBlocked:
		/* now a shuntroute is established. 
		 * continue checking for things happening 
		 * which make the shuntroute dissolve.
		 */

		shuntrouteUpdateOccupancy(pShuntroute);

		/* 
		 * check for the cases for the shuntroute as a part of a train route
		 *  this is seen by requested state == blocked
		 */
		if(ShuntrouteBlocked == pShuntroute->eRequestedState)
		{
			pShuntroute->eState = ShuntrouteBlocked;

			/* check if this shuntroute can still be kept. In case of error, change
			 * to the appropriate error state
			 */
			if(!shuntrouteTransitionPossible(pShuntroute, pShuntroute->eState, true))
			{
				return ShuntrouteRouteError;
			}
		}
		else
		{
			if(!shuntrouteTransitionPossible(pShuntroute, pShuntroute->eState, false))
			{
				/* now a reason keeping this shuntroute alive disappeared. Set the 
				 *  dwarf to halt and remark the error
				 */
				return ShuntrouteError;
			}
		}

		if((pShuntroute->dissolve) && (ShuntrouteNone == pShuntroute->eRequestedState))
		{
			/* now the shuntroute shall be dissolved
			 * this is only possible if the dwarf is not clear
			 */
			#ifdef SPDR60
				if(SIG_DW_SH1 != dwarfGetAspect(pShuntroute->start))
				{ 
					return ShuntrouteNone;
				}
			#endif

			#ifdef DOMINO55
				if(SIG_DW_HALT == dwarfGetAspect(pShuntroute->start))
				{
					return ShuntrouteNone;
				}
				if(SIG_DW_DARK == dwarfGetAspect(pShuntroute->start))
				{
					return ShuntrouteNone;
				}
			#endif
		}

		/*--------------------------------------------------------------*/
		/* HALT calculation of the dwarf                                */
		/* now set the dwarf to HALT if the first turnout is left	*/
		/*--------------------------------------------------------------*/
			
		if((! pShuntroute->pPart[pShuntroute->resetSection].isOccupied) && 
		(pShuntroute->pPart[pShuntroute->resetSection].wasOccupied))
		{
			/* now the turnout was left. Switch the dwarf to HALT	*/
			pShuntroute->toHalt = true;
			#ifdef SPDR60
				/* set the dwarf to Sh0	*/
				if(pShuntroute->toHalt)
				{
					dwarfSetAspect(pShuntroute->start, SIG_DW_SH0);
				}
			#endif

			#ifdef DOMINO55
				/* now set the start dwarfs correct aspect	*/
				if(pShuntroute->toHalt)
				{
					dwarfSetAspect(pShuntroute->start, SIG_DW_HALT);
				}
				else
				{
					dwarfSetAspectDest(pShuntroute->start, SIG_DW_CLEAR, pShuntroute->dest);
				}
			#endif
		}


		/*-------------------------------------------------------------*/
		/* Calculation of the dissolve by a train passing              */
		/*-------------------------------------------------------------*/
		/* the check for dissolve is done separately to prevent checking things like 
		 * protective switches, protective sections or protective dwarfs.
		 */

		/* first find out where is the head and the tail of the train is	*/
		tmpHead = shuntrouteTrainHead(pShuntroute);
		tmpTail = shuntrouteTrainTail(pShuntroute, tmpHead);

		/* Now calculate if one of the rules were broken by the new
		 *  values of head and tail
		 *-------------------------------------------------------------*/

		if(tmpHead < pShuntroute->headPosition)
		{
			/* now the head returned somehow. This is only allowed on the last section
		 	 * This speciality arises since the last section can be left without breaking condition 1
			 *
			 * the tmpHead can also get smaller on a separation of the train.
			 */
			if(pShuntroute->headPosition < pShuntroute->dissolveSection)
			{
				/* not allowed movement	*/
				pShuntroute->dissolveConditions	= false;
			}
		}

		if((tmpTail > pShuntroute->headPosition) && (0 <= pShuntroute->headPosition))
		{
			/* now a section got occupied without the last section still occupied (jumping)
			 * this is not allowed. Filtering out the cases where the headPosition is not
			 * really a postition inside the elements
			 */
			pShuntroute->dissolveCondition2 = false;
		}

		/* now calculate if the dissolve element was left. This is the case
		 *  case 1: the tmpTail is bigger than the dissolve Section
		 */
		if(tmpTail > pShuntroute->dissolveSection)
		{
			pShuntroute->dissolveCondition3 = true;
		}

		/* .. case 2: the tail came back from dissolveSection to a smaller Value	
		 */
		if((tmpHead < pShuntroute->dissolveSection) && (pShuntroute->headPosition >= pShuntroute->dissolveSection))
		{
			pShuntroute->dissolveCondition3 = true;
		}

		/* check if the shuntroute is dissolved by passing it correctly	*/
		if(pShuntroute->dissolveConditions
		&& pShuntroute->dissolveCondition1 
		&& pShuntroute->dissolveCondition2 
		&& pShuntroute->dissolveCondition3
		&& pShuntroute->toHalt)
		{
			if(ShuntrouteNone == pShuntroute->eRequestedState)
			{
				/* there is no request from a route	*/
				return ShuntrouteNone;
			}
			/* else */
			return ShuntrouteTurntoutsPassed;
		}
		/* else train is still driving inside route	*/
		pShuntroute->headPosition	= tmpHead;
		pShuntroute->tailPosition	= tmpTail;
		
		break;

	case ShuntrouteTurntoutsPassed:
		/* wait here until the route releases shuntroute	*/
		if(ShuntrouteNone == pShuntroute->eRequestedState)
		{
			return ShuntrouteNone;
		}
		break;

	case ShuntrouteError:
		shuntrouteUpdateOccupancy(pShuntroute);

		shuntrouteTransitionPossible(pShuntroute, pShuntroute->eState, false);

		if(pShuntroute->dissolve)
		{
			return ShuntrouteNone;
		}
		break;

	case ShuntrouteRouteError:
		shuntrouteUpdateOccupancy(pShuntroute);

		shuntrouteTransitionPossible(pShuntroute, pShuntroute->eState, false);

		if(ShuntrouteNone == pShuntroute->eRequestedState)
		/* here the error is released by setting the request to None	*/
		{
			return ShuntrouteNone;
		}
		break;

	default:
		break;
	}

	pShuntroute->destButton	= false;
	pShuntroute->twoButtons = false;
	pShuntroute->dissolve	= false;

	return pShuntroute->eState;
}

/**
 * state machine entry function
 * provides the function when entering the state given in pShuntroute->eState
 * @param pShuntroute		pointer to the shuntroute leaving the state
 */
static void shuntrouteStateEntry(struct sShuntroute * const pShuntroute);
static void shuntrouteStateEntry(struct sShuntroute * const pShuntroute)
{
	switch(pShuntroute->eState)
	{
        case ShuntrouteInit:
		break;
	
	case ShuntrouteNone:
		pShuntroute->dissolve = false;
		pShuntroute->destButton = false;
		pShuntroute->toHalt = false;
		#ifdef SPDR60
			dwarfSetAspect(pShuntroute->start, SIG_DW_SH0);
		#endif

		#ifdef DOMINO55
			dwarfSetAspect(pShuntroute->start, SIG_DW_HALT);
		#endif
		
		LOG_STW("Rangierfahrstrasse %s ist passiv", pShuntroute->name);
		break;
	
	case ShuntrouteDestButton:
		LOG_STW("Rangierfahrstrasse %s Zieltaste ist gedrueckt", pShuntroute->name);
		break; 
	
	case ShuntrouteRegistered:
		LOG_STW("Rangierfahrstrasse %s ist registriert", pShuntroute->name);
		break;
	
	case ShuntrouteReserve:
		LOG_STW("Rangierfahrstrasse %s ist reserviert", pShuntroute->name);
		break;

	case ShuntrouteSwitch:
		LOG_STW("Rangierfahrstrasse %s schaltet", pShuntroute->name);
		break;

	case ShuntrouteEstablished:
	case ShuntrouteBlocked:
		/* initializing the state. For the dissolve conditions I suppose that
		 * everything goes well for condition 1 and 2, it is set to false if
		 * something goes wrong.
		 * condition3 must be proven first
		 */
		pShuntroute->headPosition = -2;
		pShuntroute->dissolveConditions = true;
		pShuntroute->dissolveCondition1 = true;
		pShuntroute->dissolveCondition2 = true;
		pShuntroute->dissolveCondition3 = false;

		#ifdef SPDR60
			dwarfSetAspect(pShuntroute->start, SIG_DW_SH1);
		#endif

		#ifdef DOMINO55
			dwarfSetAspectDest(pShuntroute->start, SIG_DW_CLEAR, pShuntroute->dest);
		#endif

		if(ShuntrouteEstablished == pShuntroute->eState)
		{
			LOG_STW("Rangierfahrstrasse %s ist eingefahren", pShuntroute->name);
		}
		else
		{
			LOG_STW("Rangierfahrstrasse %s ist verriegelt", pShuntroute->name);
		}
		break;

	case ShuntrouteTurntoutsPassed:
		LOG_STW("Rangierfahrstrasse %s: Weichen sind passiert", pShuntroute->name);
		break;
	
	case ShuntrouteError:
		pShuntroute->dissolve = false;
		pShuntroute->destButton = false;
		#ifdef SPDR60
			dwarfSetAspect(pShuntroute->start, SIG_DW_SH0);
		#endif

		#ifdef DOMINO55
			dwarfSetAspect(pShuntroute->start, SIG_DW_HALT);
		#endif

		LOG_STW("Rangierfahrstrasse %s hat einen Fehler", pShuntroute->name);
		break;
	
	case ShuntrouteRouteError:
		pShuntroute->dissolve = false;
		pShuntroute->destButton = false;
		#ifdef SPDR60
			dwarfSetAspect(pShuntroute->start, SIG_DW_SH0);
		#endif

		#ifdef DOMINO55
			dwarfSetAspect(pShuntroute->start, SIG_DW_HALT);
		#endif
		
		LOG_STW("Rangierfahrstrasse %s hat einen Fehler", pShuntroute->name);
		break;
	
	default: break;
	}
}

/**
 * state machine exit function
 * provides the function when leaving the state given in pShuntroute->eState
 * @param pShuntroute		pointer to the main signal leaving the state
 */
static void shuntrouteStateExit(struct sShuntroute * const pShuntroute);
static void shuntrouteStateExit(struct sShuntroute * const pShuntroute)
{
	return;	/* temporary switched off this comments	*/
	switch(pShuntroute->eState)
	{
        case ShuntrouteInit:
		LOG_STW("Rangierfahrstrasse %s ist nicht mehr init", pShuntroute->name);
		break;

	case ShuntrouteNone:
		LOG_STW("Rangierfahrstrasse %s ist init", pShuntroute->name);
		break;

	case ShuntrouteDestButton:
		LOG_STW("Rangierfahrstrasse %s Zieltaste ist nicht mehr gedrueckt", pShuntroute->name);
		break; 
	
	case ShuntrouteRegistered:
		LOG_STW("Rangierfahrstrasse %s ist nicht mehr registriert", pShuntroute->name);
		break;
	
	case ShuntrouteReserve:
		LOG_STW("Rangierfahrstrasse %s ist nicht mehr reserviert", pShuntroute->name);
		break;
	
	case ShuntrouteSwitch:
		LOG_STW("Rangierfahrstrasse %s ist nicht mehr geschaltet", pShuntroute->name);
		break;
	
	case ShuntrouteEstablished:
	case ShuntrouteBlocked:
		LOG_STW("Rangierfahrstrasse %s ist nicht mehr eingefahren", pShuntroute->name);
		break;
	
		if(ShuntrouteEstablished == pShuntroute->eState)
		{
			LOG_STW("Rangierfahrstrasse %s ist nicht mehr eingefahren", pShuntroute->name);
		}
		else
		{
			LOG_STW("Rangierfahrstrasse %s ist nicht mehr verriegelt", pShuntroute->name);
		}
		break;

	case ShuntrouteTurntoutsPassed:
		LOG_STW("Rangierfahrstrasse %s: Weichen sind nicht mehr passiert", pShuntroute->name);
		break;
	
	case ShuntrouteError:
		LOG_STW("Rangierfahrstrasse %s hat keinen Fehler mehr", pShuntroute->name);
		break;

	case ShuntrouteRouteError:
		LOG_STW("Rangierfahrstrasse %s hat keinen Fehler mehr", pShuntroute->name);
		break;

	default: break;
	}
}

/**
 * Function to let all shuntroutes switch to the desired state
 */
void shuntrouteProcess(void)
{
	struct sShuntroute *	pShuntroute = m_pShuntroute;
	int			i, j;
	enum eShuntrouteState	resultState;

	assert(NULL != pShuntroute);

	for(i = 0; i < m_nrShuntroute; i++)
	{
		/**
		 * now the lines of the sections, dwarfs, mains and turnouts are activated. Since
		 *  these signals are static nature, they always must be applied
		 * you can compare these signals more to the voltage applied to a relais than 
		 *  a signal like in signal-slot combination of Qt
		 */
	
		resultState = shuntrouteStateBehaviour(pShuntroute);

		/* now set the historic values correctly	*/
		for(j = 0; j < NRSHUNTPARTS; j++)
		{
			pShuntroute->pPart[j].wasOccupied = pShuntroute->pPart[j].isOccupied;
		}

		if(pShuntroute->eState != resultState)
		{
			shuntrouteStateExit(pShuntroute);
			pShuntroute->eState = resultState;
			shuntrouteStateEntry(pShuntroute);
		}
		pShuntroute++;		
	}	
}

#ifdef DOMINO55
	/**
	 * Function for cancelling a Shuntroute either before it is established or
	 * if it is not part of a route
	 * 
	 * @param	pDwarf pointer to the destination dwarf of the shuntroute 
	 *		to be canceled
	 */
	void shuntrouteCancelDest(const struct sDwarf * const pDest)
	{
		unsigned short		i;
		struct sShuntroute *	pShuntroute;

		LOG_INF("started");

		pShuntroute = m_pShuntroute;
		for(i = 0; i < m_nrShuntroute; i++)
		{
			if(pDest == pShuntroute->dest)
			{
				pShuntroute->dissolve = true;
			}
			pShuntroute++;
		}
		LOG_INF("ended");
	}
#endif

#ifdef DOMINO55
	/**
	 * Function called when only one dwarf button is pushed. This is important
	 *  for storing the shuntroute. Remember: as soon as the destination is not
	 *  pushed any more, the shuntroute is stored
	 *
	 * on SpDr60, this function is not implemented.
	 *
	 * @param	*pDwarf	pointer to the button of the dwarf which button is pushed
	 * @exception	exception on NULL pointer
	 */
	void shuntrouteOneButton(const struct sDwarf const * pDwarf)
	{
		unsigned short		i;
		struct sShuntroute *	pShuntroute = m_pShuntroute;

		assert(NULL != pShuntroute);
		for(i = 0; i < m_nrShuntroute; i++)
		{
			if(pDwarf == pShuntroute->dest)
			{
				pShuntroute->destButton = true;
			}
			pShuntroute++;
		}
	}	
#endif


#ifdef SPDR60
	/**
	 * Function called when two dwarf buttons are pushed and the FRT timer is active
	 * @param pDwarf1	pointer to the button of the dwarf which button is pushed
	 * @param pDwarf2	pointer to the second dwarf which button is pushed
	 */
	void shuntrouteCancel(const struct sDwarf const * pDwarf1, 
				const struct sDwarf const * pDwarf2)
	{
		struct sShuntroute *	pShuntroute = m_pShuntroute;
		unsigned short		i;

		for(i = 0; i < m_nrShuntroute; i++)
		{
			if(shuntrouteFindByDwarfs(pDwarf1, pDwarf2, pShuntroute))
			{
				/* found connected shuntroute. Note: no return here
				 *  because more than one shuntroute with this
				 *  start/destination combination could exist
				 */
				if(SIG_DW_SH1 != dwarfGetAspect(pShuntroute->start))
				{
					pShuntroute->dissolve = true;
				}
			}
			pShuntroute++;
		}	 
	}
#endif

/**
 * Function called when two dwarf buttons are pushed. 
 * @param pDwarf1	pointer to the button of the dwarf which button is pushed
 * @param pDwarf2	pointer to the second dwarf which button is pushed
 */
void shuntrouteTwoButtons(const struct sDwarf const * pDwarf1, const struct sDwarf const * pDwarf2)
{
	struct sShuntroute *	pShuntroute = m_pShuntroute;
	unsigned short		i;

	for(i = 0; i < m_nrShuntroute; i++)
	{
		if(shuntrouteFindByDwarfs(pDwarf1, pDwarf2, pShuntroute))
		{
			pShuntroute->twoButtons = true;
			return;
		}
		pShuntroute++;
	}
}

_Bool shuntrouteFindByDwarfs(const struct sDwarf const * pDwarf1, 
				const struct sDwarf const * pDwarf2,
				const struct sShuntroute const * pShuntroute)
{
	if(NULL == pShuntroute)
	{
		return false;
	}

	if(pDwarf1 == pDwarf2)
	{
		/* not really two buttons.	*/
		return false;
	}

	if(((pDwarf1 == pShuntroute->start) && (pDwarf2 == pShuntroute->dest))
	|| ((pDwarf1 == pShuntroute->dest)  && (pDwarf2 == pShuntroute->start)))
	{
		/* correct combination found	*/
		return true;
	}
	/* else	*/
	return false;
}

/**
 * Function to set the name of a shuntroute
 * @param pShuntroute	pointer to the shuntroute
 * @param name		pointer to the string with the name 
 * @exception		assert on NULL pointer
 */ 
void shuntrouteSetName(struct sShuntroute * const pShuntroute, const char * const name)
{
	assert(NULL != pShuntroute);
	
	memmove(pShuntroute->name, name, NAMELEN);
}

/**
 * Function to get the name of a shuntroute
 * @param pShuntroute	pointer to the shuntroute
 * @param name		pointer to the string filled out with the name
 * 			of the turnout. Must be at least NAMELEN long
 * @return		true, if successful
 * @exception		assert on NULL pointer
 */ 
_Bool shuntrouteGetName(const struct sShuntroute * const pShuntroute, /*@out@*/ char * const name)
{
	assert(NULL != pShuntroute);

	name[NAMELEN - 1] = '\0';
	memmove(name, pShuntroute->name, NAMELEN - 1);
	return true;
}

/**
 * Function to set the direction to the right
 * @param pShuntroute	pointer to the shuntroute
 * @param toright	true if shunt route goes from left to right
 * @exception		assert on NULL pointer
 */
void shuntrouteSetToRight(struct sShuntroute * const pShuntroute, const _Bool toright)
{
	assert(NULL != pShuntroute);

	pShuntroute->toRight = toright;
}

/**
 * Function to get the direction of the shuntroute
 * @param pShuntroute	pointer to the shuntroute
 * @return		true if shunt route goes from left to right
 * @exception		assert on NULL pointer
 */
_Bool shuntrouteGetToRight(const struct sShuntroute * const pShuntroute)
{
	assert(NULL != pShuntroute);

	return pShuntroute->toRight;
}

/**
 * Function to set the start dwarf
 * @param pShuntroute	pointer to the shuntroute
 * @param start		pointer to the start dwarf
 * @exception		assert on NULL pointer
 */
void shuntrouteSetStart(struct sShuntroute * const pShuntroute, struct sDwarf * start)
{
	assert(NULL != pShuntroute);

	pShuntroute->start = start;
}

/**
 * Function to get the start dwarf
 * @param pShuntroute	pointer to the shuntroute
 * @return		pointer to the start dwarf
 * @exception		assert on NULL pointer
 */
struct sDwarf * shuntrouteGetStart(const struct sShuntroute * pShuntroute)
{
	assert(NULL != pShuntroute);

	return pShuntroute->start;
}

/**
 * Function to set the destination dwarf
 * @param pShuntroute	pointer to the shuntroute
 * @param dest		pointer to the destination dwarf
 * @exception		assert on NULL pointer
 */
void shuntrouteSetDest(struct sShuntroute * const pShuntroute, struct sDwarf * dest)
{
	assert(NULL != pShuntroute);

	pShuntroute->dest = dest;
}

/**
 * Function to get the destination dwarf
 * @param pShuntroute	pointer to the shuntroute
 * @return		pointer to the destination dwarf
 * @exception		assert on NULL pointer
 */
struct sDwarf * shuntrouteGetDest(const struct sShuntroute * pShuntroute)
{
	assert(NULL != pShuntroute);

	return pShuntroute->dest;
}

/**
 * Function to get the number of parts of the shuntroute
 * @param pShuntroute	pointer to the shuntroute
 * @return		number of parts the shuntroute consists of
 * @exception		assert on NULL pointer
 */
unsigned short shuntrouteGetParts(const struct sShuntroute * const pShuntroute)
{
	assert(NULL != pShuntroute);

	return pShuntroute->PartsAddPosition;
}

static void * shuntrouteGetPart(const struct sShuntroute * const pShuntroute, const unsigned short i)
{
	assert(NULL != pShuntroute);
	if(NRSHUNTPARTS <= i)
		return NULL;

	return pShuntroute->pPart[i].ptr;
}

/**
 * Function to get the indexed pointer of parts of a shuntroute
 * @param pShuntroute	pointer to the shuntroute
 * @param nr	        number of the shuntroute part
 * @return		the argument of this part
 * @exception		Assertion on NULL pointer or if index is too big
 */
unsigned short shuntrouteGetPartArgument(const struct sShuntroute * const pShuntroute, const unsigned short i);
unsigned short shuntrouteGetPartArgument(const struct sShuntroute * const pShuntroute, const unsigned short i)
{
	assert(NULL != pShuntroute);
	assert(NRSHUNTPARTS > i);

	return pShuntroute->pPart[i].argument;
}

/**
 * Function to add a turnout to a shuntroute
 * @param pShuntroute	pointer to the shuntroute
 * @param line		line the turnout has to be set to
 * @return		index, where the turnout has been inserted
 * @exception		Assertion on NULL pointer or too many turnout parts
 */
unsigned short shuntrouteAddTurnout(struct sShuntroute * const pShuntroute, struct sTurnout * const pTurnout, const enum eTurnoutLine line)
{
	assert(NULL != pShuntroute);
	assert(NRSHUNTPARTS > pShuntroute->PartsAddPosition);

	pShuntroute->pPart[pShuntroute->PartsAddPosition].type = ShuntroutePartTurnout;
	pShuntroute->pPart[pShuntroute->PartsAddPosition].ptr = (void *) pTurnout;
	pShuntroute->pPart[pShuntroute->PartsAddPosition].argument = line;

	pShuntroute->PartsAddPosition++;
	return pShuntroute->PartsAddPosition - 1;
}

/**
 * Function to get the indexed pointer of a turnout as a part of a shuntroute
 * @param pShuntroute	pointer to the shuntroute
 * @param nr		number of the shuntroute part
 * @return		pointer of the turnout, if index is too big or no turnout on this place, NULL is returned
 * @exception		Assertion on NULL pointer
 */
struct sTurnout * shuntrouteGetTurnout(const struct sShuntroute * const pShuntroute, const unsigned short i)
{
	enum eShuntroutePartType	type;

	type = shuntrouteGetPartType(pShuntroute, i);
	if(ShuntroutePartTurnout != type)
		return NULL;

	return (struct sTurnout *) shuntrouteGetPart(pShuntroute, i);
}
	
/**
 * Function to get the parameter of a turnout as a part of a shuntroute
 * @param pShuntroute	pointer to the shuntroute
 * @param nr		number of the shuntroute part
 * @return		argument
 * @exception		Assertion on NULL pointer, if it is not a turnout or if index is too big
 */
enum eTurnoutLine shuntrouteGetTurnoutArgument(const struct sShuntroute * const pShuntroute, const unsigned short i)
{
	assert(ShuntroutePartTurnout == shuntrouteGetPartType(pShuntroute, i));

	return (enum eTurnoutLine) shuntrouteGetPartArgument(pShuntroute, i);
}

/**
 * Function to add a protective turnout to a shuntroute
 * @param pShuntroute	pointer to the shuntroute
 * @param pTurnout	pointer to the turnout
 * @param line		line the protective line has to be set to
 * @return			index, where the turnout has been inserted
 * @exception		Assertion on NULL pointer or too many turnout parts
 */
unsigned short shuntrouteAddProtectiveTurnout(struct sShuntroute * const pShuntroute, struct sTurnout * const pTurnout, const enum eTurnoutProtectiveLine line)
{
	assert(NULL != pShuntroute);
	assert(NRSHUNTPARTS > pShuntroute->PartsAddPosition);

	pShuntroute->pPart[pShuntroute->PartsAddPosition].type = ShuntroutePartProtectiveTurnout;
	pShuntroute->pPart[pShuntroute->PartsAddPosition].ptr = (void *) pTurnout;
	pShuntroute->pPart[pShuntroute->PartsAddPosition].argument = line;

	pShuntroute->PartsAddPosition++;
	return pShuntroute->PartsAddPosition - 1;
}

/**
 * Function to get the indexed pointer of a protective turnout as a part of a shuntroute
 * @param pShuntroute	pointer to the shuntroute
 * @param nr		number of the shuntroute part
 * @return		pointer of the turnout, if index is too big or no turnout on this place, NULL is returned
 * @exception		Assertion on NULL pointer
 */
struct sTurnout * shuntrouteGetProtectiveTurnout(const struct sShuntroute * const pShuntroute, const unsigned short i)
{
	enum eShuntroutePartType	type;

	type = shuntrouteGetPartType(pShuntroute, i);
	if(ShuntroutePartProtectiveTurnout != type)
		return NULL;

	return (struct sTurnout *) shuntrouteGetPart(pShuntroute, i);
}

/**
 * Function to get the parameter of a protective turnout as a part of a shuntroute
 * @param pShuntroute	pointer to the shuntroute
 * @param nr		number of the shuntroute part
 * @return		line the protective turnout has to be set and kept
 * @exception		Assertion on NULL pointer, if it is not a protective turnout or if index is too big
 */
enum eTurnoutProtectiveLine shuntrouteGetProtectiveTurnoutArgument(const struct sShuntroute * const pShuntroute, const unsigned short i)
{
	assert(ShuntroutePartProtectiveTurnout == shuntrouteGetPartType(pShuntroute, i));

	return (enum eTurnoutProtectiveLine) shuntrouteGetPartArgument(pShuntroute, i);
}

/**
 * Function to add a section to a shuntroute
 * @param pShuntroute	pointer to the shuntroute
 * @param pTurnout	pointer to the section
 * @return		index, where the section has been inserted
 * @exception		Assertion on NULL pointer or too many section parts
 */
unsigned short shuntrouteAddSection(struct sShuntroute * const pShuntroute, struct sSection * const pSection)
{
	assert(NULL != pShuntroute);
	assert(NRSHUNTPARTS > pShuntroute->PartsAddPosition);

	pShuntroute->pPart[pShuntroute->PartsAddPosition].type = ShuntroutePartSection;
	pShuntroute->pPart[pShuntroute->PartsAddPosition].ptr = (void *) pSection;
	pShuntroute->pPart[pShuntroute->PartsAddPosition].argument = 0;

	pShuntroute->PartsAddPosition++;
	return pShuntroute->PartsAddPosition - 1;
}

/**
 * Function to get the indexed pointer of a section as a part of a shuntroute
 * @param pShuntroute	pointer to the shuntroute
 * @param nr		number of the shuntroute part
 * @return		pointer of the section, if index is too big or no section on this place, NULL is returned
 * @exception		Assertion on NULL pointer
 */
struct sSection * shuntrouteGetSection(const struct sShuntroute * const pShuntroute, const unsigned short i)
{
	enum eShuntroutePartType	type;

	type = shuntrouteGetPartType(pShuntroute, i);
	if(ShuntroutePartSection != type)
		return NULL;

	return (struct sSection *) shuntrouteGetPart(pShuntroute, i);
}

/**
 * Function to add a protective section to a shuntroute
 * @param pShuntroute	pointer to the shuntroute
 * @param pTurnout	pointer to the section
 * @return		index, where the section has been inserted
 * @exception		Assertion on NULL pointer or too many section parts
 */
unsigned short shuntrouteAddProtectiveSection(struct sShuntroute * const pShuntroute, struct sSection * const pSection)
{
	assert(NULL != pShuntroute);
	assert(NRSHUNTPARTS > pShuntroute->PartsAddPosition);

	pShuntroute->pPart[pShuntroute->PartsAddPosition].type = ShuntroutePartProtectiveSection;
	pShuntroute->pPart[pShuntroute->PartsAddPosition].ptr = (void *) pSection;
	pShuntroute->pPart[pShuntroute->PartsAddPosition].argument = 0;

	pShuntroute->PartsAddPosition++;
	return pShuntroute->PartsAddPosition - 1;
}

/**
 * Function to get the indexed pointer of a protective section as a part of a shuntroute
 * @param pShuntroute	pointer to the shuntroute
 * @param nr		number of the shuntroute part
 * @return		pointer of the section, if index is too big or no section on this place, NULL is returned
 * @exception		Assertion on NULL pointer
 */
struct sSection * shuntrouteGetProtectiveSection(const struct sShuntroute * const pShuntroute, const unsigned short i)
{
	enum eShuntroutePartType	type;

	type = shuntrouteGetPartType(pShuntroute, i);
	if(ShuntroutePartProtectiveSection != type)
		return NULL;

	return (struct sSection *) shuntrouteGetPart(pShuntroute, i);
}
	
/**
 * Function to add a protective dwarf to a shuntroute
 * @param pShuntroute	pointer to the shuntroute
 * @param pTurnout	pointer to the turnout
 * @return		index, where the dwarf has been inserted
 * @exception		Assertion on NULL pointer or too many dwarf parts
 */
unsigned short shuntrouteAddProtectiveDwarf(struct sShuntroute * const pShuntroute, struct sDwarf * const pDwarf)
{
	assert(NULL != pShuntroute);
	assert(NRSHUNTPARTS > pShuntroute->PartsAddPosition);

	pShuntroute->pPart[pShuntroute->PartsAddPosition].type = ShuntroutePartProtectiveDwarf;
	pShuntroute->pPart[pShuntroute->PartsAddPosition].ptr = (void *) pDwarf;
	pShuntroute->pPart[pShuntroute->PartsAddPosition].argument = 0;

	pShuntroute->PartsAddPosition++;
	return pShuntroute->PartsAddPosition - 1;
}

/**
 * Function to get the indexed pointer of a protective dwarf as part of a shuntroute
 * @param pShuntroute	pointer to the shuntroute
 * @param nr		number of the shuntroute part
 * @return		pointer of the dwarf, if index is too big or no dwarf on this place, NULL is returned
 * @exception		Assertion on NULL pointer
 */

struct sDwarf * shuntrouteGetProtectiveDwarf(const struct sShuntroute * const pShuntroute, const unsigned short i)
{
	enum eShuntroutePartType	type;

	type = shuntrouteGetPartType(pShuntroute, i);
	if(ShuntroutePartProtectiveDwarf != type)
		return NULL;

	return (struct sDwarf *) shuntrouteGetPart(pShuntroute, i);
}

/**
 * Function to get the indexed type of parts of a shuntroute
 * @param pShuntroute	pointer to the shuntroute
 * @param nr		number of the shuntroute part
 * @return		type of this part, if index is too big, ShuntroutePartNone is returned
 * @exception		Assertion on NULL pointer
 */
enum eShuntroutePartType shuntrouteGetPartType(const struct sShuntroute * const pShuntroute, const unsigned short i)
{
	assert(NULL != pShuntroute);
	if(NRSHUNTPARTS <= i)
		return ShuntroutePartNone;

	return pShuntroute->pPart[i].type;
}

/**
 * Function to get the number of the shuntroute part setting 
 *  the dwarf to halt position
 * @param pShuntroute	pointer to the shuntroute
 * @return		number of the shuntroute part
 * @exception		assert on NULL pointer
 */
unsigned short shuntrouteGetResetSection(const struct sShuntroute * const pShuntroute)
{
	assert(NULL != pShuntroute);

	return pShuntroute->resetSection;
}

/**
 * Function to set the number of the shuntroute part setting 
 *  the dwarf to halt position
 * @param pShuntroute	pointer to the shuntroute
 * @param handle	number of the shuntroute part
 * @exception		assert on NULL pointer
 */
void shuntrouteSetResetSection(struct sShuntroute * const pShuntroute, unsigned short sectionNr)
{
	assert(NULL != pShuntroute);

	pShuntroute->resetSection = sectionNr;
}

/**
 * Function to calculate the important elements the train is passing
 * @param pShuntroute	pointer to the shuntroute
 * @exception		assert on NULL pointer
 */
void shuntrouteCalcRoute(struct sShuntroute * const pShuntroute)
{
	short		i;

	assert(NULL != pShuntroute);

	pShuntroute->dissolveSection	= -1;
	pShuntroute->lastSection		= -1;

	for(i = 0; i < NRSHUNTPARTS; i++)
	{
		switch(pShuntroute->pPart[i].type)
		{
		case ShuntroutePartTurnout:
			pShuntroute->dissolveSection = i;
			pShuntroute->lastSection = i;
			break;

		case ShuntroutePartSection:
			pShuntroute->lastSection = i;
			break;

		default:
			break;
		}
	}
}


/**
 * Function to get the number of the shuntroute part dissolving 
 *  the shuntroute when leaving
 * @param pShuntroute	pointer to the shuntroute
 * @return		number of the shuntroute part
 * @exception		assert on NULL pointer
 */
unsigned short shuntrouteGetDissolveSection(const struct sShuntroute * const pShuntroute)
{
	assert(NULL != pShuntroute);

	return pShuntroute->dissolveSection;
}

/**
 * Function to get the number of the shuntroute part passed as a last
 *  element when driving throu the complete shuntroute
 * @param pShuntroute	pointer to the shuntroute
 * @return		number of the shuntroute part
 * @exception		assert on NULL pointer
 */
unsigned short shuntrouteGetLastSection(const struct sShuntroute * const pShuntroute)
{
	assert(NULL != pShuntroute);

	return pShuntroute->lastSection;
}

/**
 * Function to get the state of a shuntroute
 * @param pShuntroute	pointer to the shuntroute
 * @return		state of the shuntroute
 * @exception		assert on NULL pointer
 */
enum eShuntrouteState shuntrouteGetState(const struct sShuntroute * const pShuntroute)
{
	assert(NULL != pShuntroute);

	return pShuntroute->eState;
}

/**
 * Function to request a state for the shuntroute
 * @param pShuntroute	pointer to the shuntroute
 * @param state		state requested
 * @return		true if the request was made and could be fulfilled
 * @exception		assert on NULL pointer
 */
_Bool shuntrouteRequestState(struct sShuntroute * const pShuntroute, const enum eShuntrouteState state)
{
	assert(NULL != pShuntroute);

	if(shuntrouteTransitionPossible(pShuntroute, state, true))
	{
		pShuntroute->eRequestedState = state;
		return true;
	}
	/* else */
	return false;
}

	
