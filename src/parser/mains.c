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
 * Functions for parsing and printing parameters of main signals (Hauptsignale) 
 * \author
 * Kurt Zerzawy
 */

/*
 * $Author: kurt $
 * $Date: 2015-05-20 22:24:56 +0200 (Mi, 20 Mai 2015) $
 * $Revision: 2468 $
 */

#include <string.h>
#include <assert.h>
#include "../registers.h"
#include "../button.h"
#include "../mains.h"
#include "parser.h"
#include "mains.h"

#ifdef SPDR60
	/*** 
	 * parser for a single mains
	 * @param pMain	pointer to the mains
	 * @exception	warning and exit on error of the given parameters
	 */
	/*@maynotreturn@*/ 
	static void mainsSingleParser(/*@temp@*/ struct sMain * const pMain);
	static void mainsSingleParser(/*@temp@*/ struct sMain * const pMain) 
	{
		char 		name[NAMELEN];
		enum eToken	tok = tokNone;
		unsigned short	nrDistant = 0;

		parserExpectTokenNeu(tokLBracket);

		parserExpectTokenNeu(tokTaste);  
		(void) parserExpectName(name, "Tastenname erwartet");
		mainSetBI_Button(pMain, buttonFind(name));

		parserExpectTokenNeu(tokGruen);
		mainSetBO_Green(pMain, parserExpectRegister("Name der grünen Signal-LED erwartet"));

		parserExpectTokenNeu(tokRot);
		mainSetBO_Red(pMain, parserExpectRegister("Name der roten Signal-LED erwartet"));

		/* now get the next token	*/
		tok = parserGetWord(name);
		if(tokGelb == tok)
		{
			mainSetBO_Yellow(pMain, parserExpectRegister("Name der gelben Signal-LED erwartet"));		
			tok = parserGetWord(name);
		}
		if(tokZs3 == tok)
		{
			mainSetBO_Zs3(pMain, parserExpectRegister("Name der Zs3 LED erwartet"));
			tok = parserGetWord(name);
		}
		if(tokZp9 == tok)
		{
			mainSetBO_Zp9(pMain, parserExpectRegister("Name der Zp9 LED 2 erwartet"));
			tok = parserGetWord(name);
		}

		parserAssert(tokTischLEDrot == tok, "TischLEDrot erwartet");
		mainSetBO_Desk_Red(pMain, parserExpectRegister("Name der roten Tisch-LED erwartet"));

		parserExpectTokenNeu(tokTischLEDgruen);
		mainSetBO_Desk_Green(pMain, parserExpectRegister("Name der grünen Tisch-LED erwartet"));
	
		/* read the distant signals connected to this mains	*/
		parserExpectTokenNeu(tokVorsignale);
		nrDistant = parserExpectNumber("Anzahl Vorsignale erwartet");
		/*TODO to implement	*/
		assert(0 == nrDistant);
		parserExpectTokenNeu(tokLBracket);

		parserExpectTokenNeu(tokRBracket);

		parserExpectTokenNeu(tokRBracket);
		return;
	}	
#endif

