/*
 * This file is part of uPStellwerk Bad Waldsee
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
 * Functions for parsing and printing parametsers of buttons 
 * \author
 * Kurt Zerzawy
 */

/*
 * $Author: kurt $
 * $Date: 2015-05-18 22:31:02 +0200 (Mo, 18 Mai 2015) $
 * $Revision: 2464 $
 */

#include "../registers.h"
#include "button.h"

#ifdef DOMINO55
	const char cTASTE_STEUERUNG []	= "Steuerung";
	const char cTASTE_FERN_EIN []	= "Fern ein";
	const char cTASTE_FERN_AUS []	= "Fern aus";
	const char cTASTE_ISOLUMGSIG []	= "Isolierumgehung Signale";
	const char cTASTE_ISOLUMGWEI []	= "Isolierumgehung Weichen";
	const char cTASTE_NOTAUFLFHT []	= "Notaufloesung FHT";
	const char cTASTE_AUT_EIN []	= "Automat Ein";
	const char cTASTE_AUT_AUS []	= "Automat Aus";
	const char cTASTE_BETRAUFL []	= "Betriebsaufloesung";
	const char cTASTE_AUSLWEI []	= "Ausleuchtung Weichen";
	const char cTASTE_NOTFAHR []	= "Notfahrt";
#endif

#ifdef SPDR60
	const char cTASTE_DRGT []	= "DRGT";
	const char cTASTE_EaGT []	= "EaGT";
	const char cTASTE_FHT []	= "FHT";
	const char cTASTE_FRT []	= "FRT";
	const char cTASTE_HaGT []	= "HaGT";
	const char cTASTE_StET []	= "StET";
	const char cTASTE_StAT []	= "StAT";
	const char cTASTE_UfGT []	= "UfGT";
	const char cTASTE_WGT []	= "WGT";
	const char cTASTE_WHT []	= "WHT";
#endif

/**
 * Parser for a single button
 * @param ptr	pointer to the button
 * @exception	generates a warning and makes an exit on mistakes on the given data
 */
static void einzelParser(/*@temp@*/ struct sButton * const ptr);
static void einzelParser(/*@temp@*/ struct sButton * const ptr)
{
	parserExpectTokenNeu(tokLBracket);
	buttonSetBI_Button(ptr, parserExpectRegister("Name des Eingangs erwartet"));
	parserExpectTokenNeu(tokRBracket);
	return;
}
 
/**
 * parses the buttons part file and creates the buttons
 */
