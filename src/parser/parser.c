/* 
 * This file is part of Raspberry Pi steuert Modelleisenbahn
 * Copyright(C)  2009 - 2015 Kurt Zerzawy www.zerzawy.ch
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
 * General functions for parsing and printing 
 * \author
 * Kurt Zerzawy
 */

/*
 * Spreitenbacher Eisenbahn Amateur Klub SPEAK
 * www.speak.li
 *
 * $Author: Kurt $
 * $Date: 2015-11-14 23:10:12 +0100 (Sa, 14 Nov 2015) $
 * $Revision: 2672 $
 */

#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include "../constants.h"
#include "../log.h"
#include "../hw.h"
#include "../registers.h"
#include "../button.h"
#include "registers.h"
#include "led.h"
#include "duoled.h"
#include "section.h"
#include "button.h"
#include "turnout.h"
#include "dwarf.h"
#include "shuntroute.h"
#include "combishuntroute.h"
#include "mains.h"
#include "parser.h"
#include "route.h"
#include "combiroute.h"
#include "distant.h"
#include "automat.h"
#include "line.h"

const char cTOK_NONE []			= "";
const char cTOK_NAME []			= "Name";
const char cTOK_LBRACKET []		= "{";
const char cTOK_RBRACKET []		= "}";
const char cTOK_NRINPUTS []		= "AnzahlEingaenge";
const char cTOK_NROUTPUTS []		= "AnzahlAusgaenge";
const char cTOK_NRVARS []		= "AnzahlVariablen";
const char cTOK_INPUT []		= "Eingaenge";
const char cTOK_OUTPUT []		= "Ausgaenge";
const char cTOK_VARS []			= "Variablen";
const char cTOK_ADR []			= "Adr";
const char cTOK_PIN []			= "Pin";
const char cTOK_BIT []			= "Bit";
const char cTOK_TASTEN []		= "Tasten";
const char cTOK_STANDARDTASTEN []	= "Standardtasten";
#ifdef DOMINO55
	const char cTOK_TASTE_STEUERUNG []	= "TasteSteuerung";
	const char cTOK_TASTE_FERNEIN []	= "TasteFernEin";
	const char cTOK_TASTE_FERNAUS []	= "TasteFernAus";
	const char cTOK_TASTE_ISOLUMGSIG []	= "TasteIsolUmgSig";
	const char cTOK_TASTE_ISOLUMGWEI []	= "TasteIsolUmgWei";
	const char cTOK_TASTE_NOTAUFLFHT []	= "TasteNotauflFHT";
	const char cTOK_TASTE_AUTEIN []		= "TasteAutEin";
	const char cTOK_TASTE_AUTAUS []		= "TasteAutAus";
	const char cTOK_TASTE_BETRAUFL []	= "TasteBetrAufl";
	const char cTOK_TASTE_AUSLWEICHE []	= "TasteAuslWeiche";
	const char cTOK_TASTE_NOTFAHR []	= "TasteNotfahr";
#endif

#ifdef SPDR60
	const char cTOK_TASTE_DRGT []		= "DRGT";
	const char cTOK_TASTE_EaGT []		= "EaGT";
	const char cTOK_TASTE_FHT []		= "FHT";
	const char cTOK_TASTE_FRT []		= "FRT";
	const char cTOK_TASTE_HaGT []		= "HaGT";
	const char cTOK_TASTE_StET []		= "StET";
	const char cTOK_TASTE_StAT []		= "StAT";
	const char cTOK_TASTE_UfGT []		= "UfGT";
	const char cTOK_TASTE_WGT []		= "WGT";
	const char cTOK_TASTE_WHT []		= "WHT";
#endif
const char cTOK_ANZWEICHEN []		= "AnzahlWeichen";
const char cTOK_WEICHEN []		= "Weichen";
const char cTOK_WEICHE []		= "Weiche";
const char cTOK_ZUNGE_LINKS []		= "ZungeLinks";
const char cTOK_ZUNGE_RECHTS []		= "ZungeRechts";
const char cTOK_ANLASSRELAIS []		= "Anlassrelais";
const char cTOK_TASTE []		= "Taste";
const char cTOK_BELEGTMELDER []		= "BelegtMelder";
const char cTOK_MOTOR []		= "Motor";
const char cTOK_KONTAKT []		= "Kontakt";
#ifdef DOMINO55
	const char cTOK_LINKS []		= "Links";
	const char cTOK_RECHTS []		= "Rechts";
