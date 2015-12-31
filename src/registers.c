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
 * Registers input, output, variables
 * \author
 * Kurt Zerzawy
 */

/*
 * $Author: Kurt $
 * $Date: 2015-08-05 08:06:23 +0200 (Mi, 05 Aug 2015) $
 * $Revision: 2537 $
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "log.h"
#include "hw.h"
#include "inshift.h"
#include "outshift.h"
#include "vars.h"
#include "parser/parser.h"	/* for parserAssert and parserExit	*/
#include "registers.h"



/* make a list of register plug */

/*@null@*/ /*@only@*/ static struct sRegister * m_pRegister = NULL;	/**< pointer to the register data structure */
static unsigned short  m_outshiftRegLen = 0;						/**< number of outshift register words */
static unsigned short  m_inshiftRegLen = 0;							/**< number of inshift register words */
static unsigned short  m_varRegLen = 0;								/**< number of variable words */
static unsigned short  m_registerLen = 0;				

const char cNONE [] = "NONE\0";


/**
 * constructor function 
 *
 * generates the data structure for inshift, outshift and variables
 *
 * Call this function as soon as you know the number of variables
 * and know the hardware. 
 *
 * This constructor calls also the inshift and outshift constructors
 *  @param outshiftLen   number of 16 bit output words
 *  @param inshiftLen    number of 16 bit input words 
 *  @param tempLen       number of 16 bit variable words 
 *  @exception           exit is called if no RAM available 
 */
/*@maynotreturn@*/
void registers(const unsigned short outshiftLen,
				 const unsigned short inshiftLen,
				 const unsigned short varsLen) 
{
	LOG_INF("started");
	m_outshiftRegLen = outshiftLen;
	m_inshiftRegLen  = inshiftLen;
	m_varRegLen     = varsLen;
	m_registerLen    = m_outshiftRegLen + m_inshiftRegLen + m_varRegLen;
	
	assert(NULL == m_pRegister);
	m_pRegister = malloc(m_registerLen * sizeof(struct sRegister));
	if(NULL == m_pRegister) 
	{
		/* error allocating */
		LOG_ERR("cannot allocate RAM");
		exit(EXIT_FAILURE);
	}
	memset(m_pRegister, 0, m_registerLen * sizeof(unsigned short));

	/* initialise the downlaying parts */
	outshift((struct sRegister *) m_pRegister, m_outshiftRegLen);
	inshift((struct sRegister *)(m_pRegister + m_outshiftRegLen), 
		m_inshiftRegLen);
	vars((struct sRegister *)(m_pRegister + m_outshiftRegLen + m_inshiftRegLen), 
			 m_varRegLen);
	
	/* successfully allocated RAM for the input chain structure */
	LOG_INF("ended");
}

/**
 * destruktor
 *
 * destructs the data structur allocated before with the constructor
 *  after this, you must not access the registers any more.
 *
 * since the constructor allocated RAM, you must call the destructor before
 * leaving the program, otherwise memory leaks will occur.
 *
 * The destructor is calling the destructors of inshift and outshift.
 */
void registersDestroy(void) 
{
	LOG_INF("started");
	inshiftDestroy();
	outshiftDestroy();
	free(m_pRegister);
	m_pRegister = NULL;
	m_outshiftRegLen = 0;
	m_inshiftRegLen = 0;
	m_varRegLen = 0;
	m_registerLen = 0;
	LOG_INF("ended");
}

/**
 * initialised the registers
 *
 * calls the init funtions of inshift and outshift.
 */
void registersInit(void) 
{
	LOG_INF("started");
	inshiftInit();
	outshiftInit();
	LOG_INF("ended");
}

/**
 * enables sending of the data to the outshift registers
 * @param enable	true enables sending
 */
void registerEnableSend(const _Bool enable) 
{
	LOG_INF("started");
	outshiftEnable(enable);
	LOG_INF("ended");
}

/**
 * enables receiving of datas with the inshift registers
 * @param enable	true enables receiption
 */
void registerEnableReceive(const _Bool enable) 
{
	LOG_INF("started");
	inshiftEnable(enable);
	LOG_INF("ended");
}

/**
 * function to read a bit
 *  @param BitPos	position of the bit
 *  @return		true if the bit is set
 *  @exception		assert on non existing bit
 */
