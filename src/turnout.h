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
 * Functions for switches
 * Spreitenbacher Eisenbahn Amateur Klub SPEAK
 * www.speak.li
 *
 * $Author: kurt $
 * $Date: 2015-06-23 22:35:22 +0200 (Die, 23. Jun 2015) $
 * $Revision: 2490 $
 */

/* IMPORTANT NOTE FOR UNDERSTANDING
 * the turnout is named after the German convention Plus and Minus position
 * for use in Swiss Federal Railway, please see
 * Plus = Rechts
 * Minus = Links
 */

#ifndef BAD_WALDSEE_WEICHEN_H__
#define BAD_WALDSEE_WEICHEN_H__

#include "button.h"
#include "led.h"
#include "duoled.h"

enum eTurnoutLine {
	interlockMinusLR	= 0b01001111,
	sectionMinusLR		= 0b01000111,
	switchMinusLR		= 0b01000011,
	reserveMinusLR		= 0b01000001,
	interlockPlusLR		= 0b11001111,
	sectionPlusLR		= 0b11000111,
	switchPlusLR		= 0b11000011,
	reservePlusLR		= 0b11000001,
	turnoutNeutral		= 0b00000000,
	reservePlusRL		= 0b10100001,
	switchPlusRL		= 0b10100011,
	sectionPlusRL		= 0b10100111,
	interlockPlusRL		= 0b10101111,
	reserveMinusRL		= 0b00100001,
	switchMinusRL		= 0b00100011,
	sectionMinusRL		= 0b00100111,
	interlockMinusRL	= 0b00101111
};

enum eTurnoutProtectiveLine {
	interlockProtectMinus	= 0b00001111,
	sectionProtectMinus	= 0b00000111,
	switchProtectMinus	= 0b00000011,
	reserveProtectMinus	= 0b00000001,
	turnoutProtectNeutral	= 0b00000000,
	reserveProtectPlus	= 0b10000001,
	switchProtectPlus	= 0b10000011,
	sectionProtectPlus	= 0b10000111,
	interlockProtectPlus	= 0b10001111
};
		
#define LINEPLUS	0b10000000
#define TURNOUTDIRMASK	0b11100000

/**
 * States of the turnout 
 */
enum eTurnoutState 
{
	init,
	PlusIs,
	PlusSet,
	PlusErr,
	MinusIs,
	MinusSet,
	MinusErr
};

/**
 * the turnout struct is collecting all information necessary for 
 * a turnout.
 */
struct sTurnout 
{
	char		name[NAMELEN];		/**< name of the turnout */

	/*@observer@*/ struct sButton * pButton;	/**< button for controlling this turnout */
	unsigned short	BI_occupancyDetector;		/**< occupancy detector */
	_Bool		pointLeft;		/**< true if the point points to left */
	unsigned short	BI_contactPlus;		/**< feedback contact for Plus position */
	unsigned short	BI_contactMinus;	/**< feedback contact for Minus position */
	unsigned short	occupancyPlus;		/**< occupancy signal from neighbour on Plus side	*/
	unsigned short	occupancyMinus;		/**< occupancy signal from neighbour on Minus side	*/
	unsigned short	signalPlus;	/**< occupancy signal to neighbour on Plus side */
	unsigned short	signalMinus;	/**< occupancy signal to neighbour on Minus side */
	struct sDuoled * duoledPlus;	/**< duoled of the Plus switchboard indicator */
	struct sDuoled * duoledMinus;	/**< duoled of the Minus switchboard indicator */
	unsigned short	BO_motor;	/**< register of the motor */
	/*@null@*/ /*@temp@*/ struct sTurnout * pSlave;	/**< pointer to turnout contolled by this turnout */
	/*@null@*/ /*@temp@*/ struct sTurnout * pMaster;	/**< pointer to turnout contolling this turnout */	
	unsigned short	aspectPlus;	/**< signal aspect connected to Plus position */
	unsigned short	aspectMinus;	/**< signal aspect connected to Minus position */

