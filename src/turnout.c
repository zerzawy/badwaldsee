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
 * Turnouts (Weichen)
 * \author
 * Kurt Zerzawy
 */

/*
 * $Author: Kurt $
 * $Date: 2015-11-14 23:10:12 +0100 (Sa, 14 Nov 2015) $
 * $Revision: 2672 $
 */

#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constants.h" 
#include "log.h"
#include "registers.h"
#include "parser/parser.h"
#include "turnout.h"



/*@only@*/ /*@null@*/ static struct sTurnout * m_pTurnout = NULL;

static unsigned short m_groupRelaisReg = NONEVALUE;	/**< register of relais common for all turnouts */
static _Bool          m_groupRelaisFlag = false;	/**< flag for the common relais */
static unsigned short m_nrTurnout = 0;			/**< number of turnouts */
static _Bool          m_turnoutTodo = false;		/**< todo is true, when work is to be done */
static const unsigned short m_mfTime = 60;		/**< timeout for movement of turnout in cycles */
static struct sTurnout * m_pTurnoutSel = NULL;		/**< was selected by button press	*/
#ifdef SPDR60
	static char	m_minusText[] = "Minus";	/**< text for minus position of turnout	*/
	static char	m_plusText[]  = "Plus";		/**< text for plus position of turnout	*/
#endif
#ifdef DOMINO55
	static char	m_minusText[] = "Links";	/**< text for minus position of turnout	*/
	static char	m_plusText[]  = "Recht";	/**< text for plus position of turnout	*/
#endif


/**
 * constructor for turnouts
 *
 * allocates the datas for use in the turnouts. 
 * @param nr	number of switches to allocate
 * @exception	assert if no RAM available
 */
void turnout(const unsigned short nr) 
{
	struct sTurnout * ptr;
	unsigned short      i;
	LOG_INF("started");

	assert(NULL == m_pTurnout);

	m_nrTurnout = nr;
	m_pTurnout = malloc(nr * sizeof(struct sTurnout));
	if(NULL == m_pTurnout) 
	{
		/* error allocating */
		LOG_ERR("kann kein RAM allozieren");
		exit(EXIT_FAILURE);
	}

	memset(m_pTurnout, 0, nr * sizeof(struct sTurnout));
	ptr = m_pTurnout;
	for(i = 0; i < nr; i++)
	{
		ptr->name[0]	= '\0';
		ptr->pButton	= NULL;
		ptr->BI_occupancyDetector	= NONEVALUE;
		ptr->pointLeft	= false;
		ptr->BI_contactPlus	= NONEVALUE;
		ptr->BI_contactMinus	= NONEVALUE; 
		ptr->occupancyPlus	= NONEVALUE;
		ptr->occupancyMinus	= NONEVALUE;
		ptr->signalPlus	= NONEVALUE;
		ptr->signalMinus	= NONEVALUE;
		ptr->duoledPlus	= NULL;
		ptr->duoledMinus	= NULL;
		ptr->BO_motor	= NONEVALUE;
		ptr->pSlave	= ptr;
		ptr->pMaster	= ptr;
		ptr->eState	= init;
		ptr->eLineActual	= turnoutNeutral;
		ptr->eLinePrev	= turnoutNeutral;
		ptr->eProtectiveLineActual = turnoutProtectNeutral;
		ptr->eProtectiveLinePrev = turnoutProtectNeutral;
		ptr->eState	= init;
		ptr->fbPlus	= false;
		ptr->wasFbPlus	= false;
		ptr->fbMinus	= false;
		ptr->wasFbMinus	= false;
		ptr->occupied	= false;
		ptr->wasOccupied = false;
		ptr->occupied1	= false;
		ptr->wasOccupied1	= false;
		ptr->occupied2	= false;
		ptr->wasOccupied2	= false;
		ptr->free	= false;
		ptr->toDo	= false;
		ptr->mf	= 0;

		ptr++;
	}

	/** important NOTE: do not call turnoutInit() from here, because Init depends
	 * on some variables not set before all inputs are read successfully
	 */
	LOG_INF("ended");
	return;
}

/**
 * destrukor for turnouts
 */
void turnoutDestroy(void) 
{
	LOG_INF("started");
	free(m_pTurnout);
	m_pTurnout = NULL;
	LOG_INF("ended");
}

/**
 * HW reading function
 *
 * reads the hardware inputs of a turnout and stores the values into
 * the respective variables. 
 *
 * The informations where the inputs come from are stored in the 
 * data structure.
 * @param ptr	pointer to the turnout to be read
 * @exception	assert on NULL pointer
 */
static void turnoutReadRawWH(/*@null@*/ struct sTurnout * const pTurnout);

/**
 * neighbourhood function
 *
 * searches for occupancy of turnouts in the neighbourhood where the train could
 * get into the profile of the own turnout. This is the case if the neighbour is 
 * occupied and the direction is set to myself.
 * @param ptr	pointer to the turnout to be checked
 * @exception	assert on Null pointer
 */
static void turnoutCalcNeighbourhood(/*@null@*/ struct sTurnout * const pTurnout);

/**
 * internal function to set the line for a turnout.
 *  this function was implemented since the functions for the real used line
 *  and the protection line is quite similar, so it makes sence to do it only once
 *  and hide it in a static function
 * @param pTurnout	pointer to the turnout to be handled
 * @param requestedLine	line requested. For protection left-right and back are not checked
 * @param protection 	true if protection requested
 * @return 		true if it can be done
 */
static _Bool turnoutInternalLine(struct sTurnout * const pTurnout, 
                 const enum eTurnoutLine requestedLine,
		 bool protection);

/**
 * operating function for turnout. This function is not testing any more if a turn
 *  is allowed or clever, it is just doing it. So be careful if you call it
 * @param pTurnout	pointer to the turnout to be operated
 * @param straigth	true if it shall be moved to plus, false otherwise
 */
static void turnoutOperate(struct sTurnout * const pTurnout, _Bool plus);

/** 
 * set the turnout to the given direction.
 *
 * the turnout must have been reserved before 
	 * @param pMaster	pointer to the leading turnout
 * @param pSlave	pointer to the controlled turnout
 * @param plus		true for plus
 * @param outOfError	true means, it can be moved out of error conditions
 * 					of the turnout
 * @return			true when the turnout is set
 * @exceptioni		assert on NULL
 */
static _Bool stelle(/*@null@*/ /*@in@*/ struct sTurnout * const pMaster, 
					/*@null@*/ /*@in@*/ struct sTurnout * const pSlave, 
					const _Bool plus,
					const _Bool outOfError);

/** verschliesse eine Turnout in der verlangten Richtung
 * Die Turnout muss in die richtige Richtung gestellt sein:
 */

/** lege eine Turnout in der verlangten Richtung fest. Diese Funktion wird von 
 * Zugfahrstrassen genutzt als zusaetliche Sicherheit gegen das Stellen von Turnoutn
 */

_Bool turnoutStateMachine(struct sTurnout * const pSelf);

