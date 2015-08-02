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
 */

/*
 * functions for shunting routes 
 * Spreitenbacher Eisenbahn Amateur Klub SPEAK
 * www.speak.li
 * $Author: kurt $
 * $Date: 2015-06-23 22:35:22 +0200 (Die, 23. Jun 2015) $
 * $Revision: 2490 $
 */

#ifndef BAD_WALDSEE_SHUNTROUTE_H_
#define BAD_WALDSEE_SHUNTROUTE_H_

#include "dwarf.h"
#include "turnout.h"
#include "section.h"

#define NRSHUNTPARTS       16

/**
 * enumeration for shuntroute parts
 */
enum eShuntroutePartType
{
	ShuntroutePartNone,
	ShuntroutePartTurnout,
	ShuntroutePartProtectiveTurnout,
	ShuntroutePartSection,
	ShuntroutePartProtectiveSection,
	ShuntroutePartProtectiveDwarf
};

/**
 * enumeration for state machine
 */
enum eShuntrouteState
{
	ShuntrouteInit,
	ShuntrouteNone,
	ShuntrouteDestButton,
	ShuntrouteRegistered,
	ShuntrouteReserve,
	ShuntrouteSwitch,
	ShuntrouteEstablished,
	ShuntrouteBlocked,
	ShuntrouteTurntoutsPassed,
	ShuntrouteError,
	ShuntrouteRouteError
};

/**
 * Structure for 
 * shuntroute parts
 */
struct sShuntroutePart
{
	enum eShuntroutePartType	type;
	unsigned short			argument;	/**< used e.g. for turnouts	*/
	void *				ptr;
	_Bool				isOccupied;
	_Bool				wasOccupied;
};

/**
 * Structure for a shuntroute 
 */
struct sShuntroute
{
	char			name[NAMELEN];		/**< Name of this shuntroute	*/
	_Bool			toRight;	
	struct sDwarf *		start;			/**< pointer to the start dwarf	*/
	struct sDwarf *		dest;			/**< pointer to the destination dwarf	*/
	enum eShuntrouteState	eState;			/**< state of the shuntroute		*/
	_Bool			destButton;		/**< true if destination button pressed	*/
	_Bool			twoButtons;		/**< true if both buttons are pressed	*/
	_Bool			dissolve;		/**< true if the shuntroute shall be cleared by hand	*/
	_Bool			toHalt;			/**< true if the shuntrouts signal felt back to halt	*/
	unsigned short		PartsAddPosition;	/**< number of shuntroute parts	*/
	struct sShuntroutePart 	pPart[NRSHUNTPARTS]; 	/**< parts of which the route is made	*/
	unsigned short		resetSection;		/**< section telling that dwarf shall be set to Halt / Sh0	*/
	unsigned short		dissolveSection;		/**< position of the last turnout in the shuntroute for dissolving	*/
	unsigned short		lastSection;		/**< position of the last part passed when driving throu the complete shuntroute, used for routes	*/
	short			headPosition;		/**< position of the head of the train following to the movement	*/
	short			tailPosition;		/**< position of the tail of the train following to the movement	*/
	_Bool			dissolveConditions;	/**< conditions for dissolving the route are given	*/
	_Bool			dissolveCondition1;	/**< condition that the train passed from beginning to end	*/
	_Bool			dissolveCondition2;	/**< condition that neighbours of sections were occupied together	*/
	_Bool			dissolveCondition3;	/**< condition that the last section changes from occupied to free	*/
	enum eShuntrouteState	eRequestedState;	/**< state requested from route	*/
};


/*@maynotreturn@*/
void shuntroute(const unsigned short anzahl);
void shuntrouteDestroy(void);
void shuntrouteInit(void);
void shuntrouteProcess(void);

struct sShuntroute * shuntrouteFind(const char * const name);
struct sShuntroute * shuntrouteGet(const unsigned short i);
unsigned short shuntrouteGetNr(void);

_Bool shuntrouteIsFree(struct sShuntroute * const pShuntroute);

void shuntrouteTwoButtons(const struct sDwarf const * pDwarf1, const struct sDwarf const * pDwarf2);

