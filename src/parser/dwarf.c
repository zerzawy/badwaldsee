/*
 * This file is part of Raspberry Pi steuert Modelleisenbahn
 * Copyright(C)  2012 - 2015 Kurt Zerzawy www.zerzawy.ch
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
 * Functions for parsing and printing parameters of dwarf signals (Zwergsignale on Domino, Sperrsignale on SpDr60)
 * \author
 * Kurt Zerzawy
 */

/*
 * $Author: kurt $
 * $Date: 2015-05-18 22:31:02 +0200 (Mon, 18. Mai 2015) $
 * $Revision: 2464 $
 */

#include <string.h>
#include <assert.h>
#include "../registers.h"
#include "../button.h"
#include "../dwarf.h"
#include "parser.h"
#include "dwarf.h"
#include "../mains.h"		/* for attached main signal	*/

#ifdef SPDR60
	/*** 
	 * parser for a single dwarf
	 * @param ptr	pointer to the dwarf
	 * @exception	warning and exit on error of the given parameters
	 */
	/*@maynotreturn@*/ 
	static void dwarfSingleParser(/*@temp@*/ struct sDwarf * const ptr);
	static void dwarfSingleParser(/*@temp@*/ struct sDwarf * const ptr)
	{
		char		name[NAMELEN];
		enum eToken	tok;

		parserExpectTokenNeu(tokLBracket);
		parserExpectTokenNeu(tokTaste);  
		(void) parserExpectName(name, "Tastenname erwartet");
		dwarfSetBI_Button(ptr, buttonFind(name));

		parserExpectTokenNeu(tokWeiss1);
		dwarfSetBO_White1(ptr, parserExpectRegister("Name der weissen LED1 erwartet"));

		parserExpectTokenNeu(tokWeiss2);
		dwarfSetBO_White2(ptr, parserExpectRegister("Name der weissen LED2 erwartet"));

		parserExpectTokenNeu(tokRot1);
		dwarfSetBO_Red1(ptr, parserExpectRegister("Name der roten LED1 erwartet"));

		parserExpectTokenNeu(tokRot2);
		dwarfSetBO_Red2(ptr, parserExpectRegister("Name der roten LED2 erwartet"));

		parserExpectTokenNeu(tokTischLEDrot);
		dwarfSetBO_Desk_Red(ptr, parserExpectRegister("Name der roten Tisch-LED erwartet"));

		parserExpectTokenNeu(tokTischLEDweiss);
		dwarfSetBO_Desk_Green(ptr, parserExpectRegister("Name der weissen Tisch-LED erwartet"));

        	tok = parserGetToken();
		if(tokRBracket == tok)
		{
			/* dwarf is not attached to main signal	*/
			return;
		}

		/* parse the name of the main signal the dwarf is attached to	*/
	        parserAssert(tokHauptsignal == tok, "} oder Hauptsignal erwartet");
		tok = parserGetWord(name);
		parserAssert(tokNone == tok, "Hauptsignalname erwartet");
		dwarfSetMainName(ptr, name);

		parserExpectTokenNeu(tokRBracket);
		return;
	}	
#endif

#ifdef DOMINO55
	/*** 
	 * parser for a single dwarf
	 * @param ptr	pointer to the dwarf
	 * @exception	warning and exit on error of the given parameters
	 */
	/*@maynotreturn@*/ 
	static void dwarfSingleParser(/*@temp@*/ struct sDwarf * const ptr);
	static void dwarfSingleParser(/*@temp@*/ struct sDwarf * const ptr)
	{
		char		name[NAMELEN];

		parserExpectTokenNeu(tokLBracket);
		parserExpectTokenNeu(tokTaste);  
		(void) parserExpectName(name, "Tastenname erwartet");
		dwarfSetBI_Button(ptr, buttonFind(name));

		parserExpectTokenNeu(tokWeiss1);
		dwarfSetBO_LED1(ptr, parserExpectRegister("Name der weissen LED1 erwartet"));

		parserExpectTokenNeu(tokWeiss2);
		dwarfSetBO_LED2(ptr, parserExpectRegister("Name der weissen LED2 erwartet"));

		parserExpectTokenNeu(tokWeiss3);
		dwarfSetBO_LED3(ptr, parserExpectRegister("Name der weissen LED3 erwartet"));

		parserExpectTokenNeu(tokTischLED);
		dwarfSetBO_Desk_Green(ptr, parserExpectRegister("Name der Tisch-LED erwartet"));
		
		parserExpectTokenNeu(tokRBracket);
		return;
	}	
#endif

/**
 * parser for the dwarfs
 * @exception	error message when configuration file cannot be interpreted
 */
void dwarfParser(void)
{
	char		name[NAMELEN];
	struct sDwarf * ptr;
	unsigned short	nrDwarfs = 0;
	unsigned short	i = 0;
	enum eToken	tok;

	parserExpectTokenNeu(tokLBracket);

	#ifdef SPDR60
		parserExpectTokenNeu(tokSperrsignale);
	#endif

	#ifdef DOMINO55
		parserExpectTokenNeu(tokZwergsignale);
	#endif

	nrDwarfs = parserExpectNumber("Anzahl Zwergsignale erwartet");
	dwarf(nrDwarfs);	/* generate the dwarf structure	*/

	parserExpectTokenNeu(tokLBracket);
	while(NULL != (ptr = dwarfGet(i)))
	{
		tok = parserGetWord(name);
		parserAssert(tokNone == tok, "Zwergsignalname erwartet");
		dwarfSetName(ptr, name);
		dwarfSingleParser(ptr);
		/* now I finished parsing a single dwarf.
		 *  I check if there is one more
		 */
		i++;
	}

	tok = parserGetToken();
	parserAssert(tokRBracket == tok, "zu viele Zwergsignale angegeben");
	/* I did not use parserExpectToken, because I know I expect tokRBracket
	 *  but I want to make a better error message
	 */
	
	parserExpectTokenNeu(tokRBracket);
}