_Bool registerBitGet(const unsigned short bitPos) 
{
	unsigned short index;
	unsigned short mask = 0x0001;
	struct sRegister * ptr = m_pRegister;

	assert(NULL != ptr);
	
	if(NONEVALUE == bitPos) 
	{
		return false;
	}
	
	index = (unsigned short) (bitPos >> 4);
	assert(index < m_registerLen);
	ptr += index;

	if(0 != (bitPos & 0x08)) mask <<= 8;
	if(0 != (bitPos & 0x04)) mask <<= 4;
	if(0 != (bitPos & 0x02)) mask <<= 2;
	if(0 != (bitPos & 0x01)) mask <<= 1;
	
	return(0 != (ptr->value & mask));
}

/**
 * function to set a bit
 *  @param BitPos	position of the bit
 *  @exception		assert on non existing bit
 */
void registerBitSet(const unsigned short bitPos) 
{
	unsigned short index;
	unsigned short mask = 0x0001;
	struct sRegister * ptr = m_pRegister;

	assert(NULL != ptr);
	
	if (NONEVALUE == bitPos) 
	{
		/* pseudo mask	*/
		return;
	}

	index = (unsigned short) (bitPos >> 4);
	assert(index < m_registerLen);
	ptr += index;

	if(0 != (bitPos & 0x08)) mask <<= 8;
	if(0 != (bitPos & 0x04)) mask <<= 4;
	if(0 != (bitPos & 0x02)) mask <<= 2;
	if(0 != (bitPos & 0x01)) mask <<= 1;

	if(0 == (ptr->value & mask)) 
	{
		/* this bit is not set yet */
		if(index < m_outshiftRegLen) 
		{
			/* it belongs to outshift */
			outshiftToDo();
		}
		ptr->value |= mask;
	}
}

/**
 * function to clear a bit
 *  @param BitPos	position of the bit
 *  @exception		assert on non existing bit
 */
void registerBitClr(const unsigned short bitPos) 
{
	unsigned short	index;
	unsigned long	mask = 0x00000001;
	struct sRegister * ptr = m_pRegister;
	
	assert(NULL != ptr);
	
	if (NONEVALUE == bitPos) 
	{
		/* pseudo mask	*/
		return;
	}

	index = (unsigned short) (bitPos >> 4);
	assert(index < m_registerLen);
	ptr += index;

	if(0 != (bitPos & 0x08)) mask <<= 8;
	if(0 != (bitPos & 0x04)) mask <<= 4;
	if(0 != (bitPos & 0x02)) mask <<= 2;
	if(0 != (bitPos & 0x01)) mask <<= 1;

	mask = ~mask;  
	if(ptr->value != (ptr->value | mask)) 
	{
		/* this bit is still set */
		if(index < m_outshiftRegLen) 
		{
			/* it belongs to outshift */
			outshiftToDo();
		}
		ptr->value &= mask;
	}
}

/**
 * function to find out the number of a bit given by an name
 * @param name		pointer to the name, for which is searched
 * @return			number of the bit (handle)
 * @exception		if the name is not known, the function calls 
 * 					parserExit with an error message 
 */
unsigned short registerFindHandle(const char * const name) {
	unsigned short i;
	unsigned short j;
	char temp[128];
	/* special case: NONE */
	if(0 == (strcmp(cNONE, name))) {
		return NONEVALUE;
	}
	for(i = 0; i < (m_inshiftRegLen + m_outshiftRegLen + m_varRegLen); i++) {
		for(j = 0; j < 16; j++) {
			if(0 == (strcmp((m_pRegister + i)->name[j], name))) {
			   	/* now found */
			   	return((unsigned short) (i * 0x10 + j));
			}
		}
	}
	sprintf(temp, "%s nicht gefunden", name);
	parserExit(temp);
	return 0;
}

/**
 * function to set the number of the plug of a register word
 * @param handle	bit number
 * @exception		if the name is not known, the function calls 
 * 					parserExit with an error message 
 *                        macht einen Exit.
 */
void registerSetPlugName(const unsigned short handle, const char * const name) 
{
	unsigned short i = handle / 0x10;

	/* special case: NONE */
	if(NONEVALUE == handle) 
	{
		return;
	}
	parserAssert(i < registerGetLen(), "unbekannter Steckerhandle");
	strncpy((m_pRegister + i)->plugName, name, NAMELEN - 1);
	(m_pRegister + i)->plugName[NAMELEN] = '\0';
}

/**
 * function to get the name of plug of a register
 * @param handle	bit number
 * @param name		pointer to the string filled out with the name
 * 					of the turnout. Must be at least NAMELEN long
 * @return			true, if successful
 * @exception		assert on NULL pointer
 */
_Bool registerGetPlugName(const unsigned short handle, char * const name) 
{
	unsigned short i = handle / 0x10;

	/* special case: NONE */
	if(NONEVALUE == handle) 
	{
		memmove(name, cNONE, strlen(cNONE) + 1);
		return true;
	}
	parserAssert(i < registerGetLen(), "unbekannter Steckername");
	name[NAMELEN - 1] = '\0';
	memmove(name, (m_pRegister + i)->plugName, NAMELEN - 1);
	return true;
}

