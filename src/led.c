/* 
 * This file is part of Raspberry Pi steuert Modelleisenbahn
 * Copyright(C)  2011 - 2015 Kurt Zerzawy www.zerzawy.ch
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
 * single switchboard LEDs
 * \author
 * Kurt Zerzawy
 */

/*
 * $Author: kurt $
 * $Date: 2015-05-18 22:31:02 +0200 (Mon, 18. Mai 2015) $
 * $Revision: 2464 $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "log.h"
#include "registers.h"		/* registers for the io	*/
#include "outshift.h"
#include "led.h"
#include "parser/parser.h"	/* for parserExit	*/

// #define DEBUG__


/*@null@*/ static struct sLed *	m_pLed = NULL;			/**< pointer to the first led */
static unsigned short		m_nrLed = 0;			/**< number of allocated leds */
static _Bool			m_LedBlinking = false;	/**< true if at least one led blinking */
static _Bool    		m_LedBlinkIsOn = false;	/**< clock for blinking */
static _Bool			m_LedShow = true;		/**< true if turnout leds always on */

static void ledInit(void);

/*
 * constructor
 *
 * this function is generating the data structure for the leds
 *
 * from this moment, you can use the datas. You should call this function
 * as soon as you know the number of LEDs
 *
 * @param nrLed		number of LEDs to be allocated.
 * @exception		if not enough RAM available, the function calls exit
 */
/*@maynotreturn@*/
void led(/*@in@*/ const unsigned short nrLed) 
{
	LOG_INF("gestartert");
	assert(NULL == m_pLed);

	m_nrLed = nrLed;
	m_pLed = malloc(m_nrLed * sizeof(struct sLed));
	if(NULL == m_pLed) 
	{
		LOG_ERR("cannot allocate RAM");
		exit(EXIT_FAILURE);
	}

	ledInit();
	LOG_INF("ended");
}

/**
 * destructor
 *
 * destrois the data structure and releases the ressources. 
 *
 * After callling the destructor, you must not access the LEDs any more
 */
void ledDestroy(void) 
{
	LOG_INF("started");
	free(m_pLed);
	m_pLed = NULL;
	LOG_INF("ended");
}

static void ledInit(void) 
{
	/*@null@*/ struct sLed * ptr =	m_pLed;
	unsigned short i;

	LOG_INF("started");
	assert(NULL != m_pLed);

	for(i = 0; i < m_nrLed; i++)
	{
		ptr->BO_handle = NONEVALUE;
		ptr->mode = ledOff;
		ptr++;
	}

	LOG_INF("ended");
}

/** 
 * function to set the mode of an LED
 * @param ptr		pointer to LED
 * @param mode		mode, so behaviour of the LED
 * @exception		assert on NULL pointer
 */
void ledSet(struct sLed * ptr, const enum eLedMode mode)
{
	assert(NULL != ptr);

	unsigned short handle = ptr->BO_handle;

	assert(NULL != ptr);
	ptr->mode = mode;
	switch(mode) 
	{
		case ledOn:		registerBitSet(handle);
						break;
		case ledOff:	registerBitClr(handle);
						break;
		case ledAllOff:registerBitClr(handle);
						break;
		case ledBlinking:	if(m_LedBlinkIsOn)
							registerBitSet(handle);
						else
							registerBitClr(handle);
						m_LedBlinking = true;
						break;
		case ledBlankable:
						if(m_LedShow)
							registerBitSet(handle);
						else
							registerBitClr(handle);
						break;
		default:		assert(false);
	}
}

/** 
 * function to get the mode of an LED
 * @param ptr		pointer to LED
 * @return			mode, so behaviour of the LED
 * @exception		assert on NULL pointer
 */
enum eLedMode ledGetMode(struct sLed * ptr)
{
	assert(NULL != ptr);
	return ptr->mode;
}

/**
 * function to set the LEDs set to blinking mode
 * @param on		true switches the blinking LEDs on, false switches them off
 */ 
void ledSetBlinkClock(const _Bool on)
{
	struct sLed * ptr = m_pLed;
	unsigned short i;

	assert(NULL != ptr);
	
	m_LedBlinkIsOn = on;

	if(!m_LedBlinking) 
		return;

	m_LedBlinking = false;
	for(i = 0; i < m_nrLed; i++)
	{
		if(ledBlinking == ptr->mode)
		{
			m_LedBlinking = true;

			if(on) 
			{
				registerBitSet(ptr->BO_handle);
			}
			else
			{
				registerBitClr(ptr->BO_handle);
			}
		}
		ptr++;
	}
}

/**
 * function to set the LEDs set to blankable mode
 * @param on		true switches the blanked LEDs on, false switches them off
 */ 
void ledShow(const _Bool on)
{
	struct sLed * ptr = m_pLed;
	unsigned short i;

	assert(NULL != ptr);
	m_LedShow = on;
	for(i = 0; i < m_nrLed; i++)
	{
		if(ledBlankable == ptr->mode)
		{
			if(on) 
			{
				registerBitSet(ptr->BO_handle);
			}
			else
			{
				registerBitClr(ptr->BO_handle);
			}
		}
		ptr++;
	}
}

