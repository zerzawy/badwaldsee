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
 */

/*
 * \file
 * Functions for train routes 
 * \author
 * Kurt Zerzawy
 */

/*
 * functions for train routes 
 * Spreitenbacher Eisenbahn Amateur Klub SPEAK
 * www.speak.li
 * $Author: kurt $
 * $Date: 2015-06-07 17:35:47 +0200 (Son, 07. Jun 2015) $
 * $Revision: 2484 $
 */

#ifndef BAD_WALDSEE_ROUTE_H_
#define BAD_WALDSEE_ROUTE_H_

#include "mains.h"
#include "turnout.h"
#include "shuntroute.h"

#define NRROUTEPARTS       16

/**
 * enumeration for train route parts
 */
enum eRoutePartType 
{
	RoutePartNone,
	RoutePartTurnout,
	RoutePartShuntroute,
	RoutePartCondShuntroute,
	RoutePartCondOccupied,
	RoutePartIllegalRoute,
	RoutePartCondRoute,
	RoutePartLine,
	RoutePartSection
};

/**
 * enumeration for state machine
 */
enum eRouteState 
{
	RouteInit,
	RouteNone,
	RouteDestButton,
	RouteRegistered,
	RouteReserve,
	RouteSwitch,
	RouteEstablished,
	RouteBlocked
};

/**
 * Structure for 
 * train route parts
 */
struct sRoutePart
{
	enum eRoutePartType	type;
	char			name[NAMELEN];		/**< Name of this train route	*/
	unsigned short		argument;		/**< used e.g. for turnouts	*/
	void *			ptr;			/**< pointer to the main object	*/
	void *			ptr2;			/**< additional pointer for second object	*/
	_Bool			isOccupied;
	_Bool			wasOccupied;
};

/**
 * Structure for a train route 
 */
struct sRoute
{
	char			name[NAMELEN];		/**< Name of this train route	*/
	_Bool			toRight;	
	struct sMain *		start;			/**< pointer to the start dwarf	*/
	struct sMain *		dest;			/**< pointer to the destination dwarf	*/
	enum eRouteState	eState;			/**< state of the train route		*/
	_Bool			destButton;		/**< true if destination button pressed	*/
	_Bool			twoButtons;		/**< true if both buttons are pressed	*/
	_Bool			dissolve;		/**< true if the train route shall be cleared by hand	*/
	_Bool			toHalt;			/**< true if the shuntrouts signal felt back to halt	*/
	unsigned short		partsAddPosition;	/**< number of train route parts	*/
	struct sRoutePart 	pPart[NRROUTEPARTS]; 	/**< parts of which the route is made	*/
	struct sSection *	resetSection;		/**< part of the train route setting the mains to HP0 / halt	*/
	unsigned short		dissolveSection;	/**< position of the last turnout in the route for dissolving	*/
	unsigned short		occupiedPosition;	/**< position of the occupied element following to the train movement	*/
	unsigned short		nrDistants;		/**< number of distant signals (Vorsignale) of this route	*/
	_Bool			dissolveConditions;	/**< conditions for dissolving the route are given	*/
	_Bool			dissolveCondition1;	/**< condition that the train passed from beginning to end	*/
	_Bool			dissolveCondition2;	/**< condition that neighbours of blocks were occupied together	*/
	_Bool			dissolveCondition3;	/**< condition that the last block changes from occupied to free	*/
	short			headPosition;		/**< position of the head of the train following to the movement	*/
	short			tailPosition;		/**< position of the tail of the train following to the movement	*/
	unsigned short		dissolvePart;		/**< part of the train route dissolving the route	*/
	enum eMainAspect	mainAspect;		/**< aspect of the main signal for this route	*/
	enum eRouteState	eRequestedState;	/**< state requested by multi routes	*/
};


/*@maynotreturn@*/
void routes(const unsigned short anzahl);
void routesDestroy(void);
void routesInit(void);

unsigned short routesGetNr(void);
struct sRoute * routeGet(const unsigned short index);
struct sRoute * routeFind(const char * const name);

void routesProcess(void);

void routeCancel(const struct sMain const * pDest);
void routeOneButton(const struct sMain const * pMain);
void routeTwoButtons(const struct sMain const * pMain1, const struct sMain const * pMain2);

