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
 * Lines to other stations (Einspurstrecken, Doppelspurstrecken)
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
#include "line.h"
#include "parser/parser.h"
#include "registers.h"


/*@only@*/ /*@null@*/ static struct sLine * m_pLines = NULL;	/**< Pointer to the first line	*/
static unsigned short 		m_nrLines = 0;			/**< Number of lines	*/
static _Bool			m_linesTodo = true;		/**< something to do for lines	*/


/**
 * Constructor for the line
 * @param nr		number of lines to be created
 * @exception       	Calls exit if no RAM available
 */
void lines(const unsigned short nr) 
{
	LOG_INF("started");
	
	m_nrLines = nr;
	m_pLines = malloc(m_nrLines * sizeof(struct sLine));
	if(NULL == m_pLines) 
	{
		/* error allocating */
		LOG_ERR("cannot allocate RAM for lines");
		exit(EXIT_FAILURE);
	}
	linesInit();

	LOG_INF("ended");
}

/**
 * Destructor for the lines
 */
void linesDestroy(void) 
{
	LOG_INF("started");
	if(NULL == m_pLines) 
	{
		LOG_INF("zero pointer, beende");
		return;
	}
	LOG_INF("release RAM of lines");
	free(m_pLines);
	m_pLines = NULL;
	LOG_INF("ended");
}

/**
 * init function for lines
 */
void linesInit(void)
{
	struct sLine *	pLine = m_pLines;
	unsigned short  	i, j;

	LOG_INF("started");

	for(i = 0; i < m_nrLines; i++)
	{
		pLine->state	= LineNeutral;

		pLine->name[0] = '\0';
		#ifdef SPDR60
			pLine->pEaT		= NULL;
			pLine->EaTWithButton	= false;
			pLine->pErlhT		= NULL;
			pLine->ErlhTWithButton	= false;
			pLine->BO_ASpM		= NONEVALUE;
			pLine->BO_EaM		= NONEVALUE;
			pLine->BO_EeM		= NONEVALUE;
			pLine->selfblock	= false;
		#endif

		#ifdef DOMINO55
			pLine->pReqLeft		= NULL;
			pLine->ReqLeftWithButton= false;
			pLine->pReqRight	= NULL;
			pLine->ReqRightWithButton	= false;
		#endif

		pLine->nrSections = 0;
		for(j = 0; j < LINENRSECTIONS; j++)
		{
			pLine->pSection[j] = 0;
		}

		pLine->nrTo = 0;
		for(j = 0; j < LINENRBLOCKSTO; j++)
		{
			pLine->pTo[j] = 0;
			pLine->ToName[0][j] = '\0';
		}

		pLine->nrFrom = 0;
		for(j = 0; j < LINENRBLOCKSFROM; j++)
		{
			pLine->pFrom[j] = 0;
			pLine->FromName[0][j] = '\0';
		}
		pLine->getAllowance	= false;
		pLine->grantAllowance	= false;
		pLine++;
	}		
	m_linesTodo = true;
	LOG_INF("ended");
}

/**
 * Function to find the pointer of a line by giving the name
 * @param name		name of the line to be locked for
 * @return		pointer of the found line
 * @exception		a parser assert is made if not found
 */
struct sLine * lineFind(const char * const name) 
{
        unsigned short		i = 0;
	struct sLine *	pLines = NULL;
	char			temp[128];

	while(NULL != (pLines = lineGet(i)))
	{ 
		if(0 == (strcmp(pLines->name, name))) 
		{
			return pLines;
		}
		i++;
        }
        sprintf(temp, "Einspurstrecke %s nicht gefunden", name);
        parserExit(temp);
        return NULL;
}

/**
 * Function to get the pointer by giving the number of the line
 * @param i		index, starting with 0
 * @return		pointer to this line, NULL if not existing
 */ 
struct sLine * lineGet(const unsigned short i)
{
	if(i >= m_nrLines)
	{
		return NULL;
	}
	else
	{
		return m_pLines + i;
	}
}

