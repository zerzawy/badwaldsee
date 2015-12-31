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

/* Function for LEDs
 * Bad Waldsee switchboard
 * Spreitenbacher Eisenbahn Amateur Klub SPEAK
 * www.speak.li
 *
 * $Author: Kurt $
 * $Date: 2015-08-05 08:06:23 +0200 (Mi, 05 Aug 2015) $
 * $Revision: 2537 $
 */

/* 
 * \author Kurt Zerzawy
 *
 */


#ifndef LED_H_
#define LED_H_

#include <stdbool.h>

enum eLedMode
{
	ledOn,
	ledOff,
	ledAllOff,
	ledBlinking,
	ledBlankable
};

/**
 * structur of a LED
 *
 * a LED is defined by the mode, so the behaviour it has.
 */
struct sLed 
{
	unsigned short	BO_handle;
	enum eLedMode	mode;
};

/*@maynotreturn@*/
void led(const unsigned short nrLed);
void ledDestroy(void);
unsigned short ledGetNr(void);
struct sLed * ledGet(const unsigned short i);
struct sLed * ledFind(const char * const name);
struct sLed * ledGetFirstPtr(void);
void ledProcess(void);

void ledSet(struct sLed * ptr, const enum eLedMode mode);
enum eLedMode ledGetMode(struct sLed * ptr);
void ledSetBlinkClock(const _Bool on);
void ledShow(const _Bool on);

_Bool ledNameIsLed(const char * const name);
_Bool ledGetName(const struct sLed * const ptr, char * const name);
_Bool ledIsLed(const struct sLed * const ptr);

void ledSetHandle(struct sLed * const ptr, const unsigned short handle);
unsigned short ledGetHandle(const struct sLed * const ptr);

#endif /* LED_H_ */
