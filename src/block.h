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
 * blocks (Bloecke)
 * \author
 * Kurt Zerzawy
 */

/*
 * Spreitenbacher Eisenbahn Amateur Klub SPEAK
 * www.speak.li
 * $Author: kurt $
 * $Date: 2015-05-09 21:54:18 +0200 (Sam, 09. Mai 2015) $
 * $Revision: 2441 $
 */

#ifndef BAD_WALDSEE_BLOCKS_H__
#define BAD_WALDSEE_BLOCKS_H__

#include "constants.h"
#include "section.h"

#define	BLOCKNRSECTIONS	8

enum eBlockState
{
	BlockNeutral,
	BlockOccupied,
	BlockFree
};


/**
 * Structure for a block 
 */
struct sBlock
{
	char			name[NAMELEN];		/**< Name of this block */
	enum eBlockState	state;			/**< state of the state machine		*/
	unsigned short		BO_Desk_Red;		/**< occupied led in desk		*/
	struct sMain *		pMain;			/**< Entry signal of this block 	*/

	unsigned short		nrSections;		/**< number of sections for occupancy of this block	*/
	struct sSection *	pSection[BLOCKNRSECTIONS];/**< sections for occupancy of this block	*/

	_Bool           	Todo;			/**< set if work pending	*/
};


/*@maynotreturn@*/
void blocks(const unsigned short nr);
void blocksDestroy(void);
void blocksInit(void);
void blocksProcess(void);

struct sBlock * blockFind(const char * const name);
struct sBlock * blockGet(const unsigned short i);
unsigned short blocksGetNr(void);

const _Bool blockIsFree(const struct sBlock * const pBlock);

_Bool blockGetName(const struct sBlock * const pBlock, /*@out@*/char * const name);
void blockSetName(struct sBlock * const pBlock, const char * const name); 

const unsigned short blockGet_BO_DeskLED(const struct sBlock * const pBlock);
void blockSet_BO_DeskLED(struct sBlock * const pBlock, const unsigned short handle);

const struct sMain * blockGetMain(const struct sBlock * const pBlock);
void blockSetMain(struct sBlock * const pBlock, struct sMain * const pMain);

unsigned short blockAddSection(struct sBlock * const pBlock, struct sSection * const pSection);
struct sSection * blockGetSection(const struct sBlock * const pBlock, const unsigned short index);

#endif /* BAD_WALDSEE_BLOCKS_H__ */

