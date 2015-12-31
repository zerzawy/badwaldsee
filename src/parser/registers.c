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
 * Functions for parsing and printing parameters of the temporary register 
 * \author
 * Kurt Zerzawy
 */

/*
 * $Author: kurt $
 * $Date: 2015-05-18 22:31:02 +0200 (Mo, 18 Mai 2015) $
 * $Revision: 2464 $
 */


#include "parser.h"
#include "registers.h"
#include "inshift.h"
#include "outshift.h"
#include "vars.h"
#include "../registers.h"
#include "../inshift.h"
#include "../outshift.h"
#include "../vars.h"
#include "../ap216.h"

/**
 * Funktion zum Parsen des Konfigurationsfiles. 
 *  Der Filepointer hierzu ist in einer globalen Variable
 *  vorhanden. Das ist nicht sehr elegant, erspart aber das
 *  durchreichen eines Pointers durch duzende Funktionen.
 * Die Funktion fuellt alle Parameter der Register, also
 *  inshift, outshift und Merker ab. Falls es in dem 
 *  Konfigurationfile einen Fehler hat, wird eine Fehlermeldung
 *  auf LOG_ERR ausgegeben und die Funktion macht einen Exit.
 */
void registerParser(void)
{
	_Bool defined = false;
	enum eToken tok;    
	unsigned short inshiftRegLen = 0;
	unsigned short outshiftRegLen = 0;
	unsigned short varsRegLen = 0;

	inshiftEnable(false);
	outshiftEnable(false);


	parserExpectTokenNeu(tokLBracket);

	do {
		tok = parserGetToken();
		switch(tok) {
		case tokRBracket:
			break;
		case tokNrInputs:
			inshiftRegLen = parserExpectNumber("Nummer fuer Anzahl Eingaenge erwartet");
			defined = true;
			break;
		case tokNrOutputs:
			outshiftRegLen = parserExpectNumber("Nummer fuer Anzahl Ausgaenge erwartet");
			defined = true;
			break;
		case tokNrVars:
			varsRegLen = parserExpectNumber("Nummer fuer Anzahl Variablen erwartet");
			defined = true;
			break;
		default: parserExit("2\"AnzahlEingaenge\" \"AnzahlAusgaenge\" oder \"AnzahlVariablen\" oder \"}\" erwartet");
		}
	} while(tokRBracket != tok);
	if(!defined) {
		parserAssert(defined, "\"AnzahlEingaenge\" \"AnzahlAusgaenge\" oder \"AnzahlVariablen\" oder \"}\" erwartet");
	}
	parserExpectTokenNeu(tokLBracket);

	/* now create the register */
	registers(outshiftRegLen, inshiftRegLen, varsRegLen);

	while(true) {
	/* at least one definition given, only this way */
		switch(parserGetToken()) {
		case tokInput:
			inshiftParser();
			defined = true;
			break;
		case tokOutput:
			outshiftParser();
			defined = true;
			break;
		case tokVars:
			varsParser();
			defined = true;
			break;
		case tokRBracket:
			parserAssert(defined, "IO Definition erwartet"); 
			/* io is over */
			return;
		default: parserAssert(0, "IO Definition erwartet");
		}
	}
}


/** 
 * Druckt die in den Parametern gespeicherten Daten im Format des 
 *  Konfigurationsfiles aus.
 * Damit koennen Sie kontrollieren, ob das File richtig eingelesen
 *  und interpretiert wurde.
 */
void registerPrint(void) 
{
	unsigned int inshiftRegLen	= registerGetInshiftLen();
	unsigned int outshiftRegLen = registerGetOutshiftLen();
	unsigned int varsRegLen		= registerGetVarLen();

	fprintf(stdout, "\t{\n");
	fprintf(stdout, "\t\tAnzahlEingaenge %hu\n", inshiftRegLen);
	fprintf(stdout, "\t\tAnzahlAusgaenge %hu\n", outshiftRegLen);
	fprintf(stdout, "\t\tAnzahlVariablen %hu\n", varsRegLen);
	fprintf(stdout, "\t}\n");
	fprintf(stdout, "\t{\n");
	if(0 != outshiftRegLen) 
	{
		outshiftPrint();
	}
	if(0 != inshiftRegLen) 
	{
		inshiftPrint();
	}
	if(0 != varsRegLen) 
	{
		varsPrint();
	}
	fprintf(stdout, "\t}\n");
}