/**
 * function to set the name of plug of a register
 * @param handle	bit number
 * @param name		pointer to the name string
 * @return			true, if successful
 * @exception		assert on NULL pointer
 */
void registerSetName(const unsigned short handle, const char * const name) 
{
	unsigned short i = handle / 0x10;
	unsigned short j = handle % 0x10;

	/* special case: NONE */
	if(NONEVALUE == handle) 
	{
		return;
	}
	parserAssert(i < registerGetLen(), "unbekannter Registername");
	strncpy((m_pRegister + i)->name[j], name, NAMELEN - 1);
	(m_pRegister + i)->plugName[NAMELEN] = '\0';
}

/**
 * function to get the name of a register
 * @param handle	bit number
 * @param name		pointer to the string filled out with the name
 * 					of the turnout. Must be at least NAMELEN long
 * @return			true, if successful
 * @exception		assert on NULL pointer
 */
_Bool registerGetName(const unsigned short handle, char * const name) 
{
	unsigned short i = handle / 0x10;
	unsigned short j = handle % 0x10;

	/* special case: NONE */
	if(NONEVALUE == handle) 
	{
		memmove(name, cNONE, strlen(cNONE) + 1);
		return true;
	}
	parserAssert(i < registerGetLen(), "unbekannter Registerhandle");
	name[NAMELEN - 1] = '\0';
	memmove(name, (m_pRegister + i)->name[j], NAMELEN - 1);
	return true;
}

unsigned short registerGetFirstOutshiftHandle(void) 
{
	return 0;
}

/**
 * function to return the first bit handle of the inshift register
 * @return		bit number of the first inshift register first bit
 */
unsigned short registerGetFirstInshiftHandle(void)
{
	return 0x10 * m_outshiftRegLen;
}

/**
 * function to return the first bit handle of the vars register
 * @return		bit number of the first vars register first bit
 */
unsigned short registerGetFirstVarHandle(void)
{
	return 0x10 * (m_outshiftRegLen + m_inshiftRegLen);
}

unsigned short registerGetOutshiftLen(void)
{
	return m_outshiftRegLen;
}

unsigned short registerGetInshiftLen(void)
{
	return m_inshiftRegLen;
}

unsigned short registerGetVarLen(void)
{
	return m_varRegLen;
}

unsigned short registerGetLen(void)
{
	return m_registerLen;
}

/**
 * function to set the filter constant 
 *
 * sets the positive filter value for the inshift register
 * @param handle	bit number
 * @param value		value of the filter constant. The bigger the number, the
 * 					faster the state changes to 1. 
 * 					a value of 80 is changing on the first read, a value of 1
 * 					needs e.g 80 cycles of inshift for the change to 1
 * @exception		if the handle is not known, the function calls 
 * 					parserExit with an error message 
 */
void registerSetFilterPlus(const unsigned short handle, const unsigned char value)
{
	unsigned short i = handle / 0x10;
	unsigned short j = handle % 0x10;

	/* special case: NONE */
	if(NONEVALUE == handle) 
	{
		return;
	}
	parserAssert(i < registerGetLen(), "unbekannter Handle bei Filter");
	(m_pRegister + i)->filterPlus[j] = value;
}

/**
 * function to get the filter constant 
 *
 * gets the positive filter value for the inshift register
 * @param handle	bit number
 * @return			value of the filter constant. For details see registerSetFilterPlus
 * @exception		if the handle is not known, the function calls 
 * 					parserExit with an error message 
 */
unsigned char registerGetFilterPlus(const unsigned short handle)
{
	unsigned short i = handle / 0x10;
	unsigned short j = handle % 0x10;

	/* special case: NONE */
	if(NONEVALUE == handle) 
	{
		return 0;
	}
	parserAssert(i < registerGetLen(), "unbekannter Handle bei Filter");
	return (m_pRegister + i)->filterPlus[j];
}

/**
 * function to set the filter constant 
 *
 * sets the positive filter value for the inshift register
 * @param handle	bit number
 * @param value		value of the filter constant. The bigger the number, the
 * 					faster the state changes to 0. For details see registerSetFilterPlus
 * @exception		if the handle is not known, the function calls 
 * 					parserExit with an error message 
 */
