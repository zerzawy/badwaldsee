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
 * Functions for parsing and printing parameters of routes (Zugfahrstrassen) 
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
#include "../route.h"
#include "route.h"
#include "turnout.h"
#include "../turnout.h"
#include "mains.h"
#include "../line.h"

/*@maynotreturn@*/
static void routeSinglePartParser(struct sRoute * const pRoute, const unsigned short i);

/**
 * Parser for a single part of a route
 * @param pRoute	pointer to this route
 * @param i		number of the part
 * @exception   	Prints an error and exits on a case of wrong data in the
 *              	parserfile
 */
static void routeSinglePartParser(struct sRoute * const pRoute, const unsigned short i)
{
	struct sTurnout *	pTurnout = NULL;
	struct sShuntroute *	pShuntroute = NULL;
	enum eToken     	tok = tokNone;
	char    	        name[NAMELEN];
	char			dwarfName[NAMELEN];

	assert(NULL != pRoute);
	assert(i < NRROUTEPARTS);

	tok = parserGetWord(name);
	switch(tok)
	{
	case tokWeiche:
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
			if(routeGetToRight(pRoute))
			{
				routeAddTurnout(pRoute, pTurnout, interlockPlusLR);
			} else {
				routeAddTurnout(pRoute, pTurnout, interlockPlusRL);
			}
			break;
#ifdef DOMINO55
		case tokLinks:
#endif
#ifdef SPDR60
		case tokMinus:
#endif
			if(routeGetToRight(pRoute))
			{
				routeAddTurnout(pRoute, pTurnout, interlockMinusLR);
			} else {
				routeAddTurnout(pRoute, pTurnout, interlockMinusRL);
			}
			break;
		default:
			parserAssert(false, "Rechts oder Links erwartet");
			break;
		}
		break;

	case tokRaFa:
	        tok = parserGetWord(name);
		parserAssert(tokNone == tok, "Name der RaFa erwartet");
	        pShuntroute = shuntrouteFind(name);
		routeAddShuntroute(pRoute, pShuntroute);
		/* TODO here the sections of the shuntoute are inserted	*/
		break;

	case tokBedingungsRaFa:
		tok = parserGetWord(name);
		parserAssert(tokNone == tok, "Name der Bedingungs-RaFa erwartet");
		pShuntroute = shuntrouteFind(name);
		routeAddCondShuntroute(pRoute, pShuntroute);
		break;

	case tokVerbotsZuFa:
	        tok = parserGetWord(name);
		parserAssert(tokNone == tok, "Name der Verbots-ZuFa erwartet");
		routeAddIllegalRoute(pRoute, name);
		break;

	case tokBedingungsZuFa:
		tok = parserGetWord(name);
		parserAssert(tokNone == tok, "Name der Bedingungs-ZuFa erwartet");
		routeAddCondRoute(pRoute, name);
		break;

	case tokBlock:
	        tok = parserGetWord(name);
		parserAssert(tokNone == tok, "Name der Blockstrecke erwartet");

		#ifdef DOMINO55
			/* DOMINO needs to remember the last dwarf to set it correctly	*/
			tok = parserGetWord(dwarfName);
			parserAssert(tokNone == tok, "Name des Zwergsignals erwartet");
			
			routeAddLine(pRoute, name, dwarfFind(dwarfName));
			break;
		#endif

		#ifdef SPDR60
			routeAddLine(pRoute, name, NULL);
			break;
		#endif

	default:
		parserAssert(false, "Zugfahrstrassenteil erwartet");
		break;
	}
}

/**
 * Parser for one route
 * @param pRoute	pointer to this route 
 * @exception		Prints an error and exits on a case of wrong data in the 
 * 			parserfile
 */
