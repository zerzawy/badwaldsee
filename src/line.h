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
 */

/**
 * \file
 * lines from and to neighbour stations (Einspurstrecke, Doppelspurstrecke)
 * \author
 * Kurt Zerzawy
 */

/*
 * Spreitenbacher Eisenbahn Amateur Klub SPEAK
 * www.speak.li
 * $Author: kurt $
 * $Date: 2015-05-09 21:54:18 +0200 (Sam, 09. Mai 2015) $
 * $Revision: 2441 $
 */

#ifndef BAD_WALDSEE_LINES_H__
#define BAD_WALDSEE_LINES_H__

#include "constants.h"
#include "section.h"
#include "mains.h"
#include "block.h"

#define	LINENRSECTIONS	8
#define LINENRBLOCKSTO	8
#define LINENRBLOCKSFROM	8

#ifdef SPDR60
	enum eLineState
	{
		LineNeutral,
		LineToMeNeutral,
		LineToMeHp1,
		LineToMeHp0,
		LineToMeOccupied,
		LineFromMeNeutral,
		LineFromMeHp1,
		LineFromMeHp0,
		LineFromMeOccupied
	};
#endif

#ifdef DOMINO55
	enum eLineState
	{
		LineNeutral,
		LineToMeNeutral,
		LineToMe1,
		LineToMeHalt,
		LineToMeOccupied,
		LineFromMeNeutral,
		LineFromMe1,
		LineFromMeHalt,
		LineFromMeOccupied

	};
#endif



/**
 * Structure for a single track
 */
struct sLine
{
	char				name[NAMELEN];		/**< Name of this line */
	enum eLineState			state;			/**< state of the state machine		*/
	_Bool				toRight;		/**< true if single track leaves station to right	*/

	#ifdef SPDR60
		struct sButton *	pEaT;			/**< EaT (Erlaubnis Abgabe Taste) button to give right 
								      to send a train to neighbour station		*/
		_Bool			EaTWithButton;		/**< true if EaT has second button (EaGT)		*/
		struct sButton *	pErlhT;			/**< (Erlaubnis Hol Taste) button to get the right to 
								      send a train to neighbour station. Normally this
								      button exists on the neighbour station		*/
		_Bool			ErlhTWithButton;	/**< true if ErlhT has second button (normally not)	*/
		unsigned short		BO_ASpM;		/**< (Ausfahrt Sperr Melder) blue LED showing that the
								      direction to neighbour station is blocked		*/
		unsigned short		BO_EaM;			/**< (Erlaubnisabgabe Melder) led showing that the 
								      direction is to my station			*/
		unsigned short		BO_EeM;			/**< (Erlaugnisempfangs Melder) led showing that the
								      direction is from my station			*/

		_Bool			selfblock;		/**< true if the blocks are selfblocks			*/
	#endif

	#ifdef DOMINO55
		struct sButton *	pReqLeft;		/**< request to left (Anfrage links)			*/
		_Bool			ReqLeftWithButton;	/**< true if request to left has control key		*/

		struct sButton *	pReqRight;		/**< request to right (Anfrage rechts)			*/
		_Bool			ReqRightWithButton;	/**< true if request to right has control key		*/
	#endif

	unsigned short		nrSections;		/**< number of sections between block and entry signal	*/
	struct sSection *	pSection[LINENRSECTIONS];/**< sections between block and entry signal	*/

	unsigned short		nrTo;			/**< number of blocks towards my station			*/
	struct sBlock *		pTo[LINENRBLOCKSTO];	/**< blocks towards my station			*/
	char			ToName[NAMELEN][LINENRBLOCKSTO];	/**< the same as name			*/
	
	unsigned short		nrFrom;			/**< number of blocks from my station			*/
	struct sBlock *		pFrom[LINENRBLOCKSFROM];	/**< blocks from my station			*/
	char			FromName[NAMELEN][LINENRBLOCKSFROM];	/**< the same as name		*/

	_Bool			getAllowance;		/**< true if I can send train	*/
	_Bool			grantAllowance;		/**< true if I give allowance to neighbour to send train	*/
	_Bool			clear;			/**< true if an exit mains is on Hp1 or Hp2		*/

