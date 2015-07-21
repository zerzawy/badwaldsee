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
 * Input shift register
 * \author
 * Kurt Zerzawy
 */

/** 
 * \file 
 * functions for the 
 */

/*
 * $Author: kurt $
 * $Date: 2015-05-18 22:31:02 +0200 (Mon, 18. Mai 2015) $
 * $Revision: 2464 $
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "log.h"
#include "hw.h"               /* needs IO Pins for Shifting functions */
#include "inshift.h"

/*@null@*/ static struct sRegister * m_pInshiftReg = NULL;	/**< pointer to first register */
static unsigned short                m_inshiftRegLen = 0;	/**< number of inshift registers */
static _Bool                         m_inshiftEnabled;
static _Bool						 m_changed;



/**
 * This is the documentation of the part controlling how the data are
 *  read from the input prints
 *
 * Diagram of the input shift registers
 * \verbatim
           D15           D0
             |            |
             |     ..     |
             v            v
          ,--+------------+--,
          |                  |<--- Pn/S
          |                  |<--- InClock
          '-+--------------+-'
            |              |
            |              |
 InIn  <----'              '----< InOut
   \endverbatim

 * Timing diagram of the data inshift
 * Pn/S must be high for shifting data
 * the positive flange of InClock is shifting the data
 * \verbatim
          ---------,-------,----------------
 Pn/S     ---------'       '----------------
 
                     ,-----------,
 InClock  -----------^           '------

          ------------------------,-----data----,------
 InIn     ------------------------'----valid----'------
\endverbatim

 * Timing diagram for latching the data into the shift register
 * Pn/S must be low, while a positive flange on InClock 
 *  stored the data 
 * \verbatim
 *          ---------,       ,----------------
 * Pn/S     ---------'-------'----------------
 *
 *                     ,-----------,
 * InClock  -----------^           '------
\endverbatim
 */

/**
 * 'Constructor' for the input shift registers
 * This function generates the inshift registers in the data structure.
 * You should call this function as soon as you know the number of inshift registers
 * and where the place for the datas is allocated
 * On the Bad Waldsee project this function is called by the register constructor.
 *  @param pRegister	pointer to the data structure of the register, where
 *  					the inshift registers are storing their data
 *  @param regisierLen	Number of inshift registers, e.g. the nunber of EP16.
 */
void inshift(struct sRegister * const pRegister,
						 const unsigned short registerLen) 
{
	LOG_INF("started");
	assert(NULL != pRegister);
	if(NULL == pRegister) 
	{
		LOG_ERR("no pointer given");
		exit(EXIT_FAILURE);
	}

	m_pInshiftReg = pRegister;
	m_inshiftRegLen = registerLen;
	
	/* Setze alle Werte auf 0 */
	memset(m_pInshiftReg, 0, registerLen * sizeof(unsigned short));
	LOG_INF("ended");
}

/**
 * Destructor.
 * This function should be called before ending the program.
 */
void inshiftDestroy() 
{
	LOG_INF("started");
	;
	LOG_INF("ended");
}


/**
 * function to initialise the inshift registers.
 * Here, all inshift registers are prepared for their use
 * and the input filter is set to a correct state. 
 * To set the input filters correct, the input is reading the 
 * values three times and is making a majority decision on the
 * value
 */
void inshiftInit(void) 
{
	struct sRegister * ptr = m_pInshiftReg;
	unsigned short   i, j, k;

	LOG_INF("started");
	assert(NULL != ptr);
	if(!ptr) 
	{
		LOG_ERR("not existing pointer");
		return;
	}

	/* Schritt 1: Initialisiere alle Filterwerte */
	ptr = m_pInshiftReg;
	for(j = 0; j < m_inshiftRegLen; j++) 
	{
	/* wiederhole fuer jeden vorhandenen EP16-Print  */

		for(k = 0; k < 16; k++) 
		{
			/* wiederhole fuer jedes Bit des EP16 */

			ptr->filterValue[k] = 0;
		}
		ptr++;
	}
	
	
	/* Schritt 2: Lies alle Werte 3 mal */
	for(i = 0; i < 3; i++) 
	{
		ptr = m_pInshiftReg;

		/* Initialisiere Einlesen */
		
		ports_init();
		/* Bereite Einschieben vor   */
		/* Pn/S high, clock high and low          */
		port_set_in_clk_1();
		port_set_in_ps(false);
		waitALittle();
		port_set_in_clk_0();
		waitALittle();
		port_set_in_ps(true);
		
		/* Nun ist das Schieberegister bereit zum Einlesen  */
		
		waitALittle();
		port_set_in_clk_1();
		
		for(j = 0; j < m_inshiftRegLen; j++) 
		{
			/* wiederhole fuer jeden vorhandenen EP16-Print  */

			for(k = 0; k < 16; k++) 
			{
				/* wiederhole fuer jedes Bit des EP16 */

				waitALittle();       
				port_set_in_clk_0();
				waitALittle();
				if(port_get_in_in()) 
				{
					/* wenn der Eingang gesetzt ist  */
					ptr->filterValue[k] += (unsigned char) 1;
				}
				port_set_in_clk_1();
			} /* Ende fuer alle Bits */

			ptr++;

		} /* Ende fuer alle EP16 */
	} /* Ende fuer 3 mal lesen */
	
	/* Nun rechne fuer jedes Bit
	 *  2 oder 3 mal 1 --> filter >= 2 --> set
	 *  0 oder 1 mal 1 --> filter < 2  --> clear
	 */

	ptr = m_pInshiftReg; 
	for(j = 0; j < m_inshiftRegLen; j++) 
	{
		/* wiederhole fuer jeden vorhandenen EP16-Print  */
		
		ptr->value = 0;

		for(k = 0; k < 16; k++) 
		{
			/* wiederhole fuer jedes Bit des EP16 */
			ptr->value <<= 1;

			if(((unsigned char) 2) <= ptr->filterValue[k]) 
			{
				ptr->filterValue[k] = 0x0080;
				ptr->value |= 0x0001;
			}
			else 
			{
				ptr->filterValue[k] = 0;
			}
		}
		ptr++;
	}
	LOG_INF("ended");
}