static _Bool turnoutPossible(const struct sTurnout * const pMaster, 
							const struct sTurnout * const pSlave);

_Bool turnoutLine(struct sTurnout * const pTurnout, 
                 const enum eTurnoutLine requestedLine) {
	return turnoutInternalLine(pTurnout, requestedLine, false);
}

_Bool turnoutProtectiveLine(struct sTurnout * const pTurnout,
                const enum eTurnoutProtectiveLine      requestedLine) 
{
	return turnoutInternalLine(pTurnout, requestedLine, true);
}

static _Bool turnoutInternalLine(struct sTurnout * const pTurnout, 
			           const enum eTurnoutLine requestedLine,
				   bool protective) 
{
	struct sTurnout * pMaster;  /* pointer of the leading turnout	*/
	struct sTurnout * pSlave;   /* pointer of the turnout lead by the master	*/
	enum eTurnoutLine	eLineRequested	= requestedLine;
	enum eTurnoutLine	eLineActual	= neutral;
	enum eTurnoutLine	eLineMax	= neutral;	/*<< value of both now and previous	*/
	enum eTurnoutProtectiveLine	eProtectiveLineMax	= neutral;	/*<< value of both now and previous protective line	*/
	enum eTurnoutLine	eLimiting	= neutral;		/*<< value which is limiting possibilities for changes	*/
	unsigned int	maskedLine	= 0;
	unsigned int	maskedDir	= 0;

	assert(NULL != pTurnout);

	/* read pointers of the master and slave turnout	*/
	pMaster = pTurnout->pMaster;
	pSlave = pTurnout->pSlave;
	
	assert(NULL != pMaster);  
	assert(NULL != pSlave);
	
	/* if there is no request, it is always possible	*/
	if(turnoutNeutral == eLineRequested)
	{
		return true;
	}

	/*
	 * check if there is a conflict of the lines concerning 
	 *  turnout direction
	 * so this is some kind of sanity check
	 */
	if((turnoutNeutral != pMaster->eLineActual)
	&& (turnoutNeutral != pMaster->eLinePrev) 
	&& ((TURNOUTDIRMASK & pMaster->eLineActual) != (TURNOUTDIRMASK & pMaster->eLinePrev))) 
	{
		/* this is like a short circuit in the switchboard, so quiet a serious	
		 * situation that should not happen. 
		 */
		LOG_ERR("Spuren sind widerspruechlich");
		pMaster->eLineActual = turnoutNeutral;
		return false;
	}

	if((turnoutProtectNeutral != pMaster->eProtectiveLineActual)
	&& (turnoutProtectNeutral != pMaster->eProtectiveLinePrev) 
	&& ((TURNOUTDIRMASK & pMaster->eProtectiveLineActual) != (TURNOUTDIRMASK & pMaster->eProtectiveLinePrev))) 
	{
		LOG_ERR("Spuren der Schutzweiche sind widerspruechlich");
		pMaster->eProtectiveLineActual = turnoutProtectNeutral;
		return false;
	}

	if((turnoutNeutral != pMaster->eLineActual)
	&& (turnoutProtectNeutral != pMaster->eProtectiveLineActual)
	&& ((LINEPLUS & pMaster->eLineActual) != (LINEPLUS & pMaster->eProtectiveLineActual))) 
	{
		LOG_ERR("Spuren der Schutzweiche und der Weiche sind widerspruechlich");
		pMaster->eProtectiveLineActual = turnoutProtectNeutral;
		pMaster->eLineActual = turnoutNeutral;
		return false;
	}

	eLineActual = pMaster->eLineActual;
	eLineMax    = pMaster->eLineActual | pMaster->eLinePrev;
	eProtectiveLineMax = pMaster->eProtectiveLineActual | pMaster->eProtectiveLinePrev;

	/* calculate the most limiting line. This is an OR funcion of all lines	*/
	eLimiting = eLineMax | eProtectiveLineMax;

	/*
	 * for future calculations, the LineActual value of neutral makes
	 *  many problems because of its neutral nature.
	 * so if the maximum is neutral, I just say that the direction of the line
	 *  and of the turnout are the same than the requested one
	 */
	if(turnoutNeutral == eLimiting) 
	{
		/* it was neutral	*/
		eLineActual |= (TURNOUTDIRMASK & eLineRequested); 	
		eLimiting   |= (TURNOUTDIRMASK & eLineRequested); 	
	}


	if(protective) 
	{
		/* only a protective function, only direction of switch is important,
		 * not of traffic
		 */
		if((eLineRequested & LINEPLUS) != (eLimiting & LINEPLUS)) 
		{
			/* now another line or turnout direction is asked for	*/
			return false;
		}
	}
	else
	{
		/* 
		 * if a line for passing, so not only a protective switch line is asked for
		 *  make sure that there is no line yet in the other direction
		 */
		if((eLineRequested & TURNOUTDIRMASK) != (eLimiting & TURNOUTDIRMASK))
		{
			/* now another line or turnout direction is asked for	*/
			return false;
		}
	}

	/* 
	 * make a masked value to ease calculations
	 */
	maskedLine = eLineRequested & LINEMASK;
	maskedDir  = eLineRequested & TURNOUTDIRMASK;
	
	/* 
	 * if we came so far, we can be sure that
	 * - no line is pending which is in the other driving direction
	 * - no line is pending that asks for another direction of the turnout
	 * - the direction bits of eLineActual are set
	 */
 
	/*
	 * check if the switch can be thrown in the desired position or is
	 *  in the desired position
	 */
	switch(pMaster->eState)
	{
	case init:		
	case PlusErr:	
	case MinusErr:		
		/* states where nothing possible, set by hand if no contacts	*/
		pMaster->eLineActual = turnoutNeutral;
		return false;	/* no action possible	*/

	case PlusIs:
		/* all values plus possible	*/
		if(0 != (LINEPLUS & eLineRequested)) 
		{
			/* cases with plus line requested	
			 *  OR means calculating the maximum value
			 */
			if(protective) 
			{
				pMaster->eProtectiveLineActual |= eLineRequested;
			} 
			else 
			{
				pMaster->eLineActual |= eLineRequested;
			}
					return true;
		}
		/* now all cases where the turnout shall be switched to
		 *  minus position
		 */

		if(! turnoutPossible(pMaster, pSlave)) 
		{

			/* no switching possible because of occupancy	*/
			if(protective) 
			{
				pMaster->eProtectiveLineActual = turnoutProtectNeutral;
			} 
			else 
			{
				pMaster->eLineActual = turnoutNeutral;
			}
			return false;
		}
		/* changing the turnout is possible, so it can be reserved
		 * or a switch request can be made
		 * so first limit the line stored
		 */
		if(protective) 
		{
			pMaster->eProtectiveLineActual = maskedDir | LINERESERVE;
		} 
		else 
		{
			pMaster->eLineActual = maskedDir | LINERESERVE;
		}

		if(maskedLine == LINERESERVE) 
		{
			return true;
		} 

		/* 
		 * all other cases, it must be limited to switch. there is 
		 *  no other difference to be made, since the return value
		 *  is always false
		 * set the line to switch and return false, since the
		 * turnout has not reached yet the position asked for
		 */
		turnoutOperate(pMaster, false);
		if(protective) 
		{
			pMaster->eProtectiveLineActual |= LINESWITCH;
		} 
		else 
		{		 
			pMaster->eLineActual |= LINESWITCH;
		}
		return false;

	case PlusSet:
		/* here only lines with plus position are handled.
		 *  in other cases, please wait until the turnout has
		 *  reached the final position
		 */
		if(0 == (LINEPLUS & eLineRequested)) 
		{
			/* as said, no switching to reverse allowed now	*/
			if(protective) 
			{
				pMaster->eProtectiveLineActual = turnoutProtectNeutral;
			}
			else 
			{
				pMaster->eLineActual = turnoutNeutral;
			}
			return false;
		}
		/* cases with plus line requested
		 * changing the turnout is possible, so it can be reserved
		 * or a switch request can be made
		 */
		if(protective) 
		{
			pMaster->eProtectiveLineActual = maskedDir | LINERESERVE;
		} 
		else 
		{
			pMaster->eLineActual = maskedDir | LINERESERVE;
		}

		if(maskedLine == LINERESERVE) 
		{
			return true;
		} 

		/* 
		 * all other cases, it must be limited to switch. there is 
		 *  no other difference to be made, since the return value
		 *  is always false
		 * set the line to switch and return false, since the
		 * turnout has not reached yet the position asked for
		 */
		turnoutOperate(pMaster, false);
		if(protective) 
		{
			pMaster->eProtectiveLineActual |= LINESWITCH;
		} 
		else 
		{		 
			pMaster->eLineActual |= LINESWITCH;
		}
		return false;

	/* now the cases for branch. For the comments, see the two cases above	*/
	case MinusIs:	
		if(0 == (LINEPLUS & eLineRequested)) 
		{
			if(protective) 
			{
				pMaster->eProtectiveLineActual |= eLineRequested;
			}
			else 
			{
				pMaster->eLineActual |= eLineRequested;
			}
			return true;
		}
		if(! turnoutPossible(pMaster, pSlave)) 
		{
			if(protective) 
			{
				pMaster->eProtectiveLineActual = turnoutProtectNeutral;
			} 
			else 
			{
				pMaster->eLineActual = turnoutNeutral;
			}
			return false;
		}
		if(protective) 
		{
			pMaster->eProtectiveLineActual = maskedDir | LINERESERVE;
		} 
		else 
		{
			pMaster->eLineActual = maskedDir | LINERESERVE;
		}

		if(maskedLine == LINERESERVE) 
		{
			return true;
		} 

		turnoutOperate(pMaster, true);
		if(protective) 
		{
			pMaster->eProtectiveLineActual |= LINESWITCH;
		} 
		else 
		{		 
			pMaster->eLineActual |= LINESWITCH;
		}
		return false;


	case MinusSet:
		if(0 != (LINEPLUS & eLineRequested)) 
		{
			if(protective) 
			{
				pMaster->eProtectiveLineActual = turnoutProtectNeutral;
			}
			else 
			{
				pMaster->eLineActual = turnoutNeutral;
			}
			return false;
		}
		if(protective) 
		{
			pMaster->eProtectiveLineActual = maskedDir | LINERESERVE;
		} 
		else 
		{
			pMaster->eLineActual = maskedDir | LINERESERVE;
		}

		if(maskedLine == LINERESERVE) 
		{
			return true;
		} 

		turnoutOperate(pMaster, false);
		if(protective) 
		{
			pMaster->eProtectiveLineActual |= LINESWITCH;
		} 
		else 
		{		 
			pMaster->eLineActual |= LINESWITCH;
		}
		return false;
		
	default:
		assert(0);
		break;
	}
	return false;	/* never reached, to make the compiler happy	*/
};