/**
 * function to be called ones a cycle
 */
void ledProcess(void)
{
	;
}

/**
 * function to get the name of a led
 *
 * returns the name of an LED addressed by a pointer.
 *
 * if it is a single LED, the name is returned without the trailing LED_
 *
 * if it is a dual LED, the trailing DLED_ is cut off and the name is returned
 * with a trailing D_
 *
 * @param ptr		pointer to the LED
 * @param name		pointer to the string filled up with the name. There must 
 * 					at least NAMELEN space
 * @return			true if it is a LED or dual LED
 * @exception		assert on NULL pointer
 */
_Bool ledGetName(const struct sLed * const ptr, char * const name)
{
	char	pStr[NAMELEN];

	assert(NULL != ptr);
	name[NAMELEN - 1] = '\0';
	registerGetName(ptr->BO_handle, pStr); 
	if(0 == strncmp("LED_", pStr, 4)) 
	{
		memmove(name, pStr+4, NAMELEN-5);
		return true;
	}
	if(0 == strncmp("DLED_", pStr, 5))
	{
		strcpy(name, "D_");
		memmove(name+2, pStr+5, NAMELEN-6);
		return true;
	}
	return false;
}


/**
 * function to find the pointer by a name
 *
 * if the name starts with a D_ then it is searched for a DLED_name
 * otherwise for a LED_name
 *
 * if the name cannot be found, the program terminates with an error
 * message.
 * @param name		pointer to the name to be looked for.
 * @return		pointer to the found LED
 * @exception		assert if the name would get too long or if not found
 */
struct sLed * ledFind(const char * const name)
{
	struct sLed *		ptr = m_pLed;
	char 			str[NAMELEN];
	unsigned short		handle = 0;
	unsigned short		i = 0;
	char			temp[128];

	if(0 == strncmp("D_", name, 2))
	{
		assert(NAMELEN-3 > strlen(name));
		strcpy(str, "DLED_");
		memmove(str+5, name+2, NAMELEN-5);
	}
	else
	{
		assert(NAMELEN-4 > strlen(name));
		strcpy(str, "LED_");
		memmove(str+4, name, NAMELEN-4);
	}

	handle = registerFindHandle(str);
	for(i = 0; i < m_nrLed; i++)
	{
		if(handle == ptr->BO_handle)
		{
			return ptr;
		}
		ptr++;
	}
	sprintf(temp, "LED %s nicht gefunden", name);
	parserExit(temp);
	return NULL;
}

/**
 * funtion to control for a LED or duo LED
 *
 * the function checks, if a pointer could belong to a LED or duo LED.
 * For the rules see ledNameIsLed
 * @param ptr		pointer to the LED to be checked fot
 * @return		true if it could be a LED or duo LED
 * @exception		assert if the LEDs are not constructed yet
 */
_Bool ledIsLed(const struct sLed * const ptr)
{
	char			name[NAMELEN];

	assert(NULL != ptr);
	registerGetName(ptr->BO_handle, name);
	return(ledNameIsLed(name)); 
}

/**
 * Function to get the pointer by giving the number of the led
 * @param i		index, starting with 0
 * @return		Pointer to this block, NULL if not existing
 */ 
struct sLed * ledGet(const unsigned short i)
{
	if(i >= m_nrLed)
	{
		return NULL;
	}
	else
	{
		return m_pLed + i;
	}
}

/**
 * function to return the pointer to the first LED
 * @return			pointer to the first LED
 */
struct sLed * ledGetFirstPtr(void)
{
	return m_pLed;
}

/**
 * function to return the number of LEDs
 * @return		number of LEDs or duo LEDs resp.
 */
unsigned short ledGetNr(void)
{
	return m_nrLed;
}

/**
 * function to check if a name can belong to a LED or duo LED
 *
 * this is the case if the name starts with LED_ or DLED_
 * @param name		pointer to the name to be checked
 * @return			true, if it can be a LED or duo LED
 */
_Bool ledNameIsLed(const char * const name)
{
	if(0 == strncmp("LED_", name, 4)) 
		return true;

	if(0 == strncmp("DLED_", name, 5))
		return true;

	return false;
}

/**
 * function to set the the handle of the led
 * @param ptr		pointer to the led
 * @param handle	handle of the binary out or register of the led
 * @exception		assert on NULL pointer
 */
void ledSetHandle(struct sLed * const ptr, const unsigned short handle)
{
	assert(NULL != ptr);
	
	ptr->BO_handle = handle;
}

/**
 * function to get the the register handle of the led
 * @param ptr		pointer to the led
 * @return		aspect of the handle
 * @exception		assert on NULL pointer
 */
unsigned short ledGetHandle(const struct sLed * const ptr)
{
	assert(NULL != ptr);

	return ptr->BO_handle;
}