void registerSetFilterMinus(const unsigned short handle, const unsigned char value)
{
	unsigned short i = handle / 0x10;
	unsigned short j = handle % 0x10;

	/* special case: NONE */
	if(NONEVALUE == handle) 
	{
		return;
	}
	parserAssert(i < registerGetLen(), "unbekannter Handle bei Filter");
	(m_pRegister + i)->filterMinus[j] = value;
}

/**
 * function to get the filter constant 
 *
 * gets the positive filter value for the inshift register
 * @param handle	bit number
 * @return			value of the filter constant. For details see registerSetFilterPlus
 * @exception		if the handle is not known, the function calls 
 * 					parserExit with an error message 
 */
unsigned char registerGetFilterMinus(const unsigned short handle)
{
	unsigned short i = handle / 0x10;
	unsigned short j = handle % 0x10;

	/* special case: NONE */
	if(NONEVALUE == handle) 
	{
		return 0;
	}
	parserAssert(i < registerGetLen(), "unbekannter Handle bei Filter");
	return (m_pRegister + i)->filterMinus[j];
}

/**
 * function to produce an ASCII from register
 *
 * generates a human readable ASCII hex code with spaces containing the content of the register
 * @param pString	pointer to the string filled with the ASCII string. It must be able
 * 					to contain the length of the expected string.
 * @param start		bit number of the first bit of the first register to be read
 * @param len		number of 16 Bit values to be read
 */
void registerGetString(char * const pString, 
					 const unsigned short start,
					 const unsigned short len) 
{
	unsigned short     i, j;
	struct sRegister * ptr = m_pRegister + start;
	unsigned short     pos = 0;
	char               ch;
	unsigned short     val;

	assert(m_registerLen > start + len);

	LOG_STW("registerGetString mit %d %d", start, len);

	if(m_registerLen <= start + len) 
	{
		LOG_ERR("start + len too big");
		return;
	}
	
	for(i = 0; i < len; i++) 
	{
		/* for each print copy 4 Hex into string	*/
		val = ptr->value;
		for(j = 0; j < 4; j++) 
		{
			/* convert highest nibble into a character from 0..9 or a..f */
			ch = (char) ((val >> 12) & 0x0f) + '0';
			if(ch > '9') 
			{
				ch += 'a' - '0' - 10;
			}
			pString[pos++] = ch;
			val <<= 4; /* shift one nibble left */
		}
		ptr++;
		pString[pos++] = ' ';
	}
	pString[pos++] = '\0';
}

/**
 * function to fill the register with the content of an readable ASCII string
 *
 * fills a human readable ASCII hex code with spaces containing the content into the register
 * @param pString	pointer to the string filled with the ASCII string.
 * @param start		bit number of the first bit of the first register to be written
 * @param len		number of 16 Bit values to be written
 * @return			NULL on success. If the string was not understood, the
 * 					pointer is set to the first unexpected character.
 */
/*@null@*/ /*@observer@*/
char * registerSetString(/*@temp@*/ /*@in@*/ char * const pString, 
			 const unsigned short start,
			 const unsigned short len) 
{
	unsigned short     i, j;
	struct sRegister * ptr = m_pRegister + start;
	char               ch;
	char             * pch = pString;
	unsigned short     val;
	char               errtxt[30];

	assert(m_registerLen > start + len);
	assert(NULL != pch);

	if(m_registerLen <= start + len) 
	{
		LOG_ERR("start + len too big");
		return pString; 
	}
	if(start < m_outshiftRegLen)
	{
	   	outshiftToDo();
	}
	if(start + len > m_outshiftRegLen)
	{
		inshiftChanged();
	}

	for(i = 0; i < len; i++) 
	{
		/* for each register 4 hex digits to read		*/
		if (' ' != * pch) 
		{
			/* skip space	*/
			sprintf(errtxt, "Space expected, \"%c\" received", * pch); 
			LOG_ERR(errtxt);
			return pch;
		}
		val = 0;
		for(j = 0; j < 4; j++) 
		{
			pch++;
			val *= 16;	/* shift number one nibble to left	*/
			ch = * pch;
			/* decode hex number	*/
			if (('9' >= ch) && ('0' <= ch)) 
			{
				val += (unsigned short) (ch - '0');
			}
			else if (('F' >= ch) && ( 'A' <= ch)) 
			{
				val += (unsigned short) (ch - 'A') + 10;
			}
			else 
			{
				sprintf(errtxt, "unknown symbol \"%c\"", ch);
				LOG_ERR(errtxt);

				return pch;
			}
		}
		ptr->value = val;
		ptr++;
		pch++;
	}
	return NULL;
}

/**
 * returns the pointer to the first register
 * @return		pointer to the first register
 */
struct sRegister * registerGetPtr(void) 
{
	return m_pRegister;
}


