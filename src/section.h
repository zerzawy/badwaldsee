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

/*
 * functions for single sections and LEDs in the switchboard
 * Spreitenbacher Eisenbahn Amateur Klub SPEAK
 * www.speak.li
 * $Author: Kurt $
 * $Date: 2015-08-05 08:06:23 +0200 (Mi, 05 Aug 2015) $
 * $Revision: 2537 $
 */

#ifndef BAD_WALDSEE_SECTION_H__
#define BAD_WALDSEE_SECTION_H__

#include "constants.h"
#include "duoled.h"

#define NRBLOCKPL	16

/**
 * Structure for a section 
 */
struct sSection
{
	char           name[NAMELEN];		/**< Name of this section	*/
	unsigned short BI_occDetector;		/**< Occupancy detector register	*/
	unsigned short DuoledAddPosition;	/**< Number of plates with duoled	*/
	struct sDuoled * pDuoled[NRBLOCKPL];	/**< Array with duoleds visualising this section	*/
	enum eLine     line;
	enum eLine     linePrev;

	_Bool free	: 1;	/**< true when section is free */
	_Bool wasFree	: 1;	/**< true when section was free before last process */
	_Bool route	: 1;	/**< true if section is part of a route	*/
	_Bool wasRoute	: 1;	/**< true if section was part of a route before last process	*/
};


/*@maynotreturn@*/
void sections(const unsigned short anzahl);
void sectionsDestroy(void);
void sectionsInit(void);
void sectionsProcess(void);
_Bool sectionsReadHW(void);

_Bool sectionLine(struct sSection * const ptr, const enum eLine requestedLine);
/*@maynotreturn@*/ _Bool sectionIsFree(const struct sSection * const ptr);

/*@maynotreturn@*/
void sectionParser(void);

/*@maynotreturn@*/
struct sSection * sectionFind(const char * const name);

_Bool sectionGetName(const struct sSection * const ptr, char * const name);
void sectionSetName(struct sSection * const ptr, const char * const name);
struct sSection * sectionGet(const unsigned short i);
struct sDuoled * sectionGetDuoled(const struct sSection * ptr, const unsigned short i);
void sectionAddDuoled(struct sSection * const ptr, struct sDuoled * const pDuoled);
unsigned short sectionGetNr(void);

unsigned short sectionGetBI_occDetector(const struct sSection * const ptr);
void sectionSetBI_occDetector(struct sSection * const ptr, const unsigned short handle);

#endif /* BAD_WALDSEE_SECTION_H__ */