	/* note on the following names. It is necessary to store the names during parsing of the
	 *  station because a turnout can be protective which is not known at this moment.
	 *  later the protective turnouts are stored as pointers for performance reasons
	 */
	char		protectivePlusName[NAMELEN];	/**< name of the protective turnout on Plus side */
	char		protectiveMinusName[NAMELEN];	/**< name of the protective turnout on Minus side */
	struct sTurnout * pProtectivePlus;	/**< pointer to the protective turnout on Plus side */
	struct sTurnout * pProtectiveMinus;	/**< pointer to the protective turnout on Minus side */
	_Bool		protectivePlus;		/**< true if on Plus side simple protective turnout */
	_Bool		protectiveMinus;	/**< true if on Minus side simple protective turnout */
	_Bool		protectiveTwinPlus;	/**< true if on Plus side twin protective turnout */
	_Bool		protectiveTwinMinus;	/**< true if on Minus side twin protective turnout */

	/* Electric line of the turnout */
    	enum eTurnoutLine	eLineActual;
    	enum eTurnoutLine	eLinePrev;
	enum eTurnoutProtectiveLine	eProtectiveLineActual;
	enum eTurnoutProtectiveLine	eProtectiveLinePrev;
    	enum eTurnoutState	eState;

	/* Feedback of the turnout contacts */
	_Bool fbPlus		: 1;
	_Bool wasFbPlus		: 1;
	_Bool fbMinus		: 1;
	_Bool wasFbMinus	: 1;
	
	/* Feedback of the occupancy detector */
	_Bool occupied		: 1;
	_Bool wasOccupied	: 1;
	_Bool occupied1		: 1;	/**< neighbour 1	*/
	_Bool wasOccupied1	: 1;
	_Bool occupied2		: 1;	/**< neighbour 2	*/
	_Bool wasOccupied2	: 1;
	
	_Bool free		: 1;
	_Bool toDo		: 1;
	unsigned short		mf;   /**< Monoflop */ 
};


/**
 * Here is the part of the software controlling the turnouts
 *
 * With the turnoutlogic you are able to 
 * 
 * \arg to set the turnout in dependency of a set of points
 * \arg to set the position of the turnout manually
 * \arg prevent that locked turnouts are set
 * \arg prevent that occupied turnouts are set
 * \arg to make correct display on the switchboard
 * \arg to make some emergency settings
 *
 * later the functions are made to accept set of points with requieries (I will call it lines)
 */

/*@maynotreturn@*/
void turnout(const unsigned short anzahl);
void turnoutDestroy(void);
void turnoutInit(void);
void turnoutOperateManually(struct sTurnout * const ptr);

_Bool turnoutLine(struct sTurnout * const pTurnout, 
		const enum eTurnoutLine	requestedLine);

_Bool turnoutProtectiveLine(struct sTurnout * const pTurnout,
		const enum eTurnoutProtectiveLine	requestedLine);

unsigned short turnoutNrButtons(struct sTurnout ** ppSel);

void turnoutProcess(void);

void turnoutReadHW(void);

_Bool turnoutIsFree(/*@null@*/ /*@in@*/ const struct sTurnout * const pTurnout);


/*@observer@*/ 
struct sTurnout * turnoutFindPtr(const char * const name);
struct sTurnout * turnoutGet(const unsigned short i);
unsigned short turnoutGetNr(void);

void turnoutSetName(struct sTurnout * const ptr, const char * const name); 
_Bool turnoutGetName(const struct sTurnout * const ptr, char * const name);

void turnoutSetButton(struct sTurnout * const ptr, struct sButton * const button);
struct sButton * turnoutGetButton(const struct sTurnout * const ptr);

void turnoutSetGroupRelaisRegister(const unsigned short handle);
unsigned short turnoutGetGroupRelaisRegister(void);

void turnoutSetOccupancyDetector(struct sTurnout * const ptr, const unsigned short handle);
unsigned short turnoutGetOccupancyDetector(const struct sTurnout * const ptr);

void turnoutSetContactPlus(struct sTurnout * const ptr, const unsigned short handle);
unsigned short turnoutGetContactPlus(const struct sTurnout * const ptr);