#ifdef SPDR60
	/**
	 * Function to set all forward annotations correctly. For dwarfs, this are the main signals
	 * the dwarf could be attached to 
	 */
	void dwarfParserResolve(void)
	{
		struct sDwarf *	pDwarf	= NULL;
		struct sMain *	pMain	= NULL;
		unsigned short	index	= 0;
		char		name[NAMELEN];

		while(NULL != (pDwarf = dwarfGet(index)))
		{
			dwarfGetMainName(pDwarf, name);
			if('\0' != name[0])
			{
				/* there is a name stored, so a signal exists	*/
				pDwarf->pMain = mainFind(name);
			}
			index++;
		}
		/* all done	*/
	}
#endif
		
#ifdef DOMINO55
	/**
	 * Empty function to satisfy the compiler in case DOMINO option is set
	 */
	void dwarfParserResolve(void)
	{
	}
#endif

#ifdef SPDR60
	/**
	 * Prints the configuration of a single dwarf
	 * @param ptr		pointer to the dwarf to be printed
	 * @exceptioni		Assert on NULL pointer
	 */
	static void dwarfSinglePrint(struct sDwarf * const ptr);
	static void dwarfSinglePrint(struct sDwarf * const ptr) 
	{
		char		name[NAMELEN];
		struct sMain *	pMain;

		dwarfGetName(ptr, name);
		fprintf(stdout, "\t\t\t%s {\n", name);

		buttonGetName(dwarfGetBI_Button(ptr), name);
		fprintf(stdout, "\t\t\t\tTaste\t\t%s\n", name);

		registerGetName(dwarfGetBO_White1(ptr), name);
		fprintf(stdout, "\t\t\t\tweiss1\t\t%s\n", name);

		registerGetName(dwarfGetBO_White2(ptr), name);
		fprintf(stdout, "\t\t\t\tweiss2\t\t%s\n", name);

		registerGetName(dwarfGetBO_Red1(ptr), name);
		fprintf(stdout, "\t\t\t\trot1\t\t%s\n", name);

		registerGetName(dwarfGetBO_Red2(ptr), name);
		fprintf(stdout, "\t\t\t\trot2\t\t%s\n", name);

		registerGetName(dwarfGetBO_Desk_Red(ptr), name);
		fprintf(stdout, "\t\t\t\tTischLEDrot\t%s\n", name); 
		
		registerGetName(dwarfGetBO_Desk_Green(ptr), name);
		fprintf(stdout, "\t\t\t\tTischLEDweiss\t%s\n", name); 
		
		pMain = dwarfGetMain(ptr);
		if(NULL != pMain)
		{
			/* now the dwarf ist attached to a main signal	*/
			mainGetName(pMain, name);
			fprintf(stdout, "\t\t\t\tHauptsignal\t%s\n", name);
		}

		fprintf(stdout, "\t\t\t}\n");
	}
#endif

#ifdef DOMINO55
	/**
	 * Prints the configuration of a single dwarf
	 * @param ptr		pointer to the dwarf to be printed
	 * @exceptioni		Assert on NULL pointer
	 */
	static void dwarfSinglePrint(struct sDwarf * const ptr);
	static void dwarfSinglePrint(struct sDwarf * const ptr) 
	{
		char	name[NAMELEN];

		dwarfGetName(ptr, name);
		fprintf(stdout, "\t\t\t%s {\n", name);

		buttonGetName(dwarfGetBI_Button(ptr), name);
		fprintf(stdout, "\t\t\t\tTaste\t\t%s\n", name);

		registerGetName(dwarfGetBO_LED1(ptr), name);
		fprintf(stdout, "\t\t\t\tweiss1\t\t%s\n", name);

		registerGetName(dwarfGetBO_LED2(ptr), name);
		fprintf(stdout, "\t\t\t\tweiss2\t\t%s\n", name);

		registerGetName(dwarfGetBO_LED3(ptr), name);
		fprintf(stdout, "\t\t\t\tweiss3\t\t%s\n", name);
		
		registerGetName(dwarfGetBO_Desk_Green(ptr), name);
		fprintf(stdout, "\t\t\t\tTischLED\t%s\n", name); 
	
		fprintf(stdout, "\t\t\t}\n");
}
#endif

void dwarfPrint(void) 
{
	unsigned short	i = 0;
	struct sDwarf *	ptr = NULL;

	fprintf(stdout, "\t{\n");

#ifdef SPDR60
	fprintf(stdout, "\t\tSperrsignale %u {\n", dwarfGetNr());
#endif

#ifdef DOMINO55
	fprintf(stdout, "\t\tZwergsignale %u {\n", dwarfGetNr());
#endif

	while(NULL != (ptr = dwarfGet(i)))
	{
		dwarfSinglePrint(ptr);
		i++;
	}
	fprintf(stdout, "\t\t}\n");
	fprintf(stdout, "\t}\n");
}

