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

/*
 * Functions for temporary data storage
 */

/*
 * $Author: Kurt $
 * $Date: 2015-08-05 08:06:23 +0200 (Mi, 05 Aug 2015) $
 * $Revision: 2537 $
 */

#ifndef VARS_H_
#define VARS_H_

#include "registers.h"        /* registers for the io                 */

void vars(struct sRegister * const pRegister,
	  const unsigned short registerLen);
void varsDestroy(void);
void varsInit(void);
unsigned short varsGetRegLen(void);

#endif /* VARS_H_ */