void turnoutSetContactMinus(struct sTurnout * const ptr, const unsigned short handle);
unsigned short turnoutGetContactMinus(const struct sTurnout * const ptr);

void turnoutSetOccupancyPlus(struct sTurnout * const ptr, const unsigned short handle);
unsigned short turnoutGetOccupancyPlus(const struct sTurnout * const ptr);

void turnoutSetOccupancyMinus(struct sTurnout * const ptr, const unsigned short handle);
unsigned short turnoutGetOccupancyMinus(const struct sTurnout * const ptr);

void turnoutSetSignalPlus(struct sTurnout * const ptr, const unsigned short handle);
unsigned short turnoutGetSignalPlus(const struct sTurnout * const ptr);

void turnoutSetSignalMinus(struct sTurnout * const ptr, const unsigned short handle);
unsigned short turnoutGetSignalMinus(const struct sTurnout * const ptr);

void turnoutSetDuoledPlus(struct sTurnout * const ptr, struct sDuoled * pLed);
struct sDuoled * turnoutGetDuoledPlus(const const struct sTurnout * const ptr);

void turnoutSetDuoledMinus(struct sTurnout * const ptr, struct sDuoled * pLed);
struct sDuoled * turnoutGetDuoledMinus(const struct sTurnout * const ptr);

void turnoutSetMotor(struct sTurnout * const ptr, unsigned short handle);
unsigned short turnoutGetMotor(const struct sTurnout * const ptr);

void turnoutSetSlave(struct sTurnout * const ptr, struct sTurnout * const pSlave);
struct sTurnout * turnoutGetSlave(const struct sTurnout * const ptr);

void turnoutSetMaster(struct sTurnout * const ptr, struct sTurnout * const pMaster);
struct sTurnout * turnoutGetMaster(const struct sTurnout * const ptr);

void turnoutSetPointLeft(struct sTurnout * const ptr, _Bool left);
_Bool turnoutIsPointLeft(const struct sTurnout * const ptr);

void turnoutSetAspectPlus(struct sTurnout * const ptr, unsigned short aspect);
unsigned short turnoutGetAspectPlus(const struct sTurnout * const ptr);

void turnoutSetAspectMinus(struct sTurnout * const ptr, unsigned short aspect);
unsigned short turnoutGetAspectMinus(const struct sTurnout * const ptr);

void turnoutSetProtectivePlusName(struct sTurnout * const ptr, const char * const name); 
_Bool turnoutGetProtectivePlusName(const struct sTurnout * const ptr, char * const name);

void turnoutSetProtectiveMinusName(struct sTurnout * const ptr, const char * const name); 
_Bool turnoutGetProtectiveMinusName(const struct sTurnout * const ptr, char * const name);

void turnoutSetProtectivePlusPtr(struct sTurnout * const ptr, const struct sTurnout * const protective);
struct sTurnout * turnoutGetProtectivePlusPtr(const struct sTurnout * const ptr);

void turnoutSetProtectiveMinusPtr(struct sTurnout * const ptr, const struct sTurnout * const protective);
struct sTurnout * turnoutGetProtectiveMinusPtr(const struct sTurnout * const ptr);

void turnoutSetProtectivePlus(struct sTurnout * const ptr, const _Bool state);
_Bool turnoutGetProtectivePlus(struct sTurnout * const ptr);

void turnoutSetProtectiveMinus(struct sTurnout * const ptr, const _Bool state);
_Bool turnoutGetProtectiveMinus(struct sTurnout * const ptr);

void turnoutSetProtectiveTwinPlus(struct sTurnout * const ptr, const _Bool state);
_Bool turnoutGetProtectiveTwinPlus(struct sTurnout * const ptr);

void turnoutSetProtectiveTwinMinus(struct sTurnout * const ptr, const _Bool state);
_Bool turnoutGetProtectiveTwinMinus(struct sTurnout * const ptr);

#endif /* BAD_WALDSEE_WEICHEN_H__ */