/**
 * Function to get the number of lines
 * @return		Number of lines
 */
unsigned short linesGetNr(void)
{
	return m_nrLines;
}

/**
 * Function to get the name of a line
 * @param pLine:	pointer to the line
 * @param name:		pointer to the string filled out with the name
 * 			of the line. Must be at least NAMELEN long
 * @return		true, if successful
 * @exception		assert on NULL pointer
 */ 
_Bool lineGetName(const struct sLine * const pLine, /*@out@*/ char * const name)
{
	assert(NULL != pLine);

	name[NAMELEN - 1] = '\0';
	memmove(name, pLine->name, NAMELEN - 1);
	return true;
}

/**
 * Function to set the name of a line
 * @param pLine:	pointer to the line
 * @param name:		pointer to the string with the name 
 * @exception		assert on NULL pointer
 */ 
void lineSetName(struct sLine * const pLine, const char * const name)
{
	assert(NULL != pLine);

	memmove(pLine->name, name, NAMELEN);
}

/**
 * Function to get the flag showing that the single route is on the right side of the train station
 * @param pLine	pointer to the line
 * @return		true if single route is on the right side of the station
 * @exception		assert on NULL pointer
 */
const _Bool lineGetToRight(const struct sLine * const pLine)
{
	assert(NULL != pLine);

	return pLine->toRight;
}

/**
 * Function to set the flag showing that the single route is on the right side of the train station
 * @param pLine	pointer to the line 
 * @param handle	true if single route is on the right side of the station
 * @exception		assert on NULL pointer
 */
void lineSetToRight(struct sLine * const pLine, const _Bool toRight)
{
	assert(NULL != pLine);

	pLine->toRight = toRight;
}

/**
 * Function to set the flag showing that allowance to send train was received
 * @param pLine		pointer to the line 
 * @exception		assert on NULL pointer
 */
void lineGetAllowance(struct sLine * const pLine)
{
	assert(NULL != pLine);

	pLine->getAllowance = true;
}

/**
 * Function to set the flag showing that allowance to receive a train from neighbour was granted
 * @param pLine	pointer to the line 
 * @exception		assert on NULL pointer
 */
void lineGrantAllowance(struct sLine * const pLine)
{
	assert(NULL != pLine);

	pLine->getAllowance = true;
}

/**
 * Function to show that a signal leaving the station to this route is having an aspect allowing
 *  to leave the station (e.g. Hp1, Hp2). This flag must be set in each cycle as long as the
 *  signal has this aspect.
 * @param pLine	pointer to the line 
 * @exception		assert on NULL pointer
 */
void lineHp1(struct sLine * const pLine)
{
	assert(NULL != pLine);

	pLine->clear = true;
}

