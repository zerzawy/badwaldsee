/*
 * This file is part of Raspberry Pi steuert Modelleisenbahn
 * Copyright(C) 2014 - 2015 Kurt Zerzawy www.zerzawy.ch
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
 * Functions for parsing and printing parameters of shunting routes (Rangierfahrstrassen) 
 * \author
 * Kurt Zerzawy
 */

/*
 * $Author: kurt $
 * $Date: 2015-06-23 22:35:22 +0200 (Di, 23 Jun 2015) $
 * $Revision: 2490 $
 */

#include <assert.h>
#include "../log.c"
#include "parser.h"
#include "../shuntroute.h"
#include "shuntroute.h"
#include "turnout.h"

/*@maynotreturn@*/
static void shuntrouteSinglePartParser(struct sShuntroute * const pShuntroute, const unsigned short i);

/**
 * Parser for a single part of a shuntroute
 * @param pShuntroute	pointer to this shuntroute
 * @param i		number of the part
 * @exception		Prints an error and exits on a case of wrong data in the
 *			 parserfile
 */
static void shuntrouteSinglePartParser(struct sShuntroute * const pShuntroute, const unsigned short i)
{
	struct sTurnout *	pTurnout = NULL;
	struct sSection *		pSection = NULL;
	struct sDwarf *		pDwarf = NULL;
	enum eToken     	tok = tokNone;
	char    	        name[NAMELEN];

	assert(NULL != pShuntroute);
	assert(i < NRSHUNTPARTS);

	tok = parserGetWord(name);
	switch(tok)
	{
	case tokWeiche:
		/* now set the reset section in case this is the first 
		 * turnout in the way. This is known by the fact
		 * that the reset section number is bigger than i, 
		 * since it was set to NRSHUNTPARTS
		 */
		if(i < shuntrouteGetResetSection(pShuntroute))
		{
			shuntrouteSetResetSection(pShuntroute, i);
		}
		
	        tok = parserGetWord(name);
		parserAssert(tokNone == tok, "Name der Weiche erwartet");
	        pTurnout = turnoutFindPtr(name);

		tok = parserGetWord(name);
		switch(tok) 
		{
		#ifdef DOMINO55
			case tokRechts:	
		#endif

		#ifdef SPDR60
			case tokPlus:
		#endif
			if(shuntrouteGetToRight(pShuntroute))
			{
				shuntrouteAddTurnout(pShuntroute, pTurnout, interlockPlusLR);
			} else {
				shuntrouteAddTurnout(pShuntroute, pTurnout, interlockPlusRL);
			}
			break;

		#ifdef DOMINO55
			case tokLinks:
		#endif

		#ifdef SPDR60
			case tokMinus:
		#endif
			if(shuntrouteGetToRight(pShuntroute))
			{
				shuntrouteAddTurnout(pShuntroute, pTurnout, interlockMinusLR);
			} else {
				shuntrouteAddTurnout(pShuntroute, pTurnout, interlockMinusRL);
			}
			break;
		default:
			parserAssert(false, "Rechts oder Links erwartet");
			break;
		}
		break;

	case tokSchutzweiche:
	        tok = parserGetWord(name);
		parserAssert(tokNone == tok, "Name der Weiche erwartet");
	        pTurnout = turnoutFindPtr(name);

		tok = parserGetWord(name);
		switch(tok) 
		{
		#ifdef DOMINO55
			case tokRechts:	
		#endif

		#ifdef SPDR60
			case tokPlus:
		#endif
			shuntrouteAddProtectiveTurnout(pShuntroute, pTurnout, interlockProtectPlus);
			break;

		#ifdef DOMINO55
			case tokLinks:	
		#endif

		#ifdef SPDR60
			case tokMinus:
		#endif
			shuntrouteAddProtectiveTurnout(pShuntroute, pTurnout, interlockProtectMinus);
			break;

		default:		
			parserAssert(false, "Rechts oder Links erwartet");
			break;
		}
		break;

	case tokAbschnitt:
	        tok = parserGetWord(name);
		parserAssert(tokNone == tok, "Name des Abschnitts erwartet");
	        pSection = sectionFind(name);
		shuntrouteAddSection(pShuntroute, pSection);
		break;

	case tokSchutzabschnitt:
	        tok = parserGetWord(name);
		parserAssert(tokNone == tok, "Name des Abschnitts erwartet");
	        pSection = sectionFind(name);
		shuntrouteAddProtectiveSection(pShuntroute, pSection);
		break;

	case tokSchutzsignal:
	        tok = parserGetWord(name);
		parserAssert(tokNone == tok, "Name des Schutzsignales erwartet");
	        pDwarf = dwarfFind(name);
		shuntrouteAddProtectiveDwarf(pShuntroute, pDwarf);
		break;

	default:
		parserAssert(false, "Rangierfahrstrassenteil erwartet");
		break;
	}
}


