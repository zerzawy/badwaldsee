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
 * Functions and definitions for the raspberry pi IO
 * \author
 * Kurt Zerzawy
 */

/**
 * NOTE: all functions apply to the IO of the raspberry PI, not the board itself.
 *
 * On the board, the signals are invers.
 */

/* $Author: Kurt $
 * $Date: 2015-08-05 08:06:23 +0200 (Mi, 05 Aug 2015) $
 * $Revision: 2537 $
 */

#include <time.h>
#include <stdio.h>          /* for error messages */
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "hw.h"

#ifdef WITHOUT_RASPBERRY_HW_
/* compile for tests without HW attached    */
#else
/* should be compiled for real HW attached  */

#ifdef __KERNEL__
#define HW_H__IS_KERNEL__
#endif

#define __KERNEL__

#include "bcm2835.h"     	/* for io functionality */

static long long int offset;	/**< time for the clock */
static long long int compare;	/**< time to be waited for */
static long long int delay;	/**< time used for main loop */

/**
 * function to init all ports
 */
int ports_init(void) 
	
{
	if (!bcm2835_init())
        	return -1;

	bcm2835_gpio_fsel(PORT_OUT_STROBE_PIN, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(PORT_KEY_PIN, BCM2835_GPIO_FSEL_INPT);
	bcm2835_gpio_fsel(PORT_OUT_OUT_PIN, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(PORT_IN_IN_PIN, BCM2835_GPIO_FSEL_INPT);
	bcm2835_gpio_fsel(PORT_OUT_CLK_PIN, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(PORT_IN_PS_PIN, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(PORT_IN_CLK_PIN, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(PORT_WATCHDOG_PIN, BCM2835_GPIO_FSEL_OUTP);
}	/* end of ports_init */

void ports_cleanup(void) 
{
	bcm2835_close();
	return;
}	/* end of ports_cleanup */

/*
 * setter and getter functions of single in/outputs
 */
inline void port_set_in_clk_1(void) 
{
	bcm2835_gpio_write(PORT_IN_CLK_PIN, LOW);
}

inline void port_set_in_clk_0(void) 
{
	bcm2835_gpio_write(PORT_IN_CLK_PIN, HIGH);
}

void port_set_in_clk(const _Bool level) 
{
	if (level) 
		bcm2835_gpio_write(PORT_IN_CLK_PIN, LOW);
	else 
		bcm2835_gpio_write(PORT_IN_CLK_PIN, HIGH);
}

void port_set_in_ps(const _Bool level) 
{
	if (level) 
		bcm2835_gpio_write(PORT_IN_PS_PIN, LOW);
	else 
		bcm2835_gpio_write(PORT_IN_PS_PIN, HIGH);
}

inline void port_set_out_clk_1(void) 
{
	bcm2835_gpio_write(PORT_OUT_CLK_PIN, LOW);
}

inline void port_set_out_clk_0(void) 
{
	bcm2835_gpio_write(PORT_OUT_CLK_PIN, HIGH);
}

void port_set_out_clk(const _Bool value) 
{
	if (value) 
		bcm2835_gpio_write(PORT_OUT_CLK_PIN, LOW);
	else 
		bcm2835_gpio_write(PORT_OUT_CLK_PIN, HIGH);
}

void port_set_out_out(const _Bool level) 
{
	if (level) 
		bcm2835_gpio_write(PORT_OUT_OUT_PIN, LOW);
	else 
		bcm2835_gpio_write(PORT_OUT_OUT_PIN, HIGH);
}

void port_set_out_strobe(const _Bool level) 
{	
	if (level) 
		bcm2835_gpio_write(PORT_OUT_STROBE_PIN, LOW);
	else 
		bcm2835_gpio_write(PORT_OUT_STROBE_PIN, HIGH);
}

/**
 * watchdog puls.
 *
 * the puls must be set true and false in a regular mannar to keep the watchdog
 * relais energised
 * @param level		input signal of the watchdog monoflop
 */
void port_set_watchdog(const _Bool level) 
{
	/* note that the Watchdog is not inverse  */
	
	if (level) 
		bcm2835_gpio_write(PORT_WATCHDOG_PIN, HIGH);
	else 
		bcm2835_gpio_write(PORT_WATCHDOG_PIN, LOW);
}

_Bool port_get_in_in(void) 
{
	return(LOW == bcm2835_gpio_lev(PORT_IN_IN_PIN));
}

_Bool port_get_key(void) 
{
	/* note that switch is inverse, so pushed is GND  */
	return(LOW == bcm2835_gpio_lev(PORT_KEY_PIN));
}


#ifndef HW_H__IS_KERNEL__
#undef __KERNEL__
#endif
#endif /* WITHOUT_UNC20_HW__ */

/**
 * function to read the debounced state of the key.
 *
 * returns true if the key was pushed during two consecutive calls, so if
 * called in the normal loop after 200ms.
 *
 * This signal can be used to quit the program in a regular way.
 * @return	true if pushed
 */
_Bool filteredKey(void) 
{
	/* key signal is only set to true if twice the same signal	*/
	static int filterState = 0;
	
	switch(filterState) 
	{
	case 0: 
		if(port_get_key()) 
		{
			filterState = 1;
		}
		return false;
	case 1: 
		if(port_get_key()) 
		{
			filterState = 2;
			return(true);
		}
		else 
		{
			filterState = 0;
			return(false);
		}
	case 2: 
		if(!port_get_key()) 
		{
			filterState = 3;
		}
		return(true);
	case 3: 
		if(port_get_key()) 
		{
			filterState = 2;
			return true;
		}
		else 
		{
			filterState = 0;
			return false;
		}
	default: filterState = 0;
		return false;
	}
}

void tickTimeStart() 
{
	offset = bcm2835_st_read();
}

void tickTimeWait(const long long int time)
{
	/* now calculate the time to be waited for	*/
	compare = time + offset;
	sched_yield();
	if(compare < offset) 
	{
		/* there was an overflow of the time compare	*/
		while(bcm2835_st_read() > offset) ;
	}
	while((offset = bcm2835_st_read()) < compare) sched_yield();
}

long long int tickTimeUsed(const long long int time)
{
	return(time + offset - bcm2835_st_read());
}

/** 
 * function to wait for a short time
 * really short..
 * ist will wait 0.5 us.
 */
void waitALittle(void)
{
	volatile int i, dummy;
	for(i = 0; i < 20; i++)
	{
		dummy++;
	}
	
//	nanosleep((struct timespec[]){{0, 500}}, NULL);
}

