/*
 * This file is part of Raspberry Pi steuert Modelleisenbahn
 * Copyright(C)  2011 - 2014 Kurt Zerzawy www.zerzawy.ch
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
 * $Author: kurt $
 * $Date: 2015-05-18 22:31:02 +0200 (Mon, 18. Mai 2015) $
 * $Revision: 2464 $
 */


#ifndef BAD_WALDSEE_DUOLED_H__
#define BAD_WALDSEE_DUOLED_H__

#include <stdbool.h>
#include "led.h"

/**
 * Structure of a duoled
 *
 * A dual LED consists of the pointers to the two LEDs for the display
 * and the mode to be shown on this duoled.
 *
 * The occupied flag memorises if the Duoled shall show red or not.
 */
struct sDuoled 
{
	struct sLed * 	pYellow;	/**< Pointer to yellow / white LED */
	struct sLed *	pRed;		/**< Pointer to red LED	*/
	enum eLedMode	mode;		/**< Behaviour of duoled	*/
	_Bool 			occupied;	/**< true when red LED shall be on	*/
};

/*@maynotreturn@*/
void duoled(const unsigned short nrDuoleds);
void duoledDestroy(void);
void duoledInit(void);
unsigned short duoledGetNr(void);

struct sDuoled * duoledAdd(struct sLed * const yellowLED,
							struct sLed * const redLED);

void duoledSetOccupied(struct sDuoled * const ptr,
						const _Bool occupied);

void duoledSetMode(struct sDuoled * const ptr,
							const enum eLedMode mode);
enum eLedMode duoledGetMode(const struct sDuoled * const ptr);

_Bool duoledGetName(struct sDuoled * const ptr, char * name);
_Bool duoledLedIsDuoled(const struct sLed * const handle);

_Bool duoledIsDuoledName(const char * const name);
	
struct sDuoled * duoledFind(const char * const name);

struct sLed * duoledGetYellow(const char * const name);
struct sLed * duoledGetRed(const char * const name);

_Bool duoledCompareEnd(const char * const string,
							const char * const pattern);

#endif /* BAD_WALDSEE_DUOLED_H__ */