/*@maynotreturn@*/
static void shuntrouteSingleParser(struct sShuntroute * const pShuntroute);

/**
 * Parser for one shuntroute
 * @param pShuntroute	pointer to this shuntroute 
 * @exception		Prints an error and exits on a case of wrong data in the 
 * 			parserfile
 */
/*@maynotreturn@*/
static void shuntrouteSingleParser(struct sShuntroute * const pShuntroute)
{
	unsigned short	i;
	unsigned short	parts;
	enum eToken     tok = tokNone;
	char            name[NAMELEN];

	assert(NULL != pShuntroute);
	
	parserExpectTokenNeu(tokLBracket);

	tok = parserGetWord(name);
	switch(tok) 
	{
	case tokNachrechts:	shuntrouteSetToRight(pShuntroute, true); break;
	case tokNachlinks:	shuntrouteSetToRight(pShuntroute, false); break;
	default:		parserAssert(false, "NachRechts oder NachLinks erwartet");
				break;
	}
	
	parserExpectTokenNeu(tokStartsignal);
	tok = parserGetWord(name);
	parserAssert(tokNone == tok, "Name des Startsignals erwartet");
	if(0 != strcmp("NONE", name))
	{
		shuntrouteSetStart(pShuntroute, dwarfFind(name));
	}

	parserExpectTokenNeu(tokZielsignal);
	tok = parserGetWord(name);
	parserAssert(tokNone == tok, "Name des Zielsignals erwartet");
	if(0 != strcmp("NONE", name))
	{
		shuntrouteSetDest(pShuntroute, dwarfFind(name));
	}

	parserExpectTokenNeu(tokRaFateile);
	parts = parserExpectNumber("Anzahl Rangierfahrstrassen-Teile erwartet");
	parserAssert(NRSHUNTPARTS > parts, "Zu viele Rangierfahrstrassen-Teile angegeben");
	parserAssert(0 < NRSHUNTPARTS, "Mindestens ein Rangierfahrstrassen-Teil notwendig");

	parserExpectTokenNeu(tokLBracket);

	/* prepare also to find out which is the reset section, this is the section telling 
	 * the signal to fall to halt position. 
	 * since the value resetSection is unsigned, I use NRSHUNTPARTS als start value.
	 */
	
	pShuntroute->resetSection = NRSHUNTPARTS;

	for(i = 0; i < parts; i++) 
	{
		shuntrouteSinglePartParser(pShuntroute, i);
	}
	shuntrouteCalcRoute(pShuntroute);
	tok = parserGetToken();
	parserAssert(tokRBracket == tok, "zu viele Rangierfahrstrassen-Teile angegeben");
	parserExpectTokenNeu(tokRBracket);
}

/*@maynotreturn@*/ 
void shuntrouteParser(void)
{
	struct sShuntroute *	pShuntroute;
	unsigned short		nr = 0;
	unsigned short		i = 0;
	enum eToken		tok = tokNone;
	char			name[NAMELEN];
 
	parserExpectTokenNeu(tokLBracket);
	parserExpectTokenNeu(tokRaFas);
	nr = parserExpectNumber("Anzahl Rangierfahrstrassen erwartet");
	shuntroute(nr);		/* construct shuntroute structure */

	parserExpectTokenNeu(tokLBracket);
	while(NULL != (pShuntroute = shuntrouteGet(i)))
	{
		tok = parserGetWord(name);
		parserAssert(tokNone == tok, "Name der Rangierfahrstrasse erwartet");
		shuntrouteSetName(pShuntroute, name);
		shuntrouteSingleParser(pShuntroute);
		i++;
	}

	tok = parserGetToken();
	parserAssert(tokRBracket == tok, "zu viele Rangierfahrstrassen angegeben");
	/* I did not use parserExpectToken, because a better error 
	 * message can be given
	 */

	parserExpectTokenNeu(tokRBracket);
}

/**
 * Prints the part of a single shuntroute
 * @param pShuntroute	pointer to the shuntroute to be printed
 * @exception		Assert on NULL pointer
 */
