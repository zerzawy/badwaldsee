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
 * Functions for parsing and printing parameters of turnouts
 * \author
 * Kurt Zerzawy
 */

/*
 * $Author: kurt $
 * $Date: 2015-05-18 22:31:02 +0200 (Mon, 18. Mai 2015) $
 * $Revision: 2464 $
 */

#include <assert.h>
#include "../registers.h"
#include "../turnout.h"
#include "turnout.h"

/***
 * Parser for one direction of the turnout
 * @param ptr    pointer to this turnout
 * @param dir    true for straight
 * @exception    warning and exit on error of the given parameters
 */
/*@maynotreturn@*/ 
static void turnoutOneDirectionParser(/*@temp@*/ struct sTurnout * const ptr,
            const _Bool dir);
static void turnoutOneDirectionParser(/*@temp@*/ struct sTurnout * const ptr,
            const _Bool dir) {
	enum eToken tok;
 
	parserExpectTokenNeu(tokKontakt);
	if(dir) 
		turnoutSetContactPlus(ptr, parserExpectRegister("Kontaktname erwartet"));
	else 
		turnoutSetContactMinus(ptr, parserExpectRegister("Kontaktname erwartet"));

	parserExpectTokenNeu(tokBegriff);
	if(dir)
		turnoutSetAspectPlus(ptr, parserExpectNumber("Begriff erwartet"));
	else
		turnoutSetAspectMinus(ptr, parserExpectNumber("Begriff erwartet"));

	tok = parserGetToken();

	/* first check if a signal shall be sent to neighbours	*/
	if(tokWeitermeldung == tok) {
		if(dir)
			turnoutSetSignalPlus(ptr, parserExpectRegister("Name der Weitermeldung erwartet"));
		else
			turnoutSetSignalMinus(ptr, parserExpectRegister("Name der Weitermeldung erwartet"));
	
		tok = parserGetToken();
	}

	/* check if a signal is expected from neighbours */
	if(tokGrenzzeichen == tok) {
		if(dir) 
			turnoutSetOccupancyPlus(ptr, parserExpectRegister("Name des Grenzzeichens erwartet"));
		else 
			turnoutSetOccupancyMinus(ptr, parserExpectRegister("Name des Grenzzeichens erwartet"));

		tok = parserGetToken();
	}

		
	if(tokRBracket == tok) {
		return;
	}

	switch(parserGetToken()) {
	case tokRBracket:
		return;
	
		tok = parserGetToken();
		if(tokRBracket == tok) 
			return;
			parserExit("unerwartetes Zeichen");
			/*@fallthrough@*/
	case tokGrenzzeichen:
		break;
	default:
		parserAssert(false, "unerwartetes Zeichen");
	}
	parserExpectTokenNeu(tokRBracket);
	return;
}
 
/*** 
 * parser for a single turnout
 * @param ptr	pointer to the turnout
 * @exception	warning and exit on error of the given parameters
 */
/*@maynotreturn@*/ 
static void turnoutSingleParser(/*@temp@*/ struct sTurnout * const ptr);
static void turnoutSingleParser(/*@temp@*/ struct sTurnout * const ptr) {
	char name[NAMELEN];
	char turnoutName[NAMELEN];

/* TODO addieren der Duoled an der Wurzel der Weiche, wie sie bei Siemens Plättchen vorkommt, 18.10.2014	*/
 /* fill in first the things defined by the name of the turnout */

 	#ifdef SPDR60
		turnoutGetName(ptr, turnoutName);
		(void) snprintf(name, NAMELEN, "BEL_%s", turnoutName);
		turnoutSetOccupancyDetector(ptr, registerFindHandle(name));
		(void) snprintf(name, NAMELEN, "%sp", turnoutName);
		turnoutSetDuoledPlus(ptr, duoledFind(name));
		(void) snprintf(name, NAMELEN, "%sm", turnoutName);
		turnoutSetDuoledMinus(ptr, duoledFind(name));
		parserExpectTokenNeu(tokLBracket);
	#endif

	#ifdef DOMINO55
		turnoutGetName(ptr, turnoutName);
		(void) snprintf(name, NAMELEN, "BEL_%s", turnoutName);
		turnoutSetOccupancyDetector(ptr, registerFindHandle(name));
		(void) snprintf(name, NAMELEN, "%sr", turnoutName);
		turnoutSetDuoledPlus(ptr, duoledFind(name));
		(void) snprintf(name, NAMELEN, "%sl", turnoutName);
		turnoutSetDuoledMinus(ptr, duoledFind(name));
		parserExpectTokenNeu(tokLBracket);
	#endif

	switch(parserGetToken()) {
	case tokZungeLinks:
		turnoutSetPointLeft(ptr, true);
		break;
	case tokZungeRechts:
		turnoutSetPointLeft(ptr, false);
		break;
	default:
		parserAssert(false, "ZungeLinks oder ZungeRechts erwartet");
		return;
	}

	parserExpectTokenNeu(tokTaste);  
	(void) parserExpectName(name, "Tastenname erwartet");
	if(0 != (strcmp("LEER", name)))
	{
		turnoutSetButton(ptr, buttonFind(name));
	}
	parserExpectTokenNeu(tokMotor);
	(void) parserExpectName(name, "Motorname erwartet");
	if(0 != (strcmp("LEER", name))) 
	{
		turnoutSetMotor(ptr, registerFindHandle(name));
	}
#ifdef SPDR60
		parserExpectTokenNeu(tokPlus);
#endif
#ifdef DOMINO55
		parserExpectTokenNeu(tokRechts);
#endif

	parserExpectTokenNeu(tokLBracket);
	turnoutOneDirectionParser(ptr, true);

#ifdef SPDR60
		parserExpectTokenNeu(tokMinus);
#endif
#ifdef DOMINO55
		parserExpectTokenNeu(tokLinks);
#endif
	parserExpectTokenNeu(tokLBracket);
	turnoutOneDirectionParser(ptr, false);

	parserExpectTokenNeu(tokRBracket);
	return;
}
 
