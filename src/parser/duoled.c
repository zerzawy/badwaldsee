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
 * Functions for parsing and printing parameters of single switchboard LEDs
 * \author
 * Kurt Zerzawy
 */

/*
 * Functions for the leds of the switchboard 
 * Spreitenbacher Eisenbahn Amateur Klub SPEAK
 * www.speak.li
 *
 * $Author: kurt $
 * $Date: 2015-05-18 22:31:02 +0200 (Mo, 18 Mai 2015) $
 * $Revision: 2464 $
 */

#include "../constants.h"
#include "../log.h"
#include "parser.h"
#include "../duoled.h"
#include "duoled.h"

void duoledParser(void) 
{
	unsigned short 	NrYellow = 0;
	unsigned short 	NrRed = 0;
	unsigned short	NrLeds = 0;
	unsigned short	i;
	char		name[NAMELEN];
	struct sLed *	pLed = NULL;

	LOG_INF("started");
	/* count the number of duoleds */
	NrLeds = ledGetNr();
	for(i = 0; i < NrLeds; i++)
	{
		if(ledGetName(ledGet(i), name))
		{
			if(duoledIsDuoledName(name))
			{	
				if(0 == strcmp("D_", name))
				{
					parserAssert(false, "Leere Duoled Namen (DLED_ )");
				}
				if(duoledCompareEnd(name, "_gelb"))
				{
					NrYellow++;
				}
				if(duoledCompareEnd(name, "_rot"))
				{
					NrRed++;
				}
			}
		}
	}
	parserAssert((NrYellow == NrRed), 
					"ungleiche Anzahl gelber und roter Duoleds");

	duoled(NrRed);
	/* now fill up the information by adding duoleds */

	for(i = 0; i < NrLeds; i++)
	{
		pLed = ledGet(i);
		if(NULL != pLed)
		{
			if(ledGetName(ledGet(i), name))
			{
				if(duoledIsDuoledName(name))
				{	
					if(duoledCompareEnd(name, "_gelb"))
					{
						memmove(name, name+2, strlen(name)-1);
						name[strlen(name)-5] = '\0';
						duoledAdd(duoledGetYellow(name),
										duoledGetRed(name));
					}
				}
			}
		}
	}
	LOG_INF("ended");
}