_Bool turnoutStateMachine(struct sTurnout * const pTurnout) 
{
	_Bool	result = false;

	enum eTurnoutLine		eLineActual;
	enum eTurnoutLine		eLinePrev;
	enum eTurnoutProtectiveLine	eProtectiveLineActual;
	enum eTurnoutProtectiveLine	eProtectiveLinePrev;
	enum eTurnoutLine		eCombinedActual;	/*<< line of normal and protective switch combined	*/
	enum eTurnoutLine		eCombinedPrev;
	
	assert(NULL != pTurnout);

	eLineActual = pTurnout->eLineActual;
	eLinePrev   = pTurnout->eLinePrev;
	eProtectiveLineActual = pTurnout->eProtectiveLineActual;
	eProtectiveLinePrev   = pTurnout->eProtectiveLinePrev;
	
	
	/* this part is only made if changes were made	*/
	if((eLineActual != eLinePrev) 
	|| (eProtectiveLineActual != eProtectiveLinePrev))
	{
		eCombinedActual = eLineActual | eProtectiveLineActual;
		eCombinedPrev   = eLinePrev   | eProtectiveLinePrev;

		/*
		 * for the turnout, the direction left-right and right-left is not interpreted
		 *  so the statemachine can be simplified by always supposing the direction is from
		 *  left to right
		 */
		if(turnoutNeutral == (eCombinedActual & LINEMASK))
		{
			/* take care for cases where a neutral request has a direction	*/
			eCombinedActual = turnoutNeutral;
		}
		else
		{
	 		eCombinedActual &= LINEMASK | LINEPLUS;
			eCombinedActual |= LINELR;
		}
		if(turnoutNeutral == (eCombinedPrev & LINEMASK)) 
		{
			eCombinedPrev = turnoutNeutral;
		}
		else
		{
			eCombinedPrev   &= LINEMASK | LINEPLUS;
			eCombinedPrev   |= LINELR;
		}
		switch(eCombinedActual) 
		{
		case interlockMinusLR:
			switch(eCombinedPrev) 
			{
				case turnoutNeutral:	LOG_STW("Weiche %s ist %s reserviert", pTurnout->name, m_minusText);
				case reserveMinusLR:	LOG_STW("Weiche %s ist %s stellen angefragt", pTurnout->name, m_minusText);
				case switchMinusLR:	LOG_STW("Weiche %s ist %s verriegelt", pTurnout->name, m_minusText);
				case sectionMinusLR:	LOG_STW("Weiche %s ist %s verschlossen", pTurnout->name, m_minusText);
				case interlockMinusLR:	result = true;
							break;
				default:		LOG_ERR("Spuren sind widerspruechlich");
							pTurnout->eLineActual = neutral;
							pTurnout->eProtectiveLineActual = neutral;
							break;
			}
			break;
		case sectionMinusLR:
			switch(eCombinedPrev) 
			{
				case turnoutNeutral:	LOG_STW("Weiche %s ist %s reserviert", pTurnout->name, m_minusText);
				case reserveMinusLR:	LOG_STW("Weiche %s ist %s stellen angefragt", pTurnout->name, m_minusText);
				case switchMinusLR:	LOG_STW("Weiche %s ist %s verriegelt", pTurnout->name, m_minusText);
				case sectionMinusLR:	result = true;
							break;
				case interlockMinusLR:	LOG_STW("Weiche %s ist nicht mehr %s verschlossen", pTurnout->name, m_minusText);
							result = true;
							break;
				default:		LOG_ERR("Spuren sind widerspruechlich");
							pTurnout->eLineActual = neutral;
							pTurnout->eProtectiveLineActual = neutral;
							break;
			}
			break;
		case switchMinusLR:
			switch(eCombinedPrev) 
			{
				case turnoutNeutral:	LOG_STW("Weiche %s ist %s reserviert", pTurnout->name, m_minusText);
				case reserveMinusLR:	LOG_STW("Weiche %s ist %s stellen angefragt", pTurnout->name, m_minusText);
				case switchMinusLR:	result = true;
							break;
				case sectionMinusLR:	LOG_STW("Weiche %s ist nicht mehr %s verriegelt", pTurnout->name, m_minusText);
				case interlockMinusLR:	LOG_STW("Weiche %s ist nicht mehr %s verschlossen", pTurnout->name, m_minusText);
							result = true;
							break;
				default:		LOG_ERR("Spuren sind widerspruechlich");
							pTurnout->eLineActual = neutral;
							pTurnout->eProtectiveLineActual = neutral;
							break;
			}
			break;
		case reserveMinusLR:
			switch(eCombinedPrev) 
			{
				case turnoutNeutral:	LOG_STW("Weiche %s ist %s reserviert", pTurnout->name, m_minusText);
				case reserveMinusLR:	result = true;
							break;
				case switchMinusLR:	LOG_STW("Weiche %s ist nicht mehr %s stellen angefragt", pTurnout->name, m_minusText);
				case sectionMinusLR:	LOG_STW("Weiche %s ist nicht mehr %s verriegelt", pTurnout->name, m_minusText);
				case interlockMinusLR:	LOG_STW("Weiche %s ist nicht mehr %s verschlossen", pTurnout->name, m_minusText);
							result = true;
							break;
				default:		LOG_ERR("Spuren sind widerspruechlich");
							pTurnout->eLineActual = neutral;
							pTurnout->eProtectiveLineActual = neutral;
							break;
			}
			break;
		case turnoutNeutral:
			switch(eCombinedPrev) 
			{
				/* all transitions are possible to turnoutNeutral	*/
				case turnoutNeutral:	result = true;
							break;
				case interlockMinusLR:	LOG_STW("Weiche %s ist nicht mehr %s verschlossen", pTurnout->name, m_minusText);
				case sectionMinusLR:	LOG_STW("Weiche %s ist nicht mehr %s verriegelt", pTurnout->name, m_minusText);
				case switchMinusLR:	LOG_STW("Weiche %s ist nicht mehr %s stellen angefragt", pTurnout->name, m_minusText);
				case reserveMinusLR:	LOG_STW("Weiche %s ist nicht mehr %s reserviert", pTurnout->name, m_minusText);
							result = true;
							break;
				case interlockPlusLR:	LOG_STW("Weiche %s ist nicht mehr %s verschlossen", pTurnout->name, m_plusText);
				case sectionPlusLR:	LOG_STW("Weiche %s ist nicht mehr %s verriegelt", pTurnout->name, m_plusText);
				case switchPlusLR:	LOG_STW("Weiche %s ist nicht mehr %s stellen angefragt", pTurnout->name, m_plusText);
				case reservePlusLR:	LOG_STW("Weiche %s ist nicht mehr %s reserviert", pTurnout->name, m_plusText);
							result = true;
							break;
				default:		LOG_ERR("unmoeglicher default");
							pTurnout->eLineActual = neutral;
							pTurnout->eProtectiveLineActual = neutral;
							break;
			}
			break;
		case interlockPlusLR:
			switch(eCombinedPrev) 
			{
				case turnoutNeutral:	LOG_STW("Weiche %s ist %s reserviert", pTurnout->name, m_plusText);
				case reservePlusLR:	LOG_STW("Weiche %s ist %s stellen angefragt", pTurnout->name, m_plusText);
				case switchPlusLR:	LOG_STW("Weiche %s ist %s verriegelt", pTurnout->name, m_plusText);
				case sectionPlusLR:	LOG_STW("Weiche %s ist %s verschlossen", pTurnout->name, m_plusText);
				case interlockPlusLR:	result = true;
							break;
				default:		LOG_ERR("Spuren sind widerspruechlich");
							pTurnout->eLineActual = neutral;
							pTurnout->eProtectiveLineActual = neutral;
							break;
			}
			break;
		case sectionPlusLR:
			switch(eCombinedPrev) 
			{
				case turnoutNeutral:	LOG_STW("Weiche %s ist %s reserviert", pTurnout->name, m_plusText);
				case reservePlusLR:	LOG_STW("Weiche %s ist %s stellen angefragt", pTurnout->name, m_plusText);
				case switchPlusLR:	LOG_STW("Weiche %s ist %s verriegelt", pTurnout->name, m_plusText);
				case sectionPlusLR:	result = true;
							break;
				case interlockPlusLR:	LOG_STW("Weiche %s ist nicht mehr %s verschlossen", pTurnout->name, m_plusText);
							result = true;
							break;
				default:		LOG_ERR("Spuren sind widerspruechlich");
							pTurnout->eLineActual = neutral;
							pTurnout->eProtectiveLineActual = neutral;
							break;
			}
			break;
		case switchPlusLR:
			switch(eCombinedPrev) 
			{
				case turnoutNeutral:	LOG_STW("Weiche %s ist %s reserviert", pTurnout->name, m_plusText);
				case reservePlusLR:	LOG_STW("Weiche %s ist %s stellen angefragt", pTurnout->name, m_plusText);
				case switchPlusLR:	result = true;
							break;
				case sectionPlusLR:	LOG_STW("Weiche %s ist nicht mehr %s verriegelt", pTurnout->name, m_plusText);
				case interlockPlusLR:	LOG_STW("Weiche %s ist nicht mehr %s verschlossen", pTurnout->name, m_plusText);
							result = true;
							break;
				default:		LOG_ERR("Spuren sind widerspruechlich");
							pTurnout->eLineActual = neutral;
							pTurnout->eProtectiveLineActual = neutral;
							break;
			}
			break;
		case reservePlusLR:
			switch(eCombinedPrev) 
			{
				case turnoutNeutral:	LOG_STW("Weiche %s ist %s reserviert", pTurnout->name, m_plusText);
				case reservePlusLR:	result = true;
							break;
				case switchPlusLR:	LOG_STW("Weiche %s ist nicht mehr %s stellen angefragt", pTurnout->name, m_plusText);
				case sectionPlusLR:	LOG_STW("Weiche %s ist nicht mehr %s verriegelt", pTurnout->name, m_plusText);
				case interlockPlusLR:	LOG_STW("Weiche %s ist nicht mehr %s verschlossen", pTurnout->name, m_plusText);
							result = true;
							break;
				default:		LOG_ERR("Spuren sind widerspruechlich");
							pTurnout->eLineActual = neutral;
							pTurnout->eProtectiveLineActual = neutral;
							break;
			}
			break;
		default:
			LOG_ERR("unmoeglicher default");
			printf(" eCombinedActual: %xd eCombinedPrev: %x aus %xd und %xd sowie %xd und %xd\n", eCombinedActual, eCombinedPrev, pTurnout->eLineActual, pTurnout->eLinePrev,
									pTurnout->eProtectiveLineActual, pTurnout->eProtectiveLinePrev);
			pTurnout->eLineActual = neutral;
			result = false;
			break;
		}
	} 
	else 
	{
		result = true;
	}
	/* now the handling of the motor relais and the LEDs is made
	 *  the motor must only be made for masters, since slaves have no independent motor
	 */
	if(pTurnout->pMaster == pTurnout) 
	{
		switch(pTurnout->eState) 
		{
		case init:
			if((pTurnout->fbPlus) && (! pTurnout->fbMinus)) 
			{
				pTurnout->eState = PlusIs;
				LOG_STW("Weiche %s ist %s", pTurnout->name, m_plusText);
			}
			if((! pTurnout->fbPlus) && (pTurnout->fbMinus)) 
			{
				pTurnout->eState = MinusIs;
				LOG_STW("Weiche %s ist %s", pTurnout->name, m_plusText);
			}
			break;

 		case PlusIs:
			registerBitClr(pTurnout->BO_motor);		
	 		if((! pTurnout->fbPlus) || (pTurnout->fbMinus)) 
			{
				pTurnout->eState = PlusErr;
				LOG_STW("Weiche %s hat Fehler (aufgeschnitten?)", pTurnout->name);
			}
			break;

         	case PlusSet:
			registerBitClr(pTurnout->BO_motor);		
			m_groupRelaisFlag = true;	
			if((pTurnout->fbPlus) && (! pTurnout->fbMinus)) 
			{
				pTurnout->eState = PlusIs;
				LOG_STW("Weiche %s ist %s", pTurnout->name, m_plusText);
				break;
			}
			if(0 == pTurnout->mf--) 
			{
				pTurnout->eState = PlusErr;
				LOG_STW("Weiche %s hat Fehler nach %s (Stellzeit zu lange)", pTurnout->name, m_plusText);
			}	
			break;

		case PlusErr:
			registerBitClr(pTurnout->BO_motor);		
			/* error state can only be left by hand	*/
			break;

 		case MinusIs:
			registerBitSet(pTurnout->BO_motor);		
	 		if((! pTurnout->fbMinus) || (pTurnout->fbPlus)) 
			{
				pTurnout->eState = MinusErr;
				LOG_STW("Weiche %s hat Fehler (aufgeschnitten?)", pTurnout->name);
			}
			break;

         	case MinusSet:
			registerBitSet(pTurnout->BO_motor);		
			m_groupRelaisFlag = true;	
			if((pTurnout->fbMinus) && (! pTurnout->fbPlus)) 
			{
				pTurnout->eState = MinusIs;
				LOG_STW("Weiche %s ist %s", pTurnout->name, m_minusText);
				break;
			}
			if(0 == pTurnout->mf--) 
			{
				pTurnout->eState = MinusErr;
				LOG_STW("Weiche %s hat Fehler nach %s (Stellzeit zu lange)", pTurnout->name, m_minusText);
			}	
			break;

		case MinusErr:
			registerBitSet(pTurnout->BO_motor);		
			/* error state can only be left by hand	*/
			break;
		}
	}

	switch(pTurnout->eState) 
	{
	case init:
		duoledSetMode(pTurnout->duoledPlus, ledOff);
		duoledSetMode(pTurnout->duoledMinus, ledOff);
		break;
	
	case PlusIs:
		if((turnoutNeutral == (pTurnout->eLineActual | pTurnout->eLinePrev)) 
		&& (turnoutProtectNeutral == (pTurnout->eProtectiveLineActual | pTurnout->eProtectiveLinePrev)))
		{
			duoledSetMode(pTurnout->duoledPlus, ledBlankable);
		}
		else
		{
			duoledSetMode(pTurnout->duoledPlus, ledOn);
		}
		duoledSetMode(pTurnout->duoledMinus, ledOff);
		break;

	case PlusSet:
		duoledSetMode(pTurnout->duoledPlus, ledBlinking);
		duoledSetMode(pTurnout->duoledMinus, ledOff);
		break;

	case PlusErr:
		duoledSetMode(pTurnout->duoledPlus, ledBlinking);
		duoledSetMode(pTurnout->duoledMinus, ledOff);
		break;

	case MinusIs:
		duoledSetMode(pTurnout->duoledPlus, ledOff);
		if((turnoutNeutral == (pTurnout->eLineActual | pTurnout->eLinePrev)) 
		&& (turnoutProtectNeutral == (pTurnout->eProtectiveLineActual | pTurnout->eProtectiveLinePrev)))
		{
			duoledSetMode(pTurnout->duoledMinus, ledBlankable);
		}
		else
		{
			duoledSetMode(pTurnout->duoledMinus, ledOn);
		}
		break;
		
	case MinusSet:
		duoledSetMode(pTurnout->duoledPlus, ledOff);
		duoledSetMode(pTurnout->duoledMinus, ledBlinking);
		break;

	case MinusErr:
		duoledSetMode(pTurnout->duoledPlus, ledOff);
		duoledSetMode(pTurnout->duoledMinus, ledBlinking);
		break;

	default:
		assert(0);
	}

	pTurnout->eLinePrev	= pTurnout->eLineActual;
	pTurnout->eProtectiveLinePrev	= pTurnout->eProtectiveLineActual; 
	pTurnout->eLineActual	= neutral;
	pTurnout->eProtectiveLineActual	= neutral;

	return result;
}

