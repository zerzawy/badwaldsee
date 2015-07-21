/*
 * This file is part of Raspberry Pi steuert Modelleisenbahn
 * Copyright(C)  2012 - 2015 Kurt Zerzawy www.zerzawy.ch
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
 * Dwarf signals (Zwergsignale on Domino, Sperrsignale on SpDr60) 
 * \author
 * Kurt Zerzawy
 */

/*
 * $Author: kurt $
 * $Date: 2015-05-18 22:31:02 +0200 (Mon, 18. Mai 2015) $
 * $Revision: 2464 $
 */

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "log.h"
#include "registers.h"
#include "button.h"
#include "dwarf.h"


/*@only@*/ /*@null@*/ static struct sDwarf * m_pDwarf = NULL;	/**< Pointer to the first dwarf	*/
static unsigned short 	m_nrDwarf = 0;			/**< Number of dwarf signals	*/
static _Bool		m_dwarfTodo = false;		/**< something to do for dwarfs	*/
static struct sDwarf *	m_pDwarfSel1 = NULL;		/**< was selected by button press	*/
static struct sDwarf *	m_pDwarfSel2 = NULL;		/**< was selected by button press	*/


/**
 * Constructor for the dwarfs
 * @param nr		number of dwarf signals to be created
 * @exception		Calls exit if no RAM available
 */
void dwarf(const unsigned short nr) 
{
	LOG_INF("started");
	
	m_nrDwarf = nr;
	m_pDwarf = malloc(m_nrDwarf * sizeof(struct sDwarf));
	if(NULL == m_pDwarf) 
	{
		/* error allocating */
		LOG_ERR("cannot allocate RAM of blocks");
		exit(EXIT_FAILURE);
	}
	dwarfInit();

	LOG_INF("ended");
}

/**
 * Destructor for the dwarfs
 */
void dwarfDestroy(void) 
{
	LOG_INF("started");
	if(NULL == m_pDwarf) 
	{
		LOG_INF("zero pointer, beende");
		return;
	}
	LOG_INF("release RAM of blocks");
	free(m_pDwarf);
	m_pDwarf = NULL;
	LOG_INF("ended");
}

void dwarfInit(void)
{
	struct sDwarf * pDwarf = m_pDwarf;
	unsigned short  i;

	LOG_INF("started");

	m_dwarfTodo = true;
	for(i = 0; i < m_nrDwarf; i++)
	{
		pDwarf->name[0] = '\0';
		pDwarf->pButton = NULL;
		pDwarf->aspect = SIG_DW_UNDEF;
		#ifdef SPDR60
			pDwarf->BO_Red1		= NONEVALUE;
			pDwarf->BO_Red2		= NONEVALUE;
			pDwarf->BO_White1	= NONEVALUE;
			pDwarf->BO_White2	= NONEVALUE;
			pDwarf->BO_Desk_Red	= NONEVALUE;
			pDwarf->BO_Desk_Green	= NONEVALUE;
			pDwarf->mainsName[0]	= '\0';
			pDwarf->mainsClear		= false;
		#endif
		#ifdef DOMINO55
			pDwarf->BO_LED1		= NONEVALUE;
			pDwarf->BO_LED2		= NONEVALUE;
			pDwarf->BO_LED3		= NONEVALUE;
			pDwarf->BO_Desk_Green	= NONEVALUE;
		#endif
		pDwarf->Todo = true;
		pDwarf++;
	}		
	LOG_INF("ended");
}

