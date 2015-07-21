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

/* Function for reading and writing the registers of the io
 * Bad Waldsee switchboard
 * Spreitenbacher Eisenbahn Amateur Klub SPEAK
 * www.speak.li
 *
 * $Author: kurt $
 * $Date: 2015-05-18 22:31:02 +0200 (Mon, 18. Mai 2015) $
 * $Revision: 2464 $
 */

/**
 * Das ist der Teil mit dem Sie alle Signale einlesen und
 *  ausgeben koennen. Dazu koennen Sie auch noch Merkerbits 
 *  speichern und lesen.
 * Wichtig: als Registerbit wird hier ein Bit gemeint, welches
 *  im inshift, outshift oder Merkerbereich sein kann. Dadurch
 *  muessen Sie in Ihrer Software keine Ruecksicht darauf nehmen, 
 *  ob ein Bit in der Hardware tatsaechlich existiert oder ob sie
 *  es zwichengespeichert haben.
 */

#ifndef __BAD_WALDSEE_REGISTERS_H_
#define __BAD_WALDSEE_REGISTERS_H_

#include <stdbool.h>
#include <string.h>
#include "constants.h"	/* for NAMELEN	*/

extern const char cNONE[];

/**
 * Struktur eines Registereintrags
 */
struct sRegister 
{
	char           plugName[NAMELEN];   /**< Name des Steckers selber */
	unsigned short address;             /**< Adresse dieses Registers */
	unsigned short value;               /**< Werte aller Bits   */
	char           name[16][NAMELEN];   /**< Namen der Signale */
	unsigned char  filterValue[16];     /**< Werte der Filter */
	unsigned char  filterPlus[16];      /**< Werte, die bei 1 zum jeweiligen Filter addiert werden   */
	unsigned char  filterMinus[16];     /**< Werte, die bei 0 vom jeweiligen Filter subtrahiert werden */
};



/**
 * Die Register werden verwendet fuer
 *
 * x das Schreiben von Daten zu den Ausgaengen
 * x das Lesen von Daten von Eingaengen
 * x das Speichern von Temporaeren Variablen
 *
 *
\verbatim
 Diagramm

 |0                     |NR_OUT_REGISTERS     |NR:IN_REGISTERS        |
 |                      |                     |+ NR_OUT_REGISTERS     |
 |                      |                     |                       |
 |<--NR_OUT_REGISTERS-->|<--NR_IN_REGISTERS-->|<--NR_TEMP_REGISTERS-->|
 ,----------------------+---------------------+-----------------------,
 |                      |                     |                       |
 |  n 2 Byte Words      |                     |                       |
 '----------------------'---------------------'-----------------------'
\endverbatim

 * Alle Aufrufe werden ueber diese Register gemacht. So ist es moeglich, immer
 *  die selben set und get Funktionen zu verwenden.
 * Wenn Sie das Ausgaberegister 10 aufrufen wollen, so erreichen Sie dies mit
 *  (10 + NR_IN_REGISTERS).
 */

/*@maynotreturn@*/
void registers(const unsigned short outshiftLen,
				 const unsigned short inshiftLen,
				 const unsigned short tempLen);

void registersDestroy(void);

void registersInit(void);

void registerBitSet(const unsigned short BitPos);
void registerBitClr(const unsigned short BitPos);

_Bool registerBitGet(const unsigned short BitPos);

unsigned short registerFindHandle(const char * const name);

void registerSetPlugName(const unsigned short handle, const char * const name);
_Bool registerGetPlugName(const unsigned short handle, char * const name);
		
void registerSetName(const unsigned short handle, const char * const name);
_Bool registerGetName(const unsigned short handle, char * const name);

void registerSetFilterPlus(const unsigned short handle, const unsigned char value);
unsigned char  registerGetFilterPlus(const unsigned short handle);

void registerSetFilterMinus(const unsigned short handle, const unsigned char value);
unsigned char  registerGetFilterMinus(const unsigned short handle);

unsigned short registerGetFirstOutshiftHandle(void);
unsigned short registerGetFirstInshiftHandle(void);
unsigned short registerGetFirstVarHandle(void);
unsigned short registerGetOutshiftLen(void);
unsigned short registerGetInshiftLen(void);
unsigned short registerGetVarLen(void);
unsigned short registerGetLen(void);

void registerGetString(char * const pString, 
					 const unsigned short start,
					 const unsigned short len);

/*@null@*/ /*@observer@*/
char * registerSetString(/*@temp@*/ /*@in@*/ char * const pString, 
			 const unsigned short start,
			 const unsigned short len);

/*@null@*/ /*@observer@*/
struct sRegister * registerGetPtr(void);

#endif /* __BAD_WALDSEE_REGISTERS_H_ */