/*@maynotreturn@*/ 
static _Bool turnoutPossible(const struct sTurnout * const pSelf, 
							const struct sTurnout * const pOther)
{
	if(! turnoutIsFree(pSelf))
		return false;	/* if the turnout is occupied, no operation possible  */
	
	if(! turnoutIsFree(pOther))
		return false;	/* it the slave turnout is occupied, no operation possible */

	return true;
}

_Bool turnoutIsFree(/*@null@*/ const struct sTurnout * const ptr) 
{
	assert (NULL != ptr);
	
	return ptr->free;
}

static void turnoutOperate(struct sTurnout * const pTurnout, _Bool plus) {
	if(plus || (MinusIs == pTurnout->eState)) {
		pTurnout->mf = m_mfTime;
		pTurnout->eState = PlusSet;
		return;
	}
	if(PlusIs == pTurnout->eState) {
		pTurnout->mf = m_mfTime;
		pTurnout->eState = MinusSet;
	}
}

void turnoutOperateManually(struct sTurnout * const ptr) {
	struct sTurnout * pMaster;
	struct sTurnout * pSlave;

	assert (NULL != ptr);

	pMaster = ptr->pMaster;
	pSlave  = ptr->pSlave;

	assert(NULL != pMaster);
	assert(NULL != pSlave);

	/* switching by hand only possible if no reservation over a turnout	*/
	if(turnoutNeutral != ptr->eLineActual) 
		return;

	if(turnoutNeutral != ptr->eLinePrev)
		return;

	if(! turnoutPossible(pMaster, pSlave)) 
		return;

	switch(pMaster->eState) {
	case init:
	case PlusIs:
	case PlusErr:
		/* Cases to switch to minus	*/
		LOG_STW("Weiche %s schalte von Hand auf %s", pMaster->name, m_minusText);
		pMaster->mf = m_mfTime;
		pMaster->eState = MinusSet;
		break;

	case MinusIs:
	case MinusErr:
		/* Cases to switch to plus	*/
		LOG_STW("Weiche %s schalte von Hand auf %s", pMaster->name, m_plusText);
		pMaster->mf = m_mfTime;
		pMaster->eState = PlusSet;
		break;

	default:	
		/* nothing to do	*/
		break;
	}
}

