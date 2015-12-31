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
 * Output shift registers
 * \author
 * Kurt Zerzawy
 */

/* $Author: Kurt $
 * $Date: 2015-08-05 08:06:23 +0200 (Mi, 05 Aug 2015) $
 * $Revision: 2537 $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "log.h"
#include "hw.h"               /* needs IO Pins for Shifting functions */
#include "ap216.h"
#include "outshift.h"

/*@null@*/ static struct sRegister * m_pOutshiftReg = NULL;	/**< pointer to the first output register */
static unsigned short                m_outshiftRegLen = 0;	/**< number of outshift registers */
static _Bool                         m_outshiftEnabled;		/**< true if outshift is enabled */
/*@null@*/ static unsigned short   * pOutshiftFeedbackRegisters = NULL;
/*@null@*/ static unsigned short   * pOutshiftFeedbackOutput = NULL;
static unsigned short                outshiftFeedbackRegisterLen = 0;
static _Bool                         m_todo = true;			/**< true if there is work for outshift */


/** 
 * \author Kurt Zerzawy
 *
 * diagram of the outshift register
 * \verbatim
 OutIn <----,              ,----< OutOut
            |              |
            |              |
          ,-+--------------+-,
          |                  |<--- OutStrobe
          |                  |<--- OutClock
          '--+------------+--'
             |     ..     |
             |            |
             v            v
            Q31           Q0
\endverbatim

 * timing diagram for the shiftout of data. 
 *
 * OutStrobe must be low during shiftout. 
 *
 * positive flange is shifting the data
 * \verbatim
 
        -----,-----data----,------
 OutOut -----'----valid----'------
 
                     ,-----------,
 OutClock -----------^           '------
\endverbatim

 *
 * Timing fuer das Ausgeben der Daten an die Treiber
 * \verbatim
                  ,-----------,
 OutStrobe -------^           '------
\endverbatim
 *
 * constructor 
 *
 * this function is generating the data structure for the outshift registers
 *
 * from this moment, you can use the datas. You should call this function as soon
 * as you know how many outshift prints exist on your system. 
 *
 * on Bad Waldsee the register constructor is calling this function 
 * @paranm pRegister	pointer to the datastructur of the register, from 
 * 						where the outshift registers are fetching the data
 * @param registerLen	number of outshift registers a 16 Bit. Note: since the
 * 						AP216 has two 16 bit words, it is necessary to give the 
 * 						double number of AP216 prints.
 * @exception			if not enough RAM available, the function calls exit
 */ 
/*@maynotreturn@*/
void outshift(/*@only@*/ struct sRegister * const pRegister,
				/*@in@*/ const unsigned short registerLen) 
{
	LOG_INF("started");
	assert(NULL != pRegister);
	if(!pRegister) 
	{
		LOG_ERR("no pointer given");
		exit(EXIT_FAILURE);
	}

	m_pOutshiftReg = pRegister;
	m_outshiftRegLen = registerLen;
	
	/* now set all values to 0 */
	outshiftInit();
	LOG_INF("ended");
}

/**
 * destructor
 *
 * destrois the data structure and releases the ressources. 
 *
 * After callling the destructor, you must not access the outshift registers any more
 */
void outshiftDestroy(void) 
{
	LOG_INF("started");
	m_pOutshiftReg = NULL;
	LOG_INF("ended");
}

/**
 * initialiser
 *
 * prepares the outhift registers and resets all registers and outshift registers
 */
void outshiftInit(void) 
{
	LOG_INF("started");
	assert(NULL != m_pOutshiftReg);
	ports_init();
	/* now set all values to 0 */
	memset(m_pOutshiftReg, 0, m_outshiftRegLen * sizeof(unsigned short));
/*VORSICHT, nur TEST */
/*	for(i = 0; i < m_outshiftRegLen; i++) 
	{
		ptr->value = 0x5555;
		ptr++;
	}
*/	outshiftEnable(true);
	outshiftRegSend();
	LOG_INF("ended");
}

/**
 * enables outshifting
 *
 * with this function, the outshift of data can be anabled or disabled, 
 * @param enable	true allows outshifting, 
 * 					false keeps the given data on outputs
 */