#endif
#ifdef SPDR60
	const char cTOK_PLUS []			= "Plus";
	const char cTOK_MINUS []		= "Minus";
#endif

const char cTOK_BEGRIFF []		= "Begriff";
const char cTOK_LEDGELB []		= "LEDgelb";
const char cTOK_LEDROT []		= "LEDrot";
const char cTOK_WEITERMELDUNG []	= "Weitermeldung";
const char cTOK_GRENZZEICHEN []		= "Grenzzeichen";
const char cTOK_VERBUNDEN []		= "verbunden";
const char cTOK_FUEHRT []		= "fuehrt";
const char cTOK_ABSCHNITTE []		= "Abschnitte";
const char cTOK_ABSCHNITT []		= "Abschnitt";
const char cTOK_PLAETTCHEN []		= "Plaettchen";
const char cTOK_ZWERGSIGNALE []		= "Zwergsignale";
const char cTOK_SPERRSIGNALE []		= "Sperrsignale";
const char cTOK_WEISS1 []		= "weiss1";
const char cTOK_WEISS2 []		= "weiss2";
const char cTOK_WEISS3 []		= "weiss3";
const char cTOK_LED1 []			= "LED1";
const char cTOK_LED2 []			= "LED2";
const char cTOK_LED3 []			= "LED3";
const char cTOK_TISCHLED []		= "TischLED";
const char cTOK_RAFAS []		= "RaFas";
const char cTOK_RAFA []			= "RaFa";
const char cTOK_NACHRECHTS []		= "NachRechts";
const char cTOK_NACHLINKS []		= "NachLinks";
const char cTOK_STARTTASTE []		= "Starttaste";
const char cTOK_ZIELTASTE []		= "Zieltaste";
const char cTOK_STARTSIGNAL []		= "Startsignal";
const char cTOK_ZIELSIGNAL []		= "Zielsignal";
const char cTOK_RAFATEILE []		= "RaFateile";
const char cTOK_SCHUTZWEICHE []		= "Schutzweiche";
const char cTOK_SCHUTZABSCHNITT []	= "Schutzabschnitt";
const char cTOK_SCHUTZSIGNAL []		= "Schutzsignal";
const char cTOK_KOMBIRAFAS []		= "KombiRaFas";
const char cTOK_HAUPTSIGNALE []		= "Hauptsignale";
const char cTOK_HAUPTSIGNAL []		= "Hauptsignal";
const char cTOK_ROT []			= "rot";
const char cTOK_ROT1 []			= "rot1";
const char cTOK_ROT2 []			= "rot2";
const char cTOK_GRUEN []		= "gruen";
const char cTOK_GRUEN1 []		= "gruen1";
const char cTOK_GRUEN2 []		= "gruen2";
const char cTOK_GRUEN3 []		= "gruen3";
const char cTOK_GRUEN12 []		= "gruen12";
const char cTOK_GRUEN14 []		= "gruen14";
const char cTOK_GRUEN15 []		= "gruen15";
const char cTOK_GELB []			= "gelb";
const char cTOK_GELB1 []		= "gelb1";
const char cTOK_GELB2 []		= "gelb2";
const char cTOK_GELB11 []		= "gelb11";
const char cTOK_GELB13 []		= "gelb13";
const char cTOK_ABFAHRT []		= "Abfahrt";
const char cTOK_TISCHLEDROT []		= "TischLEDrot";
const char cTOK_TISCHLEDGRUEN []	= "TischLEDgruen";
const char cTOK_TISCHLEDGELB []		= "TischLEDgelb";
const char cTOK_TISCHLEDWEISS []	= "TischLEDweiss";
const char cTOK_VORSIGNALE []		= "Vorsignale";
const char cTOK_VORSIGNAL []		= "Vorsignal";
const char cTOK_ZUFAS []		= "ZuFas";
const char cTOK_ZUFA []			= "ZuFa";
const char cTOK_ZUFATEILE []		= "ZuFateile";
const char cTOK_BEDINGUNGSRAFA []	= "BedingungsRaFa";
const char cTOK_BEDINGUNGBELEGT []	= "BedingungBelegt";
const char cTOK_VERBOTSZUFA []		= "VerbotsZuFa";
const char cTOK_BEDINGUNGSZUFA []	= "BedingungsZuFa";
const char cTOK_KOMBIZUFA []		= "KombiZuFa";
const char cTOK_KOMBIZUFAS []		= "KombiZuFas";

