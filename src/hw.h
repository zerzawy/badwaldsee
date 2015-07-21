/* 
 * This file is part of uPStellwerk.
 * Copyright(C)  2008 - 2014 Kurt Zerzawy www.zerzawy.ch
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
 * If not, see <http:www.gnu.org/licenses/>.
 */

/* This is the defining file for the Bad Waldsee switchboard on UNC20
 * NOTE: all functions apply to the IO of the UNC20, not the board itself
 * On the board, the signals are invers.
 * Spreitenbacher Eisenbahn Amateur Klub SPEAK
 * www.speak.li
 *
 * $Author: kurt $
 * $Date: 2015-05-18 22:31:02 +0200 (Mon, 18. Mai 2015) $
 * $Revision: 2464 $
 */

/* 
 * Definition der Ein- und Ausgaben.
 *
 * Hier lernen Sie die unterste Schale des Programms kennen. Hier werden
 * die Ein- und Ausgaben des uCP definiert, damit Sie sie in Ihren 
 * Programmen verwenden koennen.
 
 * Die Funktionen hier sind so kurz wie irgend moeglich, weil sie zeitkritisch 
 * sind. Auf die Reservation der Ports habe ich verzichtet, wie dies bei 
 * einem richtigen Linux-Treiber der Fall ist. Das kommt eventuell spaeter. 
 * Wenn Sie das Programm mit voellig anderer Hardware einsetzen wollen, 
 * sollten Sie Sich das Schreiben eines Treibers ernsthaft ueberlegen
 * 
 * Den Anfang macht eine Anzahl von Definitionen, bei dem die Hardware
 * adressen und die verwendeten Bits des uCP definiert werden. Wenn 
 * ie aus irgend einem Grund eine andere HW verwenden, dann muessen 
 * Sie hier Aenderungen anbringen.
 */

#include "stdbool.h"

#define HW_TEST
#ifndef HW_H_
#define HW_H_

#ifndef _Bool
#define _Bool bool
#endif

#include "bcm2835.h" 

/**
 * general definition of ports
 */
#define PORT_OUT_STROBE_PIN	RPI_V2_GPIO_P1_11
#define PORT_KEY_PIN		RPI_V2_GPIO_P1_12
#define PORT_OUT_OUT_PIN	RPI_V2_GPIO_P1_13
#define PORT_IN_IN_PIN		RPI_V2_GPIO_P1_15
#define PORT_OUT_CLK_PIN	RPI_V2_GPIO_P1_16
#define PORT_IN_PS_PIN		RPI_V2_GPIO_P1_18
#define PORT_IN_CLK_PIN		RPI_V2_GPIO_P1_22
#define PORT_WATCHDOG_PIN	RPI_V2_GPIO_P1_07

int ports_init(void);
void ports_cleanup(void);

void port_set_in_clk(const _Bool level);
inline void port_set_in_clk_0(void);
inline void port_set_in_clk_1(void);
void port_set_in_ps(const _Bool level);
_Bool port_get_in_in(void);
void port_set_out_clk(const _Bool level);
inline void port_set_out_clk_0(void);
inline void port_set_out_clk_1(void);
void port_set_out_out(const _Bool level);
void port_set_out_strobe(const _Bool level);
void port_set_watchdog(const _Bool level);
_Bool port_get_key(void);
_Bool filteredKey(void);
void waitALittle(void);
void tickTimeStart();
void tickTimeWait(const long long int time);
long long int tickTimeUsed(const long long int time);


#endif /*HW_H_*/