void outshiftEnable(_Bool enable) 
{
	LOG_INF("started");
	m_outshiftEnabled = enable;
	LOG_INF("ended");
}

/**
 * output function 
 *
 * here the internal datas on the registers are shifted out to the hardware. 
 *
 * to do this, 16 Bit wise the datas are shifted out until all datas are on the right place in 
 * the outshift registers. Then a Strobe pulse is writing the shifted data to the output registers
 * where they stay until the next function call.
 *
 * The information about the number of outshift registers and the location of the internal 
 * regisers was given during construction of the outshift.
 *
 * Normally this function is used in the main loop as last instruction.
 */
void outshiftRegSend(void) 
{  

	unsigned short    	i, j;
	unsigned short     	outputWord = 0;
	struct sRegister * 	ptr = NULL;
	assert(NULL != m_pOutshiftReg);
	
	if(! m_outshiftEnabled) 
	{
//		return;
	}

	if(! m_todo) 
	{
//		return;
	}
//	LOG_INF("started");
	/* initialize for start of shifting out    */
	/* Strobe low, clock low, data out high    */
	port_set_out_strobe(true);
	port_set_out_clk_0();
	port_set_out_out(true);
	
	ptr = m_pOutshiftReg;
	
	for(i = 0; i < m_outshiftRegLen; i++) 
	{
		/* repeat for each register available
		 * note: this is the double number of the prints
		 */
		outputWord = ptr->value;      /* copy value  */
		for (j = 0; j < 16; j++) 
		{
			/* repeat for all 16 Bits of a register
			 * if most significant bit is high, set output
			 */
			port_set_out_out((outputWord & 0x8000) != 0x8000);
			/* note: Datas are given over to UNC20 inversely	*/
//			fprintf(stderr, "%c",((outputWord & 0x8000)) ? '1' : '0');
			
			port_set_out_clk_1();  /* set same time the passiv flange of Clock*/
			
			outputWord <<= 1;  /* shift word one left  */
			waitALittle();
			/* set the active flange of Clock  */
			port_set_out_clk_0();
			waitALittle();
		}
		ptr++;
	}
	/* now the shifting is over, so make a strobe puls  */
	port_set_out_strobe(false);
	waitALittle();
	waitALittle();
	waitALittle();
	waitALittle();
	//nanosleep(& interval, &interval); TODO must have some delay!!
	port_set_out_strobe(true);
	
	m_todo = false;
//	LOG_INF("ended");
}

void outshiftToDo(void) 
{
	m_todo = true;
}

/* NOTE: later also dynamic tests on outputs */
void outshiftFeedback(const unsigned short nrRegisters) 
{
	assert (0 < nrRegisters);
	
	outshiftFeedbackRegisterLen = nrRegisters;
	pOutshiftFeedbackOutput = malloc(outshiftFeedbackRegisterLen * sizeof(unsigned short *));
	pOutshiftFeedbackRegisters = malloc(outshiftFeedbackRegisterLen * sizeof(unsigned short *));

	if(NULL == pOutshiftFeedbackOutput) 
	{
		LOG_ERR("not enough RAM");
		exit(EXIT_FAILURE);
	}
	if(NULL == pOutshiftFeedbackRegisters) 
	{
		LOG_ERR("not enough RAM");
		exit(EXIT_FAILURE);
	}

	/* NOTE: register values are not initialized */
}

unsigned short outshiftFeedbackRead(const unsigned short outputRegister) 
{
	assert(pOutshiftFeedbackOutput);
	assert(outshiftFeedbackRegisterLen < outputRegister);
	
	return ( * (pOutshiftFeedbackOutput + outputRegister));
}
	
void outshiftFeedbackDestroy(void) 
{
	free(pOutshiftFeedbackOutput);
	free(pOutshiftFeedbackRegisters);
	outshiftFeedbackRegisterLen = 0;
}

unsigned short outshiftGetRegLen(void) 
{
	return m_outshiftRegLen;
}

/**
 * function to get the handle of an output register
 * @param word		word address
 * @param bit		bit
 * @return			handle
 * @exception		assert if the word or bit are not valid
 */
unsigned short outshiftGetHandle(const unsigned short word, const unsigned short bit)
{
	assert(outshiftGetRegLen() > word);
	assert(16 > bit);

	return(0x10 * word + bit);
}

