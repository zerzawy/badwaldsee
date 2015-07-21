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
 * Functions for combined shunting routes 
 * \author
 * Kurt Zerzawy
 */

/*
 * Spreitenbacher Eisenbahn Amateur Klub SPEAK
 * www.speak.li
 * $Author: kurt $
 * $Date: 2015-05-18 22:31:02 +0200 (Mon, 18. Mai 2015) $
 * $Revision: 2464 $
 */

#ifndef BAD_WALDSEE_COMBI_SHUNTROUTE_H_
#define BAD_WALDSEE_COMBI_SHUNTROUTE_H_

#include "dwarf.h"
#include "turnout.h"

#define NRCOMBISHUNTS	16


/**
 * Structure for a shuntroute 
 */
struct sCombiShuntroute
{
	char			name[NAMELEN];		/**< Name of this shuntroute	*/
	struct sDwarf *		start;			/**< pointer to the start dwarf	*/
	struct sDwarf *		dest;			/**< pointer to the destination dwarf	*/
};


/*@maynotreturn@*/
void combishuntroute(const unsigned short anzahl);
void combishuntrouteDestroy(void);
void combishuntrouteInit(void);
unsigned short combishuntrouteGetNr(void);

void combishuntrouteProcess(void);

_Bool combishuntrouteGetName(const struct sCombiShuntroute * const ptr, char * const name);
void combishuntrouteSetName(struct sCombiShuntroute * const ptr, const char * const name);
struct sCombiShuntroute * combishuntrouteGet(const unsigned short i);

#endif /* BAD_WALDSEE_COMBI_SHUNTROUTE_H_ */