/**
 * Function to return the number of turnout buttons pressed 
 * and the pointer to the turnout which button was pressed
 * If more than one button was pressed, the return may be 
 * incorrect.
 * @param pSel	pointer to pointer of the turnout with pushed button
 * return		Number of buttons
 */
unsigned short turnoutNrButtons(struct sTurnout ** ppSel) 
{
	unsigned short		i;
	unsigned short		nr = 0;
	struct sTurnout *	ptr = m_pTurnout;

   	* ppSel = NULL;	
	for(i = 0; i < m_nrTurnout; i++) 
	{
		if((ptr->pMaster == ptr) && (buttonSingleReadHW(ptr->pButton)))
		{
			nr++;
			m_pTurnoutSel = ptr;	/* remember internal pointer	*/
			* ppSel = ptr;		/* set return value		*/
			ptr->toDo = true; 
			m_turnoutTodo = true;
		}
		ptr++;
	}
	return nr;
} 
	 
void turnoutReadHW(void) 
{
	struct sTurnout * ptr = m_pTurnout;
	unsigned short      i;
	
	assert (NULL != ptr);
	for (i = 0; i < m_nrTurnout; i++) 
	{
		turnoutReadRawWH(ptr++);
	}
	ptr = m_pTurnout;
	for (i = 0; i < m_nrTurnout; i++) 
	{
		turnoutCalcNeighbourhood(ptr++);
	}
}