static void shuntrouteSinglePartPrint(struct sShuntroute * const pShuntroute, unsigned short part);
static void shuntrouteSinglePartPrint(struct sShuntroute * const pShuntroute, unsigned short part) 
{
	struct sTurnout *	pTurnout = NULL;
	struct sSection *		pSection = NULL;
	struct sDwarf *		pDwarf = NULL;
	enum eTurnoutLine	turnoutLine;
	enum eTurnoutProtectiveLine	turnoutProtectiveLine;
	char			name[NAMELEN];

	switch(shuntrouteGetPartType(pShuntroute, part))
	{
	case ShuntroutePartNone:
		break;
	case ShuntroutePartTurnout:
		fprintf(stdout, "\t\t\t\t\tWeiche");
		
		pTurnout = shuntrouteGetTurnout(pShuntroute, part);
		turnoutGetName(pTurnout, name);
		fprintf(stdout, "\t\t%s", name);

		turnoutLine = shuntrouteGetTurnoutArgument(pShuntroute, part);
		switch(turnoutLine)
		{
		case interlockPlusLR:
		case interlockPlusRL:
#ifdef DOMINO55
			fprintf(stdout, "\tRechts\n");
#endif
#ifdef SPDR60
			fprintf(stdout, "\tPlus\n");
#endif
			break;
		case interlockMinusLR:
		case interlockMinusRL:
#ifdef DOMINO55
			fprintf(stdout, "\tLinks\n");
#endif
#ifdef SPDR60
			fprintf(stdout, "\tMinus\n");
#endif
			break;
		default:
			fprintf(stdout, "unbekannter default");
			assert(false);
		}
		break;
	case ShuntroutePartProtectiveTurnout:
		fprintf(stdout, "\t\t\t\t\tSchutzweiche");

		pTurnout = shuntrouteGetProtectiveTurnout(pShuntroute, part);
		turnoutGetName(pTurnout, name);
		fprintf(stdout, "\t%s", name);

		turnoutProtectiveLine = shuntrouteGetProtectiveTurnoutArgument(pShuntroute, part);
		switch(turnoutProtectiveLine)
		{
		case interlockProtectPlus:
#ifdef DOMINO55
			fprintf(stdout, "\tRechts\n");
#endif
#ifdef SPDR60
			fprintf(stdout, "\tPlus\n");
#endif
			break;
		case interlockProtectMinus:
#ifdef DOMINO55
			fprintf(stdout, "\tLinks\n");
#endif
#ifdef SPDR60
			fprintf(stdout, "\tMinus\n");
#endif
			break;
		default:
			fprintf(stdout, "unbekannter default");
			assert(false);
		}
		break;
	case ShuntroutePartSection:
		fprintf(stdout, "\t\t\t\t\tAbschnitt");

		pSection = shuntrouteGetSection(pShuntroute, part);
		sectionGetName(pSection, name);
		fprintf(stdout, "\t%s\n", name);
		break;
	case ShuntroutePartProtectiveSection:
		fprintf(stdout, "\t\t\t\t\tSchutzabschnitt");

		pSection = shuntrouteGetProtectiveSection(pShuntroute, part);
		sectionGetName(pSection, name);
		fprintf(stdout, "\t%s\n", name);
		break;
		break;
	case ShuntroutePartProtectiveDwarf:
		fprintf(stdout, "\t\t\t\t\tSchutzsignal");

		pDwarf = shuntrouteGetProtectiveDwarf(pShuntroute, part);
		dwarfGetName(pDwarf, name);
		fprintf(stdout, "\t%s\n", name);
		break;
	default:
		LOG_ERR("unexpected default\n");
		assert(false);
	}

}

/**
 * Prints the single shuntroute
 * @param pShuntroute	pointer to the shuntroute to be printed
 * @exception	Assert on NULL pointer
 */
static void shuntrouteSinglePrint(struct sShuntroute * const pShuntroute);
static void shuntrouteSinglePrint(struct sShuntroute * const pShuntroute) 
{
	char			name[NAMELEN];
	unsigned short		i;
	unsigned short		nrParts;

	shuntrouteGetName(pShuntroute, name);
	fprintf(stdout, "\t\t\t%s {\n", name);
	if(shuntrouteGetToRight(pShuntroute))
	{
		fprintf(stdout, "\t\t\t\tNachRechts\n");
	} else {
		fprintf(stdout, "\t\t\t\tNachLinks\n");
	}

	dwarfGetName(shuntrouteGetStart(pShuntroute), name);
	fprintf(stdout, "\t\t\t\tStartsignal\t%s\n", name);

	dwarfGetName(shuntrouteGetDest(pShuntroute), name);
	fprintf(stdout, "\t\t\t\tZielsignal\t%s\n", name);

	nrParts = shuntrouteGetParts(pShuntroute);
	fprintf(stdout, "\t\t\t\tRaFateile\t%d {\n", nrParts);
	
	for(i = 0; i < nrParts; i++)
	{
		shuntrouteSinglePartPrint(pShuntroute, i);
	}

	fprintf(stdout, "\t\t\t\t}\n");
	fprintf(stdout, "\t\t\t}\n");
}

void shuntroutePrint(void) 
{
	unsigned short		i = 0;
	struct sShuntroute *	pShuntroute;

	fprintf(stdout, "\t{\n");
	fprintf(stdout, "\t\tRaFas %u {\n", shuntrouteGetNr());
	while(NULL != (pShuntroute = shuntrouteGet(i)))
	{
		shuntrouteSinglePrint(pShuntroute);
		i++;
	}
	fprintf(stdout, "\t\t}\n");
	fprintf(stdout, "\t}\n");
}

