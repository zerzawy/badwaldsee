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
 * Functions for parsing and printing parameters of distant signals (Hauptsignale) 
 * \author
 * Kurt Zerzawy
 */

/*
 * $Author: kurt $
 * $Date: 2015-05-11 23:40:36 +0200 (Mon, 11. Mai 2015) $
 * $Revision: 2447 $
 */

#include <string.h>
#include <assert.h>
#include "../registers.h"
#include "../button.h"
#include "../mains.h"
#include "../distant.h"
#include "parser.h"
#include "distant.h"

#ifdef SPDR60
	/*** 
	 * parser for a single distants signal
	 * @param pDistant	pointer to the distant signal
	 * @exception		warning and exit on error of the given parameters
	 */
	/*@maynotreturn@*/ 
	static void distantSingleParser(/*@temp@*/ struct sDistant * const pDistant);
	static void distantSingleParser(/*@temp@*/ struct sDistant * const pDistant) 
	{
		char 		name[NAMELEN];
		enum eToken	tok = tokNone;

		parserExpectTokenNeu(tokLBracket);

		parserExpectTokenNeu(tokGelb11);
		distantSetBO_Yellow11(pDistant, parserExpectRegister("Name der linken unteren gelben LED erwartet"));

		parserExpectTokenNeu(tokGelb13);
		distantSetBO_Yellow13(pDistant, parserExpectRegister("Name der linken oberen gelbenLED erwartet"));
		
		parserExpectTokenNeu(tokGruen12);
		distantSetBO_Green12(pDistant, parserExpectRegister("Name der rechten unteren grünen LED erwartet"));

		parserExpectTokenNeu(tokGruen14);
		distantSetBO_Green14(pDistant, parserExpectRegister("Name der rechten oberen grünen LED erwartet"));

		/* now get the next token and check if Zs3v shall be set	*/
		tok = parserGetWord(name);
		if(tokZs3v == tok)
		{
			distantSetBO_Zs3v(pDistant, parserExpectRegister("Name der Zs3v-LED erwartet"));		
			tok = parserGetWord(name);
		}

		parserAssert(tokTischLEDgelb == tok, "Zs3v oder TischLEDgelb erwartet");
		distantSetBO_Desk_Yellow(pDistant, parserExpectRegister("Name der gelben Tisch-LED erwartet"));

		parserExpectTokenNeu(tokTischLEDgruen);
		distantSetBO_Desk_Green(pDistant, parserExpectRegister("Name der grünen Tisch-LED erwartet"));

		/* now get the next token and check if a main signal is on the same mast */
		tok = parserGetWord(name);
		if(tokHauptsignal == tok)
		{
			tok = parserGetWord(name);
			parserAssert(tokNone == tok, "Name des Hauptsignals erwartet");
			pDistant->pMain = mainFind(name);
			tok = parserGetWord(name);
		}

		parserAssert(tokRBracket == tok, "Hauptsignal oder } erwartet");
		return;
	}	
#endif

#ifdef DOMINO55
	/*** 
	 * parser for a single distants signal
	 * @param pDistant	pointer to the distant signal
	 * @exception		warning and exit on error of the given parameters
	 */
	/*@maynotreturn@*/ 
	static void distantSingleParser(/*@temp@*/ struct sDistant * const pDistant);
	static void distantSingleParser(/*@temp@*/ struct sDistant * const pDistant) 
	{
		char 		name[NAMELEN];
		enum eToken	tok = tokNone;

		parserExpectTokenNeu(tokLBracket);

		parserExpectTokenNeu(tokGelb11);
		distantSetBO_Yellow11(pDistant, parserExpectRegister("Name der linken oberen gelben LED erwartet"));

		parserExpectTokenNeu(tokGelb13);
		distantSetBO_Yellow13(pDistant, parserExpectRegister("Name der rechten oberen gelben LED erwartet"));
		
		parserExpectTokenNeu(tokGruen14);
		distantSetBO_Green14(pDistant, parserExpectRegister("Name der rechten oberen grünen LED erwartet"));

		/* now get the next token and check if a lower left green LED shall be set	*/
		tok = parserGetWord(name);
		if(tokGruen12 == tok)
		{
			distantSetBO_Green12(pDistant, parserExpectRegister("Name der linken unteren grünen LED erwartet"));
			tok = parserGetWord(name);
		}

		/* check for a lower right red LED	*/
		if(tokGruen15 == tok)
		{
			distantSetBO_Green15(pDistant, parserExpectRegister("Name der rechten unteren grünen LED erwartet"));
			tok = parserGetWord(name);
		}

		/* check for a main signal on the same mast	*/
		if(tokHauptsignal == tok)
		{
			tok = parserGetWord(name);
			parserAssert(tokNone == tok, "Name des Hauptsignals erwartet");
			pDistant->pMain = mainFind(name);
			tok = parserGetWord(name);

			/* check if the distant shall be blankened on mains Halt aspect	*/
			if(tokDunkelBeiHalt == tok)
			pDistant->darkOnMainHalt = true;
			tok = parserGetWord(name);
		}

		parserAssert(tokRBracket == tok, "gruen12, gruen15, Hauptsignal oder } erwartet");
		return;
	}	
#endif

/**
 * parser for the distant
 * @exception	error message when configuration file cannot be interpreted
 */