void buttonParser(void) 
{
	struct sButton *	ptr;
	short			stdButton;
	unsigned short 		i = 0;
	unsigned short  	nrButton;
	enum eToken     	tok = tokNone;
	char            	name[NAMELEN];
	char            	errText[32];

	LOG_INF("started");


	/* parsing normal buttons	*/	
	parserExpectTokenNeu(tokLBracket);
	parserExpectTokenNeu(tokTasten);
	nrButton = parserExpectNumber("Anzahl Tasten erwartet");
	button(nrButton); /* erzeuge die Tastenstruktur */
	parserExpectTokenNeu(tokLBracket);
	i = 0; 
	while(NULL != (ptr = buttonGet(i)))
	{
		/* es werden noch Tasten geparst */
		tok = parserGetWord(name);
		parserAssert(tokNone == tok, "Tastenname erwartet");
		buttonSetName(ptr, name);
		einzelParser(ptr);
		i++;
	}
	
	parserExpectTokenNeu(tokRBracket);
	parserExpectTokenNeu(tokRBracket);

	parserExpectTokenNeu(tokLBracket);
	parserExpectTokenNeu(tokStandardTasten);
	parserExpectTokenNeu(tokLBracket);

	/* now standard buttons are initialized	*/
	#ifdef DOMINO55
		stdButtonSetName(0, cTASTE_STEUERUNG);
		stdButtonSetToken(0, tokTasteSteuerung);

		stdButtonSetName(1, cTASTE_FERN_EIN); 
		stdButtonSetToken(1, tokTasteFernEin);
	
		stdButtonSetName(2, cTASTE_FERN_AUS);
		stdButtonSetToken(2, tokTasteFernAus);
	
		stdButtonSetName(3, cTASTE_ISOLUMGSIG);
		stdButtonSetToken(3, tokTasteIsolUmgSig);
	
		stdButtonSetName(4, cTASTE_ISOLUMGWEI);
		stdButtonSetToken(4, tokTasteIsolUmgWei);
	
		stdButtonSetName(5, cTASTE_NOTAUFLFHT);
		stdButtonSetToken(5, tokTasteNotauflFHT);
	
		stdButtonSetName(6, cTASTE_AUT_EIN);
		stdButtonSetToken(6, tokTasteAutEin);
	
		stdButtonSetName(7, cTASTE_AUT_AUS);
		stdButtonSetToken(7, tokTasteAutAus);
	
		stdButtonSetName(8, cTASTE_BETRAUFL);
		stdButtonSetToken(8, tokTasteBetrAufl);
	
		stdButtonSetName(9, cTASTE_AUSLWEI);
		stdButtonSetToken(9, tokTasteAuslWeiche);
	
		stdButtonSetName(10, cTASTE_NOTFAHR);
		stdButtonSetToken(10, tokTasteNotfahr);
	#endif

	#ifdef SPDR60
		stdButtonSetName(0, cTASTE_DRGT);
		stdButtonSetToken(0, tokTasteDRGT);

		stdButtonSetName(1, cTASTE_EaGT);
		stdButtonSetToken(1, tokTasteEaGT);

		stdButtonSetName(2, cTASTE_FHT);
		stdButtonSetToken(2, tokTasteFHT);

		stdButtonSetName(3, cTASTE_FRT);
		stdButtonSetToken(3, tokTasteFRT);

		stdButtonSetName(4, cTASTE_HaGT);
		stdButtonSetToken(4, tokTasteHaGT);

		stdButtonSetName(5, cTASTE_StET);
		stdButtonSetToken(5, tokTasteStET);

		stdButtonSetName(6, cTASTE_StAT);
		stdButtonSetToken(6, tokTasteStAT);

		stdButtonSetName(7, cTASTE_UfGT);
		stdButtonSetToken(7, tokTasteUfGT);

		stdButtonSetName(8, cTASTE_WGT);
		stdButtonSetToken(8, tokTasteWGT);

		stdButtonSetName(9, cTASTE_WHT);
		stdButtonSetToken(9, tokTasteWHT);
	#endif
	i = 0;
	while(0 <= (stdButton = stdButtonGet(i)))
	{ 
		stdButtonGetName(stdButton, name);
		sprintf(errText, "Name der Taste %s erwartet", name);
		parserExpectTokenNeu(stdButtonGetToken(stdButton));
		parserExpectTokenNeu(tokLBracket);
		stdButtonSetBI_Button(stdButton, parserExpectRegister(errText));
		parserExpectTokenNeu(tokRBracket);
		i++;
	}

	parserExpectTokenNeu(tokRBracket);
	parserExpectTokenNeu(tokRBracket);
	LOG_INF("ended");
}

/**
 * prints out the parser file
 */