static void turnoutReadRawWH(/*@null@*/ struct sTurnout * const pTurnout) 
{
	assert (NULL != pTurnout);
	
	/* no more checks since internal function	*/
	
	pTurnout->occupied = registerBitGet(pTurnout->BI_occupancyDetector);
	if((! pTurnout->occupied) && (pTurnout->wasOccupied)) 
	{
		LOG_STW("Weiche %s wird frei", pTurnout->name); 
		pTurnout->toDo = true; 
		m_turnoutTodo = true;
	}
	pTurnout->fbPlus = registerBitGet(pTurnout->BI_contactPlus);
	if((pTurnout->occupied) && (! pTurnout->wasOccupied)) 
	{
		LOG_STW("Weiche %s wird besetzt", pTurnout->name); 
		pTurnout->toDo = true; 
		m_turnoutTodo = true;
	}
	pTurnout->fbPlus = registerBitGet(pTurnout->BI_contactPlus);
	if(   ((pTurnout->fbPlus) && (! pTurnout->wasFbPlus))
		 || ((! pTurnout->fbPlus) && (pTurnout->wasFbPlus))) 
	{
		pTurnout->toDo = true;
		m_turnoutTodo = true;
	}
	pTurnout->fbMinus = registerBitGet(pTurnout->BI_contactMinus);
	if(   ((pTurnout->fbMinus) && (! pTurnout->wasFbMinus))
		 || ((! pTurnout->fbMinus) && (pTurnout->wasFbMinus))) 
	{
		pTurnout->toDo = true;
		m_turnoutTodo = true;
	}
	
	/* handle the neighbourhood occupations here	*/
	
	if(pTurnout->occupied && !pTurnout->fbPlus)
		registerBitSet(pTurnout->signalMinus);
	else
		registerBitClr(pTurnout->signalMinus);

	if(pTurnout->occupied && !pTurnout->fbMinus)
		registerBitSet(pTurnout->signalPlus);
	else
		registerBitClr(pTurnout->signalPlus);

	/* now handle the buttons of the turnout	*/
	/* TODO */
	pTurnout->wasOccupied = pTurnout->occupied; 
	pTurnout->wasFbPlus = pTurnout->fbPlus;
	pTurnout->wasFbMinus = pTurnout->fbMinus;

}


static void turnoutCalcNeighbourhood(/*@null@*/ struct sTurnout * const pTurnout) 
{
	assert (NULL != pTurnout);
	
	pTurnout->occupied1 = registerBitGet(pTurnout->occupancyPlus);
	pTurnout->occupied2 = registerBitGet(pTurnout->occupancyMinus);
	
	/* calculate the conditions for the occupancy of the complete turnout	*/
	pTurnout->free = ! (pTurnout->occupied || pTurnout->occupied1 || pTurnout->occupied2);
}

void turnoutInit(void) 
{
	struct sTurnout * ptr = m_pTurnout;  
	unsigned short i;
	
	LOG_INF("started");
	/* Note: all variables are written, also the ones never used any more
	 * in case it is a slave turnout
	 */
	
	assert(NULL != ptr);
	for (i = 0; i < m_nrTurnout; i++) 
	{
		/* init all variables	*/
		ptr->eState 	= init;
		ptr->fbPlus	= false;
		ptr->fbMinus	= false;
		ptr->occupied	= false;
		ptr->occupied1	= false;
		ptr->occupied2	= false;
		ptr->free	= false;
		ptr->mf         = 0;
		ptr->toDo		= true;
		
		/* read all signals from HW and connect them to the variable names
		 * set the relais equal to the feedback of the contacts
		 * TODO error cases and occupied turnout not handled yet
		 */
		turnoutReadRawWH(ptr);
		if(ptr->fbPlus) 
		{
			registerBitClr(ptr->BO_motor);
		}
		if(ptr->fbMinus) 
		{
			registerBitSet(ptr->BO_motor);
		}
		turnoutStateMachine(ptr);
		ptr++;
	}
	m_turnoutTodo = true;
	LOG_INF("ended");
}

void turnoutProcess(void) 
{
	struct sTurnout *	ptr = m_pTurnout;
	unsigned short		i;
	
	assert(NULL != ptr);
//	if(! m_turnoutTodo) 
//		return;

	m_turnoutTodo = false;   /* gehe davon aus, dass in Zukunft nichts zu tun ist */
	m_groupRelaisFlag = false;
	for (i = 0; i < m_nrTurnout; i++) 
	{
		if(true); //ptr->toDo) 
		{
			ptr->toDo = false;
			/*@void@*/ turnoutStateMachine(ptr);
		}
		ptr++;
	}

	if(m_groupRelaisFlag) 
	{
		registerBitSet(m_groupRelaisReg);
	}
	else 
	{
		registerBitClr(m_groupRelaisReg);
	} 
}

