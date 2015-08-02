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
 * Buttons and standard buttons of the desk
 * \author
 * Kurt Zerzawy
 */

/*
* Functions for buttons
* Spreitenbacher Eisenbahn Amateur Klub SPEAK
* www.speak.li
*
* $Author: kurt $
* $Date: 2015-05-18 22:31:02 +0200 (Mon, 18. Mai 2015) $
* $Revision: 2464 $
*/

#ifndef BAD_WALDSEE_BUTTONS_H__
#define BAD_WALDSEE_BUTTONS_H__

#include "constants.h"
#include "parser/parser.h"		/* necessary for special key token	*/

#ifdef DOMINO55
	#define TASTE_STEUERUNG   0
	#define TASTE_FERN_EIN    1
	#define TASTE_FERN_AUS    2
	#define TASTE_ISOLUMGSIG  3
	#define TASTE_ISOLUMGWEI  4
	#define TASTE_NOTAUFLFHT  5
	#define TASTE_AUT_EIN     6
	#define TASTE_AUT_AUS     7
	#define TASTE_BETRAUFL    8
	#define TASTE_AUSLWEI     9
	#define TASTE_NOTFAHR     10
	#define TASTE_ANZ_SPEZIAL 11
#endif

#ifdef SPDR60
	#define TASTE_DRGT	0
	#define TASTE_EaGT	1
	#define TASTE_FHT	2
	#define TASTE_FRT	3
	#define TASTE_HaGT	4
	#define	TASTE_StET	5
	#define TASTE_StAT	6
	#define TASTE_UfGT	7
	#define TASTE_WGT	8
	#define TASTE_WHT	9
	#define TASTE_ANZ_SPEZIAL 10
#endif

/**
 * Structur of a pushbutton
 */
struct sButton 
{
	char	 	name[NAMELEN];		/**< name of the pushbutton	*/
	unsigned short 	BI_Button;		/**< register handle of the input 	*/
	_Bool	 	pressed;		/**< true if pressed	*/
	_Bool	 	wasPressed;		/**< true if pressed before last process */
}; 

/**
 * Structure of a standard button
 */
struct sStandardButton 
{
	char 		name[NAMELEN];		/**< Name of the pushbutton	*/
	enum eToken	token;			/**< Token for this special key	*/
	unsigned short	BI_Button;		/**< Register handle of the input 	*/
	_Bool		pressed;		/**< true if pressed	*/
	_Bool		wasPressed;		/**< true if pressed before last process */
};

/**
 * Liste der Richtungen, die eine Taste annehmen kann
 */
enum eTasteDir 
{
	TasteGerade,
	TasteAbzweig,
		
	ANZ_TASTEN_DIR
};

void button(const unsigned short anzahl);
void buttonDestroy(void);

void buttonInit(void);

void buttonProcess(void);
_Bool buttonReadHW(void);
_Bool buttonSingleReadHW(struct sButton * const ptr);

/*@maynotreturn@*/
_Bool buttonIsPressed(/*@null@*/ const struct sButton * const pTaste);

/*@maynotreturn@*/
/*@observer@*/ 
struct sButton * buttonFind(const char * const name);

_Bool buttonGetName(const struct sButton * const pTaste, char * const name);
void buttonSetName(struct sButton * const ptr, const char * const name);

struct sButton * buttonGet(const unsigned short i);

unsigned short buttonGetNr(void);

unsigned short buttonGetBI_Button(const struct sButton * const ptr);
void buttonSetBI_Button(struct sButton * const ptr, const unsigned short handle);


/*@maynotreturn@*/
_Bool stdButtonIsPressed(const unsigned short stdButton);

#ifdef SPDR60
	_Bool stdButtonFRTTimerIsRunning(void);
	void stdButtonFRTTimerReset(void);
	_Bool stdButtonFHTTimerIsRunning(void);
	void stdButtonFHTTimerReset(void);
	_Bool stdButtonUfGTTimerIsRunning(void);
	void stdButtonUfGTTimerReset(void);
#endif

/*@maynotreturn@*/
/*@observer@*/ 
unsigned short stdButtonFind(const char * const name);

_Bool stdButtonGetName(const unsigned short button, char * const name);
void stdButtonSetName(const unsigned short button, const char * const name);

short stdButtonGet(const unsigned short i);

unsigned short stdButtonGetBI_Button(const unsigned short button);
void stdButtonSetBI_Button(const unsigned short button, const unsigned short handle);

void stdButtonSetToken(const unsigned short button, const enum eToken token);
enum eToken stdButtonGetToken(const unsigned short button);

#endif /* BAD_WALDSEE_BUTTONS_H__ */
