/*
 * This file is part of Raspberry Pi steuert Modelleisenbahn
 * Copyright(C)  2008 - 2015 Kurt Zerzawy www.zerzawy.ch
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

/* Function for shifting data to hardware of
 * Bad Waldsee switchboard
 * Spreitenbacher Eisenbahn Amateur Klub SPEAK
 * www.speak.li
 *
 * $Author: kurt $
 * $Date: 2015-05-18 22:31:02 +0200 (Mon, 18. Mai 2015) $
 * $Revision: 2464 $
 */

#ifndef SHIFTOUT_H_
#define SHIFTOUT_H_

#include "registers.h"        /* registers for the io                 */

// #define __TEST__

/*@maynotreturn@*/
void outshift(struct sRegister * const pRegister,
	      const unsigned short registerLen);
void outshiftDestroy(void);
void outshiftInit(void);
void outshiftEnable(_Bool enable);
void outshiftRegSend(void);
void outshiftToDo(void);

unsigned short outshiftGetRegLen(void);
unsigned short outshiftGetHandle(const unsigned short word, const unsigned short bit);

#endif /* SHIFTOUT_H_ */
