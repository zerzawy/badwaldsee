/* 
 * This file is part of Raspberry Pi steuert Modelleisenbahn
 * Copyright(C)  2011 - 2015 Kurt Zerzawy www.zerzawy.ch
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
 * Functions for parsing and printing parameters of single switchboard LED 
 * \author
 * Kurt Zerzawy
 */

/*
 * Spreitenbacher Eisenbahn Amateur Klub SPEAK
 * www.speak.li
 *
 * $Author: kurt $
 * $Date: 2015-05-18 22:31:02 +0200 (Mo, 18 Mai 2015) $
 * $Revision: 2464 $
 */

#include "../constants.h"
#include "../log.h"
#include "../led.h"
#include "../registers.h"
#include "parser.h"
#include "led.h"

/**
 * read the LED properties
 * NOTE: this is done out of the register properties and NOT out of the configuration file directly
 */
void ledParser(void) 
{
	unsigned short	nrLed = 0;			/**< number of LEDs found */
	unsigned short	i;
	struct sLed *	ptr = NULL;
	char		name[NAMELEN];

	LOG_INF("started");
	/* first find out, how many LEDs are to be allocated */
	for(i = 0; i < 16 * (registerGetOutshiftLen() + registerGetInshiftLen() + registerGetVarLen()); i++)
	{
		registerGetName(i, name);
		if(ledNameIsLed(name))
		{
			nrLed++;
		}
	}

	led(nrLed);

	nrLed = 0;

	/* now connect the handles to the Leds	*/
	for(i = 0; i < 16 * (registerGetOutshiftLen() + registerGetInshiftLen() + registerGetVarLen()); i++)
	{
		registerGetName(i, name);
		if(ledNameIsLed(name))
		{
			ptr = ledGet(nrLed);
			ledSetHandle(ptr, i);
			nrLed++;
		}
	}

	LOG_INF("ended");
}