#ifdef SPDR60
	const char cTOK_HP1 []			= "Hp1";
	const char cTOK_HP1_ZS3 []		= "Hp0_Zs3";
	const char cTOK_HP2 []			= "Hp2";
	const char cTOK_HP2_ZS3 []		= "Hp2_Zs3";
	const char cTOK_HP0 []			= "Hp0";
	const char cTOK_ZS3 []			= "Zs3";
	const char cTOK_ZS3V []			= "Zs3v";
	const char cTOK_ZP9 []			= "Zp9";
#endif

#ifdef DOMINO55
	const char cTOK_HALT []			= "Halt";
#endif

const char cTOK_FAHRBEGRIFF []		= "Fahrbegriff";
const char cTOK_HALTMELDER []		= "Haltmelder";
const char cTOK_DUNKELBEIHALT []	= "DunkelBeiHalt";

const char cTOK_STRECKEN []		= "Strecken";
#ifdef SPDR60
	const char cTOK_EAT []			= "EaT";
	const char cTOK_MITSTEUERUNGSTASTE []	= "MitSteuerungstaste";
	const char cTOK_ERLHT []		= "ErlHT";
	const char cTOK_ASPM []			= "ASpM";
	const char cTOK_EAM []			= "EaM";
	const char cTOK_EEM []			= "EeM";
#endif

const char cTOK_BLOCK []		= "Block";
const char cTOK_BLOECKE []		= "Bloecke";
const char cTOK_SELBSTBLOECKE []	= "Selbsbloecke";
const char cTOK_TISCHLEDBELEGT []	= "TischLEDBelegt";
const char cTOK_BLOCKSIGNAL []		= "Blocksignal";

const char cTOK_AUTOMAT []		= "Automat";
const char cTOK_TASTE_EIN []		= "TasteEin";
const char cTOK_TASTE_AUS []		= "TasteAus";
const char cTOK_LED_EIN []		= "LEDein";

static int inLineNr	= 1;
/*@null@*/ static FILE * pConfFile	= NULL;
static char cStationName [NAMELEN];
static char cConfFileName [128];

/**
 * returns the token of a given string
 * @param pWord		pointer to the word to be looked for
 * @return		token, tokNone if not found
 */
static enum eToken stringToToken(const char * const pWord);

/**
 * returns a string for a token looked for
 * @param token		token to be searched for
 * @return		pointer to the string representing the token
 *			 empty string if not found
 */
static const char * const tokenToString(const enum eToken token);

/**
 * read function for the configuration file
 * @return		charakter read from file
 * @exception		exits with EXIT_FAILURE if file ends prematurely
 */
static char parserGetc(void);

/**
 * unget function for configuration fil
 * @param ch		character to be pushed back
 * @exception		if called to often, assert will be untrue
 */
static void parserUngetc(char ch);

/**
 * read function for the configuration file. It 
 * reads and interprets the complete file. 
 * @exception thows assert if an error in the file occured
 */
void parserReadConf(void) {
	char name[NAMELEN];

	/* expecting name */
	parserExpectTokenNeu(tokName);
	/* copy name of station */
	parserExpectName(name, "Bahnhofname erwartet");
	strncpy(cStationName, name, NAMELEN - 1);
	cStationName[NAMELEN] = '\0';

	parserExpectTokenNeu(tokLBracket);
	registerParser();     /* now ios are made */
	parserExpectTokenNeu(tokRBracket);

	ledParser();
	duoledParser();

	parserExpectTokenNeu(tokLBracket);
	buttonParser();			/* now keys are parsed			*/
	turnoutParser();		/* the switches				*/
	sectionsParser();		/* the sections				*/
	dwarfParser();			/* the dwarfs (Zwergsignale, Sperrsignale)	*/
	shuntrouteParser();		/* the shunting routes (RaFas)		*/
	combishuntrouteParser();	/* the combined shunting routes	*/
	mainsParser();			/* the main signals (Hauptsignale)	*/
	routeParser();			/* the train routes (ZuFas)		*/
	dwarfParserResolve();		/* resolve unknown issues		*/
	combirouteParser();		/* the combined train routes		*/
	distantsParser();		/* the distant signals (Vorsignale	*/
	lineParser();			/* the lines (Strecken)			*/
	routeParserResolve();		/* resolve unknown lines in route	*/
	blockParser();			/* the blocks				*/
	lineParserResolve();		/* resolve unknown blocks in line	*/	
	automatParser();		/* the automat				*/
	parserExpectTokenNeu(tokRBracket);
}
	
