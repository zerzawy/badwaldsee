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
 * Functions for parsing and printing parameters of input shift register 
 * \author
 * Kurt Zerzawy
 */

/*
 * $Author: kurt $
 * $Date: 2015-05-18 22:31:02 +0200 (Mon, 18. Mai 2015) $
 * $Revision: 2464 $
 */

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "assert.h"
#include "parser.h"
#include "../registers.h"
#include "../inshift.h"
#include "../ep16.h"

/*@maynotreturn@*/
static void inshiftBitParser(const unsigned short plughandle);
/*@maynotreturn@*/
static void inshiftBitParser(const unsigned short plughandle) {
	char            name[NAMELEN];
	short           bit = 0;
	unsigned short  fPlus = 0;
	unsigned short  fMinus = 0;
	unsigned short 	handle;

	assert(plughandle < (0x10 * registerGetLen()));

	while(true) {
		switch(parserGetToken()) {
		case tokRBracket:
		   	/* now a pin number will be given */
			return;
	   	case tokPin:
		   	/* now a pin number will be given */
		   	bit = ep16PinToBit(parserExpectNumber("Pin-Nummer erwartet"));
		   	parserAssert(0 <= bit, "kenne Pin-Nummer nicht");
		   	assert(bit < 16);
		   	parserExpectName(name, "Name des Eingangspins erwartet");
			handle = plughandle + bit;
		   	registerSetName(handle, name); 
			
			/* copy the filter plus value */
		   	fPlus = parserExpectNumber("Filterwert Plus erwartet");
		   	parserAssert(0x80 > fPlus, "Filterwert muss kleiner als 127 sein");

			registerSetFilterPlus(handle, fPlus); 
			
			/* copy the filter minus value */
		   	fMinus = parserExpectNumber("Filterwert Minus erwartet");
		   	parserAssert(0x80 > fMinus, "Filterwert muss kleiner als 127 sein"); 
			
			registerSetFilterMinus(handle, fMinus); 
			break;
		default:
		   	parserExit("\"Pin\" erwartet");
		}
	}      
}

/*@maynotreturn@*/ 
void inshiftParser(void) {

	unsigned short addr = 0;
	unsigned short handle;
	char           name[NAMELEN];

	addr = 0;
	strcpy(name, "");

	parserExpectTokenNeu(tokLBracket);
	while(true) {
		switch(parserGetWord(name)) {
		case tokRBracket:
			/* now, the register parsing is over */
			return;
		case tokNone:
			/* it is a case like PL1 { */
			break;
		default:
			parserExit("Name des Steckers erwartet");
		}
		
		parserExpectTokenNeu(tokLBracket);
		parserExpectTokenNeu(tokAdr);
		addr = parserExpectNumber("Adresse erwartet");
		parserAssert(inshiftGetRegLen() >= addr, "Adresse ist zu gross");
		parserAssert(0 != addr, "Adresse null nicht erlaubt");
		/* copy the name of the plug
		 *  for calculation of address note that addr 1 is first
		 */
		handle = registerGetFirstInshiftHandle() + (addr - 1) * 0x10;
		registerSetPlugName(handle, name);
		inshiftBitParser(handle);
	}
}

/**
 * Druckt die vorhandenen Objekte als Parserfile aus.
 * Diese Funktion wird zum Testen des Parsers verwendet.
 */
void inshiftPrint(void) 
{
	unsigned short	handle;
	unsigned short	firstHandle;
	unsigned short	i;
	short			pin;
	unsigned short	bit;
	char			name[NAMELEN];

	firstHandle = registerGetFirstInshiftHandle();
	fprintf(stdout, "\t\tEingaenge {\n");
	for(i = 0; i < inshiftGetRegLen(); i++) 
	{
		handle = firstHandle + 0x10 * i;
		registerGetPlugName(handle, name);
		fprintf(stdout, "\t\t\t%s {\n", name);
		fprintf(stdout, "\t\t\t\tAdr\t%hu\n", i + 1);
		for(pin = 1; pin < 9; pin++) 
		{
			/* find this pin */
			for(bit = 0; bit < 16; bit++) 
			{
				if(pin == ep16BitToPin(bit)) 
				{
					break;
				}
			}
			handle = firstHandle + 0x10 * i + bit;
			registerGetName(handle, name);
			if(0 != (strlen(name)))
			{
				fprintf(stdout, "\t\t\t\tPin %d\t%s %hu %hu\n",
								pin, name, 
								(unsigned short) registerGetFilterPlus(handle),
								(unsigned short) registerGetFilterMinus(handle));
			}
		}
		
		fprintf(stdout, "\n");

		for(pin = 11; pin < 19; pin++) 
		{
			/* finde den Pin */
			for(bit = 0; bit < 16; bit++) 
			{
				if(pin == ep16BitToPin(bit)) 
				{
					break;
				}
			}
			handle = firstHandle + 0x10 * i + bit;
			registerGetName(handle, name);
			if(0 != (strlen(name))) 
			{
				fprintf(stdout, "\t\t\t\tPin %d\t%s %hu %hu\n",
								pin, name, 
								(unsigned short) registerGetFilterPlus(handle),
								(unsigned short) registerGetFilterMinus(handle));
			}
		}
		fprintf(stdout, "\t\t\t}\n");
	}
	fprintf(stdout, "\t\t}\n");
}