/*@maynotreturn@*/
static void routeSingleParser(struct sRoute * const pRoute);
/*@maynotreturn@*/
static void routeSingleParser(struct sRoute * const pRoute)
{
	unsigned short	i;
	unsigned short	parts;
	enum eToken     tok = tokNone;
	char            name[NAMELEN];

	assert(NULL != pRoute);
	
	parserExpectTokenNeu(tokLBracket);

	tok = parserGetWord(name);
	switch(tok) 
	{
	case tokNachrechts:	
		routeSetToRight(pRoute, true); 
		break;

	case tokNachlinks:
		routeSetToRight(pRoute, false); 
		break;

	default:
		parserAssert(false, "NachRechts oder NachLinks erwartet");
		break;
	}
	
	#ifdef SPDR60
		/* TODO possibility of 'Umfahrstrasse'	*/
	#endif

	parserExpectTokenNeu(tokStartsignal);
	tok = parserGetWord(name);
	parserAssert(tokNone == tok, "Name des Startsignals erwartet");
	routeSetStart(pRoute, mainFind(name));

	parserExpectTokenNeu(tokZielsignal);
	tok = parserGetWord(name);
	parserAssert(tokNone == tok, "Name des Zielsignals erwartet");
	routeSetDest(pRoute, mainFind(name));

	parserExpectTokenNeu(tokZuFateile);
	parts = parserExpectNumber("Anzahl Zugfahrstrassen-Teile erwartet");
	parserAssert(NRROUTEPARTS > parts, "Zu viele Zugfahrstrassen-Teile angegeben");
	parserAssert(0 < NRROUTEPARTS, "Mindestens ein Zugfahrstrassen-Teil notwendig");
	parserExpectTokenNeu(tokLBracket);

	for(i = 0; i < parts; i++) 
	{
		routeSinglePartParser(pRoute, i);
	}
	tok = parserGetToken();
	parserAssert(tokRBracket == tok, "zu viele Zugfahrstrassen-Teile angegeben");

	#ifdef SPDR60
		/* TODO 'Durchrutschwege'	*/
	#endif

	/* parsing aspect of main signal for this route	*/
	parserExpectTokenNeu(tokFahrbegriff);
	tok = parserGetWord(name);

	#ifdef SPDR60
		switch(tok)
		{
		case tokHP0:	
			routeSetMainAspect(pRoute, SIG_MN_HP0);
			break;

		case tokHP1:	
			routeSetMainAspect(pRoute, SIG_MN_HP1);
			break;

		case tokHP1_ZS3:
			routeSetMainAspect(pRoute, SIG_MN_HP1_ZS3);
			break;

		case tokHP2:
			routeSetMainAspect(pRoute, SIG_MN_HP2);
			break;

		case tokHP2_ZS3:
			routeSetMainAspect(pRoute, SIG_MN_HP2_ZS3);
			break;

		default:
			parserAssert(tokRBracket == tok, "Fahrbegriff für Hauptsignal erwartet (Hp1, Hp1_Zs3, Hp2, Hp2_Zs3 oder Hp0)");
			break;
		}
	#endif

	#ifdef DOMINO55
		/* a little a special case. 1, 2, 3, and 5 should not be used as a token
		 * this is why these numbers are only compared as strings in this case
		 */
		switch(tok)
		{
		case tokHALT:	
			routeSetMainAspect(pRoute, SIG_MN_HALT);
			break;
	
		case tokNone:	
			if(0 == strcmp("1", name))
			{
				routeSetMainAspect(pRoute, SIG_MN_1);
			}
			if(0 == strcmp("2", name))
			{
				routeSetMainAspect(pRoute, SIG_MN_2);
			}
			if(0 == strcmp("3", name))
			{
				routeSetMainAspect(pRoute, SIG_MN_3);
			}
			if(0 == strcmp("5", name))
			{
				routeSetMainAspect(pRoute, SIG_MN_5);
			}
			if(0 == strcmp("6", name))
			{
				routeSetMainAspect(pRoute, SIG_MN_6);
			}
			break;

		default:
			parserAssert(tokRBracket == tok, "Fahrbegriff für Hauptsignal erwartet (1, 2, 3, 5, 6 oder Halt)");
			break;
		}
	#endif
	
	parserExpectTokenNeu(tokHaltmelder);
	tok = parserGetWord(name);
	parserAssert(tokNone == tok, "Name des Abschnitts erwartet");
	routeSetResetSection(pRoute, sectionFind(name));
	
	parserExpectTokenNeu(tokVorsignale);
	routeSetNrDistants(pRoute, parserExpectNumber("Anzahl Vorsignale erwartet"));
	/* TODO	*/
	assert(0 == routeGetNrDistants(pRoute));
	
	parserExpectTokenNeu(tokLBracket);

	parserExpectTokenNeu(tokRBracket);
	parserExpectTokenNeu(tokRBracket);
}