void buttonPrint(void) 
{
	unsigned short		i;
	struct sButton *	ptr;
	char            	name[NAMELEN];
	char			buttonName[NAMELEN];

	fprintf(stdout, "\t{\n"); 
	fprintf(stdout, "\t\tTasten %u {\n", buttonGetNr());
 
	i = 0; 
	while(NULL != (ptr = buttonGet(i)))
	{
		buttonGetName(ptr, name);
		registerGetName(buttonGetBI_Button(ptr), buttonName);
		fprintf(stdout, "\t\t\t%s\t{ %s }\n", name, buttonName);
		i++;
	}
	fprintf(stdout, "\t\t}\n");
	fprintf(stdout, "\t}\n");

	fprintf(stdout, "\t{\n");
	fprintf(stdout, "\t\tStandardtasten {\n");

	#ifdef DOMINO55
		registerGetName(stdButtonGetBI_Button(0), buttonName);
		fprintf(stdout, "\t\t\tTasteSteuerung\t{ %s }\n", buttonName); 
		
		registerGetName(stdButtonGetBI_Button(1), buttonName);
		fprintf(stdout, "\t\t\tTasteFernEin\t{ %s }\n", buttonName); 
		
		registerGetName(stdButtonGetBI_Button(2), buttonName);
		fprintf(stdout, "\t\t\tTasteFernAus\t{ %s }\n", buttonName); 
		
		registerGetName(stdButtonGetBI_Button(3), buttonName);
		fprintf(stdout, "\t\t\tTasteIsolUmgSig\t{ %s }\n", buttonName); 
		
		registerGetName(stdButtonGetBI_Button(4), buttonName);
		fprintf(stdout, "\t\t\tTasteIsolUmgWei\t{ %s }\n", buttonName); 
		
		registerGetName(stdButtonGetBI_Button(5), buttonName);
		fprintf(stdout, "\t\t\tTasteNotauflFHT\t{ %s }\n", buttonName); 
		
		registerGetName(stdButtonGetBI_Button(6), buttonName);
		fprintf(stdout, "\t\t\tTasteAutEin\t{ %s }\n", buttonName); 
		
		registerGetName(stdButtonGetBI_Button(7), buttonName);
		fprintf(stdout, "\t\t\tTasteAutAus\t{ %s }\n", buttonName); 
		
		registerGetName(stdButtonGetBI_Button(8), buttonName);
		fprintf(stdout, "\t\t\tTasteBetrAufl\t{ %s }\n", buttonName); 
		
		registerGetName(stdButtonGetBI_Button(9), buttonName);
		fprintf(stdout, "\t\t\tTasteAuslWeiche\t{ %s }\n", buttonName); 
		
		registerGetName(stdButtonGetBI_Button(10), buttonName);
		fprintf(stdout, "\t\t\tTasteNotfahr\t{ %s }\n", buttonName); 
	#endif

	#ifdef SPDR60
		registerGetName(stdButtonGetBI_Button(0), buttonName);
		fprintf(stdout, "\t\t\tDRGT\t\t{ %s }\n", buttonName); 

		registerGetName(stdButtonGetBI_Button(1), buttonName);
		fprintf(stdout, "\t\t\tEaGT\t\t{ %s }\n", buttonName); 

		registerGetName(stdButtonGetBI_Button(2), buttonName);
		fprintf(stdout, "\t\t\tFHT\t\t{ %s }\n", buttonName); 

		registerGetName(stdButtonGetBI_Button(3), buttonName);
		fprintf(stdout, "\t\t\tFRT\t\t{ %s }\n", buttonName); 

		registerGetName(stdButtonGetBI_Button(4), buttonName);
		fprintf(stdout, "\t\t\tHaGT\t\t{ %s }\n", buttonName); 

		registerGetName(stdButtonGetBI_Button(5), buttonName);
		fprintf(stdout, "\t\t\tStET\t\t{ %s }\n", buttonName); 

		registerGetName(stdButtonGetBI_Button(6), buttonName);
		fprintf(stdout, "\t\t\tStAT\t\t{ %s }\n", buttonName); 

		registerGetName(stdButtonGetBI_Button(7), buttonName);
		fprintf(stdout, "\t\t\tUfGT\t\t{ %s }\n", buttonName); 

		registerGetName(stdButtonGetBI_Button(8), buttonName);
		fprintf(stdout, "\t\t\tWGT\t\t{ %s }\n", buttonName); 

		registerGetName(stdButtonGetBI_Button(9), buttonName);
		fprintf(stdout, "\t\t\tWHT\t\t{ %s }\n", buttonName); 
	#endif

	fprintf(stdout, "\t\t}\n");
	fprintf(stdout, "\t}\n");
}