#ifdef DOMINO55
	/*** 
	 * parser for a single mains
	 * @param pMain	pointer to the mains
	 * @exception	warning and exit on error of the given parameters
	 */
	/*@maynotreturn@*/ 
	static void mainsSingleParser(/*@temp@*/ struct sMain * const pMain);
	static void mainsSingleParser(/*@temp@*/ struct sMain * const pMain) 
	{
		char 		name[NAMELEN];
		enum eToken	tok = tokNone;
		unsigned short	nrDistant = 0;

		parserExpectTokenNeu(tokLBracket);

		parserExpectTokenNeu(tokTaste);  
		(void) parserExpectName(name, "Tastenname erwartet");
		mainSetBI_Button(pMain, buttonFind(name));

		parserExpectTokenNeu(tokGruen1);
		mainSetBO_Green1(pMain, parserExpectRegister("Name der grünen LED 1 erwartet"));

		parserExpectTokenNeu(tokRot1);
		mainSetBO_Red1(pMain, parserExpectRegister("Name der roten LED 1 erwartet"));

		/* now get the next token	*/
		tok = parserGetWord(name);
		if(tokGelb1 == tok)
		{
			mainSetBO_Yellow1(pMain, parserExpectRegister("Name der gelben LED 1 erwartet"));		
			tok = parserGetWord(name);
		}
		if(tokGruen2 == tok)
		{
			mainSetBO_Green2(pMain, parserExpectRegister("Name der grünen LED 2 erwartet"));
			tok = parserGetWord(name);
		}
		if(tokGelb2 == tok)
		{
			mainSetBO_Yellow2(pMain, parserExpectRegister("Name der gelben LED 2 erwartet"));
			tok = parserGetWord(name);
		}
		if(tokGruen3 == tok)
		{
			mainSetBO_Green3(pMain, parserExpectRegister("Name der grünen LED 3 erwartet"));
			tok = parserGetWord(name);
		}
		if(tokAbfahrt == tok)
		{
			mainSetBO_Depart(pMain, parserExpectRegister("Name des Abfahrtssignals erwartet"));
			tok = parserGetWord(name);
		}

		parserAssert(tokTischLEDrot == tok, "TischLEDrot erwartet");
		mainSetBO_Desk_Red(pMain, parserExpectRegister("Name der roten Tisch-LED erwartet"));

		parserExpectTokenNeu(tokTischLEDgruen);
		mainSetBO_Desk_Green(pMain, parserExpectRegister("Name der grünen Tisch-LED erwartet"));

		/* read the distant signals connected to this mains	*/
		parserExpectTokenNeu(tokVorsignale);
		nrDistant = parserExpectNumber("Anzahl Vorsignale erwartet");
		/*TODO to implement	*/
		assert(0 == nrDistant);
		parserExpectTokenNeu(tokLBracket);

		parserExpectTokenNeu(tokRBracket);
		
		parserExpectTokenNeu(tokRBracket);

		/* TODO here, the connected dwarf must be found. 
		 * if existing, then the dwarf gets the red light from the mains
		 * the pointer is remembered to signal changes of the aspect
		 * to the dwarf
		 */
		return;
	}	
#endif

/**
 * parser for the mains
 * @exception	error message when configuration file cannot be interpreted
 */
void mainsParser(void) {
	char		name[NAMELEN];
	struct sMain * pMain;
	unsigned short	nrMains = 0;
	unsigned short	i = 0;
	enum eToken	tok = tokNone;

	parserExpectTokenNeu(tokLBracket);
	parserExpectTokenNeu(tokHauptsignale);
	nrMains = parserExpectNumber("Anzahl Hauptsignale erwartet");
	mains(nrMains);	/* generate the mains structure	*/

	parserExpectTokenNeu(tokLBracket);
	while(NULL != (pMain = mainGet(i)))
	{
		tok = parserGetWord(name);
		parserAssert(tokNone == tok, "Hauptsignalname erwartet");
		mainSetName(pMain, name);
		mainsSingleParser(pMain);
		/* now I finished parsing a single mains.
		 *  I check if there is one more
		 */
		i++;
	}

	tok = parserGetToken();
	parserAssert(tokRBracket == tok, "zu viele Hauptsignale angegeben");
	/* I did not use parserExpectToken, because I know I expect tokRBracket
	 *  but I want to make a better error message
	 */
	
	parserExpectTokenNeu(tokRBracket);
}