#ifdef SPDR60
	void dwarfProcess(void)
	{
		struct sDwarf * pDwarf = m_pDwarf;
		unsigned short	i;
		if(m_dwarfTodo)
		{
			LOG_INF("started");
			
			m_dwarfTodo = false;	
			assert(NULL != pDwarf);
			for(i = 0; i < m_nrDwarf; i++)
			{
				pDwarf = dwarfGet(i);
				assert(NULL != pDwarf);
				if(pDwarf->Todo)
				{
					pDwarf->Todo = false;
					
					if(pDwarf->mainsClear) 
					{
						LOG_STW("Zwergsignal %s: verbundenes Hauptsignal schaltet auf HP1/2", pDwarf->name);
						/* if connected main signal has clear aspect as HP1 or HP2 
						 *  then all lamps of the dwarf including desk lamps are dark
						 */
						registerBitClr(pDwarf->BO_White1);
						registerBitClr(pDwarf->BO_White2);
						registerBitClr(pDwarf->BO_Red1);
						registerBitClr(pDwarf->BO_Red2);
						registerBitClr(pDwarf->BO_Desk_Green);
						registerBitClr(pDwarf->BO_Desk_Red);
					}
					else 
					{
						switch(pDwarf->aspect)
						{
						case SIG_DW_UNDEF:
							pDwarf->aspect = SIG_DW_SH0;
							/*@falltrough@*/

						case SIG_DW_SH0:
							LOG_STW("Zwergsignal %s schaltet auf SH0", pDwarf->name);
							registerBitClr(pDwarf->BO_White1);
							registerBitClr(pDwarf->BO_White2);
							registerBitSet(pDwarf->BO_Red1);
							registerBitSet(pDwarf->BO_Red2);
							registerBitClr(pDwarf->BO_Desk_Green);
							registerBitSet(pDwarf->BO_Desk_Red);
							break;

						case SIG_DW_SH1:
							LOG_STW("Zwergsignal %s schaltet auf SH1", pDwarf->name);
							registerBitSet(pDwarf->BO_White1);
							registerBitSet(pDwarf->BO_White2);
							registerBitClr(pDwarf->BO_Red1);
							registerBitClr(pDwarf->BO_Red2);
							registerBitSet(pDwarf->BO_Desk_Green);
							registerBitClr(pDwarf->BO_Desk_Red);
							break;

						case SIG_DW_DARK:
							LOG_STW("Zwergsignal %s schaltet auf Dunkel (Kennlicht)", pDwarf->name);
							registerBitClr(pDwarf->BO_White1);
							registerBitSet(pDwarf->BO_White2);
							registerBitClr(pDwarf->BO_Red1);
							registerBitClr(pDwarf->BO_Red2);
							registerBitClr(pDwarf->BO_Desk_Green);
							registerBitClr(pDwarf->BO_Desk_Red);
							break;

						default: 
							LOG_ERR("impossible default");
							pDwarf->aspect = SIG_DW_UNDEF;
							break;
						}
					}
				}
			}
			LOG_INF("ended");
		}
	}
#endif
#ifdef DOMINO55
	void dwarfProcess(void)
	{
		struct sDwarf * pDwarf = m_pDwarf;
		unsigned short	i;
		if(m_dwarfTodo)
		{
			LOG_INF("started");
			
			m_dwarfTodo = false;	
			assert(NULL != pDwarf);
			for(i = 0; i < m_nrDwarf; i++)
			{
				pDwarf = dwarfGet(i);
				assert(NULL != pDwarf);
				if(pDwarf->Todo)
				{
					pDwarf->Todo = false;
					switch(pDwarf->aspect)
					{
					case SIG_DW_UNDEF:
						pDwarf->aspect = SIG_DW_HALT;
						/*@falltrough@*/

					case SIG_DW_HALT:
						LOG_STW("Zwergsignal %s schaltet auf Halt", pDwarf->name);
						registerBitClr(pDwarf->BO_LED1);
						registerBitSet(pDwarf->BO_LED2);
						registerBitSet(pDwarf->BO_LED3);
						registerBitClr(pDwarf->BO_Desk_Green);
						break;

					case SIG_DW_WARNING:
						LOG_STW("Zwergsignal %s schaltet auf Warnung", pDwarf->name);
						registerBitSet(pDwarf->BO_LED1);
						registerBitClr(pDwarf->BO_LED2);
						registerBitSet(pDwarf->BO_LED3);
						registerBitSet(pDwarf->BO_Desk_Green);
						break;

					case SIG_DW_CLEAR:
						LOG_STW("Zwergsignal %s schaltet auf Fahrt", pDwarf->name);
						registerBitSet(pDwarf->BO_LED1);
						registerBitSet(pDwarf->BO_LED2);
						registerBitClr(pDwarf->BO_LED3);
						registerBitSet(pDwarf->BO_Desk_Green);
						break;

					case SIG_DW_DARK:
						LOG_STW("Zwergsignal %s schaltet auf Dunkel (Kennlicht)", pDwarf->name);
						registerBitClr(pDwarf->BO_LED1);
						registerBitClr(pDwarf->BO_LED2);
						registerBitClr(pDwarf->BO_LED3);
						registerBitClr(pDwarf->BO_Desk_Green);
					break;

					default: 
						LOG_ERR("impossible default");
						pDwarf->aspect = SIG_DW_UNDEF;
						break;
					}
				}
			}
			LOG_INF("ended");
		}
	}
#endif

