/*
 * This file is part of Raspberry Pi steuert Modelleisenbahn
 * Copyright(C)  2008 - 2015 Kurt Zerzawy www.zerzawy.ch
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
 * functions for parsing and printing parameters of sections
 * \author
 * Kurt Zerzawy
 */

/*
 * $Author: kurt $
 * $Date: 2015-06-23 22:35:22 +0200 (Die, 23. Jun 2015) $
 * $Revision: 2490 $
 */

#include <assert.h>
#include "parser.h"
#include "../registers.h"
#include "../duoled.h"
#include "../section.h"
#include "section.h"

/*@maynotreturn@*/
static void sectionSingleParser(struct sSection * const ptr);

/**
 * Parser for one section
 * @param ptr	pointer to this section
 * @exception	Prints an error and exits on a case of wrong data in the 
 * 				parserfile
 */
/*@maynotreturn@*/
static void sectionSingleParser(struct sSection * const ptr) {
	/*@only@*/ /*@null@*/ struct sDuoled *  pLed = NULL;
	unsigned short	nrDuoled = 0;
	unsigned short	i;
	enum eToken     tok = tokNone;
	char            name[NAMELEN];

	assert(NULL != ptr);
	
	parserExpectTokenNeu(tokLBracket);
	parserExpectTokenNeu(tokBelegtmelder);
	sectionSetBI_occDetector(ptr, parserExpectRegister("Belegtmelder erwartet"));

	parserExpectTokenNeu(tokPlaettchen);
	nrDuoled = parserExpectNumber("Anzahl LEDs erwartet");
	parserAssert(NRBLOCKPL >= nrDuoled, "Anzahl LEDs zu gross");

	parserExpectTokenNeu(tokLBracket);

	/* weise die Werte zu */
	for(i = 0; i < nrDuoled; i++) 
	{
		tok = parserGetWord(name);
		parserAssert(tokNone == tok, "Plaettchenname erwartet");
		pLed = duoledFind(name);
		sectionAddDuoled(ptr, pLed);
	}
	parserExpectTokenNeu(tokRBracket);
	parserExpectTokenNeu(tokRBracket);
}

/*@maynotreturn@*/ 
void sectionsParser(void) {

	struct sSection *  ptr;
	unsigned short  anzahl = 0;
	unsigned short  i = 0;
	enum eToken     tok = tokNone;
	char            name[NAMELEN];
 
	parserExpectTokenNeu(tokLBracket);
	parserExpectTokenNeu(tokAbschnitte);
	anzahl = parserExpectNumber("Anzahl Abschnitte erwartet");
	sections(anzahl);	/* construct section structure */

	parserExpectTokenNeu(tokLBracket);
	while(NULL != (ptr = sectionGet(i)))
	{
		tok = parserGetWord(name);
		parserAssert(tokNone == tok, "Abschnittname erwartet");
		sectionSetName(ptr, name);
		sectionSingleParser(ptr);
		i++;
	}

	tok = parserGetToken();
	parserAssert(tokRBracket == tok, "zu viele Abschnitte angegeben");
	/* I did not use parserExpectToken, because a better error 
	 * message can be given
	 */

	parserExpectTokenNeu(tokRBracket);
}

/**
 * Prints the single parts of the section
 * @param ptr		pointer to the section to be printed
 * @exceptioni		Assert on NULL pointer
 */
static void sectionSinglePrint(struct sSection * const ptr);
static void sectionSinglePrint(struct sSection * const ptr) 
{
	char			name[NAMELEN];
	struct sDuoled *	pDuoled;
	unsigned short		i;

	/* calculate the number of duoleds for display	*/
	i = 0;
	while(NULL != sectionGetDuoled(ptr, i))
	{
		i++;
	}

	sectionGetName(ptr, name);
	fprintf(stdout, "\t\t\t%s {\n", name);
	registerGetName(sectionGetBI_occDetector(ptr), name);
	fprintf(stdout, "\t\t\t\tBelegtMelder\t%s\n", name);
	fprintf(stdout, "\t\t\t\tPlaettchen\t%hu\n", i);
	fprintf(stdout, "\t\t\t\t{ ");
	
	i = 0;
	while(NULL != (pDuoled = sectionGetDuoled(ptr, i)))
	{
		duoledGetName(pDuoled, name);
		fprintf(stdout, "%s ", name);
		i++;
	}
	fprintf(stdout, "}\n");
	fprintf(stdout, "\t\t\t}\n");
}

void sectionsPrint(void) 
{
	unsigned short		i = 0;
	struct sSection *	ptr;

	fprintf(stdout, "\t{\n");
	fprintf(stdout, "\t\tAbschnitte %u {\n", sectionsGetNr());
	while(NULL != (ptr = sectionGet(i)))
	{
		sectionSinglePrint(ptr);
		i++;
	}
	fprintf(stdout, "\t\t}\n");
	fprintf(stdout, "\t}\n");
}