/**
 * Enable function.
 * Switches the hardware on or off. So it is possible to force the input values for
 * tests.
 * @param enable	true datas are truely coming from hardware
 *                	false hardware is not read
 */
void inshiftEnable(const _Bool enable) 
{
	LOG_INF("started");
	m_inshiftEnabled = enable;
	LOG_INF("ended");
}

/**
 * function to tell the inshift function that there was a change
 */
void inshiftChanged(void) 
{
	m_changed = true;
}

/**
 * function to tell the inshift function that the change was processed
 */
void inshiftAck(void)
{
	m_changed = false;
}

/**
 * function to read all datas from the hardware into the internal storage
 * Here is the main point of this file. Here 
 * * the inshift registers are read with a toggle pulse 
 * * each bit is shifted in
 * This is done 16-Bit word by word. 
 * To eliminat noise, the signals are filtered. To achieve this, each Bit has a Byte
 * remembering the state as a filter variable. If the bit read is zero, then a value
 * given in the parameters is subtracted, if it is one, another value is added.
 * 
 * Normally this is the first function in the loop to make you see what is going on.
 * 
 * If a value has changed since last time, the function returns true.
 * @return    true if the value read changed
 */
_Bool inshiftRegGet(void) 
{
	unsigned short     i, j;
	unsigned short     mask;
	unsigned short     old;
	struct sRegister * ptr = m_pInshiftReg;
	
	if(! m_inshiftEnabled) 
	{
		return m_changed;
	}

	assert(NULL != ptr);
	if(NULL == ptr) 
	{
		LOG_ERR("not existing pointer");
		return false;
	}

	/* Normaler Fall mit Einlesen, nimm an, dass es keine 
	 *  Aenderung gegeben hat.
	 */
	m_changed = false;
	ptr = m_pInshiftReg;

	/* Bereite Einschieben vor   */
	/* Pn/S high, clock high and low          */
	port_set_in_clk_1();
	port_set_in_ps(false);
	waitALittle();
	port_set_in_clk_0();
	waitALittle();
	port_set_in_ps(true);
	
	/* Nun ist das Schieberegister bereit zum Einlesen  */
	
	waitALittle();
	port_set_in_clk_1();
	
	for(i = 0; i < m_inshiftRegLen; i++) 
	{
		/* wiederhole fuer jeden vorhandenen EP16-Print  */

		old = ptr->value;
		mask = 0x8000;
		for(j = 0; j < 16; j++) {
			/* wiederhole fuer jedes Bit des EP16 */

			waitALittle();
			port_set_in_clk_0();
			waitALittle();
			if(port_get_in_in ()) 
			{
				/* Wenn der Eingang gesetzt ist  */
				
				ptr->filterValue[j] += ptr->filterPlus[j];
				if(0x0080 <= ptr->filterValue[j]) 
				{
					/* Wenn der Filterwert zu gross ist, dann setze das Bit 1
					 * und limitiere den Filterwert
					 */
					ptr->value |= mask;
					ptr->filterValue[j] = 0x0080;
				}
			}
			else 
			{
				/* Wenn der Eingang geloescht ist  */
				if(ptr->filterValue[j] <= ptr->filterMinus[j]) 
				{
					/* Wenn der Filter nach der Subtraktion ganz leer waere, dann loesche
					 * das Bit und setze den Filterwert auf 0.
					 */
					ptr->value &= ~mask;
					ptr->filterValue[j] = 0;
				}
				else 
				{
					/* Eine Subtraktion wird nur ausgefuehrt, wenn der Wert nicht unter
					 * Null faellt. 
					 */
					ptr->filterValue[j] -= ptr->filterMinus[j];
				}
			}
			waitALittle();
			port_set_in_clk_1 ();
			mask >>= 1;
		} /* Ende fuer alle Bits */

		/* kontrolliere, ob der Wert geaendert hat */
		if(old != ptr->value) 
		{
			m_changed = true;
		}
		ptr++;
		
	} /* Ende fuer alle EP16 */
	return m_changed;
}

/**
 * function to get the length of the inshift register
 * @return:			length in words a 16 Bits
 */
unsigned short inshiftGetRegLen(void) 
{
	return m_inshiftRegLen;
}

/**
 * function to get the handle of an input register
 * @param word		word address
 * @param bit		bit
 * @return			handle
 * @exception		assert if the word or bit are not valid
 */
unsigned short inshiftGetHandle(const unsigned short word, const unsigned short bit)
{
	assert(inshiftGetRegLen() > word);
	assert(16 > bit);

	return(inshiftGetRegLen() + 0x10 * word + bit);
}