#ifdef SPDR60
	/**
	 * Function to get the button for EaT (Erlaubnis Abgabe Taste)
	 * @param pLine	pointer to the line
	 * @return		pointer to the EaT button
	 * @exception		assert on NULL pointer
	 */
	const struct sButton * lineGetEaT(const struct sLine * const pLine)
	{
		assert(NULL != pLine);

		return pLine->pEaT;
	}

	/**
	 * Function to set the button for EaT (Erlaubnis Abgabe Taste)
	 * @param pLine	pointer to the line 
	 * @param pButton	pointer to the EaT button
	 * @exception		assert on NULL pointer
	 */
	void lineSetEaT(struct sLine * const pLine, struct sButton * const pButton)
	{
			assert(NULL != pLine);

			pLine->pEaT = pButton;
	}

	/**
	 * Function to get the flag telling that with the EaT also the EaGT must be pushed
	 * @param pLine	pointer to the line
	 * @return		true if the control button EaGT must be pushed as well
	 * @exception		assert on NULL pointer
	 */
	const _Bool lineGetEaTWithControl(const struct sLine * const pLine)
	{
		assert(NULL != pLine);

		return pLine->EaTWithButton;
	}

	/**
	 * Function to set the flag telling that with the EaT also the EaGT must be pushed
	 * @param pLine	pointer to the line
	 * @param with		if the control button EaGT must be pushed as well
	 * @exception		assert on NULL pointer
	 */
	void lineSetEaTWithControl(struct sLine * const pLine, const _Bool with)
	{
		assert(NULL != pLine);

		pLine->EaTWithButton = with;
	}

	/**
	 * Function to get the button for ErlhT (Erlaubnis Hol Taste)
	 * @param pLine	pointer to the line
	 * @return		pointer to the ErhT button
	 * @exception		assert on NULL pointer
	 */
	const struct sButton * lineGetErlhT(const struct sLine * const pLine)
	{
		assert(NULL != pLine);

		return pLine->pErlhT;
	}

	/**
	 * Function to set the button for ErhlT (Erlaubnis Hol Taste)
	 * @param pLine	pointer to the line 
	 * @param pButton	pointer to the ErhT button
	 * @exception		assert on NULL pointer
	 */
	void lineSetErlhT(struct sLine * const pLine, struct sButton * const pButton)
	{
			assert(NULL != pLine);

			pLine->pErlhT = pButton;
	}

	/**
	 * Function to get the flag telling that with the ErhT also the EaGT must be pushed
	 * @param pLine	pointer to the line
	 * @return		true if the control button EaGT must be pushed as well
	 * @exception		assert on NULL pointer
	 */
	const _Bool lineGetErhTWithControl(const struct sLine * const pLine)
	{
		assert(NULL != pLine);

		return pLine->ErlhTWithButton;
	}

	/**
	 * Function to set the flag telling that with the ErhT also the EaGT must be pushed
	 * @param pLine	pointer to the line
	 * @param with		if the control button EaGT must be pushed as well
	 * @exception		assert on NULL pointer
	 */
	void lineSetErhTWithControl(struct sLine * const pLine, const _Bool with)
	{
		assert(NULL != pLine);

		pLine->ErlhTWithButton = with;
	}

	/**
	 * Function to get the led for the ASpM (Ausfahrt Sperr Melder), a blue LED showing that the direction to 
	 * neighbour station is blocked
	 * @param pLine	pointer to the line
	 * @return		register handle of the ASpM led
	 * @exception		assert on NULL pointer
	 */
	const unsigned short lineGetBO_ASpM(const struct sLine * const pLine)
	{
		assert(NULL != pLine);

		return pLine->BO_ASpM;
	}

	/**
	 * Function to set the led for the ASpM (Ausfahrt Sperr Melder), a blue LED showing that the direction to 
	 * neighbour station is blocked
	 * @param pLine	pointer to the line
	 * @param handle	register handle of the ASpM led
	 * @exception		assert on NULL pointer
	 */
	void lineSetBO_ASpM(struct sLine * const pLine, const unsigned short handle)
	{
		assert(NULL != pLine);

		pLine->BO_ASpM = handle;
	}

	/**
	 * Function to get the led for the EaM (Erlaubnisabgabe Melder), a white LED showing that the direction 
	 * changed towards my station
	 * @param pLine		pointer to the line
	 * @return		register handle of the EaM led
	 * @exception		assert on NULL pointer
	 */
	const unsigned short lineGetBO_EaM(const struct sLine * const pLine)
	{
		assert(NULL != pLine);

		return pLine->BO_EaM;
	}

	/**
	 * Function to set the led for the EaM (Erlaubnisabgabe Melder), a white LED showing that the direction 
	 * changed towards my station
	 * @param pLine	pointer to the line
	 * @param handle	register handle of the EaM led
	 * @exception		assert on NULL pointer
	 */
	void lineSetBO_EaM(struct sLine * const pLine, const unsigned short handle)
	{
		assert(NULL != pLine);

		pLine->BO_EaM = handle;
	}

	/**
	 * Function to get the led for the EeM (Erlaubnisempfangs Melder), a white LED showing that the direction 
	 * changed away from my station (I am allowed to send train)
	 * @param pLine		pointer to the line
	 * @return		register handle of the EeM led
	 * @exception		assert on NULL pointer
	 */
	const unsigned short lineGetBO_EeM(const struct sLine * const pLine)
	{
		assert(NULL != pLine);

		return pLine->BO_EeM;
	}

	/**
	 * Function to set the led for the EaM (Erlaubnisabgabe Melder), a white LED showing that the direction 
	 * changed away from my station (I am allowed to send train)
	 * @param pLine		pointer to the line
	 * @param handle	register handle of the EeM led
	 * @exception		assert on NULL pointer
	 */
	void lineSetBO_EeM(struct sLine * const pLine, const unsigned short handle)
	{
		assert(NULL != pLine);

		pLine->BO_EeM = handle;
	}

	/**
	 * Function to get the flag for selfblock (Selbsblock)
	 * @param pLine		pointer to the line
	 * @return 		true if selfblock on the line
	 * @exception		assert on NULL pointer
	 */
	const _Bool lineGetSelfblock(const struct sLine * const pLine)
	{
		assert(NULL != pLine);

		return pLine->selfblock;
	}

	/**
	 * Function to set the flag for selfblock (Selbsblock)
	 * @param pLine		pointer to the line
	 * @param selfblock	true if selfblock on the line
	 * @exception		assert on NULL pointer
	 */
	void lineSetSelfblock(struct sLine * const pLine, const _Bool selfblock)
	{
		assert(NULL != pLine);

		pLine->selfblock = selfblock;
	}