	_Bool           	Todo;			/**< set if work pending	*/
};


/*@maynotreturn@*/
void lines(const unsigned short nr);
void linesDestroy(void);
void linesInit(void);
void linesProcess(void);

struct sLine * lineFind(const char * const name);
struct sLine * lineGet(const unsigned short i);
unsigned short linesGetNr(void);

_Bool lineGetName(const struct sLine * const pLine, /*@out@*/char * const name);
void lineSetName(struct sLine * const pLine, const char * const name); 

const _Bool lineGetToRight(const struct sLine * const pLine);
void lineSetToRight(struct sLine * const pLine, const _Bool toRight);

void lineGetAllowance(struct sLine * const pLine);
void lineGrantAllowance(struct sLine * const pLine);
void lineHp1(struct sLine * const pLine);

#ifdef SPDR60
	const struct sButton * lineGetEaT(const struct sLine * const pLine);
	void lineSetEaT(struct sLine * const pLine, struct sButton * const pButton);

	const _Bool lineGetEaTWithControl(const struct sLine * const pLine);
	void lineSetEaTWithControl(struct sLine * const pLine, const _Bool with);

	const struct sButton * lineGetErlhT(const struct sLine * const pLine);
	void lineSetErlhT(struct sLine * const pLine, struct sButton * const pButton);

	const _Bool lineGetErhTWithControl(const struct sLine * const pLine);
	void lineSetErhTWithControl(struct sLine * const pLine, const _Bool with);

	const unsigned short lineGetBO_ASpM(const struct sLine * const pLine);
	void lineSetBO_ASpM(struct sLine * const pLine, const unsigned short handle);

	const unsigned short lineGetBO_EaM(const struct sLine * const pLine);
	void lineSetBO_EaM(struct sLine * const pLine, const unsigned short handle);

	const unsigned short lineGetBO_EeM(const struct sLine * const pLine);
	void lineSetBO_EeM(struct sLine * const pLine, const unsigned short handle);

	const _Bool lineGetSelfblock(const struct sLine * const pLine);
	void lineSetSelfblock(struct sLine * const pLine, const _Bool selfblock);
#endif

#ifdef DOMINO55
	const struct sButton * lineGetReqLeft(const struct sLine * const pLine);
	void lineSetReqLeft(struct sLine * const pLine, struct sButton * const pButton);

	_Bool lineGetReqLeftWithButton(const struct sLine * const pLine);
	void lineSetReqLeftWithButton(struct sLine * const pLine, const _Bool reqLeftWithButton);

	const struct sButton * lineGetReqRight(const struct sLine * const pLine);
	void lineSetReqRight(struct sLine * const pLine, struct sButton * const pButton);

	_Bool lineGetReqRightWithButton(const struct sLine * const pLine);
	void lineSetReqRightWithButton(struct sLine * const pLine, const _Bool reqRightWithButton);
#endif

unsigned short lineAddSection(struct sLine * const pLine, struct sSection * const pSection);
struct sSection * lineGetSection(const struct sLine * const pLine, const unsigned short index);

#include "block.h"
_Bool lineGetBlockToName(const struct sLine * const pLine, /*@out@*/char * const name, const unsigned short index);
unsigned short lineAddBlockToName(struct sLine * const pLine, const char * const name); 
struct sBlock * lineGetBlockTo(const struct sLine * const pLine, const unsigned short index);
void lineSetBlockTo(struct sLine * pLine, struct sBlock * const pBlock, const unsigned short index);

_Bool lineGetBlockFromName(const struct sLine * const pLine, /*@out@*/char * const name, const unsigned short index);
unsigned short lineAddBlockFromName(struct sLine * const pLine, const char * const name); 
struct sBlock * lineGetBlockFrom(const struct sLine * const pLine, const unsigned short index);
void lineSetBlockFrom(struct sLine * pLine, struct sBlock * const pBlock, const unsigned short index);

#endif /* BAD_WALDSEE_LINES_H__ */