/**
 * Function to return the number of dwarf buttons pressed 
 * and the pointer to the dwarf which button was pressed
 * If more than one button was pressed, the return may be 
 * incorrect.
 * @param pSel		pointer to pointer of the dwarf
 * return		Number of buttons
 */
unsigned short dwarfNrButtons(struct sDwarf ** ppSel1, struct sDwarf ** ppSel2) 
{
	unsigned short		i;
	unsigned short		nr = 0;
	struct sDwarf *		pDwarf = m_pDwarf;

	m_pDwarfSel1 = NULL;
	m_pDwarfSel2 = NULL;
	for(i = 0; i < m_nrDwarf; i++) 
	{
		if(buttonSingleReadHW(pDwarf->pButton))
		{
			nr++;
			m_pDwarfSel2 = m_pDwarfSel1;
			m_pDwarfSel1 = pDwarf;	/* remember internal pointer	*/
			pDwarf->Todo = true; 
			m_dwarfTodo = true;
		}
		pDwarf++;
	}
	* ppSel1 = m_pDwarfSel1;	/* set return value	*/
	* ppSel2 = m_pDwarfSel2;
	return nr;
}

/**
 * function to find the pointer of a dwarf by giving the name
 * @param name		name of the dwarf to be locked for
 * @return		pointer of the found dwarf
 * @exception		a parser assert is made if not found
 */
struct sDwarf * dwarfFind(const char * const name) 
{
        unsigned short	i = 0;
	struct sDwarf *	pDwarf = NULL;
        char		temp[128];

	i = 0;
	while(NULL != (pDwarf = dwarfGet(i)))
	{ 
		if(0 == (strcmp(pDwarf->name, name))) 
		{
			return pDwarf;
		}
		i++;
        }
	#ifdef SPDR60
		sprintf(temp, "Sperrsignal %s nicht gefunden", name);
	#endif

	#ifdef DOMINO55
		sprintf(temp, "Zwergsignal %s nicht gefunden", name);
	#endif
        parserExit(temp);
        return NULL;
}

/**
 * function to get the pointer by giving the number of the dwarf
 * @param i		index, starting with 0
 * @return		pointer to this dwarf, NULL if not existing
 */ 
struct sDwarf * dwarfGet(const unsigned short i)
{
	if(i >= m_nrDwarf)
	{
		return NULL;
	}
	else
	{
		return m_pDwarf + i;
	}
}

/**
 * Function to get the number of dwarf signals 
 * @return		Number of dwarf signals
 */
unsigned short dwarfGetNr(void)
{
	return m_nrDwarf;
}

/**
 * function to get the name of a dwarf
 * @param pDwarf:	pointer to the dwarf
 * @param name:		pointer to the string filled out with the name
 * 			of the dwarf. Must be at least NAMELEN long
 * @return		true, if successful
 */ 
_Bool dwarfGetName(const struct sDwarf * const pDwarf, /*@out@*/ char * const name)
{
	if(NULL != pDwarf)
	{
		name[NAMELEN - 1] = '\0';
		memmove(name, pDwarf->name, NAMELEN - 1);
	}
	else
	{
		memmove(name, "NONE", NAMELEN - 1);
	}
	return true;
}

/**
 * function to set the name of a dwarf
 * @param pDwarf:	pointer to the dwarf
 * @param name:		pointer to the string with the name 
 * @exception		assert on NULL pointer
 */ 
void dwarfSetName(struct sDwarf * const pDwarf, const char * const name)
{
	assert(NULL != pDwarf);

	memmove(pDwarf->name, name, NAMELEN);
}