#endif

#ifdef DOMINO55
	/**
	 * Function to get the button for request to left
	 * @param pLine		pointer to the line
	 * @return		pointer to the button for request to left
	 * @exception		assert on NULL pointer
	 */
	const struct sButton * lineGetReqLeft(const struct sLine * const pLine)
	{
		assert(NULL != pLine);

		return pLine->pReqLeft;
	}

	/**
	 * Function to set the button for request to left
	 * @param pLine		pointer to the line 
	 * @param pButton	pointer to the button for request to left
	 * @exception		assert on NULL pointer
	 */
	void lineSetReqLeft(struct sLine * const pLine, struct sButton * const pButton)
	{
		assert(NULL != pLine);

		pLine->pReqLeft = pButton;
	}

	/**
	 * Function to get the flag telling that with the request Left key also the control 
	 *  key must be pushed
	 * @param pLine		pointer to the line
	 * @return		true if the control button must be pushed as well
	 * @exception		assert on NULL pointer
	 */
	_Bool lineGetReqLeftWithButton(const struct sLine * const pLine)
	{
		assert(NULL != pLine);

		return pLine->ReqLeftWithButton;
	}

	/**
	 * Function to set the flag telling that with the request Left key also the control
	 *  key must be pushed
	 * @param pLine		pointer to the line
	 * @param with		if the control button must be pushed as well
	 * @exception		assert on NULL pointer
	 */
	void lineSetReqLeftWithButton(struct sLine * const pLine, const _Bool with)
	{
		assert(NULL != pLine);

		pLine->ReqLeftWithButton = with;
	}

	/**
	 * Function to get the button for request to right
	 * @param pLine		pointer to the line
	 * @return		pointer to the button for request to right
	 * @exception		assert on NULL pointer
	 */
	const struct sButton * lineGetReqRight(const struct sLine * const pLine)
	{
		assert(NULL != pLine);

		return pLine->pReqRight;
	}

	/**
	 * Function to set the button for request to right
	 * @param pLine		pointer to the line 
	 * @param pButton	pointer to the button for request to right
	 * @exception		assert on NULL pointer
	 */
	void lineSetReqRight(struct sLine * const pLine, struct sButton * const pButton)
	{
		assert(NULL != pLine);

		pLine->pReqRight = pButton;
	}

	/**
	 * Function to get the flag telling that with the request Right key also the control 
	 *  key must be pushed
	 * @param pLine		pointer to the line
	 * @return		true if the control button must be pushed as well
	 * @exception		assert on NULL pointer
	 */
	_Bool lineGetReqRightWithButton(const struct sLine * const pLine)
	{
		assert(NULL != pLine);

		return pLine->ReqRightWithButton;
	}

	/**
	 * Function to set the flag telling that with the request Right key also the control
	 *  key must be pushed
	 * @param pLine		pointer to the line
	 * @param with		if the control button must be pushed as well
	 * @exception		assert on NULL pointer
	 */
	void lineSetReqRightWithButton(struct sLine * const pLine, const _Bool with)
	{
		assert(NULL != pLine);

		pLine->ReqRightWithButton = with;
	}