#ifdef SPDR60
	/**
	 * Prints the configuration of a single mains
	 * @param pMain		pointer to the mains to be printed
	 * @exceptioni		Assert on NULL pointer
	 */
	static void mainsSinglePrint(struct sMain * const pMain);
	static void mainsSinglePrint(struct sMain * const pMain) 
	{
		char	name[NAMELEN];

		mainGetName(pMain, name);
		fprintf(stdout, "\t\t\t%s {\n", name);

		buttonGetName(mainGetBI_Button(pMain), name);
		fprintf(stdout, "\t\t\t\tTaste\t\t%s\n", name);

		registerGetName(mainGetBO_Green(pMain), name);
		fprintf(stdout, "\t\t\t\tgruen\t\t%s\n", name);

		registerGetName(mainGetBO_Red(pMain), name);
		fprintf(stdout, "\t\t\t\trot\t\t%s\n", name);

		if(NONEVALUE != mainGetBO_Yellow(pMain))
		{
			registerGetName(mainGetBO_Yellow(pMain), name);
			fprintf(stdout, "\t\t\t\tgelb\t\t%s\n", name);
		}
		if(NONEVALUE != mainGetBO_Zs3(pMain))
		{
			registerGetName(mainGetBO_Zs3(pMain), name);
			fprintf(stdout, "\t\t\t\tZs3\t\t%s\n", name);
		}
		if(NONEVALUE != mainGetBO_Zp9(pMain))
		{
			registerGetName(mainGetBO_Zp9(pMain), name);
			fprintf(stdout, "\t\t\t\tZp9\t\t%s\n", name);
		}

		registerGetName(mainGetBO_Desk_Red(pMain), name);
		fprintf(stdout, "\t\t\t\tTischLEDrot\t%s\n", name);

		registerGetName(mainGetBO_Desk_Green(pMain), name);
		fprintf(stdout, "\t\t\t\tTischLEDgruen\t%s\n", name);

		fprintf(stdout, "\t\t\t\tVorsignale\t0 {\n");	/* TODO */
		fprintf(stdout, "\t\t\t\t}\n");
		fprintf(stdout, "\t\t\t}\n");
	}
#endif

#ifdef DOMINO55
	/**
	 * Prints the configuration of a single mains
	 * @param pMain		pointer to the mains to be printed
	 * @exceptioni		Assert on NULL pointer
	 */
	static void mainsSinglePrint(struct sMain * const pMain);
	static void mainsSinglePrint(struct sMain * const pMain) 
	{
		char	name[NAMELEN];

		mainGetName(pMain, name);
		fprintf(stdout, "\t\t\t%s {\n", name);

		buttonGetName(mainGetBI_Button(pMain), name);
		fprintf(stdout, "\t\t\t\tTaste\t\t%s\n", name);

		registerGetName(mainGetBO_Green1(pMain), name);
		fprintf(stdout, "\t\t\t\tgruen1\t\t%s\n", name);

		registerGetName(mainGetBO_Red1(pMain), name);
		fprintf(stdout, "\t\t\t\trot1\t\t%s\n", name);

		if(NONEVALUE != mainGetBO_Yellow1(pMain))
		{
			registerGetName(mainGetBO_Yellow1(pMain), name);
			fprintf(stdout, "\t\t\t\tgelb1\t\t%s\n", name);
		}
		if(NONEVALUE != mainGetBO_Green2(pMain))
		{
			registerGetName(mainGetBO_Green2(pMain), name);
			fprintf(stdout, "\t\t\t\tgruen2\t\t%s\n", name);
		}
		if(NONEVALUE != mainGetBO_Yellow2(pMain))
		{
			registerGetName(mainGetBO_Yellow2(pMain), name);
			fprintf(stdout, "\t\t\t\tgelb2\t\t%s\n", name);
		}
		if(NONEVALUE != mainGetBO_Green3(pMain))
		{
			registerGetName(mainGetBO_Green3(pMain), name);
			fprintf(stdout, "\t\t\t\tgruen3\t\t%s\n", name);
		}
		if(NONEVALUE != mainGetBO_Depart(pMain))
		{
			registerGetName(mainGetBO_Depart(pMain), name);
			fprintf(stdout, "\t\t\t\tAbfahrt\t\t%s\n", name);
		}

		registerGetName(mainGetBO_Desk_Red(pMain), name);
		fprintf(stdout, "\t\t\t\tTischLEDrot\t%s\n", name);

		registerGetName(mainGetBO_Desk_Green(pMain), name);
		fprintf(stdout, "\t\t\t\tTischLEDgruen\t%s\n", name);

		fprintf(stdout, "\t\t\t\tVorsignale\t0 {\n");	/* TODO */
		fprintf(stdout, "\t\t\t\t}\n");
		fprintf(stdout, "\t\t\t}\n");
	}
#endif

void mainsPrint(void) 
{
	unsigned short	i = 0;
	struct sMain *	pMain;

	fprintf(stdout, "\t{\n");
	fprintf(stdout, "\t\tHauptsignale %u {\n", mainsGetNr());
	while(NULL != (pMain = mainGet(i)))
	{
		mainsSinglePrint(pMain);
		i++;
	}
	fprintf(stdout, "\t\t}\n");
	fprintf(stdout, "\t}\n");
}