/**
 * parser for the turnout
 * @exception           error message when configuration file cannot be interpreted
 */
void turnoutParser(void) {
	struct sTurnout * ptr;
	/*@null@*/ struct sTurnout *  pLeadTurnout = NULL;
	/*@null@*/ struct sTurnout *  pLeadingTurnout = NULL;
	unsigned short	nrTurnouts = 0;
	unsigned short  i = 0;
	enum eToken     tok = tokNone;
	char            name[NAMELEN];

	parserExpectTokenNeu(tokLBracket);
	parserExpectTokenNeu(tokWeichen);
	nrTurnouts = parserExpectNumber("Anzahl Weichen erwartet");
	turnout(nrTurnouts);	/* generate the turnout structure	*/
	
	parserExpectTokenNeu(tokLBracket);
	while(NULL != (ptr = turnoutGet(i)))
	{
		tok = parserGetWord(name);
		if(tokRBracket == tok) 
		{
			/* I found closing bracket in configuration, telling 
			 *  me that parsing of single turnouts is over
			 */
			break;
		}
		parserAssert(tokNone == tok, "Weichenname erwartet");
		turnoutSetName(ptr, name);
		turnoutSingleParser(ptr);
		/* now I finished parsing a single turnout.
		 *  I check if there is one more
		 */
		i++;
	}
	
	tok = parserGetToken();
	parserAssert(tokRBracket == tok, "zu viele Weichen angegeben");
	/* I did not use parserExpectToken, because I know I expect tokRBracket
	 *  but I want to make a better error message
	 */
		
	tok = parserGetToken();
	if(tokRBracket == tok) {
		return;
	}
	parserAssert(tokVerbunden == tok, "\"verbunden\" erwartet");
	while(true) {
		tok = parserGetWord(name);
		if(tokAnlassrelais == tok) {
			/* Parsing for connected turnouts is over	*/
			break;
		}
		/*@-observertrans@*//*@-dependenttrans@*/
		parserAssert(tokNone == tok, "Weichenname erwartet");
		pLeadingTurnout = turnoutFindPtr(name);

		parserExpectTokenNeu(tokFuehrt);
		(void) parserExpectName(name, "Weichenname erwartet");
		pLeadTurnout = turnoutFindPtr(name);

		turnoutSetSlave(pLeadingTurnout, pLeadTurnout);
		turnoutSetMaster(pLeadTurnout, pLeadingTurnout);
		/*@+observertrans@*//*@+dependenttrans@*/
	}
	/* now the relais common for all turnouts */
	turnoutSetGroupRelaisRegister(parserExpectRegister("Register fuer Anlassrelais erwartet")); 
	parserExpectTokenNeu(tokRBracket);
}

static void turnoutDirectionPrint(const struct sTurnout * const ptr,
				const _Bool dir);