/**
 * 'Constructor' for the parser.
 * @param pFileName   pointer to the name of the file
 * @exception assert if the file cannot be opened
 */
void parser(const char * const pFileName) {
	/*@globals undef pConfFileName@*/
	char  errtxt[64];

	assert(NULL  == pConfFile);

	strcpy(cConfFileName, pFileName);
	pConfFile = fopen(cConfFileName, "r");
	
	if(NULL == pConfFile) {
		sprintf(errtxt, "cannot open File %s %s", cConfFileName, strerror(errno));
		LOG_ERR(errtxt);
		exit(EXIT_FAILURE);
	}
}

/**
 * 'Destructor' for the parser. Closes the open file, if any
 */
void parserDestroy(void) 
{
	/*@globals killed pConfFileName@*/
	if(NULL != pConfFile) 
	{
		fclose(pConfFile);
		pConfFile = NULL;
	}
}

/**
 * helper function to check for a given token
 * if the pattern is found, true is returned, 
 * otherwise an error text is printed to 
 *  LOG_ERR and the function calls exit()
 * @param is      real value
 * @param shall   value expected
 */
void parserExpectToken(const enum eToken is, 
			 const enum eToken shall ) 
{
	char  errtxt[64];
	if(shall != is) 
	{
		sprintf(errtxt, "\"%s\" erwartet in Zeile %d, gefunden \"%s\"", 
			tokenToString(shall), 
			inLineNr,
			tokenToString(is));
		LOG_ERR(errtxt);
		exit(EXIT_FAILURE);
	}
}

/**
 * function to get a name. If the given word is a known token, an error is
 * asserted and the text in pErrText is printed out. If the word is not
 * a known token, it is given back in pName
 * @param pName         pointer to name given back in case of success
 * @param pErrText      pointer to error text printed out in case of error
 * @exception           exits() unexpected input.
 */
void parserExpectName(char * const pName, const char * const pErrText) 
{
	if(parserGetWord(pName) != tokNone) 
	{
		LOG_ERR(pErrText);
		exit(EXIT_FAILURE);
	}
	return;
}

unsigned short parserExpectNumber(const char * const pErrText) 
{
	char word[NAMELEN];
	unsigned short number;

	parserGetWord(word);
	parserAssert((1 == sscanf(word, "%hu", &number)), pErrText);
	return number;
}

/**
 * function to get a register handler
 * If the given name is not known, an error is asserted and the
 * text in pErrText is printed out.
 */
unsigned short parserExpectRegister(const char * const pErrText) 
{
	char word[NAMELEN];

	parserExpectName(word, pErrText);
	return registerFindHandle(word);
}

enum eToken parserGetToken(void) 
{
	char word[NAMELEN];

	return parserGetWord(word);
}

void parserExpectTokenNeu(const enum eToken shall) 
{
	parserExpectToken(parserGetToken(), shall);
}

/**
 * checks for the condition. If false, the text function 
 * gives out the text pText and calls exit()
 * @param condition  condition to be checked
 * @param pErrText       text to be printed to LOG_ERR
 */
void parserAssert(const _Bool condition, const char * const pText) {
	if(!condition) {
		parserExit(pText);
	}
}

/**
 * prints out the error text and calls exit()
 * @param pText  text to be printed to LOG_ERR
 */
void parserExit(const char * const pErrText) {
	char   errtxt[64];
	sprintf(errtxt, "%s in Zeile %d\n", pErrText, inLineNr);
	LOG_ERR(errtxt);
	exit(EXIT_FAILURE);
}