#endif

/**
 * Function to add a section to the line plate, containing the sections between block and entry signal
 * @param pLine	pointer to the single route
 * @param pSection	pointer to the section
 * @return		index, where the section has been inserted
 * @exception		Assertion on NULL pointer or too many sections
 */
unsigned short lineAddSection(struct sLine * const pLine, struct sSection * const pSection)
{
	assert(NULL != pLine);
	assert(LINENRSECTIONS > pLine->nrSections);

	pLine->pSection[pLine->nrSections] = pSection;
	pLine->nrSections++;
	return pLine->nrSections - 1;
}

/**
 * Function to get the section to the line plate, containing the sections between block and entry signal
 * @param pLine	pointer to the single route
 * @param nr		number of the train route part section
 * @return		pointer of the section, if index is too big, NULL is returned
 * @exception		Assertion on NULL pointer
 */
struct sSection * lineGetSection(const struct sLine * const pLine, const unsigned short index)
{
	assert(NULL != pLine);
	if(LINENRSECTIONS <= index)
	{
		return NULL;
	}
	return pLine->pSection[index];
}

/**
 * Function to get the name of the indexed block towards my train station.
 *  Note that in this function, the name must be given since the pointer of blocks is
 *   unknown.
 * @param pLine	pointer to the line
 * @param name		name of the block
 * @return		true if index in range and block on this index, false otherwise
 * @exception		Assertion on NULL pointer
 */
_Bool lineGetBlockToName(const struct sLine * const pLine, /*@out@*/char * const name, const unsigned short index)
{
	assert(NULL != pLine);
	if(LINENRBLOCKSTO <= index)
	{
		return false;
	}
	if(pLine->nrTo <= index)
	{
		return false;
	}

	memmove(name, pLine->ToName[index], NAMELEN);
	return true;
}

/**
 * Function to add a block that is directed towards my station
 *  Note that in this function, the name must be given since the pointer of blocks is not known yet 
 * @param pLine	pointer to the line
 * @param name		name of the block
 * @return		index, where the block has been inserted
 * @exception		Assertion on NULL pointer or too many blocks inserted
 */
unsigned short lineAddBlockToName(struct sLine * const pLine, const char * const name)
{
	assert(NULL != pLine);
	assert(LINENRBLOCKSTO > pLine->nrTo);

	memmove(pLine->pTo[pLine->nrTo], name, NAMELEN);
	pLine->nrTo++;
	return pLine->nrTo - 1;
}

/**
 * Function to get the indexed pointer of a block towords my station
 * @param pLine	pointer to the line
 * @param index		number of the block 
 * @return		pointer of the block, if index is too big or no condition shuntroute on this place, NULL is returned
 * @exception		Assertion on NULL pointer
 */
struct sBlock * lineGetBlockTo(const struct sLine * const pLine, const unsigned short index)
{
	assert(NULL != pLine);
	if(LINENRBLOCKSTO <= index)
	{
		return NULL;
	}

	return pLine->pTo[index];
}

