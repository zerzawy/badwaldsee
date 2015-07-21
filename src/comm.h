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
 * TCP/IP communication
 */

/* 
 * $Author: kurt $
 * $Date: 2015-05-18 22:31:02 +0200 (Mon, 18. Mai 2015) $
 * $Revision: 2464 $
 */

#ifndef COMMUNICATE_H_
#define COMMUNICATE_H_

// #define __TEST__

/**
 * Prepares the communication
 * @param pReg			pointer to the datas of the registers
 * @param outshiftLen	number of words of the outshift register
 * @param inshiftLen	number of words of the inshift register
 * @param tempLen		number of words of the variable register
 */
void comm(struct sRegister * const pReg,
				const unsigned short outshiftLen,
				const unsigned short inshiftLen,
				const unsigned short tempLen);

/**
 * Stops the communication and frees all resources
 */
void commDestroy(void);

/**
 * Initialises and starts the communication
 */
void commInit(void);

/**
 * Function to be called on a regular base to respawn a died communication
 * @param timeout       number of call waited for until thread is calle agoin
 */
void commTask(unsigned short timeout);

/**
 * Function to return if the TCP socket is now forcing the output and thus
 *  the logic shall not overwrite the values of the AP216
 * @return              true if TCP socket is forcing
 */ 
_Bool commForcing(void);

/**
 * Returns the running state of the communication thread
 * @return              true if running
 */
_Bool commRunning(void);

#endif /*COMMUNICATE_H_*/