#ifdef SPDR60
	/**
	 * function to get the register of the left red led
	 * @param pDwarf	pointer to the dwarf
	 * @return		register handle of the left red led, NONEVALUE on NULL pointer
	 */
	unsigned short dwarfGetBO_Red1(const struct sDwarf * const pDwarf)
	{
		if(NULL != pDwarf)
		{
			return pDwarf->BO_Red1;
		}
		else
		{	
			return NONEVALUE;
		}
	}

	/**
	 * function to set the register of the left red led
	 * @param pDwarf	pointer to the dwarf
	 * @param handle	register handle of the left red led
	 * @exception		assert on NULL pointer
	 */
	void dwarfSetBO_Red1(struct sDwarf * const pDwarf, const unsigned short handle)
	{
		assert(NULL != pDwarf);

		pDwarf->BO_Red1 = handle;
	}

	/**
	 * function to get the register of the right red led
	 * @param pDwarf	pointer to the dwarf
	 * @return		register handle of the right red led, NONEVALUE on NULL pointer
	 */
	unsigned short dwarfGetBO_Red2(const struct sDwarf * const pDwarf)
	{
		if(NULL != pDwarf)
		{
			return pDwarf->BO_Red2;
		}
		else
		{
			return NONEVALUE;
		}
	}

	/**
	 * function to set the register of the right red led
	 * @param pDwarf	pointer to the dwarf
	 * @param handle	register handle of the right red led
	 * @exception		assert on NULL pointer
	 */
	void dwarfSetBO_Red2(struct sDwarf * const pDwarf, const unsigned short handle)
	{
		assert(NULL != pDwarf);

		pDwarf->BO_Red2 = handle;
	}


	/**
	 * function to get the register of the lower left white led
	 * @param pDwarf	pointer to the dwarf
	 * @return		register handle of the lower left white led, NONEVALUE on NULL pointer
	 */
	unsigned short dwarfGetBO_White1(const struct sDwarf * const pDwarf)
	{
		if(NULL != pDwarf)
		{
			return pDwarf->BO_White1;
		}
		else
		{
			return NONEVALUE;
		}
	}

	/**
	 * function to set the register of the lower left white led
	 * @param pDwarf	pointer to the dwarf
	 * @param handle	register handle of the lower left white led
	 * @exception		assert on NULL pointer
	 */
	void dwarfSetBO_White1(struct sDwarf * const pDwarf, const unsigned short handle)
	{
		assert(NULL != pDwarf);

		pDwarf->BO_White1 = handle;
	}

	/**
	 * function to get the register of the upper right white led
	 * @param pDwarf	pointer to the dwarf
	 * @return		register handle of the upper right white led, NONEVALUE on NULL pointer
	 */
	unsigned short dwarfGetBO_White2(const struct sDwarf * const pDwarf)
	{
		if(NULL != pDwarf)
		{
			return pDwarf->BO_White2;
		}
		else
		{
			return NONEVALUE;
		}
	}

	/**
	 * function to set the register of the upper right white led
	 * @param pDwarf	pointer to the dwarf
	 * @param handle	register handle of the upper right white led
	 * @exception		assert on NULL pointer
	 */
	void dwarfSetBO_White2(struct sDwarf * const pDwarf, const unsigned short handle)
	{
		assert(NULL != pDwarf);

		pDwarf->BO_White2 = handle;
	}

	/**
	 * function to get the register of the red desk led
	 * @param pDwarf	pointer to the dwarf
	 * @return		register handle of the red desk led, NONEVALUE on NULL pointer
	 */
	unsigned short dwarfGetBO_Desk_Red(const struct sDwarf * const pDwarf)
	{
		if(NULL != pDwarf)
		{
			return pDwarf->BO_Desk_Red;
		}
		else
		{
			return NONEVALUE;
		}
	}

	/**
	 * function to set the register of the red desk led
	 * @param pDwarf	pointer to the dwarf
	 * @param handle	register handle of the red desk led
	 * @exception		assert on NULL pointer
	 */
	void dwarfSetBO_Desk_Red(struct sDwarf * const pDwarf, const unsigned short handle)
	{
		assert(NULL != pDwarf);

		pDwarf->BO_Desk_Red = handle;
	}

	/**
	 * function to get the register of the green desk led
	 * @param pDwarf	pointer to the dwarf
	 * @return		register handle of the green desk led, NONEVALUE on NULL pointer
	 */
	unsigned short dwarfGetBO_Desk_Green(const struct sDwarf * const pDwarf)
	{
		if(NULL != pDwarf)
		{
			return pDwarf->BO_Desk_Green;
		}
		else
		{
			return NONEVALUE;
		}
	}

	/**
	 * function to set the register of the green desk led
	 * @param pDwarf	pointer to the dwarf
	 * @param handle	register handle of the green desk led
	 * @exception		assert on NULL pointer
	 */
	void dwarfSetBO_Desk_Green(struct sDwarf * const pDwarf, const unsigned short handle)
	{
		assert(NULL != pDwarf);

		pDwarf->BO_Desk_Green = handle;
	}

	/**
	 * function to get the name of the main signal combined with the dwarf
	 * @param pDwarf:	pointer to the dwarf
	 * @param name:		pointer to the string filled out with the name of the main signal, NONE on NULL pointer
	 * 			Must be at least NAMELEN long
	 * @return		true, if successful
	 */ 
	_Bool dwarfGetMainName(const struct sDwarf * const pDwarf, /*@out@*/ char * const name)
	{
		name[NAMELEN - 1] = '\0';
		if(NULL != pDwarf)
		{
			memmove(name, pDwarf->mainsName, NAMELEN - 1);
		}
		else
		{
			memmove(name, "NONE", NAMELEN - 1);
		}
		return true;
	}

	/**
	 * function to set the name of the main signal combined with the dwarf
	 * @param pDwarf:	pointer to the dwarf
	 * @param name:		pointer to the string with the name of the main signal
	 * @exception		assert on NULL pointer
	 */ 
	void dwarfSetMainName(struct sDwarf * const pDwarf, const char * const name)
	{
		assert(NULL != pDwarf);

		memmove(pDwarf->mainsName, name, NAMELEN);
	}

	/**
	 * function to get the clear aspect of the main signal combined with this dwarf
	 * @param pDwarf	pointer to the dwarf
	 * @return		true if main signal has a clear aspect as HP1 or HP2, false on NULL pointer
	 * @exception		assert on NULL pointer
	 */
	_Bool dwarfGetMainClear(const struct sDwarf * const pDwarf)
	{
		if(NULL != pDwarf)
		{
			return pDwarf->mainsClear;
		}
		else
		{
			return false;
		}
	}

	/**
	 * function to set the clear aspect of the main signal combined with this dwarf
	 * @param pDwarf	pointer to the dwarf
	 * @param clear		true if main signal has a clear aspect as HP1 or HP2
	 * @exception		assert on NULL pointer
	 */
	void dwarfSetMainClear(struct sDwarf * const pDwarf, const _Bool clear)
	{
		assert(NULL != pDwarf);

		if(pDwarf->mainsClear != clear) {
			pDwarf->mainsClear = clear;
			pDwarf->Todo = true;
			m_dwarfTodo = true;
		}
	}

	/**
	 * Function to get the pointer of the main signal the dwarf is attached o
	 * @param pDwarf	pointer to the dwarf attached to the mains
	 * @returns		pointer to the mains, NULL on NULL pointer of the dwarf
	 * @exception		assert on NULL pointer
	 */
	struct sMain * dwarfGetMain(const struct sDwarf * const pDwarf)
	{
		if(NULL != pDwarf)
		{
			return pDwarf->pMain;
		}
		else
		{
			return NULL;
		}
	}

	/**
	 * Function to set the pointer of the main signal the dwarf is attached o
	 * @param pDwarf	pointer to the dwarf attached to the mains
	 * @param pMain	pointer to the mains
	 * @exception		assert on NULL pointer
	 */
	void dwarfSetMain(struct sDwarf * const pDwarf, const struct sMain * const pMain)
	{
		assert(NULL != pDwarf);

		pDwarf->pMain = pMain;
	}