static char parserGetc(void) {
	int ch;
	
	assert(NULL != pConfFile);
	ch = fgetc(pConfFile);
	if(EOF == ch) {
		parserExit("ERROR: unexpected end of file");
	}
	return (char) ch;
}

static void parserUngetc(char ch) {
	assert(NULL != pConfFile);
	
	assert(EOF != ungetc(ch, pConfFile));
}

/**
 * function to get one word from the parsed file
 * @param               pointer to the word returned
 * @return token        token, if found, otherwise tokNone
 */
enum eToken parserGetWord(char * const pWord) {
	int    ch;
	char * ptr = pWord;
	unsigned short i = 0;

	assert(NULL != pWord);

	ch = ' ';
	while(true) 
	{
		while((' ' == ch) || 
		('\t' == ch) || 
		('\n' == ch) || 
		('\r' == ch)) {
			ch = parserGetc();
			if('\n' == ch) {
				inLineNr++;
			}
		}
		/* we stand on a non empty char */
		/* step 1: check if it is #, if so, don't care
		 * until end of line
		 */
		if('#' == ch) 
		{
			do 
			{
				ch = parserGetc();
			} while('\n' != ch);
			inLineNr++;
			continue;
		} 
		else 
		{
			break;
		}
	}
	* ptr = ch;
	ptr++;
	i++;
	
	while(NAMELEN > i) 
	{
		ch = parserGetc();
		if((' ' == ch) || 
			 ('\t' == ch) || 
			 ('\n' == ch) || 
			 ('\r' == ch)) 
		{
			parserUngetc(ch);
			* ptr = '\0';
			return stringToToken(pWord);
		}
		else 
		{
			* ptr = ch;
			ptr++;
			i++;
		}
	}
	parserExit("name too long");
	/* do never come to here, but write it for compilers sake	*/
	return tokNone;
}
	
static enum eToken stringToToken(const char * const pWord) 
{
	enum eToken	token;

	for(token = tokNone; token < tokLast; token++)
	{
		if(!strcmp(pWord, tokenToString(token)))
		{
			/* token with this name was found	*/
			return token;
		}
	}
	return tokNone;
}

