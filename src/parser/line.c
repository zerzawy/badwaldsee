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
 * Functions for parsing and printing parameters of lines (Einspurstrecken oder Doppelspurstrecken) 
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
#include "../dwarf.h"
#include "parser.h"
#include "../line.h"
#include "line.h"
#include "mains.h"
#include "button.h"
#include "../route.h"

/**
 * Parser for one line
 * @param pLine	pointer to this line 
 * @exception		Prints an error and exits on a case of wrong data in the 
 * 			parserfile
 */
/*@maynotreturn@*/
static void lineSingleParser(struct sLine * const pLine);
/*@maynotreturn@*/
static void lineSingleParser(struct sLine * const pLine)
{
	unsigned short	i;
	unsigned short	sections	= 0;
	unsigned short	blocks	= 0;
	enum eToken     tok	= tokNone;
	char            name[NAMELEN];

	assert(NULL != pLine);
	
	parserExpectTokenNeu(tokLBracket);

	tok = parserGetWord(name);
	switch(tok)
	{
	case tokNachrechts:	
		lineSetToRight(pLine, true); 
		break;

	case tokNachlinks:
		lineSetToRight(pLine, false); 
		break;

	default:
		parserAssert(false, "NachRechts oder NachLinks erwartet");
		break;
	}
	
	#ifdef SPDR60
		tok = parserGetWord(name);
		if(tokEaT == tok)
		{
			/* now a single line is used, expect all keys	*/
			(void) parserExpectName(name, "Tastenname von EaT erwartet");
			lineSetEaT(pLine, buttonFind(name));

			/* now get the next token       */
			tok = parserGetWord(name);
			if(tokMitSteuerungstaste == tok)
			{
				lineSetEaTWithControl(pLine, true);
				tok = parserGetWord(name);
			}

			parserAssert(tokErlhT == tok, "MitSteuerungstaste oder ErlhT erwartet");
			(void) parserExpectName(name, "Tastenname von ErlhT erwartet");
			lineSetErlhT(pLine, buttonFind(name));

			/* now get the next token       */
			tok = parserGetWord(name);
			if(tokMitSteuerungstaste == tok)
			{
				lineSetErhTWithControl(pLine, true);
				tok = parserGetWord(name);
			}

			parserAssert(tokASpM == tok, "MitSteuerungstaste oder ASpM erwartet");
			lineSetBO_ASpM(pLine, parserExpectRegister("Name des ASpM erwartet"));

			parserExpectTokenNeu(tokEaM);  
			lineSetBO_EaM(pLine, parserExpectRegister("Name des EaM erwartet"));
				
			parserExpectTokenNeu(tokEeM);  
			lineSetBO_EeM(pLine, parserExpectRegister("Name des EeM erwartet"));
			
			tok = parserGetWord(name);
		}
		/* parse the sections and add them to the structure	*/

		parserAssert(tokAbschnitte == tok, "Abschnitte erwartet");
		sections = parserExpectNumber("Anzahl Abschnitte erwartet");
		parserAssert(LINENRSECTIONS > sections, "Zu viele Abschnitte angegeben");
		parserAssert(0 < sections, "Mindestens ein Abschnitt notwendig");
		parserExpectTokenNeu(tokLBracket);

		for(i = 0; i < sections; i++) 
		{
			tok = parserGetWord(name);
			parserAssert(tokNone == tok, "Name des Abschnitts erwartet");
			lineAddSection(pLine, sectionFind(name));
		}

		/* now a decision is to be made. either it is a single line, in this
		 * case two series of blocks (on from my station and one towards my
		 * station have to be parsed
		 * or it is a double line, in this case, one series of block away
		 * from my station have to be parse. But it can be Bloecke (central
		 * blocks) or Selbsbloecke
		 */

		if(NULL == lineGetEaT(pLine))
		{
			/* ne EaT means a double line	*/
			tok = parserGetWord(name);
			switch(tok)
			{
			case tokBloecke:	
				lineSetSelfblock(pLine, false); 
				break;

			case tokSelbstbloecke:
				lineSetToRight(pLine, true); 
				break;

			default:
				parserAssert(false, "Bloecke oder Selbstbloecke erwartet");
				break;
			}
		}
		else
		{
			/* parse the blocks towards my station	*/
			parserExpectTokenNeu(tokBloecke);
			blocks = parserExpectNumber("Anzahl Bloecke erwartet");
			parserAssert(LINENRBLOCKSTO > blocks, "Zu viele Bloecke angegeben");
			parserAssert(0 < sections, "Mindestens ein Block notwendig");
			parserExpectTokenNeu(tokLBracket);

			for(i = 0; i < blocks; i++)
			{
				tok = parserGetWord(name);
				parserAssert(tokNone == tok, "Name des Abschnitts erwartet");
				lineAddBlockToName(pLine, name);
			}

			/* parse the blocks away from my station	*/
			parserExpectTokenNeu(tokBloecke);
		}

		blocks = parserExpectNumber("Anzahl Bloecke erwartet");
		parserAssert(LINENRBLOCKSFROM > blocks, "Zu viele Bloecke angegeben");
		parserAssert(0 < sections, "Mindestens ein Block notwendig");
		parserExpectTokenNeu(tokLBracket);

		for(i = 0; i < blocks; i++)
		{
			tok = parserGetWord(name);
			parserAssert(tokNone == tok, "Name des Abschnitts erwartet");
			lineAddBlockFromName(pLine, name);
		}

		tok = parserGetToken();
		parserAssert(tokRBracket == tok, "zu viele Abschnitte angegeben");
	#endif
}