#endif
#ifdef DOMINO55
	/**
	 * function to get the register of the led 1
	 * @param pDwarf	pointer to the dwarf
	 * @return		register handle of the led 1, NONEVALUE on NULL pointer
	 */
	unsigned short dwarfGetBO_LED1(const struct sDwarf * const pDwarf)
	{
		if(NULL != pDwarf)
		{
			return pDwarf->BO_LED1;
		}
		else
		{
			return NONEVALUE;
		}
	}

	/**
	 * function to set the register of the led 1
	 * @param pDwarf	pointer to the dwarf
	 * @param handle	register handle of the led 1
	 * @exception		assert on NULL pointer
	 */
	void dwarfSetBO_LED1(struct sDwarf * const pDwarf, const unsigned short handle)
	{
		assert(NULL != pDwarf);

		pDwarf->BO_LED1 = handle;
	}

	/**
	 * function to get the register of the led 2
	 * @param pDwarf	pointer to the dwarf
	 * @return		register handle of the led 2, NONEVALUE on NULL pointer
	 */
	unsigned short dwarfGetBO_LED2(const struct sDwarf * const pDwarf)
	{
		if(NULL != pDwarf)
		{
			return pDwarf->BO_LED2;
		}
		else
		{
			return NONEVALUE;
		}
	}

	/**
	 * function to set the register of the led 2
	 * @param pDwarf	pointer to the dwarf
	 * @param handle	register handle of the led 2
	 * @exception		assert on NULL pointer
	 */
	void dwarfSetBO_LED2(struct sDwarf * const pDwarf, const unsigned short handle)
	{
		assert(NULL != pDwarf);

		pDwarf->BO_LED2 = handle;
	}

	/**
	 * function to get the register of the led 3
	 * @param pDwarf	pointer to the dwarf
	 * @return		register handle of the led 3, NONEVALUE on NULL pointer
	 */
	unsigned short dwarfGetBO_LED3(const struct sDwarf * const pDwarf)
	{
		if(NULL != pDwarf)
		{
			return pDwarf->BO_LED3;
		}
		else
		{
			return NONEVALUE;
		}
	}

	/**
	 * function to set the register of the led 3
	 * @param pDwarf	pointer to the dwarf
	 * @param handle	register handle of the led 3
	 * @exception		assert on NULL pointer
	 */
	void dwarfSetBO_LED3(struct sDwarf * const pDwarf, const unsigned short handle)
	{
		assert(NULL != pDwarf);

		pDwarf->BO_LED3 = handle;
	}

	/**
	 * function to get the register of the led on the desk
	 * @param pDwarf	pointer to the dwarf
	 * @return		register handle of the led on the desk, NONEVALUE on NULL pointer
	 */
	unsigned short dwarfGetBO_Desk_Green(const struct sDwarf * const pDwarf)
	{
		if(NULL != pDwarf)
		{
			return pDwarf->BO_Desk_Green;
		}
		else
		{
			return NONEVALUE;
		}
	}

	/**
	 * function to set the register of the led on the desk
	 * @param pDwarf	pointer to the dwarf
	 * @param handle	register handle of the led on the desk
	 * @exception		assert on NULL pointer
	 */
	void dwarfSetBO_Desk_Green(struct sDwarf * const pDwarf, const unsigned short handle)
	{
		assert(NULL != pDwarf);

		pDwarf->BO_Desk_Green = handle;
	}