/*@maynotreturn@*/ 
void routeParser(void)
{
	struct sRoute *		pRoute = NULL;
	unsigned short		nr = 0;
	unsigned short		i = 0;
	unsigned short		j = 0;
	enum eToken		tok = tokNone;
	char			name[NAMELEN];
	enum eRoutePartType	ePartType;
 
	parserExpectTokenNeu(tokLBracket);
	parserExpectTokenNeu(tokZuFas);
	nr = parserExpectNumber("Anzahl Zugfahrstrassen erwartet");
	routes(nr);		/* construct route structure */

	parserExpectTokenNeu(tokLBracket);
	while(NULL != (pRoute = routeGet(i)))
	{
		tok = parserGetWord(name);
		parserAssert(tokNone == tok, "Name der Zugfahrstrasse erwartet");
		routeSetName(pRoute, name);
		routeSingleParser(pRoute);
		i++;
	}

	tok = parserGetToken();
	parserAssert(tokRBracket == tok, "zu viele Zugfahrstrassen angegeben");
	/* I did not use parserExpectToken, because a better error 
	 * message can be given
	 */

	parserExpectTokenNeu(tokRBracket);

	/* now make the resolving of the route names given as illegal route
	 *  or conditional route. Note, these routes were probably not known
	 *  before the parsing of the last route
	 */
	i = 0;
	while(NULL != (pRoute = routeGet(i)))
	{
		for(j = 0; j < routeGetParts(pRoute); j++)
		{
			switch(routeGetPartType(pRoute, j))
			{
			case RoutePartIllegalRoute:
				/* now get the name of the illegal route, search for it
				 *  in the routes and store the pointer to the route found
				 */
				(void) routeGetIllegalRouteName(pRoute, name, j);
				routeSetIllegalRoute(pRoute, routeFind(name), j);
				break;
			
			case RoutePartCondRoute:
				(void) routeGetCondRouteName(pRoute, name, j);
				routeSetCondRoute(pRoute, routeFind(name), j);
				break;

			default:
				break;
			}
		}
	}
}

/**
 * Function to set all forward annotations correctly. For routes, this are the blocks. 
 */
void routeParserResolve(void)
{	
	struct sRoute *		pRoute;
	enum eRoutePartType	type;
	struct sLine *		pLine;
	unsigned short		i, j;
	char			name[NAMELEN];

	for(i = 0; i < routesGetNr(); i++)
	{
		pRoute = routeGet(i);

		for(j = 0; j < NRROUTEPARTS; j++)
		{
			
			type = routeGetPartType(pRoute, j);
			if(RoutePartLine == type)
			{
				/* a line must be found.	*/
				(void) routeGetLineName(pRoute, name, j);
				routeSetLine(pRoute, lineFind(name), j);

			}		
		}
	}
}

/**
 * Prints the part of a single route
 * @param pRoute	pointer to the route to be printed
 * @exception	Assert on NULL pointer
 */
static void routeSinglePartPrint(struct sRoute * const pRoute, unsigned short part);
static void routeSinglePartPrint(struct sRoute * const pRoute, unsigned short part) 
{
	struct sTurnout *	pTurnout	= NULL;
	struct sSection *	pSection	= NULL;
	struct sDwarf *		pDwarf	= NULL;
	struct sRoute *		pCondRoute	= NULL;
	struct sShuntroute *	pShuntroute	= NULL;
	struct sLine *		pLine	= NULL;
	enum eTurnoutLine	turnoutLine;
	char			name[NAMELEN];
	#ifdef DOMINO55
		char			lineDwarfName[NAMELEN];
	#endif

	switch(routeGetPartType(pRoute, part))
	{
	case RoutePartNone:
		break;

	case RoutePartTurnout:
		pTurnout = routeGetTurnout(pRoute, part);
		turnoutGetName(pTurnout, name);
		fprintf(stdout, "\t\t\t\t\tWeiche\t\t%s", name);

		turnoutLine = routeGetTurnoutArgument(pRoute, part);
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
			break;
		}
		break;

	case RoutePartShuntroute:
		pShuntroute = routeGetShuntroute(pRoute, part);
		shuntrouteGetName(pShuntroute, name);
		fprintf(stdout, "\t\t\t\t\tRaFa\t\t%s\n", name);
		break;
	
	case RoutePartCondShuntroute:
		pShuntroute = routeGetCondShuntroute(pRoute, part);
		shuntrouteGetName(pShuntroute, name);
		fprintf(stdout, "\t\t\t\t\tBedingungsRaFa\t\t%s\n", name);
		break;
	
	case RoutePartCondOccupied:
		pSection = routeGetCondOccupied(pRoute, part);
		sectionGetName(pSection, name);
		fprintf(stdout, "\t\t\t\t\tBedingungBelegt\t\t%s\n", name);
		break;
	
	case RoutePartIllegalRoute:
		pCondRoute = routeGetIllegalRoute(pRoute, part);
		routeGetName(pCondRoute, name);
		fprintf(stdout, "\t\t\t\t\tVerbotsZuFa\t\t%s\n", name);
		break;

	case RoutePartCondRoute:
		fprintf(stdout, "\t\t\t\t\tBedingungZuFa");

		pCondRoute = routeGetCondRoute(pRoute, part);
		routeGetName(pCondRoute, name);
		fprintf(stdout, "\t%s\n", name);
		break;
	
	case RoutePartLine:
		/* TODO
		pLine = routeGetLine(pRoute, part);
		lineGetName(pLine, name); 
		*/ 
		routeGetLineName(pRoute, name, part);
		#ifdef SPDR60
			fprintf(stdout, "\t\t\t\t\tBlock\t\t%s\n", name);
		#endif

		#ifdef DOMINO55
			pDwarf = routeGetLinePtr2(pRoute, part);
			dwarfGetName(pDwarf, lineDwarfName);
			fprintf(stdout, "\t\t\t\t\tBlock\t%s\n", name);
			fprintf(stdout, "\t%s %s\n", name, lineDwarfName);
		#endif
		break;


	default:
		LOG_ERR("unexpected default\n");
		assert(false);
	}
}