/**
 * function to find the pointer of a turnout by giving the name
 * @param name		name of the turnout to be locked for
 * @return		pointer of the found turnout
 * @exception		a parser assert is made if not found
 */
struct sTurnout * turnoutFindPtr(const char * const name) 
{
        unsigned short		i;
		struct sTurnout *	ptr;
        char				temp[128];

		i = 0;
		while(NULL != (ptr = turnoutGet(i)))
		{ 
			if(0 == (strcmp(ptr->name, name))) 
				return ptr;
			i++;
        }
        sprintf(temp, "Weiche %s nicht gefunden", name);
        parserExit(temp);
        return 0;
}

/**
 * function to get the pointer by giving the number of the turnout
 * @param i		index, starting with 0
 * @return		pointer to this turnout, NULL if not existing
 */ 
struct sTurnout * turnoutGet(const unsigned short i)
{
	if(i >= m_nrTurnout)
	{
		return NULL;
	}
	else
	{
		return m_pTurnout + i;
	}
}

/**
 * function to get the number of turnouts 
 * @return:			number of turnouts
 */
unsigned short turnoutGetNr(void)
{
	return m_nrTurnout;
}

/**
 * function to get the name of a turnout
 * @param ptr:		pointer to the turnout
 * @param name:		pointer to the string filled out with the name
 * 					of the turnout. Must be at least NAMELEN long
 * @return			true, if successful
 * @exception		assert on NULL pointer
 */ 
_Bool turnoutGetName(const struct sTurnout * const ptr, char * const name)
{
	assert(NULL != ptr);

	name[NAMELEN - 1] = '\0';
	memmove(name, ptr->name, NAMELEN - 1);
	return true;
}

/**
 * function to set the name of a turnout
 * @param ptr:		pointer to the turnout
 * @param name:		pointer to the string with the name 
 * @exception		assert on NULL pointer
 */ 
void turnoutSetName(struct sTurnout * const ptr, const char * const name)
{
	assert(NULL != ptr);

	memmove(ptr->name, name, NAMELEN);
}

/**
 * function to get the pointer to the button
 * @param ptr		pointer to the turnout
 * @return		pointer to the button 
 * @exception		assert on NULL pointer
 */
struct sButton * turnoutGetButton(const struct sTurnout * const ptr)
{
	assert(NULL != ptr);

	return ptr->pButton;
}

/**
 * function to set the button 
 * @param ptr		pointer to the turnout
 * @param pButton	pointer to the button 
 * @exception		assert on NULL pointer
 */
void turnoutSetButton(struct sTurnout * const ptr, struct sButton * const pButton)
{
	assert(NULL != ptr);

	ptr->pButton = pButton;
}


/**
 * function to get the group relais register
 * @return		group relais register
 * @exception		assert on NULL pointer
 */
unsigned short turnoutGetGroupRelaisRegister(void)
{
	return m_groupRelaisReg;
}

/**
 * function to set the group relais register 
 * @param handle	group relais register 
 * @exception		assert on NULL pointer
 */
void turnoutSetGroupRelaisRegister(const unsigned short handle)
{
	m_groupRelaisReg = handle;
}

/**
 * function to set the direction of the point of the turnout
 * @param ptr		pointer to the turnout
 * @param left		true if the point points to left
 * @exception		assert on NULL pointer
 */
void turnoutSetPointLeft(struct sTurnout * const ptr, _Bool left)
{
	assert(NULL != ptr);

	ptr->pointLeft = left;
}

/**
 * function to get the direction of the point of the turnout
 * @param ptr		pointer to the turnout
 * @return		true if the point points to left
 * @exception		assert on NULL pointer
 */
_Bool turnoutIsPointLeft(const struct sTurnout * const ptr)
{
	assert(NULL != ptr);

	return ptr->pointLeft;
}

/**
 * function to set the aspect for the plus direction of the turnout
 * @param ptr		pointer to the turnout
 * @param aspect	aspect for driving over the plus direction
 * @exception		assert on NULL pointer
 */
void turnoutSetAspectPlus(struct sTurnout * const ptr, unsigned short aspect)
{
	assert(NULL != ptr);

	ptr->aspectPlus = aspect;
}

/**
 * function to get the aspect for the plus direction of the turnout
 * @param ptr		pointer to the turnout
 * @return		aspect for driving over the plus direction
 * @exception		assert on NULL pointer
 */
unsigned short turnoutGetAspectPlus(const struct sTurnout * const ptr)
{
	assert(NULL != ptr);

	return ptr->aspectPlus;
}


/**
 * function to set the aspect for the minus direction of the turnout
 * @param ptr		pointer to the turnout
 * @param aspect	aspect for driving over the minus direction
 * @exception		assert on NULL pointer
 */
void turnoutSetAspectMinus(struct sTurnout * const ptr, unsigned short aspect)
{
	assert(NULL != ptr);

	ptr->aspectMinus = aspect;
}

/**
 * function to get the aspect for the minus direction of the turnout
 * @param ptr		pointer to the turnout
 * @return		aspect for driving over the minus direction
 * @exception		assert on NULL pointer
 */
unsigned short turnoutGetAspectMinus(const struct sTurnout * const ptr)
{
	assert(NULL != ptr);

	return ptr->aspectMinus;
}

/**
 * function to get the handle of the occupancy detector
 * @param ptr		pointer to the turnout
 * @return		handle of the occupancy detector
 * @exception		assert on NULL pointer
 */
unsigned short turnoutGetOccupancyDetector(const struct sTurnout * const ptr)
{
	assert(NULL != ptr);

	return ptr->BI_occupancyDetector;
}

/**
 * function to set the handle of the occupancy detector 
 * @param ptr		pointer to the turnout
 * @param handle	handle of the occupancy detector 
 * @exception		assert on NULL pointer
 */
void turnoutSetOccupancyDetector(struct sTurnout * const ptr, const unsigned short handle)
{
	assert(NULL != ptr);

	ptr->BI_occupancyDetector = handle;
}


/**
 * function to get the handle of the plus contact
 * @param ptr		pointer to the turnout
 * @return		handle of the plus contact
 * @exception		assert on NULL pointer
 */
unsigned short turnoutGetContactPlus(const struct sTurnout * const ptr)
{
	assert(NULL != ptr);

	return ptr->BI_contactPlus;
}

/**
 * function to set the handle of the plus contact 
 * @param ptr		pointer to the turnout
 * @param handle	handle of the plus contact 
 * @exception		assert on NULL pointer
 */
void turnoutSetContactPlus(struct sTurnout * const ptr, const unsigned short handle)
{
	assert(NULL != ptr);

	ptr->BI_contactPlus = handle;
}


/**
 * function to get the handle of the minus contact
 * @param ptr		pointer to the turnout
 * @return		handle of the minus contact
 * @exception		assert on NULL pointer
 */