#endif

/**
 * function to get the pointer of the Button
 * @param pDwarf	pointer to the dwarf
 * @return		pointer to the button, NULL on NULL pointer of sDwarf
 */
struct sButton * dwarfGetBI_Button(const struct sDwarf * const pDwarf)
{
	if(NULL != pDwarf)
	{
		return pDwarf->pButton;
	}
	else
	{
		return NULL;
	}
}

/**
 * function to set the pointer of the Button
 * @param pDwarf	pointer to the dwarf
 * @param handle	pointer to the Button
 * @exception		assert on NULL pointer
 */
void dwarfSetBI_Button(struct sDwarf * const pDwarf, struct sButton * const pButton)
{
	assert(NULL != pDwarf);

	pDwarf->pButton = pButton;
}

/**
 * function to get the the aspect of the dwarf
 * @param pDwarf	pointer to the dwarf
 * @return		aspect of the dwarf, SIG_DW_UNDEF on NULL pointer
 */
enum eDwarfAspect dwarfGetAspect(const struct sDwarf * const pDwarf)
{
	if(NULL != pDwarf)
	{
		return pDwarf->aspect;
	}
	else
	{
		return SIG_DW_UNDEF;
	}
}

/**
 * function to set the the aspect of the dwarf, if pDwarf is NULL pointer, nothing is done
 * @param pDwarf	pointer to the dwarf
 * @param aspect	aspect of the dwarf
 */
void dwarfSetAspect(struct sDwarf * const pDwarf, const enum eDwarfAspect aspect)
{
	if(NULL == pDwarf)
	{
		return;
	}
	
	if(pDwarf->aspect != aspect)
	{
		pDwarf->aspect = aspect;
		pDwarf->Todo = true;
		m_dwarfTodo = true;
	}
}

#ifdef DOMINO55
	/**
	 * function to set the aspect of the dwarf in dependence of
	 *  the distand dwarf, if pDwarf or pDest is NULL pointer, nothing is done
	 * @param pDwarf	pointer to the aspect
	 * @param aspect	aspect 
	 * @param pDest		pointer to the destination dwarf
	 */
	void dwarfSetAspectDest(struct sDwarf * const pDwarf, 
				const enum eDwarfAspect aspect,
				const struct sDwarf * const pDest)
	{
		if(NULL == pDwarf)
		{
			return;
		}
		if(NULL == pDest)
		{
			return;
		}

		switch(dwarfGetAspect(pDest))
		{
		case SIG_DW_WARNING:
		case SIG_DW_CLEAR:
			dwarfSetAspect(pDwarf, aspect);
			break;
		case SIG_DW_DARK:
		case SIG_DW_HALT:
		default:
			if(SIG_DW_CLEAR == aspect) 
			{
				dwarfSetAspect(pDwarf, SIG_DW_WARNING);
			}
		else
		{
			dwarfSetAspect(pDwarf, aspect);
		}
		break;
		}       
	}
#endif
