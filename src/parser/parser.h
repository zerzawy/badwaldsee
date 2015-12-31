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
 * Functions for parsing the file with switchboard information
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

#ifndef PARSER_H__
#define PARSER_H__ 

#include <stdbool.h>
#include <string.h>
#include <stdio.h>

enum eToken {
	tokNone,
	tokInit,
	tokName,
	tokLBracket,
	tokRBracket,
	tokNrInputs,
	tokNrOutputs,
	tokNrVars,
	tokInput,
	tokOutput,
	tokVars,
	tokAdr,
	tokPin,
	tokBit,
	tokTasten,
	tokStandardTasten,
#ifdef DOMINO55
		tokTasteSteuerung,
		tokTasteFernEin,
		tokTasteFernAus,
		tokTasteIsolUmgSig,
		tokTasteIsolUmgWei,
		tokTasteNotauflFHT,
		tokTasteAutEin,
		tokTasteAutAus,
		tokTasteBetrAufl,
		tokTasteAuslWeiche,
		tokTasteNotfahr,
#endif

#ifdef SPDR60
		tokTasteDRGT,
		tokTasteEaGT,
		tokTasteFHT,
		tokTasteFRT,
		tokTasteHaGT,
		tokTasteStET,
		tokTasteStAT,
		tokTasteUfGT,
		tokTasteWGT,
		tokTasteWHT,
#endif

	tokWeichen,
	tokWeiche,
	tokZungeRechts,
	tokZungeLinks,
	tokAnlassrelais,
	tokTaste,
	tokBelegtmelder,
	tokMotor,
	tokKontakt,
	
#ifdef DOMINO55
		tokRechts,
		tokLinks,
#endif

#ifdef SPDR60
		tokPlus,
		tokMinus,
#endif

	tokBegriff,
	tokLEDgelb,
	tokLEDrot,
	tokWeitermeldung,
	tokGrenzzeichen,
	tokVerbunden,
	tokFuehrt,
	tokAbschnitte,
	tokAbschnitt,
	tokPlaettchen,
	tokZwergsignale,
	tokSperrsignale,
	tokWeiss1,
	tokWeiss2,
	tokWeiss3,
	tokLED1,
	tokLED2,
	tokLED3,
	tokTischLED,
	tokRaFas,
	tokRaFa,
	tokNachrechts,
	tokNachlinks,
	tokStarttaste,
	tokZieltaste,
	tokStartsignal,
	tokZielsignal,
	tokRaFateile,
	tokSchutzweiche,
	tokSchutzabschnitt,
	tokSchutzsignal,
	tokKombiRaFas,
	tokHauptsignale,
	tokHauptsignal,
	tokRot,
	tokRot1,
	tokRot2,
	tokGruen,
	tokGruen1,
	tokGruen2,
	tokGruen3,
	tokGruen12,
	tokGruen14,
	tokGruen15,
	tokGelb,
	tokGelb1,
	tokGelb2,
	tokGelb11,
	tokGelb13,
	tokAbfahrt,
	tokTischLEDrot,
	tokTischLEDgruen,
	tokTischLEDgelb,
	tokTischLEDweiss,
	tokVorsignale,
	tokVorsignal,
	tokZuFa,
	tokZuFas,
	tokZuFateile,
	tokBedingungsRaFa,
	tokBedingungBelegt,
	tokVerbotsZuFa,
	tokBedingungsZuFa,
	tokKombiZuFa,
	tokKombiZuFas,
	#ifdef SPDR60
		tokHP1,
		tokHP1_ZS3,
		tokHP2,
		tokHP2_ZS3,
		tokHP0,
		tokZs3,
		tokZs3v,
		tokZp9,
	#endif

	#ifdef DOMINO55
		tokHALT,
	#endif
	tokFahrbegriff,
	tokHaltmelder,
	tokDunkelBeiHalt,

	tokStrecken,
	#ifdef SPDR60
		tokEaT,
		tokMitSteuerungstaste,
		tokErlhT,
		tokASpM,
		tokEaM,
		tokEeM,
	#endif

	tokBlock,
	tokBloecke,
	tokSelbstbloecke,
	tokTischLEDBelegt,
	tokBlocksignal,
	tokAutomat,
	tokTasteEin,
	tokTasteAus,
	tokLEDein,

	tokLast
};

void parser(/*@in@*/ const char * const pFileName);

void parserDestroy(void);

/*@maynotreturn@*/ 
void parserReadConf(void);

/*@maynotreturn@*/ 
void parserExpectToken(const enum eToken is, const enum eToken shall);

/*@maynotreturn@*/ 
void parserExpectTokenNeu(const enum eToken shall);

/*@maynotreturn@*/ 
void parserExpectName(/*@out@*/ char * const pName, 
			/*@in@*/ const char * const pErrText)
	/*@ensures maxRead(pName) <= NAMELEN@*/;

unsigned short parserExpectNumber(/*@in@*/ const char * const pErrText);

/*@maynotreturn@*/
unsigned short parserExpectRegister(/*@in@*/ const char * const pErrText);

/*@maynotreturn@*/ 
enum eToken parserGetWord(/*@out@*/ char * const pWord)
	/*@ensures maxRead(pWord) <= NAMELEN@*/;

enum eToken parserGetToken(void);

/*@noreturnwhenfalse@*/ 
void parserAssert(_Bool condition, /*@in@*/ const char * const pErrText);

/*@noreturn@*/ 
void parserExit(/*@in@*/ const char * const pErrText);

/*@observer@*/
char * parserGetStation(void);

void parserPrint(void);

#endif /* PARSER_H__ */

