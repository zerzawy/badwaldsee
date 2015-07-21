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
 * Functions for parsing and printing parameters of blocks (Einspurstrecken) 
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
#include "../block.h"
#include "block.h"
#include "mains.h"

/**
 * Parser for one block
 * @param pBlock	pointer to this block 
 * @exception		Prints an error and exits on a case of wrong data in the 
 * 			parserfile
 */
/*@maynotreturn@*/
static void blockSingleParser(struct sBlock * const pBlock);
/*@maynotreturn@*/
static void blockSingleParser(struct sBlock * const pBlock)
{
	unsigned short	i;
	unsigned short	sections	= 0;
	unsigned short	blocks	= 0;
	enum eToken     tok	= tokNone;
	char            name[NAMELEN];

	assert(NULL != pBlock);
	
	parserExpectTokenNeu(tokLBracket);

	parserExpectTokenNeu(tokTischLEDBelegt);
	blockSet_BO_DeskLED(pBlock, parserExpectRegister("Name der Tisch-LED erwartet"));

	parserExpectTokenNeu(tokBlocksignal);
	tok = parserGetWord(name);
	parserAssert(tokNone == tok, "Name des Blocksignals erwartet");
	blockSetMain(pBlock, mainFind(name));

	parserExpectTokenNeu(tokAbschnitte);
	sections = parserExpectNumber("Anzahl Abschnitte erwartet");
	parserAssert(BLOCKNRSECTIONS > sections, "Zu viele Abschnitte angegeben");
	parserAssert(0 < sections, "Mindestens ein Abschnitt notwendig");
	parserExpectTokenNeu(tokLBracket);

	for(i = 0; i < sections; i++) 
	{
		tok = parserGetWord(name);
		parserAssert(tokNone == tok, "Name des Abschnitts erwartet");
		blockAddSection(pBlock, sectionFind(name));
	}

	tok = parserGetToken();
	parserAssert(tokRBracket == tok, "zu viele Abschnitte angegeben");
}

/**
 * Function to parse a block from a .conf file
 */
/*@maynotreturn@*/ 
void blockParser(void)
{
	struct sBlock *		pBlock = NULL;
	unsigned short		nr = 0;
	unsigned short		i = 0;
	enum eToken		tok = tokNone;
	char			name[NAMELEN];
 
	parserExpectTokenNeu(tokLBracket);
	parserExpectTokenNeu(tokBloecke);
	nr = parserExpectNumber("Anzahl Bloecke erwartet");
	blocks(nr);		/* construct block structure */

	parserExpectTokenNeu(tokLBracket);
	while(NULL != (pBlock = blockGet(i)))
	{
		tok = parserGetWord(name);
		parserAssert(tokNone == tok, "Name des Blocks erwartet");
		blockSetName(pBlock, name);
		blockSingleParser(pBlock);
		i++;
	}

	tok = parserGetToken();
	parserAssert(tokRBracket == tok, "zu viele Bloecke angegeben");
	/* I did not use parserExpectToken, because a better error 
	 * message can be given
	 */

	parserExpectTokenNeu(tokRBracket);
}

/**
 * Prints the single block
 * @param pBlock	pointer to the block to be printed
 * @exception		Assert on NULL pointer
 */
static void blockSinglePrint(struct sBlock * const pBlock);
static void blockSinglePrint(struct sBlock * const pBlock) 
{
	char			name[NAMELEN];
	unsigned short		i;
	struct sSection *	pSection	= NULL;

	assert(NULL != pBlock);

	blockGetName(pBlock, name);
	fprintf(stdout, "\t\t\t%s {\n", name);
	
	registerGetName(blockGet_BO_DeskLED(pBlock), name);
	fprintf(stdout, "\t\t\t\tTischBelegtLED\t%s\n", name);

	mainGetName(blockGetMain(pBlock), name);
	fprintf(stdout, "\t\t\t\tBlocksignal\t%s\n", name);

	/* count number of sections	*/
	i = 0;
	while(NULL != blockGetSection(pBlock, i))
	{
		i++;
	}
	
	fprintf(stdout, "\t\t\t\tAbschnitte\t%d {\n", i);

	fprintf(stdout, "\t\t\t\t\t");
	while(NULL != (pSection = blockGetSection(pBlock, i)))
	{
		sectionGetName(pSection, name);
		fprintf(stdout, "%s ", name);
	}

	fprintf(stdout, "\t\t\t\t}\n");
	fprintf(stdout, "\t\t\t}\n");
}

/**
 * Function to print out a single track (Einspurstrecke)
 * used to check the parser function
 */
void blockPrint(void) 
{
	unsigned short	i = 0;
	struct sBlock *	pBlock = NULL;

	fprintf(stdout, "\t{\n");
	fprintf(stdout, "\t\tZuFas %u {\n", blocksGetNr());
	while(NULL != (pBlock = blockGet(i)))
	{
		blockSinglePrint(pBlock);
		i++;
	}
	fprintf(stdout, "\t\t}\n");
	fprintf(stdout, "\t}\n");
}