/**
 * Prints the single route
 * @param pRoute	pointer to the route to be printed
 * @exception		Assert on NULL pointer
 */
static void routeSinglePrint(struct sRoute * const pRoute);
static void routeSinglePrint(struct sRoute * const pRoute) 
{
	char			name[NAMELEN];
	unsigned short		i;
	unsigned short		nrParts;
	struct sSection *	pSection = NULL;

	assert(NULL != pRoute);

	routeGetName(pRoute, name);
	fprintf(stdout, "\t\t\t%s {\n", name);
	if(routeGetToRight(pRoute))
	{
		fprintf(stdout, "\t\t\t\tNachRechts\n");
	} else {
		fprintf(stdout, "\t\t\t\tNachLinks\n");
	}

	#ifdef SPDR60
		/* TODO Umfahrstrasse	*/
	#endif

	mainGetName(routeGetStart(pRoute), name);
	fprintf(stdout, "\t\t\t\tStartsignal\t%s\n", name);

	mainGetName(routeGetDest(pRoute), name);
	fprintf(stdout, "\t\t\t\tZielsignal\t%s\n", name);

	nrParts = routeGetParts(pRoute);
	fprintf(stdout, "\t\t\t\tZuFateile\t%d {\n", nrParts);
	
	for(i = 0; i < nrParts; i++)
	{
		routeSinglePartPrint(pRoute, i);
	}

	fprintf(stdout, "\t\t\t\t}\n", nrParts);

	#ifdef SPDR60
		/* TODO Durchrutschwege	*/
	#endif

	#ifdef SPDR60
		switch(routeGetMainAspect(pRoute))
		{
		case SIG_MN_HP0:
			fprintf(stdout, "\t\t\t\tFahrbegriff\tHp0\n");
			break;

		case SIG_MN_HP1:
			fprintf(stdout, "\t\t\t\tFahrbegriff\tHp1\n");
			break;

		case SIG_MN_HP1_ZS3:
			fprintf(stdout, "\t\t\t\tFahrbegriff\tHp1_Zs3\n");
			break;

		case SIG_MN_HP2:
			fprintf(stdout, "\t\t\t\tFahrbegriff\tHp2\n");
			break;

		case SIG_MN_HP2_ZS3:
			fprintf(stdout, "\t\t\t\tFahrbegriff\tHp2_Zs3\n");
			break;

		default:
			fprintf(stdout, "Fehler: unbekannter Fahrbegriff\n");
			break;
		}
	#endif

	#ifdef DOMINO55
		switch(routeGetMainAspect(pRoute))
		{
		case SIG_MN_HALT:
			fprintf(stdout, "\t\t\t\tFahrbegriff\tHalt\n");
			break;

		case SIG_MN_1:
			fprintf(stdout, "\t\t\t\tFahrbegriff\t1\n");
			break;

		case SIG_MN_2:
			fprintf(stdout, "\t\t\t\tFahrbegriff\t2\n");
			break;

		case SIG_MN_3:
			fprintf(stdout, "\t\t\t\tFahrbegriff\t3\n");
			break;

		case SIG_MN_5:
			fprintf(stdout, "\t\t\t\tFahrbegriff\t5\n");
			break;

		case SIG_MN_6:
			fprintf(stdout, "\t\t\t\tFahrbegriff\t6\n");
			break;
		default:
			break;
		}
	#endif

	pSection = routeGetResetSection(pRoute);
	sectionGetName(pSection, name);
	fprintf(stdout, "\t\t\t\tHaltmelder\t%s\n", name);
	/* TODO */
	fprintf(stdout, "\t\t\t\tVorsignale\t%d {\n", routeGetNrDistants(pRoute));
	fprintf(stdout, "\t\t\t\t}\n");
	fprintf(stdout, "\t\t\t}\n");
}

void routePrint(void) 
{
	unsigned short	i = 0;
	struct sRoute *	pRoute = NULL;

	fprintf(stdout, "\t{\n");
	fprintf(stdout, "\t\tZuFas %u {\n", routesGetNr());
	while(NULL != (pRoute = routeGet(i)))
	{
		routeSinglePrint(pRoute);
		i++;
	}
	fprintf(stdout, "\t\t}\n");
	fprintf(stdout, "\t}\n");
}