/**
 * Function to parse a single track from a .conf file
 */
/*@maynotreturn@*/ 
void lineParser(void)
{
	struct sLine *	pLine = NULL;
	unsigned short		nr = 0;
	unsigned short		i = 0;
	enum eToken		tok = tokNone;
	char			name[NAMELEN];
 
	parserExpectTokenNeu(tokLBracket);
	parserExpectTokenNeu(tokStrecken);
	nr = parserExpectNumber("Anzahl Strecken erwartet");
	lines(nr);		/* construct line structure */

	parserExpectTokenNeu(tokLBracket);
	while(NULL != (pLine = lineGet(i)))
	{
		tok = parserGetWord(name);
		parserAssert(tokNone == tok, "Name der Strecke erwartet");
		lineSetName(pLine, name);
		lineSingleParser(pLine);
		i++;
	}

	tok = parserGetToken();
	parserAssert(tokRBracket == tok, "zu viele Einspurstrecken angegeben");
	/* I did not use parserExpectToken, because a better error 
	 * message can be given
	 */

	parserExpectTokenNeu(tokRBracket);
}

/**
 * Function to set all forward annotations correctly. For lines, this are the block names
 */
void lineParserResolve(void)
{	
	struct sLine *		pLine;
	enum eRoutePartType	type;
	struct sBlock *		pBlock;
	unsigned short		i, j;
	char			name[NAMELEN];

	for(i = 0; i < linesGetNr(); i++)
	{
		pLine = lineGet(i);

		for(j = 0; j < LINENRBLOCKSTO; j++)
		{
			/* a line must be found.	*/
			(void) lineGetBlockToName(pLine, name, j);
			lineSetBlockTo(pLine, blockFind(name), j);
		}

		for(j = 0; j < LINENRBLOCKSFROM; j++)
		{
			/* a line must be found.	*/
			(void) lineGetBlockToName(pLine, name, j);
			lineSetBlockFrom(pLine, blockFind(name), j);
		}
	}
}

/**
 * Prints the single line
 * @param pLine	pointer to the line to be printed
 * @exception		Assert on NULL pointer
 */
