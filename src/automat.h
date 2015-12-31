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
 * automats. The automats set a route without any manual action
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

#ifndef BAD_WALDSEE_AUTOMATS_H__
#define BAD_WALDSEE_AUTOMATS_H__

#include "constants.h"
#include "section.h"
#include "route.h"

#define	AUTOMATNRSECTIONS	8
#define AUTOMATNRROUTES		8

enum eAutomatState
{
	AutomatNeutral,
	AutomatOff,
	AutomatOn
};


/**
 * Structure for a automat 
 */
struct sAutomat
{
	char			name[NAMELEN];		/**< Name of this automat */
	enum eAutomatState	state;			/**< state of the state machine		*/

	struct sRoute *		pAutomatedRoute;	/**< route to be switched when conditions apply	*/

	#ifdef DOMINO55
		struct sButton *	pOnButton;		/**< button to switch the automat on	*/
		struct sButton *	pOffButton;		/**< button to switch the automat off	*/
	#endif
	unsigned short		BO_DeskLED;		/**< LED in desk to show active state of automat	*/

	unsigned short		nrSections;		/**< number of sections for occupancy activation of this automat	*/
	struct sSection *	pSection[AUTOMATNRSECTIONS];	/**< sections for occupancy activation of this automat	*/

	unsigned short 		nrRoutes;		/**< number of routes for activation of this automat	*/
	struct sRoute *		pRoute[AUTOMATNRROUTES];	/**< routes for activation of this automat	*/

	_Bool			switchOn;		/**< set if automat shall be switched on	*/
	_Bool			switchOff;		/**< set if automat shall be switched off	*/
	_Bool           	Todo;			/**< set if work pending	*/
};


/*@maynotreturn@*/
void automats(const unsigned short nr);
void automatsDestroy(void);
void automatsInit(void);
void automatsProcess(void);

struct sAutomat * automatFind(const char * const name);
struct sAutomat * automatGet(const unsigned short i);
unsigned short automatsGetNr(void);

_Bool automatGetOn(const struct sAutomat * const pAutomat);
void automatSetOn(struct sAutomat * const pAutomat, _Bool on);

_Bool automatGetName(const struct sAutomat * const pAutomat, /*@out@*/char * const name);
void automatSetName(struct sAutomat * const pAutomat, const char * const name); 

struct sRoute * automatGetAutomatedRoute(const struct sAutomat * const pAutomat);
void automatSetAutomatedRoute(struct sAutomat * const pAutomat, struct sRoute * const pRoute);

#ifdef DOMONO55
	const struct sButton * automatGetOnButton(const struct sAutomat * const pAutomat);
	void automatSetOnButton(struct sAutomat * const pAutomat, struct sButton * const pButton);

	const struct sButton * automatGetOffButton(const struct sAutomat * const pAutomat);
	void automatSetOffButton(struct sAutomat * const pAutomat, struct sButton * const pButton);
#endif

const unsigned short automatGet_BO_DeskLED(const struct sAutomat * const pAutomat);
void automatSet_BO_DeskLED(struct sAutomat * const pAutomat, const unsigned short handle);

unsigned short automatAddSection(struct sAutomat * const pAutomat, struct sSection * const pSection);
struct sSection * automatGetSection(const struct sAutomat * const pAutomat, const unsigned short index);

unsigned short automatAddRoute(struct sAutomat * const pAutomat, struct sRoute * const pRoute);
struct sRoute * automatGetRoute(const struct sAutomat * const pAutomat, const unsigned short index);

#endif /* BAD_WALDSEE_AUTOMATS_H__ */