#ifdef SPDR60
	void shuntrouteCancel(const struct sDwarf const * pDwarf1, const struct sDwarf const * pDwarf2);
#endif

#ifdef DOMINO55
	void shuntrouteCancel(const struct sDwarf const * pDest);
	void shuntrouteOneButton(const struct sDwarf const * pDwarf);
#endif

_Bool shuntrouteGetName(const struct sShuntroute * const pShuntroute, char * const name);
void shuntrouteSetName(struct sShuntroute * const pShuntroute, const char * const name);

struct sShuntroute * shuntrouteGet(const unsigned short i);

_Bool shuntrouteGetName(const struct sShuntroute * const pShuntroute, /*@out@*/char * const name);
void shuntrouteSetName(struct sShuntroute * const pShuntroute, const char * const name); 

_Bool shuntrouteGetToRight(const struct sShuntroute * const pShuntroute);
void shuntrouteSetToRight(struct sShuntroute * const pShuntroute, const _Bool toright); 

struct sDwarf * shuntrouteGetStart(const struct sShuntroute * pShuntroute);
void shuntrouteSetStart(struct sShuntroute * const pShuntroute, struct sDwarf * start);

struct sDwarf * shuntrouteGetDest(const struct sShuntroute * pShuntroute);
void shuntrouteSetDest(struct sShuntroute * const pShuntroute, struct sDwarf * dest);

unsigned short shuntrouteGetParts(const struct sShuntroute * const pShuntroute);

unsigned short shuntrouteAddTurnout(struct sShuntroute * const pShuntroute, struct sTurnout * const pTurnout, const enum eTurnoutLine line);
struct sTurnout * shuntrouteGetTurnout(const struct sShuntroute * const pShuntroute, const unsigned short i);
enum eTurnoutLine shuntrouteGetTurnoutArgument(const struct sShuntroute * const pShuntroute, const unsigned short i);

unsigned short shuntrouteAddProtectiveTurnout(struct sShuntroute * const pShuntroute, struct sTurnout * const pTurnout, const enum eTurnoutProtectiveLine line);
struct sTurnout * shuntrouteGetProtectiveTurnout(const struct sShuntroute * const pShuntroute, const unsigned short i);
enum eTurnoutProtectiveLine shuntrouteGetProtectiveTurnoutArgument(const struct sShuntroute * const pShuntroute, const unsigned short i);

unsigned short shuntrouteAddSection(struct sShuntroute * const pShuntroute, struct sSection * const pSection);
struct sSection * shuntrouteGetSection(const struct sShuntroute * const pShuntroute, const unsigned short i);

unsigned short shuntrouteAddProtectiveSection(struct sShuntroute * const pShuntroute, struct sSection * const pSection);
struct sSection * shuntrouteGetProtectiveSection(const struct sShuntroute * const pShuntroute, const unsigned short i);

unsigned short shuntrouteAddProtectiveDwarf(struct sShuntroute * const pShuntroute, struct sDwarf * const pDwarf);
struct sDwarf * shuntrouteGetProtectiveDwarf(const struct sShuntroute * const pShuntroute, const unsigned short i);

enum eShuntroutePartType shuntrouteGetPartType(const struct sShuntroute * const pShuntroute, const unsigned short i);

unsigned short shuntrouteGetResetSection(const struct sShuntroute * const pShuntroute);
void shuntrouteSetResetSection(struct sShuntroute * const pShuntroute, unsigned short sectionNr);

void shuntrouteCalcRoute(struct sShuntroute * const pShuntroute);
unsigned short shuntrouteGetDissolveSection(const struct sShuntroute * const pShuntroute);
unsigned short shuntrouteGetLastSection(const struct sShuntroute * const pShuntroute);

enum eShuntrouteState shuntrouteGetState(const struct sShuntroute * const pShuntroute);
_Bool shuntrouteRequestState(struct sShuntroute * const pShuntroute, const enum eShuntrouteState state);

#endif /* BAD_WALDSEE_SHUNTROUTE_H_ */
