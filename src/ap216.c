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
 *
 * Spreitenbacher Eisenbahn Amateur Klub SPEAK
 * www.speak.li
*/

/**
 * \file 
 * 2 x 16 Bit output print AP216 
 * \author 
 * Kurt Zerzawy
 */
 
/*
 * $Author: kurt $
 * $Date: 2015-05-18 22:31:02 +0200 (Mon, 18. Mai 2015) $
 * $Revision: 2464 $
 */

/**
 * function to calculate the pin on the output plug of the AP216 board
 *  @param bit	bit to be convertet into the pin number 
 *  @return		pin number or -1 in case of error
 */
short ap216BitToPin(const unsigned short Bit) 
{
	switch(Bit) 
	{
	case 0: return 17;
	case 1: return 18;
	case 2: return 15;
	case 3: return 16;
	case 4: return 13;
	case 5: return 14;
	case 6: return 11;
	case 7: return 12;
	case 8: return 7;
	case 9: return 8;
	case 10: return 5;
	case 11: return 6;
	case 12: return 3;
	case 13: return 4;
	case 14: return 1;
	case 15: return 2;
	default:
		return -1;
	}
}

/**
 * function to calculate the bit for the pin on the plug of the AP216 board
 *  @param bit	pin number to be switched 
 *  @return		bit number or -1 if that pin is not an output
 */
short ap216PinToBit(const unsigned short pin) 
{
	switch(pin) {
	case 1: return 0x0e; 
	case 2: return 0x0f; 
	case 3: return 0x0c; 
	case 4: return 0x0d; 
	case 5: return 0x0a; 
	case 6: return 0x0b; 
	case 7: return 0x08; 
	case 8: return 0x09; 
		
	case 11: return 0x06; 
	case 12: return 0x07; 
	case 13: return 0x04; 
	case 14: return 0x05; 
	case 15: return 0x02; 
	case 16: return 0x03; 
	case 17: return 0x00; 
	case 18: return 0x01; 
		
	default:
		return -1;
	}
}
