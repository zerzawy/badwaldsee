/*
 * This file is part of Raspberry Pi steuert Modelleisenbahn
 * Copyright(C)  2012 - 2015 Kurt Zerzawy www.zerzawy.ch
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

/*
 * functions for dwarf signals
 *
 * the lamps of the dwarf are 
 *
 *       +-\         /--------\
 *       |1 \        |r1 w2 r2|
 *       |2 3\       |w1      |
 *       +----\      +--------+
 *         |             ||
 *
 *
 *
 * Spreitenbacher Eisenbahn Amateur Klub SPEAK
 * www.speak.li
 * $Author: kurt $
 * $Date: 2015-05-18 22:31:02 +0200 (Mon, 18. Mai 2015) $
 * $Revision: 2464 $
 */

#ifndef BAD_WALDSEE_DWARF_H__
#define BAD_WALDSEE_DWARF_H__

#include "constants.h"
#include "duoled.h"

#ifdef SPDR60
enum eDwarfAspect
	{
		SIG_DW_DARK,
		SIG_DW_SH0,
		SIG_DW_SH1,
		SIG_DW_UNDEF
	};
#endif

#ifdef DOMINO55
	enum eDwarfAspect
	{
		SIG_DW_DARK,
		SIG_DW_HALT,
		SIG_DW_WARNING,
		SIG_DW_CLEAR,
		SIG_DW_UNDEF
	};
#endif

/**
 * Structure for a dwarf 
 */
struct sDwarf
{
	char			name[NAMELEN];			/**< Name of this dwarf	*/
	struct sButton *	pButton;			/**< switchboard button of this dwarf */
	enum eDwarfAspect 	aspect;				/**< aspect of the dwarf */
	#ifdef SPDR60
		unsigned short		BO_Red1;			/**< left red LED of the dwarf	*/
		unsigned short		BO_Red2;			/**< right red LED of the dwarf	*/		
		unsigned short		BO_White1;			/**< lower left white LED of the dwarf	*/		
		unsigned short		BO_White2;			/**< upper right white LED of the dwarf	*/		
		unsigned short  	BO_Desk_Red;			/**< red LED on the switchboard	*/
		unsigned short  	BO_Desk_Green;			/**< green LED on the switchboard	*/
		char			mainsName[NAMELEN];		/**< Name of the main signal combined with this dwarf	*/
		struct sMain *		pMain;				/**< Pointer to the main signal combined with this dwarf	*/
		_Bool			mainsClear;			/**< true if combined main signal has clear aspect (HP1, HP2)	*/
	#endif
	#ifdef DOMINO55
		unsigned short  	BO_LED1;			/**< LED 1 of the dwarf	*/
		unsigned short  	BO_LED2;			/**< LED 2 of the dwarf	*/
		unsigned short  	BO_LED3;			/**< LED 3 of the dwarf	*/
		unsigned short  	BO_Desk_Green;			/**< LED on the switchboard	*/
	#endif
	_Bool           	Todo;				/**< set if work pending	*/
};


/*@maynotreturn@*/
void dwarf(const unsigned short nr);
void dwarfDestroy(void);
void dwarfInit(void);
void dwarfProcess(void);

unsigned short dwarfNrButtons(struct sDwarf ** ppSel1, struct sDwarf ** ppSel2);

struct sDwarf * dwarfFind(const char * const name);
struct sDwarf * dwarfGet(const unsigned short index);
unsigned short dwarfGetNr(void);

void dwarfSetName(struct sDwarf * const pDwarf, const char * const name); 
_Bool dwarfGetName(const struct sDwarf * const pDwarf, /*@out@*/char * const name);

#ifdef SPDR60
	unsigned short dwarfGetBO_Red1(const struct sDwarf * const pDwarf);
	void dwarfSetBO_Red1(struct sDwarf * const pDwarf, const unsigned short handle);

	unsigned short dwarfGetBO_Red2(const struct sDwarf * const pDwarf);
	void dwarfSetBO_Red2(struct sDwarf * const pDwarf, const unsigned short handle);

	unsigned short dwarfGetBO_White1(const struct sDwarf * const pDwarf);
	void dwarfSetBO_White1(struct sDwarf * const pDwarf, const unsigned short handle);

	unsigned short dwarfGetBO_White2(const struct sDwarf * const pDwarf);
	void dwarfSetBO_White2(struct sDwarf * const pDwarf, const unsigned short handle);

	unsigned short dwarfGetBO_Desk_Red(const struct sDwarf * const pDwarf);
	void dwarfSetBO_Desk_Red(struct sDwarf * const pDwarf, const unsigned short handle);

	unsigned short dwarfGetBO_Desk_Green(const struct sDwarf * const pDwarf);
	void dwarfSetBO_Desk_Green(struct sDwarf * const pDwarf, const unsigned short handle);

	_Bool dwarfGetMainName(const struct sDwarf * const pDwarf, /*@out@*/char * const name);
	void dwarfSetMainName(struct sDwarf * const pDwarf, const char * const name); 
	struct sDwarf * dwarfFindMain(char * const name);
	struct sMain * dwarfGetMain(const struct sDwarf * const pDwarf);
	void dwarfSetMain(struct sDwarf * const pDwarf, const struct sMain * const pMain);

	void dwarfSetMainClear(struct sDwarf * const pDwarf, const _Bool clear);
	_Bool dwarfGetMainClear(const struct sDwarf * const pDwarf);
#endif
#ifdef DOMINO55
	unsigned short dwarfGetBO_LED1(const struct sDwarf * const pDwarf);
	void dwarfSetBO_LED1(struct sDwarf * const pDwarf, const unsigned short handle);

	unsigned short dwarfGetBO_LED2(const struct sDwarf * const pDwarf);
	void dwarfSetBO_LED2(struct sDwarf * const pDwarf, const unsigned short handle);

	unsigned short dwarfGetBO_LED3(const struct sDwarf * const pDwarf);
	void dwarfSetBO_LED3(struct sDwarf * const pDwarf, const unsigned short handle);

	unsigned short dwarfGetBO_Desk_Green(const struct sDwarf * const pDwarf);
	void dwarfSetBO_Desk_Green(struct sDwarf * const pDwarf, const unsigned short handle);
#endif

struct sButton * dwarfGetBI_Button(const struct sDwarf * const pDwarf);
void dwarfSetBI_Button(struct sDwarf * const pDwarf, struct sButton * const pButton);

enum eDwarfAspect dwarfGetAspect(const struct sDwarf * const pDwarf);
void dwarfSetAspect(struct sDwarf * const pDwarf, const enum eDwarfAspect aspect);
void dwarfSetAspectDest(struct sDwarf * const pDwarf, 
			const enum eDwarfAspect aspect,
			const struct sDwarf * const pDest);

#endif /* BAD_WALDSEE_DWARF_H__ */