/**
 * Function to set the indexed pointer of a block towords my statin
 * @param pLine	pointer to the line
 * @param pBlock	pointer of the block
 * @param index		number of the block 
 * @exception		Assertion on NULL pointer
 */
void lineSetBlockTo(struct sLine * const pLine, struct sBlock * const pBlock, const unsigned short index)
{
	assert(NULL != pLine);
	if(LINENRBLOCKSTO <= index)
	{
		return;
	}

	pLine->pTo[index] = pBlock;
}

/**
 * Function from get the name of the indexed block from my train station.
 *  Note that in this function, the name must be given since the pointer of blocks is
 *   unknown.
 * @param pLine	pointer from the line
 * @param name		name of the block
 * @return		true if index in range and block on this index, false otherwise
 * @exception		Assertion on NULL pointer
 */
_Bool lineGetBlockFromName(const struct sLine * const pLine, /*@out@*/char * const name, const unsigned short index)
{
	assert(NULL != pLine);
	if(LINENRBLOCKSFROM <= index)
	{
		return false;
	}
	if(pLine->nrFrom <= index)
	{
		return false;
	}

	memmove(name, pLine->FromName[index], NAMELEN);
	return true;
}

/**
 * Function from add a block that is directed from my station
 *  Note that in this function, the name must be given since the pointer of blocks is not known yet 
 * @param pLine	pointer from the line
 * @param name		name of the block
 * @return		index, where the block has been inserted
 * @exception		Assertion on NULL pointer or too many blocks inserted
 */
unsigned short lineAddBlockFromName(struct sLine * const pLine, const char * const name)
{
	assert(NULL != pLine);
	assert(LINENRBLOCKSFROM > pLine->nrFrom);

	memmove(pLine->pFrom[pLine->nrFrom], name, NAMELEN);
	pLine->nrFrom++;
	return pLine->nrFrom - 1;
}

/**
 * Function from get the indexed pointer of a block from my station
 * @param pLine	pointer from the line
 * @param index		number of the block 
 * @return		pointer of the block, if index is too big or no condition shuntroute on this place, NULL is returned
 * @exception		Assertion on NULL pointer
 */
struct sBlock * lineGetBlockFrom(const struct sLine * const pLine, const unsigned short index)
{
	assert(NULL != pLine);
	if(LINENRBLOCKSFROM <= index)
	{
		return NULL;
	}

	return pLine->pFrom[index];
}

/**
 * Function from set the indexed pointer of a block from my statin
 * @param pLine	pointer from the line
 * @param pBlock	pointer of the block
 * @param index		number of the block 
 * @exception		Assertion on NULL pointer
 */
void lineSetBlockFrom(struct sLine * const pLine, struct sBlock * const pBlock, const unsigned short index)
{
	assert(NULL != pLine);
	if(LINENRBLOCKSFROM <= index)
	{
		return;
	}

	pLine->pFrom[index] = pBlock;
}


