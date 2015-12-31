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
 *
 * Spreitenbacher Eisenbahn Amateur Klub SPEAK
 * www.speak.li
 */

/**
 * \file
 * Functions for parsing and printing parameters of shunting routes combined from several single shunting routes
 * \author 
 * Kurt Zerzawy
 */

/*
 * $Author: kurt $
 * $Date: 2015-05-18 22:31:02 +0200 (Mo, 18 Mai 2015) $
 * $Revision: 2464 $
 */

#include <assert.h>
#include "../log.c"
#include "parser.h"
#include "../combishuntroute.h"
#include "combishuntroute.h"


/*@maynotreturn@*/
static void combishuntrouteSingleParser(struct sCombiShuntroute * const ptr);

/**
 * Parser for one shuntroute
 * @param ptr	pointer to this shuntroute 
 * @exception	Prints an error and exits on a case of wrong data in the 
 * 		parserfile
 */
/*@maynotreturn@*/
static void combishuntrouteSingleParser(struct sCombiShuntroute * const ptr)
{
	return;
}

/*@maynotreturn@*/ 
void combishuntrouteParser(void)
{
	struct sCombiShuntroute *	ptr;
	unsigned short			nr = 0;
	unsigned short			i = 0;
	enum eToken			tok = tokNone;
	char				name[NAMELEN];
 
	parserExpectTokenNeu(tokLBracket);
	parserExpectTokenNeu(tokKombiRaFas);
	nr = parserExpectNumber("Anzahl KombiRangierfahrstrassen erwartet");
	combishuntroute(nr);		/* construct combi shuntroute structure */

	parserExpectTokenNeu(tokLBracket);
	while(NULL != (ptr = combishuntrouteGet(i)))
	{
		tok = parserGetWord(name);
		parserAssert(tokNone == tok, "Name der KombiRangierfahrstrasse erwartet");

		combishuntrouteSetName(ptr, name);
		combishuntrouteSingleParser(ptr);
		i++;
	}

	tok = parserGetToken();
	parserAssert(tokRBracket == tok, "zu viele KombiRangierfahrstrassen angegeben");
	/* I did not use parserExpectToken, because a better error 
	 * message can be given
	 */

	parserExpectTokenNeu(tokRBracket);
}

/**
 * Prints the single combi shuntroute
 * @param ptr   pointer to the combi shuntroute to be printed
 * @exception   Assert on NULL pointer
 */
static void combishuntrouteSinglePrint(struct sCombiShuntroute * const ptr);
static void combishuntrouteSinglePrint(struct sCombiShuntroute * const ptr)
{
	;
}

void combishuntroutePrint(void) 
{
	unsigned short			i = 0;
	struct sCombiShuntroute *	ptr;

	fprintf(stdout, "\t{\n");
	fprintf(stdout, "\t\tKombiRaFas %u {\n", combishuntrouteGetNr());
	while(NULL != (ptr = combishuntrouteGet(i)))
	{
		combishuntrouteSinglePrint(ptr);
		i++;
	}
	fprintf(stdout, "\t\t}\n");
	fprintf(stdout, "\t}\n");
}

