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
 * Functions for parsing and printing parameters of routes combined from several single routes
 * \author 
 * Kurt Zerzawy
 */

/*
 * $Author: kurt $
 * $Date: 2015-05-09 21:54:18 +0200 (Sam, 09. Mai 2015) $
 * $Revision: 2441 $
 */

#include <assert.h>
#include "../log.c"
#include "parser.h"
#include "../route.h"
#include "../combiroute.h"
#include "combiroute.h"


/*@maynotreturn@*/
static void combirouteSingleParser(struct sCombiroute * const pCombiroute);

/**
 * Parser for one combiroute
 * @param pCombiroute	pointer to this combi route 
 * @exception	Prints an error and exits on a case of wrong data in the 
 * 		parserfile
 */
/*@maynotreturn@*/
static void combirouteSingleParser(struct sCombiroute * const pCombiroute)
{
	enum eToken		tok = tokNone;
	char			name[NAMELEN];
	unsigned short		nrParts;
	unsigned short		i;

	parserExpectTokenNeu(tokLBracket);

	parserExpectTokenNeu(tokStartsignal);
	tok = parserGetWord(name);
	parserAssert(tokNone == tok, "Name des Startsignals erwartet");
	combirouteSetStart(pCombiroute, mainFind(name));

	parserExpectTokenNeu(tokZielsignal);
	tok = parserGetWord(name);
	parserAssert(tokNone == tok, "Name des Zielsignals erwartet");
	combirouteSetDest(pCombiroute, mainFind(name));

/* TODO Umfahrstrassen	*/

	parserExpectTokenNeu(tokZuFas);
	nrParts = parserExpectNumber("Anzahl der ZuFa-Teile erwartet");

	parserAssert((nrParts < NRCOMBIPARTS), "Zu viele ZuFa-Teile angegeben");

	parserExpectTokenNeu(tokLBracket);
	for(i = 0; i < nrParts; i++)
	{
		tok = parserGetWord(name);
		parserAssert(tokNone == tok, "Name des ZuFa-Teils erwartet");
		(void) combirouteAddRoute(pCombiroute, routeFind(name));
	}
	parserExpectTokenNeu(tokRBracket);

/* TODO mehrere Möglichkeiten angeben für Umfahrstrassen	*/
	parserExpectTokenNeu(tokRBracket);
	return;
}

/*@maynotreturn@*/ 
void combirouteParser(void)
{
	struct sCombiroute *	pCombiroute;
	unsigned short		nr = 0;
	unsigned short		i = 0;
	enum eToken		tok = tokNone;
	char			name[NAMELEN];
 
	parserExpectTokenNeu(tokLBracket);
	parserExpectTokenNeu(tokKombiZuFas);
	nr = parserExpectNumber("Anzahl KombiFahrstrassen erwartet");
	combiroutes(nr);	/* construct route structure */

	parserExpectTokenNeu(tokLBracket);
	while(NULL != (pCombiroute = combirouteGet(i)))
	{
		tok = parserGetWord(name);
		parserAssert(tokNone == tok, "Name der Kombifahrstrasse erwartet");

		combirouteSetName(pCombiroute, name);
		combirouteSingleParser(pCombiroute);
		i++;
	}

	tok = parserGetToken();
	parserAssert(tokRBracket == tok, "zu viele Kombifahrstrassen angegeben");
	/* I did not use parserExpectToken, because a better error 
	 * message can be given
	 */

	parserExpectTokenNeu(tokRBracket);
}

/**
 * Prints the single combi route
 * @param pCombiroute	pointer to the combi route to be printed
 * @exception		Assert on NULL pointer
 */
static void combirouteSinglePrint(struct sCombiroute * const pCombiroute);
static void combirouteSinglePrint(struct sCombiroute * const pCombiroute)
{
	char		name[NAMELEN];
	struct sRoute * pRoute = NULL;
	unsigned short	i = 0;

	combirouteGetName(pCombiroute, name);
	fprintf(stdout, "\t\t\t%s {\n", name);

	mainGetName(combirouteGetStart(pCombiroute), name);
	fprintf(stdout, "\t\t\t\tStartsignal\t%s\n", name);

	mainGetName(combirouteGetDest(pCombiroute), name);
	fprintf(stdout, "\t\t\t\tZielsignal\t%s\n", name);

	/* TODO Umfahrstrassen	*/

	fprintf(stdout, "\t\t\t\tZuFas\t\t%d {\n", combirouteGetNrRoutes(pCombiroute));
	
	while(NULL != (pRoute = combirouteGetRoute(pCombiroute, i)))
	{
		routeGetName(pRoute, name);
		fprintf(stdout, "\t\t\t\t\t%s\n", name);
		i++;
	}

	fprintf(stdout, "\t\t\t\t}\n");
	fprintf(stdout, "\t\t\t}\n");
}

void combiroutePrint(void) 
{
	unsigned short		i = 0;
	struct sCombiroute *	pCombiroute;

	fprintf(stdout, "\t{\n");
	fprintf(stdout, "\t\tKombiZuFas %u {\n", combiroutesGetNr());
	while(NULL != (pCombiroute = combirouteGet(i)))
	{
		combirouteSinglePrint(pCombiroute);
		i++;
	}
	fprintf(stdout, "\t\t}\n");
	fprintf(stdout, "\t}\n");
}