#ifdef SPDR60
	/**
	 * state machine function
	 * provides the function when being inside the state given in pLine->eState
	 * @param pLine		pointer to the line leaving the state
	 */
	static enum eLineState lineStateBehaviour(struct sLine * const pLine);
	static enum eLineState lineStateBehaviour(struct sLine * const pLine)
	{	
		unsigned short		i;
		enum eLineState		state;

		assert(NULL != pLine);

		if(pLine->Todo)
		{
			switch(pLine->state)
			{
			case LineNeutral:
				/* just jump to ToMe	*/
				state = LineToMeNeutral;
				break;

			case LineToMeNeutral:
				/* this state can be left in two ways:
				 * by becoming occupied.
				 * by receiving allowance
				 *  Note: in SBB, allowance is automatic if requested 
				 *        simplification: request cannot be blocked yet
				 */

				/* part with occupancy	*/
				for(i = 0; i < pLine->nrTo; i++)
				{
					if(! blockIsFree(pLine->pTo[i]));
					state = LineToMeOccupied;
				}
				for(i = 0; i < pLine->nrSections; i++)
				{
					if(! sectionIsFree(pLine->pSection[i]));
					state = LineToMeOccupied;
				}

				/* getting allowance	*/
				if(pLine->getAllowance)
				{
					state = LineFromMeNeutral;
				}
				break;

			case LineToMeOccupied:
				/* NOTE: the state machine is simplified. Normally, a state
				 * for the entry signal getting allowed and restricted 
				 * aspect is necessary. This is left out in this state machine
				 */

				/* this state can only be left by getting all blocks/sections free	*/

				state = LineToMeNeutral;	/* suppose all free	*/

				for(i = 0; i < pLine->nrTo; i++)
				{
					if(! blockIsFree(pLine->pTo[i]));
					state = LineToMeOccupied;
				}
				for(i = 0; i < pLine->nrSections; i++)
				{
					if(! sectionIsFree(pLine->pSection[i]));
					state = LineToMeOccupied;
				}
				break;

			LineFromMeNeutral:
				/* this state can be left in two ways:
				 * by an exit signal getting an aspect to leave the station
				 * by granting allowance
				 */

				if(pLine->getAllowance)
				{
					state = LineFromMeNeutral;
				}

				if(pLine->clear)
				{
					state = LineFromMeHp1;
				}
				break;

			LineFromMeHp1:
				/* this state is left as soon as the exit signal falls back to stop aspect	*/
				if(! pLine->clear)
				{
					state = LineFromMeHp0;
				}
				break;

			LineFromMeHp0:
				/* this state is left when a block is occupied	*/
				
				for(i = 0; i < pLine->nrFrom; i++)
				{
					if(! blockIsFree(pLine->pFrom[i]));
					state = LineFromMeOccupied;
				}
				for(i = 0; i < pLine->nrSections; i++)
				{
					if(! sectionIsFree(pLine->pSection[i]));
					state = LineFromMeOccupied;
				}
				break;

			case LineFromMeOccupied:
				/* this state is left as soon as all blocks are clear	*/

				pLine->state = LineFromMeNeutral;	/* suppose all is clear	*/

				for(i = 0; i < pLine->nrFrom; i++)
				{
					/* if one of the blocks is occupied, stay in this state	*/
					if(! blockIsFree(pLine->pFrom[i]));
					state = LineFromMeOccupied;
				}
				for(i = 0; i < pLine->nrSections; i++)
				{
					if(! sectionIsFree(pLine->pSection[i]));
					state = LineFromMeOccupied;
				}
				break;

			default:
				state = LineNeutral;
				break;
			}

		}
		pLine->getAllowance	= false;
		pLine->grantAllowance	= false;
		pLine->clear	= false;
		pLine->Todo	= false;
		return state;
	}
#endif

#ifdef DOMINO55
	/**
	 * state machine function
	 * provides the function when being inside the state given in pLine->eState
	 * @param pLine		pointer to the line leaving the state
	 */
	static enum eLineState lineStateBehaviour(struct sLine * const pLine);
	static enum eLineState lineStateBehaviour(struct sLine * const pLine)
	{
		return LineNeutral;
	}
#endif

