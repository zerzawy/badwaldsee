/*
 * This file is part of Raspberry Pi steuert Modelleisenbahn
 * Copyright(C)  2014 - 2015 Kurt Zerzawy www.zerzawy.ch
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
 * Main signals (Hauptsignale)
 * \author
 * Kurt Zerzawy
 */

/*
 * functions for main signals
 *
 * the lamps of the main signal are 
 *
 *         +---+                +---+
 * green1  |x  |          green | x |
 * yellow2 |x x| red1       red |x x| red1*          * from dwarf
 * green2  |x x|                |  x| white2*
 * yellow1 |x  |        white1* |x  |
 * green3  |x  |         yellow | x |
 *         +---+                +---+
 *           |                    |
 *
 *
 *
 * Spreitenbacher Eisenbahn Amateur Klub SPEAK
 * www.speak.li
 * $Author: Kurt $
 * $Date: 2015-11-14 23:10:12 +0100 (Sa, 14 Nov 2015) $
 * $Revision: 2672 $
 */

#ifndef BAD_WALDSEE_MAINS_H__
#define BAD_WALDSEE_MAINS_H__

#include "constants.h"
#include "duoled.h"

#ifdef SPDR60
	enum eMainAspect
	{
		SIG_MN_UNDEF,
		SIG_MN_HP0,
		SIG_MN_HP1,
		SIG_MN_HP1_ZS3,
		SIG_MN_HP2,
		SIG_MN_HP2_ZS3
	};
#endif

#ifdef DOMINO55
	enum eMainAspect
	{
		SIG_MN_UNDEF,
		SIG_MN_HALT,
		SIG_MN_1,
		SIG_MN_2,
		SIG_MN_3,
		SIG_MN_5,
		SIG_MN_6
	};
#endif



/**
 * Structure for a main signal
 */
struct sMain
{
	char			name[NAMELEN];		/**< Name of this main signal	*/
	struct sButton *	pButton;		/**< switchboard button of this main signal	*/
	enum eMainAspect 	eAspect;		/**< aspect of the main signal	*/
	enum eMainAspect 	eRequestedAspect;	/**< requested aspect of the main signal	*/
	#ifdef SPDR60
		unsigned short		BO_Green;		/**< green LED of the main signal	*/
		unsigned short		BO_Red;			/**< red LED of the main signal	*/
		unsigned short		BO_Yellow;		/**< yellow LED of the main signal	*/
		unsigned short		BO_Zs3;			/**< Zs3 signal on this main signal	*/
		unsigned short		BO_Zp9;			/**< Zp9 signal on this main signal	*/

		struct sDwarf *		pDwarf;			/**< pointer to the dwarf attached to this main	*/
	#endif

	#ifdef DOMINO55
		unsigned short  	BO_Green1;		/**< green LED 1 of the main signal	*/
		unsigned short  	BO_Green2;		/**< green LED 2 of the main signal	*/
		unsigned short  	BO_Green3;		/**< green LED 3 of the main signal	*/
		unsigned short  	BO_Yellow1;		/**< yellow LED 1 of the main signal	*/
		unsigned short  	BO_Yellow2;		/**< yellow LED 2 of the main signal	*/
		unsigned short  	BO_Red1;		/**< red LED of the main signal	*/
		unsigned short  	BO_Depart;		/**< departure LED belonging to this main signal	*/
	#endif

	char			distantName[NAMELEN];	/**< Name of the distant signal on the same mast	*/
	struct sDistant *	pDistant;		/**< pointer to the distant signal on the same mast	*/
	unsigned short  	BO_Desk_Red;		/**< red LED on the switchboard	*/
	unsigned short  	BO_Desk_Green;		/**< green LED on the switchboard	*/
	_Bool           	ToDo;			/**< set if work pending	*/
};


/*@maynotreturn@*/
void mains(const unsigned short nr);
void mainsDestroy(void);
void mainsInit(void);
void mainsProcess(void);

unsigned short mainNrButtons(struct sMain ** ppSel1, struct sMain ** ppSel2);

struct sMain * mainFind(const char * const name);
struct sMain * mainGet(const unsigned short i);
unsigned short mainsGetNr(void);

_Bool mainGetName(const struct sMain * const pMains, /*@out@*/char * const name);
void mainSetName(struct sMain * const pMains, const char * const name); 

#ifdef SPDR60
	unsigned short mainGetBO_Green(const struct sMain * const pMains);
	void mainSetBO_Green(struct sMain * const pMains, const unsigned short handle);

	unsigned short mainGetBO_Red(const struct sMain * const pMains);
	void mainSetBO_Red(struct sMain * const pMains, const unsigned short handle);

	unsigned short mainGetBO_Yellow(const struct sMain * const pMains);
	void mainSetBO_Yellow(struct sMain * const pMains, const unsigned short handle);

	unsigned short mainGetBO_Zs3(const struct sMain * const pMains);
	void mainSetBO_Zs3(struct sMain * const pMains, const unsigned short handle);

	unsigned short mainGetBO_Zp9(const struct sMain * const pMains);
	void mainSetBO_Zp9(struct sMain * const pMains, const unsigned short handle);
#endif

#ifdef DOMINO55
	unsigned short mainGetBO_Green1(const struct sMain * const pMains);
	void mainSetBO_Green1(struct sMain * const pMains, const unsigned short handle);

	unsigned short mainGetBO_Green2(const struct sMain * const pMains);
	void mainSetBO_Green2(struct sMain * const pMains, const unsigned short handle);

	unsigned short mainGetBO_Green3(const struct sMain * const pMains);
	void mainSetBO_Green3(struct sMain * const pMains, const unsigned short handle);

	unsigned short mainGetBO_Yellow1(const struct sMain * const pMains);
	void mainSetBO_Yellow1(struct sMain * const pMains, const unsigned short handle);

	unsigned short mainGetBO_Yellow2(const struct sMain * const pMains);
	void mainSetBO_Yellow2(struct sMain * const pMains, const unsigned short handle);

	unsigned short mainGetBO_Red1(const struct sMain * const pMains);
	void mainSetBO_Red1(struct sMain * const pMains, const unsigned short handle);

	unsigned short mainGetBO_Depart(const struct sMain * const pMains);
	void mainSetBO_Depart(struct sMain * const pMains, const unsigned short handle);
#endif

unsigned short mainGetBO_Desk_Red(const struct sMain * const pMains);
void mainSetBO_Desk_Red(struct sMain * const pMains, const unsigned short handle);

unsigned short mainGetBO_Desk_Green(const struct sMain * const pMains);
void mainSetBO_Desk_Green(struct sMain * const pMains, const unsigned short handle);

struct sButton * mainGetBI_Button(const struct sMain * const pMains);
void mainSetBI_Button(struct sMain * const pMains, struct sButton * const pButton);

#include "distant.h"
_Bool mainGetDistantName(const struct sMain * const pMains, /*@out@*/char * const name);
void mainSetDistantName(struct sMain * const pMains, const char * const name); 
struct sDistant * mainGetDistant(const struct sMain * pMains);
void mainSetDistant(struct sMain * const pMains, struct sDistant * const pDistant);

enum eMainAspect mainGetAspect(const struct sMain * const pMains);
void mainSetAspect(struct sMain * const pMains, const enum eMainAspect eRequestedAspect);

#endif /* BAD_WALDSEE_MAINS_H__ */

