/*
 * This file is part of Raspberry Pi steuert Modelleisenbahn
 * Copyright(C)  2012 - 1015 Kurt Zerzawy www.zerzawy.ch
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
 * Constants 
 */

/*
 * $Author: kurt $
 * $Date: 2015-06-23 22:35:22 +0200 (Die, 23. Jun 2015) $
 * $Revision: 2490 $
 */


#ifndef __BAD_WALDSEE_CONSTANTS_H_
#define __BAD_WALDSEE_CONSTANTS_H_


#define NONEVALUE 0xFFFF
#define NAMELEN 25 
/*@constant int NAMELEN@*/

/**
 * lines for building up and keeping a route. 
 */
enum eLine {
	interlockLR	= 0b01001111,
	sectionLR	= 0b01000111,
	switchLR	= 0b01000011,
	reserveLR	= 0b01000001,
	neutral		= 0b00000000,
	reserveRL	= 0b00100001,
	switchRL	= 0b00100011,
	sectionRL	= 0b00100111,
	interlockRL	= 0b00101111,
};

#define LINERESERVE	0b00000001
#define LINESWITCH	0b00000011
#define LINEBLOCK	0b00000111
#define LINEINTERLOCK	0b00001111
#define LINELR		0b01000000
#define LINERL		0b00100000
#define LINEMASK        0b00001111
#define LINEDIRMASK	0b01100000

#endif 

