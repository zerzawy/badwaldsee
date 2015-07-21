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
*/

/**
 * \file
 * Functions for parsing and printing parameters of output shift register
 * \author
 * Kurt Zerzawy
 */

/*
 * Spreitenbacher Eisenbahn Amateur Klub SPEAK
 * www.speak.li
 *
 * $Author: kurt $
 * $Date: 2015-05-18 22:31:02 +0200 (Mon, 18. Mai 2015) $
 * $Revision: 2464 $
 */

#include "outshift.h"

/**
 * function to parse one word (= plug) of an outshift register
 */
/*@maynotreturn@*/
static void outshiftPlugParser(const unsigned short plugaddr);
/*@maynotreturn@*/
static void outshiftPlugParser(const unsigned short plugaddr) {
	char            name[NAMELEN];
	short           bit = 0;
	unsigned short 	handle;

	while(true) {
		switch(parserGetToken()) {
		case tokRBracket:
		   	/* now the parsing of this plug is over */
			return;
	   	case tokPin:
		   	bit = ap216PinToBit(parserExpectNumber("Pin-Nummer erwartet"));
		   	parserAssert(0 <= bit, "kenne Pin-Nummer nicht");
		   	parserExpectName(name, "Name des Ausgangspins erwartet");
			handle = outshiftGetHandle(plugaddr - 1, bit);
		   	registerSetName(handle, name);
		   	break;
		default:
			parserExit("\"Pin\" erwartet");
		}
	}
}

/*@maynotreturn@*/ 
void outshiftParser(void) 
{

	unsigned short addr = 0;
	unsigned short handle;
	char           name[NAMELEN];

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
			parserExit("outshift: Name des Steckers erwartet");
		}
		
		parserExpectTokenNeu(tokLBracket);
		parserExpectTokenNeu(tokAdr);
		addr = parserExpectNumber("Adresse erwartet");
		parserAssert(outshiftGetRegLen() >= addr, "Adresse ist zu gross");
		parserAssert(0 != addr, "Adresse null nicht erlaubt");
		/* copy the name of the plug
		 *  for calculation of address note that addr 1 is first
		 */
		handle = outshiftGetHandle(addr - 1, 0);
		registerSetPlugName(handle, name);
		outshiftPlugParser(addr);
	}
}

/** 
 * prints the data stored in the configuration in the same format
 * than the .conf file
 * to control the interpretation of the file
 */
void outshiftPrint(void) 
{
	unsigned short	i;
	short			pin;
	unsigned short	bit;
	char			name[NAMELEN];

	fprintf(stdout, "\t\tAusgaenge {\n");
	for(i = 0; i < outshiftGetRegLen(); i++) 
	{	
		registerGetPlugName(outshiftGetHandle(i, 0), name);
		fprintf(stdout, "\t\t\t%s {\n", name);
		fprintf(stdout, "\t\t\t\tAdr\t%hu\n", i + 1);

		for(pin = 1; pin < 9; pin++) 
		{
			/* find this pin */
			for(bit = 0; bit < 16; bit++) 
			{
				if(pin == ap216BitToPin(bit)) 
				{
					break;
				}
			}
			registerGetName(outshiftGetHandle(i, bit), name);
			if(0 != (strlen(name))) 
			{
				fprintf(stdout, "\t\t\t\tPin %d\t%s\n", pin, name);
			}
		}

		fprintf(stdout, "\n");

		for(pin = 11; pin < 19; pin++) 
		{
			/* find this pin */
			for(bit = 0; bit < 16; bit++) 
			{
				if(pin == ap216BitToPin(bit)) 
				{
					break;
				}
			}
			registerGetName(outshiftGetHandle(i, bit), name);
			if(0 != (strlen(name))) 
			{
				fprintf(stdout, "\t\t\t\tPin %d\t%s\n", pin, name);
			}
		}
		fprintf(stdout, "\t\t\t}\n");
	}
	fprintf(stdout, "\t\t}\n");
}