void distantsParser(void) {
	char			name[NAMELEN];
	struct sDistant * 	pDistant;
	unsigned short		nrDwarfs = 0;
	unsigned short		i = 0;
	enum eToken		tok = tokNone;

	parserExpectTokenNeu(tokLBracket);
	parserExpectTokenNeu(tokVorsignale);
	nrDwarfs = parserExpectNumber("Anzahl Vorsignale erwartet");
	distants(nrDwarfs);	/* generate the distant structure	*/

	parserExpectTokenNeu(tokLBracket);
	while(NULL != (pDistant = distantGet(i)))
	{
		tok = parserGetWord(name);
		parserAssert(tokNone == tok, "Vorsignalname erwartet");
		distantSetName(pDistant, name);
		distantSingleParser(pDistant);
		/* now I finished parsing a single distant.
		 *  I check if there is one more
		 */
		i++;
	}

	tok = parserGetToken();
	parserAssert(tokRBracket == tok, "zu viele Vorsignale angegeben");
	/* I did not use parserExpectToken, because I know I expect tokRBracket
	 *  but I want to make a better error message
	 */
	
	parserExpectTokenNeu(tokRBracket);
}

#ifdef SPDR60
	/**
	 * Prints the configuration of a single distant
	 * @param pDistant	pointer to the distant to be printed
	 * @exceptioni		Assert on NULL pointer
	 */
	static void distantSinglePrint(struct sDistant * const pDistant);
	static void distantSinglePrint(struct sDistant * const pDistant) 
	{
		char		name[NAMELEN];
		struct sMain *	pMain = NULL;

		distantGetName(pDistant, name);
		fprintf(stdout, "\t\t\t%s {\n", name);

		registerGetName(distantGetBO_Yellow11(pDistant), name);
		fprintf(stdout, "\t\t\t\tgelb11\t\t%s\n", name);

		registerGetName(distantGetBO_Yellow13(pDistant), name);
		fprintf(stdout, "\t\t\t\tgelb13\t\t%s\n", name);

		registerGetName(distantGetBO_Green12(pDistant), name);
		fprintf(stdout, "\t\t\t\tgruen12\t\t%s\n", name);

		registerGetName(distantGetBO_Green14(pDistant), name);
		fprintf(stdout, "\t\t\t\tgruen14\t\t%s\n", name);
	
		if(NONEVALUE != distantGetBO_Zs3v(pDistant))
		{
			registerGetName(distantGetBO_Zs3v(pDistant), name);
			fprintf(stdout, "\t\t\t\tZs3v\t\t%s\n", name);
		}

		registerGetName(distantGetBO_Desk_Yellow(pDistant), name);
		fprintf(stdout, "\t\t\t\tTischLEDgelb\t%s\n", name);

		registerGetName(distantGetBO_Desk_Green(pDistant), name);
		fprintf(stdout, "\t\t\t\tTischLEDgruen\t%s\n", name);

		if(NULL != (pMain = distantGetMain(pDistant)))
		{
			mainGetName(pMain, name);
			fprintf(stdout, "\t\t\t\tHauptsignal\t%s\n", name);
		}
		fprintf(stdout, "\t\t\t}\n");
	}
#endif

#ifdef DOMINO55
	/**
	 * Prints the configuration of a single distant
	 * @param pDistant	pointer to the distant to be printed
	 * @exceptioni		Assert on NULL pointer
	 */
	static void distantSinglePrint(struct sDistant * const pDistant);
	static void distantSinglePrint(struct sDistant * const pDistant) 
	{
		char		name[NAMELEN];
		unsigned short	handle;
		struct sMain *	pMain = NULL;

		distantGetName(pDistant, name);
		fprintf(stdout, "\t\t\t%s {\n", name);

		registerGetName(distantGetBO_Yellow11(pDistant), name);
		fprintf(stdout, "\t\t\t\tgelb11\t\t%s\n", name);

		registerGetName(distantGetBO_Yellow13(pDistant), name);
		fprintf(stdout, "\t\t\t\tgelb13\t\t%s\n", name);

		registerGetName(distantGetBO_Green14(pDistant), name);
		fprintf(stdout, "\t\t\t\tgruen14\t\t%s\n", name);

		if(NONEVALUE != (handle = distantGetBO_Green12(pDistant)))
		{
			registerGetName(handle, name);
			fprintf(stdout, "\t\t\t\tgruen12\t\t%s\n", name);
		}	

		if(NONEVALUE != (handle = distantGetBO_Green15(pDistant)))
		{
			registerGetName(handle, name);
			fprintf(stdout, "\t\t\t\tgruen15\t\t%s\n", name);
		}	

		if(NULL != (pMain = distantGetMain(pDistant)))
		{
			mainGetName(pMain, name);
			fprintf(stdout, "\t\t\t\tHauptsignal\t\t%s\n", name);
			if(distantGetDarkOnHalt(pDistant))
			{
				fprintf(stdout, "\t\t\t\tDunkelBeiHalt\t\t%s\n", name);
			}
		}
		fprintf(stdout, "\t\t\t\t}\n");
		fprintf(stdout, "\t\t\t}\n");
	}
#endif

void distantsPrint(void) 
{
	unsigned short	i = 0;
	struct sDistant *	pDistant;

	fprintf(stdout, "\t{\n");
	fprintf(stdout, "\t\tVorsignale %u {\n", distantsGetNr());
	while(NULL != (pDistant = distantGet(i)))
	{
		distantSinglePrint(pDistant);
		i++;
	}
	fprintf(stdout, "\t\t}\n");
	fprintf(stdout, "\t}\n");
}