static void turnoutDirectionPrint(const struct sTurnout* const ptr,
				const _Bool dir) 
{
	char	name[NAMELEN];

	if(dir)
	{
		registerGetName(turnoutGetContactPlus(ptr), name);
		fprintf(stdout, "\t\t\t\t\tKontakt\t\t%s\n", name);
		fprintf(stdout, "\t\t\t\t\tBegriff\t\t%d\n", turnoutGetAspectPlus(ptr));

		if(ptr->signalPlus != NONEVALUE) 
		{
			registerGetName(ptr->signalPlus, name);
			fprintf(stdout, "\t\t\t\t\tWeitermeldung\t%s\n", name);
		}

		if(ptr->occupancyPlus != NONEVALUE) 
		{
			registerGetName(ptr->occupancyPlus, name);
			fprintf(stdout, "\t\t\t\t\tGrenzzeichen\t%s\n", name);
		}
	}
	else 
	{
		registerGetName(turnoutGetContactMinus(ptr), name);
		fprintf(stdout, "\t\t\t\t\tKontakt\t\t%s\n", name);
		fprintf(stdout, "\t\t\t\t\tBegriff\t\t%d\n", turnoutGetAspectMinus(ptr));

		if(ptr->signalMinus != NONEVALUE) 
		{
			registerGetName(ptr->signalMinus, name);
			fprintf(stdout, "\t\t\t\t\tWeitermeldung\t%s\n", name);
		}

		if(ptr->occupancyMinus != NONEVALUE) 
		{
			registerGetName(ptr->occupancyMinus, name);
			fprintf(stdout, "\t\t\t\t\tGrenzzeichen\t%s\n", name);
		}
	}
}

static void turnoutSinglePrint(const struct sTurnout * const ptr);
static void turnoutSinglePrint(const struct sTurnout * const ptr) 
{
	char name[NAMELEN];

	turnoutGetName(ptr, name);
	fprintf(stdout, "\t\t\t%s {\n", name);
	if(turnoutIsPointLeft(ptr))
		fprintf(stdout, "\t\t\t\tZungeLinks\n");
	else
		fprintf(stdout, "\t\t\t\tZungeRechts\n");

	if(NULL == turnoutGetButton(ptr)) {
		fprintf(stdout, "\t\t\t\tTaste\tLEER\n");
	}
	else {
		buttonGetName(turnoutGetButton(ptr), name);
		fprintf(stdout, "\t\t\t\tTaste\t%s\n", name);
	}
	if(NONEVALUE == turnoutGetMotor(ptr)) {
		fprintf(stdout, "\t\t\t\tMotor\tLEER\n");
	}
	else {
		registerGetName(turnoutGetMotor(ptr), name);
		fprintf(stdout, "\t\t\t\tMotor\t%s\n", name);
	}
#ifdef DOMINO55
	fprintf(stdout, "\t\t\t\tRechts {\n");
#endif
#ifdef SPDR60
	fprintf(stdout, "\t\t\t\tPlus {\n");
#endif

	turnoutDirectionPrint(ptr, true);
	fprintf(stdout, "\t\t\t\t}\n");
#ifdef DOMINO55
	fprintf(stdout, "\t\t\t\tLinks {\n");
#endif
#ifdef SPDR60
	fprintf(stdout, "\t\t\t\tMinus {\n");
#endif

	turnoutDirectionPrint(ptr, false);
	fprintf(stdout, "\t\t\t\t}\n");
	fprintf(stdout, "\t\t\t}\n");
}  

void turnoutPrint(void) 
{
	unsigned short i = 0;
	struct sTurnout * ptr;
	_Bool	connectedTurnout = false;
	char	masterName[NAMELEN];
	char	slaveName[NAMELEN];
	char	name[NAMELEN];

	fprintf(stdout, "\t{\n");
	fprintf(stdout, "\t\tWeichen %hu {\n", turnoutGetNr());
		
	while(NULL != (ptr = turnoutGet(i)))
	{
		assert(NULL != ptr);
		turnoutSinglePrint(ptr);
		i++;
	}
	fprintf(stdout, "\t\t}\n");

	/* now print out the connected turnouts. For this check each turnout
	 *  and see if it is controlled by another turnout
	 */
	i = 0;
	while(NULL != (ptr = turnoutGet(i)))
	{
		assert(NULL != ptr);
		if(turnoutGetSlave(ptr) != ptr) 
		{
			connectedTurnout = true;
			break;
		}
		i++;
	}
	fprintf(stdout, "\t\tverbunden\n");
	fprintf(stdout, "\n");

	if(connectedTurnout) 
	{
		/* print a list of all connected turnouts */
		i = 0;
		while(NULL != (ptr = turnoutGet(i)))
		{
			if(ptr != turnoutGetSlave(ptr)) 
			{
				assert(NULL != turnoutGetSlave(ptr)); 
				turnoutGetName(ptr, masterName);
				turnoutGetName(turnoutGetSlave(ptr), slaveName);
				fprintf(stdout, "\t\t%s fuehrt %s\n", masterName, slaveName);
			}
			i++;
		}
	}
	/* now print out common relais */
	registerGetName(turnoutGetGroupRelaisRegister(), name);
	fprintf(stdout, "\n");
	fprintf(stdout, "\t\tAnlassrelais\t%s\n", name);
	fprintf(stdout, "\t}\n");
}