unsigned short turnoutGetContactMinus(const struct sTurnout * const ptr)
{
	assert(NULL != ptr);

	return ptr->BI_contactMinus;
}

/**
 * function to set the handle of the minus contact 
 * @param ptr		pointer to the turnout
 * @param handle	handle of the minus contact 
 * @exception		assert on NULL pointer
 */
void turnoutSetContactMinus(struct sTurnout * const ptr, const unsigned short handle)
{
	assert(NULL != ptr);

	ptr->BI_contactMinus = handle;
}


/**
 * function to get the handle of the signal from the plus side neighbour
 * @param ptr		pointer to the turnout
 * @return		handle of the signal from the plus side neighbour
 * @exception		assert on NULL pointer
 */
unsigned short turnoutGetOccupancyPlus(const struct sTurnout * const ptr)
{
	assert(NULL != ptr);

	return ptr->occupancyPlus;
}

/**
 * function to set the handle of the signal from the plus side neighbour 
 * @param ptr		pointer to the turnout
 * @param handle	handle of the signal from the plus side neighbour 
 * @exception		assert on NULL pointer
 */
void turnoutSetOccupancyPlus(struct sTurnout * const ptr, const unsigned short handle)
{
	assert(NULL != ptr);

	ptr->occupancyPlus = handle;
}


/**
 * function to get the handle of the signal from the minus side neighbour
 * @param ptr		pointer to the turnout
 * @return		handle of the signal from the minus side neighbour
 * @exception		assert on NULL pointer
 */
unsigned short turnoutGetOccupancyMinus(const struct sTurnout * const ptr)
{
	assert(NULL != ptr);

	return ptr->occupancyMinus;
}

/**
 * function to set the handle of the signal from the minus side neighbour 
 * @param ptr		pointer to the turnout
 * @param handle	handle of the signal from the minus side neighbour 
 * @exception		assert on NULL pointer
 */
void turnoutSetOccupancyMinus(struct sTurnout * const ptr, const unsigned short handle)
{
	assert(NULL != ptr);

	ptr->occupancyMinus = handle;
}


/**
 * function to get the handle of the signal to the plus side neighbour
 * @param ptr		pointer to the turnout
 * @return		handle of the signal to the plus side neighbour
 * @exception		assert on NULL pointer
 */
unsigned short turnoutGetSignalPlus(const struct sTurnout * const ptr)
{
	assert(NULL != ptr);

	return ptr->signalPlus;
}

/**
 * function to set the handle of the signal to the plus side neighbour 
 * @param ptr		pointer to the turnout
 * @param handle	handle of the signal to the plus side neighbour 
 * @exception		assert on NULL pointer
 */
void turnoutSetSignalPlus(struct sTurnout * const ptr, const unsigned short handle)
{
	assert(NULL != ptr);

	ptr->signalPlus = handle;
}


/**
 * function to get the handle of the signal to the minus side neighbour
 * @param ptr		pointer to the turnout
 * @return		handle of the signal to the minus side neighbour
 * @exception		assert on NULL pointer
 */
unsigned short turnoutGetSignalMinus(const struct sTurnout * const ptr)
{
	assert(NULL != ptr);

	return ptr->signalMinus;
}

/**
 * function to set the handle of the signal to the minus side neighbour 
 * @param ptr		pointer to the turnout
 * @param handle	handle of the signal to the minus side neighbour 
 * @exception		assert on NULL pointer
 */
void turnoutSetSignalMinus(struct sTurnout * const ptr, const unsigned short handle)
{
	assert(NULL != ptr);

	ptr->signalMinus = handle;
}


/**
 * function to get the pointer to the led of the plus side
 * @param ptr		pointer to the turnout
 * @return		pointer to the led of the plus side
 * @exception		assert on NULL pointer
 */
struct sDuoled * turnoutGetDuoledPlus(const struct sTurnout * const ptr)
{
	assert(NULL != ptr);

	return ptr->duoledPlus;
}

/**
 * function to set the pointer to the led of the plus side 
 * @param ptr		pointer to the turnout
 * @param handle	pointer to the led of the plus side 
 * @exception		assert on NULL pointer
 */
void turnoutSetDuoledPlus(struct sTurnout * const ptr, struct sDuoled * const pLed)
{
	assert(NULL != ptr);

	ptr->duoledPlus = pLed;
}


/**
 * function to get the pointer to the led of the minus side
 * @param ptr		pointer to the turnout
 * @return		pointer to the led of the minus side
 * @exception		assert on NULL pointer
 */
struct sDuoled * turnoutGetDuoledMinus(const struct sTurnout * const ptr)
{
	assert(NULL != ptr);

	return ptr->duoledMinus;
}

/**
 * function to set the pointer to the led of the minus side 
 * @param ptr		pointer to the turnout
 * @param handle	pointer to the led of the minus side 
 * @exception		assert on NULL pointer
 */
void turnoutSetDuoledMinus(struct sTurnout * const ptr, struct sDuoled * const pLed)
{
	assert(NULL != ptr);

	ptr->duoledMinus = pLed;
}


/**
 * function to get the handle to the motor
 * @param ptr		pointer to the turnout
 * @return		handle to the motor
 * @exception		assert on NULL pointer
 */
unsigned short turnoutGetMotor(const struct sTurnout * const ptr)
{
	assert(NULL != ptr);

	return ptr->BO_motor;
}

/**
 * function to set the handle to the motor 
 * @param ptr		pointer to the turnout
 * @param handle	handle to the motor 
 * @exception		assert on NULL pointer
 */
void turnoutSetMotor(struct sTurnout * const ptr, const unsigned short handle)
{
	assert(NULL != ptr);

	ptr->BO_motor = handle;
}


/**
 * function to get the pointer to the slave turnout
 * @param ptr		pointer to the turnout
 * @return		pointer to the slave turnout
 * @exception		assert on NULL pointer
 */
struct sTurnout * turnoutGetSlave(const struct sTurnout * const ptr)
{
	assert(NULL != ptr);

	return ptr->pSlave;
}

/**
 * function to set the pointer to the slave turnout 
 * @param ptr		pointer to the turnout
 * @param handle	pointer to the slave turnout 
 * @exception		assert on NULL pointer
 */
void turnoutSetSlave(struct sTurnout * const ptr, struct sTurnout * const pSlave)
{
	assert(NULL != ptr);

	ptr->pSlave = pSlave;
}


/**
 * function to get the pointer to the leading turnout
 * @param ptr		pointer to the turnout
 * @return		pointer to the leading turnout
 * @exception		assert on NULL pointer
 */
struct sTurnout * turnoutGetMaster(const struct sTurnout * const ptr)
{
	assert(NULL != ptr);

	return ptr->pMaster;
}

/**
 * function to set the pointer to the leading turnout 
 * @param ptr		pointer to the turnout
 * @param handle	pointer to the leading turnout 
 * @exception		assert on NULL pointer
 */
void turnoutSetMaster(struct sTurnout * const ptr, struct sTurnout * const pMaster)
{
	assert(NULL != ptr);

	ptr->pMaster = pMaster;
}

