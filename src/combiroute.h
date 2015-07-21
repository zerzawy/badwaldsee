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

/*
 * \file
 * Functions for combi train routes 
 * \author
 * Kurt Zerzawy
 */

/*
 * Spreitenbacher Eisenbahn Amateur Klub SPEAK
 * www.speak.li
 * $Author: kurt $
 * $Date: 2015-06-05 22:17:07 +0200 (Fre, 05. Jun 2015) $
 * $Revision: 2482 $
 */

#ifndef BAD_WALDSEE_COMBIROUTE_H_
#define BAD_WALDSEE_COMBIROUTE_H_

#include "mains.h"

#define NRCOMBIPARTS       16

/**
 * Structure for a train route 
 */
struct sCombiroute
{
	char			name[NAMELEN];		/**< Name of this train route	*/
	struct sMain *		start;			/**< pointer to the start main signal	*/
	struct sMain *		dest;			/**< pointer to the destination main signal	*/
	unsigned short		partsAddPosition;	/**< position the next element is added	*/
	struct sRoute *		pRoute[NRCOMBIPARTS];	/**< array with the single routes	*/
};


/*@maynotreturn@*/
void combiroutes(const unsigned short anzahl);
void combiroutesDestroy(void);
void combiroutesInit(void);

unsigned short combiroutesGetNr(void);
struct sCombiroute * combirouteGet(const unsigned short index);
struct sCombiroute * combirouteFind(const char * const name);

_Bool combirouteGetName(const struct sCombiroute * const pCombiroute, /*@out@*/char * const name);
void combirouteSetName(struct sCombiroute * const pCombiroute, const char * const name); 

struct sMain * combirouteGetStart(const struct sCombiroute * pMain);
void combirouteSetStart(struct sCombiroute * const pMain, struct sMain * start);

struct sMain * combirouteGetDest(const struct sCombiroute * pMain);
void combirouteSetDest(struct sCombiroute * const pCombiroute, struct sMain * dest);

unsigned short combirouteGetNrRoutes(const struct sCombiroute * const pCombiroute);

unsigned short combirouteAddRoute(struct sCombiroute * const pCombiroute, struct sRoute * const pRoute);
struct sRoute * combirouteGetRoute(const struct sCombiroute * const pCombiroute, const unsigned short index);

#endif /* BAD_WALDSEE_COMBIROUTE_H_ */
