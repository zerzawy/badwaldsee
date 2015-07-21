/* 
 * This file is part of Raspberry Pi steuert Modelleisenbahn
 * Copyright(C)  2010 - 2015 Kurt Zerzawy www.zerzawy.ch
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
 * Functions for parsing and printing parameters of temporary data storage
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

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "assert.h"
#include "parser.h"
#include "../registers.h"
#include "../vars.h"

/**
 * parser for a single bit of the register
 * @param mode   tells, if input, output or vars registers are parsed
 * @param plughandle	 address of the word inside this mode
 * @exception    assertion thrown on NULL pointer
 */
/*@maynotreturn@*/
static void varsBitParser(const unsigned short plughandle);

/*@maynotreturn@*/
static void varsBitParser(const unsigned short plughandle) {
	char            name[NAMELEN];
	unsigned short  varbit = 0;
	unsigned short 	handle;

	assert(plughandle < (0x10 * registerGetLen()));

	while(true) {
		switch(parserGetToken()) {
		case tokRBracket:
		   	/* it was last one */
			return;
		case tokBit:
			/* now a bit number will be given */
		   	varbit = parserExpectNumber("Bit-Nummer erwartet");
		   	parserAssert(16 > varbit, "Wert muss kleiner als 16 sein");
		   	parserExpectName(name, "Name des Bits erwartet"); 
			handle = plughandle + varbit;
			registerSetName(handle, name);
		   	break;
		default:
			parserExit("\"Bit\" erwartet");
		}
	}      
}

/**
 * parser for a plug or word of the register
 * @param mode   tells, if input, output or vars registers are parsed
 * @param basehandle   number of the first register of this kind
 * @param regLen number of registers in this mode
 * @exception    assertion thrown on NULL pointer
 */
/*@maynotreturn@*/ 
void varsParser(void)
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
			parserExit("Name des Steckers erwartet");
		}
		
		parserExpectTokenNeu(tokLBracket);
		parserExpectTokenNeu(tokAdr);
		addr = parserExpectNumber("Adresse erwartet");
		parserAssert(varsGetRegLen() >= addr, "Adresse ist zu gross");
		parserAssert(0 != addr, "Adresse null nicht erlaubt");
		/* copy the name of the plug
		 *  for calculation of address note that addr 1 is first
		 */
		handle = registerGetFirstVarHandle() + (addr - 1) * 0x10;
		registerSetPlugName(handle, name);
		varsBitParser(handle);
	}
}

void varsPrint(void) 
{
	unsigned short	wordHandle;
	unsigned short	handle;
	unsigned short	i;
	unsigned short	bit;
	char		name[NAMELEN];

	wordHandle = registerGetFirstVarHandle();
	fprintf(stdout, "\t\tVariablen {\n");
	for(i = 0; i < varsGetRegLen(); i++) 
	{
		registerGetPlugName(wordHandle, name);
		fprintf(stdout, "\t\t\t%s {\n", name);
		fprintf(stdout, "\t\t\t\tAdr\t%hu\n", i + 1);
		for(bit = 0; bit < 16; bit++) {
			handle = wordHandle + bit;
			registerGetName(handle, name); 
			if(0 != (strlen(name))) 
			{
				fprintf(stdout, "\t\t\t\tBit %d\t%s\n",
					bit, name);
			}
		}

		fprintf(stdout, "\t\t\t}\n");
		wordHandle += 0x10;
	}
	fprintf(stdout, "\t\t}\n");
}