static const char * const tokenToString(const enum eToken token) 
{
	switch(token) {
	case tokName:		return cTOK_NAME;
	case tokLBracket:	return cTOK_LBRACKET;
	case tokRBracket:	return cTOK_RBRACKET;
	case tokNrInputs:	return cTOK_NRINPUTS;
	case tokNrOutputs:	return cTOK_NROUTPUTS;
	case tokNrVars:		return cTOK_NRVARS;
	case tokInput:		return cTOK_INPUT;
	case tokOutput:		return cTOK_OUTPUT;
	case tokVars:		return cTOK_VARS;
	case tokAdr:		return cTOK_ADR;
	case tokPin:		return cTOK_PIN;
	case tokBit:		return cTOK_BIT;
	case tokNone:		return cTOK_NONE;
	case tokTasten:		return cTOK_TASTEN;	       
	case tokStandardTasten:	return cTOK_STANDARDTASTEN; 

#ifdef DOMINO55
		case tokTasteSteuerung:	return cTOK_TASTE_STEUERUNG; 
		case tokTasteFernEin:	return cTOK_TASTE_FERNEIN;   
		case tokTasteFernAus:	return cTOK_TASTE_FERNAUS;   
		case tokTasteIsolUmgSig:return cTOK_TASTE_ISOLUMGSIG;
		case tokTasteIsolUmgWei:return cTOK_TASTE_ISOLUMGWEI;
		case tokTasteNotauflFHT:return cTOK_TASTE_NOTAUFLFHT;
		case tokTasteAutEin:	return cTOK_TASTE_AUTEIN;    
		case tokTasteAutAus:	return cTOK_TASTE_AUTAUS;    
		case tokTasteBetrAufl:	return cTOK_TASTE_BETRAUFL;  
		case tokTasteAuslWeiche:return cTOK_TASTE_AUSLWEICHE;
		case tokTasteNotfahr:	return cTOK_TASTE_NOTFAHR;
#endif

#ifdef SPDR60
		case tokTasteDRGT:	return cTOK_TASTE_DRGT;
		case tokTasteEaGT:	return cTOK_TASTE_EaGT;
		case tokTasteFHT:	return cTOK_TASTE_FHT;
		case tokTasteFRT:	return cTOK_TASTE_FRT;
		case tokTasteHaGT:	return cTOK_TASTE_HaGT;
		case tokTasteStET:	return cTOK_TASTE_StET;
		case tokTasteStAT:	return cTOK_TASTE_StAT;
		case tokTasteUfGT:	return cTOK_TASTE_UfGT;
		case tokTasteWGT:	return cTOK_TASTE_WGT;
		case tokTasteWHT:	return cTOK_TASTE_WHT;
#endif
	
	case tokWeichen:	return cTOK_WEICHEN;
	case tokWeiche:	  	return cTOK_WEICHE;
	case tokZungeLinks:	return cTOK_ZUNGE_LINKS;
	case tokZungeRechts:	return cTOK_ZUNGE_RECHTS;
	case tokAnlassrelais:	return cTOK_ANLASSRELAIS;
	case tokTaste:		return cTOK_TASTE;
	case tokBelegtmelder:	return cTOK_BELEGTMELDER;
	case tokMotor:		return cTOK_MOTOR;
	case tokKontakt:	return cTOK_KONTAKT;
#ifdef DOMINO55
		case tokLinks:		return cTOK_LINKS;
		case tokRechts:		return cTOK_RECHTS;
#endif
#ifdef SPDR60
		case tokPlus:		return cTOK_PLUS;
		case tokMinus:		return cTOK_MINUS;
#endif
	case tokBegriff:	return cTOK_BEGRIFF;
	case tokLEDgelb:	return cTOK_LEDGELB;
	case tokLEDrot:		return cTOK_LEDROT;
	case tokWeitermeldung:	return cTOK_WEITERMELDUNG;
	case tokGrenzzeichen:	return cTOK_GRENZZEICHEN;
	case tokVerbunden:	return cTOK_VERBUNDEN;
	case tokFuehrt:		return cTOK_FUEHRT;
	case tokAbschnitte:	return cTOK_ABSCHNITTE;
	case tokAbschnitt:	return cTOK_ABSCHNITT;
	case tokPlaettchen:	return cTOK_PLAETTCHEN;
	case tokZwergsignale:	return cTOK_ZWERGSIGNALE;
	case tokSperrsignale:	return cTOK_SPERRSIGNALE;
	case tokWeiss1:		return cTOK_WEISS1;
	case tokWeiss2:		return cTOK_WEISS2;
	case tokWeiss3:		return cTOK_WEISS3;
	case tokLED1:		return cTOK_LED1;
	case tokLED2:		return cTOK_LED2;
	case tokLED3:		return cTOK_LED3;
	case tokTischLED:	return cTOK_TISCHLED;
	case tokRaFas:		return cTOK_RAFAS;
	case tokRaFa:		return cTOK_RAFA;
	case tokNachrechts:	return cTOK_NACHRECHTS;
	case tokNachlinks:	return cTOK_NACHLINKS;
	case tokStarttaste:	return cTOK_STARTTASTE;
	case tokZieltaste:	return cTOK_ZIELTASTE;
	case tokStartsignal:	return cTOK_STARTSIGNAL;
	case tokZielsignal:	return cTOK_ZIELSIGNAL;
	case tokRaFateile:	return cTOK_RAFATEILE;
	case tokSchutzweiche:	return cTOK_SCHUTZWEICHE;
	case tokSchutzabschnitt:return cTOK_SCHUTZABSCHNITT;
	case tokSchutzsignal:	return cTOK_SCHUTZSIGNAL;
	case tokKombiRaFas:	return cTOK_KOMBIRAFAS;
	case tokHauptsignale:	return cTOK_HAUPTSIGNALE;
	case tokHauptsignal:	return cTOK_HAUPTSIGNAL;
	case tokRot:		return cTOK_ROT; 
	case tokRot1:		return cTOK_ROT1; 
	case tokRot2:		return cTOK_ROT2; 
	case tokGruen:		return cTOK_GRUEN; 
	case tokGruen1:		return cTOK_GRUEN1; 
	case tokGruen2:		return cTOK_GRUEN2; 
	case tokGruen3:		return cTOK_GRUEN3; 
	case tokGruen12:	return cTOK_GRUEN12; 
	case tokGruen14:	return cTOK_GRUEN14; 
	case tokGruen15:	return cTOK_GRUEN15; 
	case tokGelb:		return cTOK_GELB; 
	case tokGelb1:		return cTOK_GELB1; 
	case tokGelb2:		return cTOK_GELB2; 
	case tokGelb11:		return cTOK_GELB11; 
	case tokGelb13:		return cTOK_GELB13; 
	case tokAbfahrt:	return cTOK_ABFAHRT;
	case tokTischLEDrot:	return cTOK_TISCHLEDROT; 
	case tokTischLEDgruen:	return cTOK_TISCHLEDGRUEN; 
	case tokTischLEDgelb:	return cTOK_TISCHLEDGELB; 
	case tokTischLEDweiss:	return cTOK_TISCHLEDWEISS; 
	case tokVorsignale:	return cTOK_VORSIGNALE;
	case tokVorsignal:	return cTOK_VORSIGNAL;
	case tokZuFa:		return cTOK_ZUFA;
	case tokZuFas:		return cTOK_ZUFAS;
	case tokZuFateile:	return cTOK_ZUFATEILE;
	case tokBedingungsRaFa:	return cTOK_BEDINGUNGSRAFA; 
	case tokBedingungBelegt:return cTOK_BEDINGUNGBELEGT;
	case tokVerbotsZuFa:	return cTOK_VERBOTSZUFA;
	case tokBedingungsZuFa:	return cTOK_BEDINGUNGSZUFA;
	case tokKombiZuFa:	return cTOK_KOMBIZUFA;
	case tokKombiZuFas:	return cTOK_KOMBIZUFAS;
	#ifdef SPDR60
		case tokHP1:		return cTOK_HP1;
		case tokHP1_ZS3:	return cTOK_HP1_ZS3;
		case tokHP2:		return cTOK_HP2;
		case tokHP2_ZS3:	return cTOK_HP2_ZS3;
		case tokHP0:		return cTOK_HP0;
		case tokZs3:		return cTOK_ZS3;
		case tokZs3v:		return cTOK_ZS3V;
		case tokZp9:		return cTOK_ZP9;
	#endif

	#ifdef DOMINO55
		case tokHALT:		return cTOK_HALT;
	#endif

	case tokFahrbegriff:	return cTOK_FAHRBEGRIFF;
	case tokHaltmelder:	return cTOK_HALTMELDER;
	case tokDunkelBeiHalt:	return cTOK_DUNKELBEIHALT;

	case tokStrecken:	return cTOK_STRECKEN;
	#ifdef SPDR60	
		case tokEaT:		return cTOK_EAT;
		case tokMitSteuerungstaste:	return cTOK_MITSTEUERUNGSTASTE;
		case tokErlhT:		return cTOK_ERLHT;
		case tokASpM:		return cTOK_ASPM;
		case tokEaM:		return cTOK_EAM;
		case tokEeM:		return cTOK_EEM;
	#endif

	case tokBlock:		return cTOK_BLOCK;
	case tokBloecke:	return cTOK_BLOECKE;
	case tokSelbstbloecke:	return cTOK_SELBSTBLOECKE;
	case tokTischLEDBelegt:	return cTOK_TISCHLEDBELEGT;
	case tokBlocksignal:	return cTOK_BLOCKSIGNAL;

	case tokAutomat:	return cTOK_AUTOMAT;
	case tokTasteEin:	return cTOK_TASTE_EIN;
	case tokTasteAus:	return cTOK_TASTE_AUS;
	case tokLEDein:		return cTOK_LED_EIN;

	default:		return cTOK_NONE;
	}
}

/**
 * returns the name of the station parsed
 * @return              pointer to the stations name
 */
char * parserGetStation(void) 
{
	return cStationName;
}

/**
 * prints the parserfile
 */
void parserPrint(void) 
{
	fprintf(stdout, "Name %s {\n", cStationName);
	registerPrint();
	fprintf(stdout, "}\n");
	fprintf(stdout, "{\n");
	buttonPrint();
	turnoutPrint();
	sectionsPrint();
	dwarfPrint();
	shuntroutePrint();
	combishuntroutePrint();
	mainsPrint();
	routePrint();
	combiroutePrint();
	distantsPrint();
	linePrint();
	blockPrint();
	automatPrint();
	fprintf(stdout, "}\n");
}