_Bool routeGetName(const struct sRoute * const pRoute, /*@out@*/char * const name);
void routeSetName(struct sRoute * const pRoute, const char * const name); 

_Bool routeGetToRight(const struct sRoute * const pRoute);
void routeSetToRight(struct sRoute * const pRoute, const _Bool toright); 

struct sMain * routeGetStart(const struct sRoute * pMain);
void routeSetStart(struct sRoute * const pMain, struct sMain * start);

struct sMain * routeGetDest(const struct sRoute * pMain);
void routeSetDest(struct sRoute * const pRoute, struct sMain * dest);

enum eMainAspect routeGetMainAspect(const struct sRoute * const pRoute);
void routeSetMainAspect(struct sRoute * const pRoute, enum eMainAspect aspect);

struct sSection * routeGetResetSection(const struct sRoute * const pRoute);
void routeSetResetSection(struct sRoute * const pRoute, struct sSection * const pSection);

unsigned short routeGetNrDistants(const struct sRoute * const pRoute);
void routeSetNrDistants(struct sRoute * const pRoute, const unsigned short numberDistants);

unsigned short routeGetParts(const struct sRoute * const pRoute);

unsigned short routeAddTurnout(struct sRoute * const pRoute, struct sTurnout * const pShuntroute, const enum eTurnoutLine line);
struct sTurnout * routeGetTurnout(const struct sRoute * const pRoute, const unsigned short index);
enum eTurnoutLine routeGetTurnoutArgument(const struct sRoute * const pRoute, const unsigned short index);

unsigned short routeAddShuntroute(struct sRoute * const pRoute, struct sShuntroute * const pShuntroute);
struct sShuntroute * routeGetShuntroute(const struct sRoute * const pRoute, const unsigned short index);

unsigned short routeAddCondShuntroute(struct sRoute * const pRoute, struct sShuntroute * const pShuntroute);
struct sShuntroute * routeGetCondShuntroute(const struct sRoute * const pRoute, const unsigned short index);

unsigned short routeAddCondOccupied(struct sRoute * const pRoute, struct sSection * const pSection);
struct sSection * routeGetCondOccupied(const struct sRoute * const pRoute, const unsigned short index);

unsigned short routeAddIllegalRoute(struct sRoute * const pRoute, const char * const name);
_Bool routeGetIllegalRouteName(const struct sRoute * const pRoute, /*@out@*/char * const name, const unsigned short index);
struct sRoute * routeGetIllegalRoute(const struct sRoute * const pRoute, const unsigned short index);
void routeSetIllegalRoute(struct sRoute * const pRoute, struct sRoute * const pIllegalRoute, const unsigned short index);

unsigned short routeAddCondRoute(struct sRoute * const pRoute, const char * const name);
_Bool routeGetCondRouteName(const struct sRoute * const pRoute, /*@out@*/char * const name, const unsigned short index);
struct sRoute * routeGetCondRoute(const struct sRoute * const pRoute, const unsigned short index);
void routeSetCondRoute(struct sRoute * const pRoute, struct sRoute * const pCondRoute, const unsigned short index);

unsigned short routeAddLine(struct sRoute * const pRoute, const char * const name, void * const ptr2);
_Bool routeGetLineName(const struct sRoute * const pRoute, /*@out@*/char * const name, const unsigned short index);
struct sLine * routeGetLine(const struct sRoute * const pRoute, const unsigned short index);
void * routeGetLinePtr2(const struct sRoute * const pRoute, const unsigned short index);
void routeSetLine(struct sRoute * const pRoute, struct sLine * const pLine, const unsigned short index);

unsigned short routeAddSection(struct sRoute * const pRoute, struct sSection * const pSection);
struct sSection * routeGetSection(const struct sRoute * const pRoute, const unsigned short index);

enum eRoutePartType routeGetPartType(const struct sRoute * const pRoute, const unsigned short index);

void routeCalcRoute(struct sRoute * const pRoute);
unsigned short routeGetDissolveSection(const struct sRoute * const pRoute);
unsigned short routeGetLastSection(const struct sRoute * const pRoute);

enum eRouteState routeGetState(const struct sRoute * const pRoute);
_Bool routeRequestState(struct sRoute * const pRoute, const enum eRouteState state);



#endif /* BAD_WALDSEE_ROUTE_H_ */
