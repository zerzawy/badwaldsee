/*
 * This file is part of Raspberry Pi steuert Modelleisenbahn
 * Copyright(C) 2008 - 2015 Kurt Zerzawy www.zerzawy.ch
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
 * Functions for parsing and printing parameters of automats (Einspurstrecken) 
 * \author
 * Kurt Zerzawy
 */

/*
 * $Author: kurt $
 * $Date: 2015-06-04 23:46:10 +0200 (Don, 04. Jun 2015) $
 * $Revision: 2480 $
 */

#include <assert.h>
#include "../log.c"
#include "../registers.h"
#include "../mains.h"
#include "parser.h"
#include "../automat.h"
#include "automat.h"
#include "mains.h"

/**
 * Parser for one automat
 * @param pAutomat	pointer to this automat 
 * @exception		Prints an error and exits on a case of wrong data in the 
 * 			parserfile
 */
/*@maynotreturn@*/
static void automatSingleParser(struct sAutomat * const pAutomat);
/*@maynotreturn@*/
static void automatSingleParser(struct sAutomat * const pAutomat)
{
	unsigned short	i;
	unsigned short	sections	= 0;
	unsigned short	automats	= 0;
	unsigned short	routes		= 0;
	enum eToken     tok	= tokNone;
	char            name[NAMELEN];

	assert(NULL != pAutomat);
	
	parserExpectTokenNeu(tokLBracket);

	#ifdef DOMINO55
		parserExpectTokenNeu(tokTasteEin);
		(void) parserExpectName(name, "Tastenname von Taste Ein erwartet");
		automatSetOnButton(pAutomat, buttonFind(name));

		parserExpectTokenNeu(tokTasteAus);
		(void) parserExpectName(name, "Tastenname von Taste Aus erwartet");
		automatSetOffButton(pAutomat, buttonFind(name));
	#endif

	parserExpectTokenNeu(tokLEDein);
	automatSet_BO_DeskLED(pAutomat, parserExpectRegister("Name der LEDein erwartet"));

	parserExpectTokenNeu(tokAbschnitte);
	sections = parserExpectNumber("Anzahl Abschnitte erwartet");
	parserAssert(AUTOMATNRSECTIONS > sections, "Zu viele Abschnitte angegeben");
	parserAssert(0 < sections, "Mindestens ein Abschnitt notwendig");
	parserExpectTokenNeu(tokLBracket);

	for(i = 0; i < sections; i++) 
	{
		tok = parserGetWord(name);
		parserAssert(tokNone == tok, "Name des Abschnitts erwartet");
		automatAddSection(pAutomat, sectionFind(name));
	}

	tok = parserGetToken();
	parserAssert(tokNone != tok, "zu viele Abschnitte angegeben");

	/* now see if also routes are used to start the automat	*/
	if(tokZuFa == tok)
	{
		/* now routes are defined	*/
		routes = parserExpectNumber("Anzahl Zugfahrstrassen erwartet");
		parserAssert(AUTOMATNRROUTES > routes, "Zu viele Zugfahrstrasse angegeben");
		parserAssert(0 < routes, "Mindestens eine Zugfahrstrasse notwendig");
		parserExpectTokenNeu(tokLBracket);

		for(i = 0; i < routes; i++)
		{
			tok = parserGetWord(name);
			parserAssert(tokNone == tok, "Name der Zugfahrstrasse erwartet");
			automatAddRoute(pAutomat, routeFind(name));
		}

		tok = parserGetToken();
		parserAssert(tokRBracket == tok, "zu viele Zugfahrstrassen angegeben");
	}

	parserExpectTokenNeu(tokRBracket);
}

/**
 * Function to parse a automat from a .conf file
 */
/*@maynotreturn@*/ 
void automatParser(void)
{
	struct sAutomat *		pAutomat = NULL;
	unsigned short		nr = 0;
	unsigned short		i = 0;
	enum eToken		tok = tokNone;
	char			name[NAMELEN];
 
	parserExpectTokenNeu(tokLBracket);
	parserExpectTokenNeu(tokAutomat);
	nr = parserExpectNumber("Anzahl Automaten erwartet");
	automats(nr);		/* construct automat structure */

	parserExpectTokenNeu(tokLBracket);
	while(NULL != (pAutomat = automatGet(i)))
	{
		tok = parserGetWord(name);
		parserAssert(tokNone == tok, "Name des Automats erwartet");
		automatSetName(pAutomat, name);
		automatSingleParser(pAutomat);
		i++;
	}

	tok = parserGetToken();
	parserAssert(tokRBracket == tok, "zu viele Automaten angegeben");
	/* I did not use parserExpectToken, because a better error 
	 * message can be given
	 */

	parserExpectTokenNeu(tokRBracket);
}

/**
 * Prints the single automat
 * @param pAutomat	pointer to the automat to be printed
 * @exception		Assert on NULL pointer
 */
static void automatSinglePrint(struct sAutomat * const pAutomat);
static void automatSinglePrint(struct sAutomat * const pAutomat) 
{
	char			name[NAMELEN];
	unsigned short		i;
	struct sSection *	pSection	= NULL;
	struct sRoute *		pRoute		= NULL;

	assert(NULL != pAutomat);

	automatGetName(pAutomat, name);
	fprintf(stdout, "\t\t\t%s {\n", name);
	
	#ifdef DOMINO55
		buttonGetName(automatGetOnButton(pAutomat), name);
		fprintf(stdout, "\t\t\t\tTasteEin\t\t%s", name);

		buttonGetName(automatGetOffButton(pAutomat), name);
		fprintf(stdout, "\t\t\t\tTasteAus\t\t%s", name);

	#endif
	registerGetName(automatGet_BO_DeskLED(pAutomat), name);
	fprintf(stdout, "\t\t\t\tLEDein\t%s\n", name);

	/* count number of sections	*/
	i = 0;
	while(NULL != automatGetSection(pAutomat, i))
	{
		i++;
	}
	
	fprintf(stdout, "\t\t\t\tAbschnitte\t%d {\n", i);

	fprintf(stdout, "\t\t\t\t\t");
	while(NULL != (pSection = automatGetSection(pAutomat, i)))
	{
		sectionGetName(pSection, name);
		fprintf(stdout, "%s ", name);
	}

	fprintf(stdout, "\t\t\t\t}\n");

	/* count number of routes	*/
	i = 0;
	while(NULL != automatGetRoute(pAutomat, i))
	{
		i++;
	}

	if(0 < i)
	{
		fprintf(stdout, "\t\t\t\tZuFa\t%d {\n", i);

		fprintf(stdout, "\t\t\t\t\t");
		while(NULL != (pRoute = automatGetRoute(pAutomat, i)))
		{
			routeGetName(pRoute, name);
			fprintf(stdout, "%s ", name);
		}

		fprintf(stdout, "\t\t\t\t}\n");
	}
	fprintf(stdout, "\t\t\t}\n");
}

/**
 * Function to print out an automat
 * used to check the parser function
 */
void automatPrint(void) 
{
	unsigned short	i = 0;
	struct sAutomat *	pAutomat = NULL;

	fprintf(stdout, "\t{\n");
	fprintf(stdout, "\t\tAutomat %u {\n", automatsGetNr());
	while(NULL != (pAutomat = automatGet(i)))
	{
		automatSinglePrint(pAutomat);
		i++;
	}
	fprintf(stdout, "\t\t}\n");
	fprintf(stdout, "\t}\n");
}