static void lineSinglePrint(struct sLine * const pLine);
static void lineSinglePrint(struct sLine * const pLine) 
{
	char			name[NAMELEN];
	unsigned short		i;
	unsigned short		nrParts;
	struct sSection *	pSection	= NULL;
	struct sBlock *		pBlock	= NULL;

	assert(NULL != pLine);

	lineGetName(pLine, name);
	fprintf(stdout, "\t\t\t%s {\n", name);
	if(lineGetToRight(pLine))
	{
		fprintf(stdout, "\t\t\t\tNachRechts\n");
	} else {
		fprintf(stdout, "\t\t\t\tNachLinks\n");
	}

	#ifdef SPDR60
		buttonGetName(lineGetEaT(pLine), name);
		fprintf(stdout, "\t\t\t\tEaT\t\t%s", name);
		
		if(lineGetEaTWithControl(pLine))
		{
			fprintf(stdout, "\tmitSteuerungstaste\n");
		}
		else
		{
			fprintf(stdout, "\n");
		}

		buttonGetName(lineGetErlhT(pLine), name);
		fprintf(stdout, "\t\t\t\tErlhT\t\t%s", name);

		if(lineGetErhTWithControl(pLine))
		{
			fprintf(stdout, "\tmitSteuerungstaste\n");
		}
		else
		{
			fprintf(stdout, "\n");
		}

		registerGetName(lineGetBO_ASpM(pLine), name);
		fprintf(stdout, "\t\t\t\tASpM\t\t%s\n", name);

		registerGetName(lineGetBO_EaM(pLine), name);
		fprintf(stdout, "\t\t\t\tEaM\t\t%s\n", name);

		registerGetName(lineGetBO_EeM(pLine), name);
		fprintf(stdout, "\t\t\t\tEeM\t\t%s\n", name);
	#endif

	/* count number of sections	*/
	i = 0;
	while(NULL != lineGetSection(pLine, i))
	{
		i++;
	}
	
	fprintf(stdout, "\t\t\t\tAbschnitte\t%d {\n", i);

	fprintf(stdout, "\t\t\t\t\t");
	while(NULL != (pSection = lineGetSection(pLine, i)))
	{
		sectionGetName(pSection, name);
		fprintf(stdout, "%s ", name);
	}
	fprintf(stdout, "\t\t\t\t}\n", i);

	/* now the blocks to my station are printed	*/
	i = 0;
	while(NULL != lineGetBlockTo(pLine, i))
	{
		i++;
	}
	
	fprintf(stdout, "\t\t\t\tBloecke\t%d {\n", i);

	fprintf(stdout, "\t\t\t\t\t");
	while(NULL != (pBlock = lineGetBlockTo(pLine, i)))
	{
		blockGetName(pBlock, name);
		fprintf(stdout, "%s ", name);
	}
	fprintf(stdout, "\t\t\t\t}\n", i);

	/* now the blocks from my station are printed	*/
	i = 0;
	while(NULL != lineGetBlockFrom(pLine, i))
	{
		i++;
	}
	
	fprintf(stdout, "\t\t\t\tBloecke\t%d {\n", i);

	fprintf(stdout, "\t\t\t\t\t");
	while(NULL != (pBlock = lineGetBlockFrom(pLine, i)))
	{
		blockGetName(pBlock, name);
		fprintf(stdout, "%s ", name);
	}
	fprintf(stdout, "\t\t\t\t}\n", i);
	fprintf(stdout, "\t\t\t}\n");
}

/**
 * Function to print out a single track (Einspurstrecke)
 * used to check the parser function
 */
void linePrint(void) 
{
	unsigned short	i = 0;
	struct sLine *	pLine = NULL;

	fprintf(stdout, "\t{\n");
	fprintf(stdout, "\t\tZuFas %u {\n", linesGetNr());
	while(NULL != (pLine = lineGet(i)))
	{
		lineSinglePrint(pLine);
		i++;
	}
	fprintf(stdout, "\t\t}\n");
	fprintf(stdout, "\t}\n");
}