#ifdef SPDR60
	/**
	 * state machine entry function
	 * provides the function when entering the state given in pLine->eState
	 * @param pLine		pointer to the line leaving the state
	 */
	static void lineStateEntry(struct sLine * const pLine);
	static void lineStateEntry(struct sLine * const pLine)
	{
		assert(NULL != pLine);

		switch(pLine->state)
		{
		case LineNeutral:
			LOG_STW("Einspurstrecke %s ist neu undefiniert", pLine->name);
			break;

		case LineToMeNeutral:
			LOG_STW("Einspurstrecke %s Erlaubnis abgegeben", pLine->name);
			registerBitSet(pLine->BO_EaM);
			break;

		case LineToMeOccupied:
			LOG_STW("Einspurstrecke %s belegt", pLine->name);
			registerBitSet(pLine->BO_EaM);
			break;

		case LineFromMeNeutral:
			LOG_STW("Einspurstrecke %s Erlaubnis erhalten", pLine->name);
			registerBitSet(pLine->BO_EeM);
			break;

		case LineFromMeHp1:
			LOG_STW("Einspurstrecke %s Ausfahrsignal offen", pLine->name);
			registerBitSet(pLine->BO_EeM);
			registerBitSet(pLine->BO_ASpM);
			break;

		case LineFromMeHp0:
			LOG_STW("Einspurstrecke %s Ausfahrsignal geschlossen", pLine->name);
			registerBitSet(pLine->BO_EeM);
			registerBitSet(pLine->BO_ASpM);
			break;

		case LineFromMeOccupied:
			LOG_STW("Einspurstrecke %s belegt", pLine->name);
			registerBitSet(pLine->BO_EeM);
			break;

		default:
			break;
		}
	}
#endif

#ifdef DOMINO55
	/**
	 * state machine entry function
	 * provides the function when entering the state given in pLine->eState
	 * @param pLine		pointer to the line leaving the state
	 */
	static void lineStateEntry(struct sLine * const pLine);
	static void lineStateEntry(struct sLine * const pLine)
	{
		assert(NULL != pLine);

		switch(pLine->state)
		{
		case LineNeutral:
			LOG_STW("Einspurstrecke %s ist neu undefiniert", pLine->name);
			break;

		case LineToMeNeutral:
			LOG_STW("Einspurstrecke %s Erlaubnis abgegeben", pLine->name);
			break;

		case LineToMeOccupied:
			LOG_STW("Einspurstrecke %s belegt", pLine->name);
			break;

		case LineToMe1:
			LOG_STW("Einspurstrecke %s Einfahrsignal offen", pLine->name);
			break;

		case LineToMeHalt:
			LOG_STW("Einspurstrecke %s Einfahrsignal geschlossen", pLine->name);
			break;

		case LineFromMeNeutral:
			LOG_STW("Einspurstrecke %s Anforderung gegeben", pLine->name);
			break;

		case LineFromMe1:
			LOG_STW("Einspurstrecke %s Ausfahrsignal offen", pLine->name);
			break;

		case LineFromMeHalt:
			LOG_STW("Einspurstrecke %s Ausfahrsignal geschlossen", pLine->name);
			break;

		case LineFromMeOccupied:
			LOG_STW("Einspurstrecke %s belegt", pLine->name);
			break;

		default:
			break;
		}
	}
#endif

#ifdef SPDR60
	/**
	 * state machine exit function
	 * provides the function when leaving the state given in pLine->eState
	 * @param pLine		pointer to the line leaving the state
	 */
	static void lineStateExit(struct sLine * const pLine);
	static void lineStateExit(struct sLine * const pLine)
	{
		assert(NULL != pLine);

		registerBitClr(pLine->BO_ASpM);
		registerBitClr(pLine->BO_EaM);
		registerBitClr(pLine->BO_EeM);
						 
	}
#endif

#ifdef DOMINO55
	/**
	 * state machine exit function
	 * provides the function when leaving the state given in pLine->eState
	 * @param pLine		pointer to the line leaving the state
	 */
	static void lineStateExit(struct sLine * const pLine);
	static void lineStateExit(struct sLine * const pLine)
	{
		assert(NULL != pLine);
	}
#endif

/**
 * Function to let all lines switch to the desired state
 */
void linesProcess(void)
{
	struct sLine *		pLine = m_pLines;
	enum eLineState		resultState;
	int 	i;

	assert(NULL != pLine);

	for(i = 0; i < m_nrLines; i++)
	{
		resultState = lineStateBehaviour(pLine);
		if(pLine->state != resultState)
		{
			lineStateExit(pLine);
			pLine->state = resultState;
			lineStateEntry(pLine);
		}
		pLine++;		
	}	
}
