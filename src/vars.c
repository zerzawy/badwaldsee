/*
 * This file is part of Raspberry Pi steuert Modelleisenbahn
 * Copyright(C)  2010 - 2015 Kurt Zerzawy www.zerzawy.ch
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
 *
 * Spreitenbacher Eisenbahn Amateur Klub SPEAK
 * www.speak.li
 */

/**
 * \file
 * Temporary data storage
 * \author
 * Kurt Zerzawy
 */

/*
 * $Author: Kurt $
 * $Date: 2015-08-05 08:06:23 +0200 (Mi, 05 Aug 2015) $
 * $Revision: 2537 $
 */

#include "vars.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "assert.h"
#include "log.h"


/*@null@*/ static struct sRegister * pVarsReg = NULL;
static unsigned short varsRegLen = 0;

/**
 * 'Constructor' for the variable section object. It expects a
 * RAM area allocated of the register type.
 * @param pRegister      pointer to the output register
 * @param registerLen    length of the output register in 16 Bit 
 *                       units.
 */
void vars(struct sRegister * const pRegister,
		const unsigned short registerLen) 
{
	LOG_INF("started");
	assert(NULL != pRegister);
	if(!pRegister) {
		LOG_ERR("no pointer given");
		exit(EXIT_FAILURE);
	}

	pVarsReg = pRegister;
	varsRegLen = registerLen;
	
	/* now set all values to 0 */
	memset(pVarsReg, 0, registerLen * sizeof(unsigned short));

	LOG_INF("ended");
}

/**
 * 'Destructor' for the variable object
 */
void varsDestroy(void) 
{
	LOG_INF("started");
	free(pVarsReg);
	LOG_INF("ended");
}

/**
 * Initializes the variables 
 * @param pRegisterStart beginning of the variable register area
 * @param registerLen
 */
void varsInit(void) 
{
	LOG_INF("started");
	assert(NULL != pVarsReg);
	
	/* now set all values to 0 */
	memset(pVarsReg, 0, varsRegLen * sizeof(unsigned short));
	LOG_INF("ended");
}

/**
 * Getter function for the variable register length
 * @return               register length
 */
unsigned short varsGetRegLen(void) 
{
	return varsRegLen;
}

