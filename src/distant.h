/*
 * This file is part of Raspberry Pi steuert Modelleisenbahn
 * Copyright(C)  2015 Kurt Zerzawy www.zerzawy.ch
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
 * Distant signals (Vorsignale)
 * \author
 * Kurt Zerzawy
 */

/*
 * functions for distant signals
 *
 * the lamps of the distant signal are 
 *
 *           +-----+                       +---+
 * yellow11  |x   x| yellow13    yellow13 /x x/ green14
 *           |    x| green14             /   / 
 * green12   |x   x| green15   yellow11 /x x/ green12  
 *           +-----+                   +---+
 *              |                         |
 *
 *
 *
 * Spreitenbacher Eisenbahn Amateur Klub SPEAK
 * www.speak.li
 * $Author: kurt $
 * $Date: 2015-05-09 21:54:18 +0200 (Sam, 09. Mai 2015) $
 * $Revision: 2441 $
 */

#ifndef BAD_WALDSEE_DISTANTS_H__
#define BAD_WALDSEE_DISTANTS_H__

#include "constants.h"
#include "duoled.h"
#include "mains.h"

#ifdef SPDR60
	enum eDistantsAspect
	{
		SIG_DI_UNDEF,
		SIG_DI_VR0,
		SIG_DI_VR1,
		SIG_DI_VR1_ZS3V,
		SIG_DI_VR2,
		SIG_DI_VR2_ZS3V,
		SIG_DI_BLANKED
	};
#endif

#ifdef DOMINO55
	enum eDistantsAspect
	{
		SIG_DI_UNDEF,
		SIG_DI_WARN,
		SIG_DI_1,
		SIG_DI_2,
		SIG_DI_3,
		SIG_DI_5,
		SIG_DI_6,
		SIG_DI_BLANKED
	};
#endif



/**
 * Structure for a distant signal
 */
struct sDistant
{
	char			name[NAMELEN];		/**< Name of this distant signal	*/
	enum eDistantsAspect 	aspect;			/**< aspect of the distant signal	*/
	enum eDistantsAspect	state;			/**< state of the state maching		*/


	#ifdef SPDR60
		unsigned short		BO_Yellow13;		/**< upper left yellow LED of the distant signal	*/
		unsigned short		BO_Green14;		/**< upper right green LED of the distant signal	*/
		unsigned short		BO_Yellow11;		/**< lower left yellow LED of the distant signal	*/
		unsigned short		BO_Green12;		/**< lower right green LED of the distant signal	*/
		unsigned short		BO_Zs3v;		/**< Zs3v signal on this distant signal	*/
	#endif

	#ifdef DOMINO55
		unsigned short  	BO_Yellow11;		/**< upper left yellow LED of the distant signal	*/
		unsigned short  	BO_Yellow13;		/**< upper right yellow LED of the distant signal	*/
		unsigned short  	BO_Green14;		/**< middle right green LED of the distant signal	*/
		unsigned short  	BO_Green12;		/**< lower left green LED of the distant signal	*/
		unsigned short  	BO_Green15;		/**< lower right green LED of the distant signal	*/
		_Bool			darkOnMainHalt;		/**< true if distant is dark if mains on same mast is Halt	*/
	#endif

	unsigned short  	BO_Desk_Yellow;		/**< yellow LED on the switchboard	*/
	unsigned short  	BO_Desk_Green;		/**< green LED on the switchboard	*/
	struct sMain *		pMain;			/**< pointer to the mains signal this dwarf is mounted on	*/
	enum eMainAspect	mainAspect;		/**< aspect of the mains on the same mast	*/
	_Bool           	Todo;			/**< set if work pending	*/
};


/*@maynotreturn@*/
void distants(const unsigned short nr);
void distantsDestroy(void);
void distantsInit(void);
void distantsProcess(void);

struct sDistant * distantFind(const char * const name);
struct sDistant * distantGet(const unsigned short i);
unsigned short distantsGetNr(void);

void distantSetName(struct sDistant * const pDistant, const char * const name); 
_Bool distantGetName(const struct sDistant * const pDistant, /*@out@*/char * const name);

unsigned short distantGetBO_Yellow13(const struct sDistant * const pDistant);
void distantSetBO_Yellow13(struct sDistant * const pDistant, const unsigned short handle);

unsigned short distantGetBO_Green14(const struct sDistant * const pDistant);
void distantSetBO_Green14(struct sDistant * const pDistant, const unsigned short handle);

unsigned short distantGetBO_Yellow11(const struct sDistant * const pDistant);
void distantSetBO_Yellow11(struct sDistant * const pDistant, const unsigned short handle);

unsigned short distantGetBO_Green12(const struct sDistant * const pDistant);
void distantSetBO_Green12(struct sDistant * const pDistant, const unsigned short handle);

#ifdef SPDR60
	unsigned short distantGetBO_Zs3v(const struct sDistant * const pDistant);
	void distantSetBO_Zs3v(struct sDistant * const pDistant, const unsigned short handle);

	unsigned short distantGetBO_Desk_Yellow(const struct sDistant * const pDistant);
	void distantSetBO_Desk_Yellow(struct sDistant * const pDistant, const unsigned short handle);

	unsigned short distantGetBO_Desk_Green(const struct sDistant * const pDistant);
	void distantSetBO_Desk_Green(struct sDistant * const pDistant, const unsigned short handle);
#endif

#ifdef DOMINO55
	unsigned short distantGetBO_Green15(const struct sDistant * const pDistant);
	void distantSetBO_Green15(struct sDistant * const pDistant, const unsigned short handle);

	_Bool distantGetDarkOnHalt(const struct sDistant * const pDistant);
	void distantSetDarkOnHalt(struct sDistant * const pDistant, const _Bool dark);
#endif

enum eMainAspect distantGetMainAspect(const struct sDistant * const pDistant);
void distantSetMainAspect(struct sDistant * const pDistant, const enum eMainAspect aspect);

struct sMain * distantGetMain(const struct sDistant * const pDistant);
void distantSetMain(struct sDistant * const pDistant, const struct sMain * pMain);

enum eDistantsAspect distantGetAspect(const struct sDistant * const pDistant);
void distantSetAspect(struct sDistant * const pDistant, const enum eDistantsAspect aspect);

#endif /* BAD_WALDSEE_DISTANTS_H__ */

