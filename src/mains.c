/*
 * This file is part of Raspberry Pi steuert Modelleisenbahn
 * Copyright(C)  2014 - 2015 Kurt Zerzawy www.zerzawy.ch
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
 * Main signals (Hauptsignale)
 * \author
 * Kurt Zerzawy
 */

/*
 * $Author: kurt $
 * $Date: 2015-06-07 15:52:39 +0200 (Son, 07. Jun 2015) $
 * $Revision: 2483 $
 */

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "log.h"
#include "registers.h"
#include "button.h"
#include "mains.h"
#include "dwarf.h"


/*@only@*/ /*@null@*/ static struct sMain * m_pMain = NULL;	/**< Pointer to the first main signal	*/
static unsigned short 	m_nrMains = 0;			/**< Number of main signals	*/
static _Bool		m_mainsTodo = false;		/**< something to do for main signals	*/
static struct sMain *	m_pMainSel1 = NULL;		/**< was selected by button press	*/
static struct sMain *	m_pMainSel2 = NULL;		/**< was selected by button press	*/


/**
 * Constructor for the main
 * @param nr		number of main signals to be created
 * @exception       	Calls exit if no RAM available
 */
void mains(const unsigned short nr) 
{
	LOG_INF("started");
	
	m_nrMains = nr;
	m_pMain = malloc(m_nrMains * sizeof(struct sMain));
	if(NULL == m_pMain) 
	{
		/* error allocating */
		LOG_ERR("cannot allocate RAM for main signals");
		exit(EXIT_FAILURE);
	}
	mainsInit();

	LOG_INF("ended");
}

/**
 * Destructor for the main signals
 */
void mainsDestroy(void) 
{
	LOG_INF("started");
	if(NULL == m_pMain) 
	{
		LOG_INF("zero pointer, beende");
		return;
	}
	LOG_INF("release RAM of main signals");
	free(m_pMain);
	m_pMain = NULL;
	LOG_INF("ended");
}

void mainsInit(void)
{
	struct sMain * pMain = m_pMain;
	unsigned short  i;

	LOG_INF("started");

	m_mainsTodo = true;
	for(i = 0; i < m_nrMains; i++)
	{
		pMain->name[0] = '\0';
		pMain->pButton = NULL;
		pMain->eAspect	= SIG_MN_UNDEF;
		#ifdef SPDR60
			pMain->BO_Green	= NONEVALUE;
			pMain->BO_Red	= NONEVALUE;
			pMain->BO_Yellow	= NONEVALUE;
			pMain->BO_Zs3	= NONEVALUE;
			pMain->BO_Zp9	= NONEVALUE;

			pMain->eRequestedAspect	= SIG_MN_HP0;
			pMain->pDwarf	= NULL;
		#endif

		#ifdef DOMINO55
			pMain->BO_Green1	= NONEVALUE;
			pMain->BO_Green2	= NONEVALUE;
			pMain->BO_Green3	= NONEVALUE;
			pMain->BO_Yellow1	= NONEVALUE;
			pMain->BO_Yellow2	= NONEVALUE;
			pMain->BO_Red1	= NONEVALUE;
			pMain->BO_Depart	= NONEVALUE;

			pMain->eRequestedAspect	= SIG_MN_HALT;
		#endif

		pMain->pDistant		= NULL;
		pMain->BO_Desk_Red	= NONEVALUE;
		pMain->BO_Desk_Green	= NONEVALUE;

		pMain->ToDo		= true;
		pMain++;
	}		
	LOG_INF("ended");
}

/**
 * Function to return the number of mains buttons pressed 
 * and the pointer to the mains which button was pressed
 * If more than one button was pressed, the return may be 
 * incorrect.
 * @param pSel	pointer to pointer of the mains
 * return		Number of buttons
 */
unsigned short mainNrButtons(struct sMain ** ppSel1, struct sMain ** ppSel2) 
{
	unsigned short		i;
	unsigned short		nr = 0;
	struct sMain *		pMain = m_pMain;

	m_pMainSel1 = NULL;
	m_pMainSel2 = NULL;
	for(i = 0; i < m_nrMains; i++) 
	{
		if(buttonSingleReadHW(pMain->pButton))
		{
			nr++;
			m_pMainSel2 = m_pMainSel1;
			m_pMainSel1 = pMain;	/* remember internal pointer	*/
			pMain->ToDo = true; 
			m_mainsTodo = true;
		}
		pMain++;
	}
	* ppSel1 = m_pMainSel1;	/* set return value	*/
	* ppSel2 = m_pMainSel2;
	return nr;
}

/**
 * Function to find the pointer of a mains by giving the name
 * @param name		name of the mains to be locked for
 * @return		pointer of the found mains
 * @exception		a parser assert is made if not found
 */
struct sMain * mainFind(const char * const name) 
{
        unsigned short		i = 0;
	struct sMain *		pMain = NULL;
	char			temp[128];

	while(NULL != (pMain = mainGet(i)))
	{ 
		if(0 == (strcmp(pMain->name, name))) 
		{
			return pMain;
		}
		i++;
        }
        sprintf(temp, "Hauptsignal %s nicht gefunden", name);
        parserExit(temp);
        return NULL;
}

/**
 * Function to get the pointer by giving the number of the mains
 * @param i		index, starting with 0
 * @return		pointer to this mains, NULL if not existing
 */ 
struct sMain * mainGet(const unsigned short i)
{
	if(i >= m_nrMains)
	{
		return NULL;
	}
	else
	{
		return m_pMain + i;
	}
}

/**
 * Function to get the number of mains signals 
 * @return		Number of mains signals
 */
unsigned short mainsGetNr(void)
{
	return m_nrMains;
}

/**
 * Function to get the name of a main signal
 * @param pMain:	pointer to the main signal
 * @param name:		pointer to the string filled out with the name
 * 			of the main signal. Must be at least NAMELEN long
 * @return		true, if successful
 * @exception		assert on NULL pointer
 */ 
_Bool mainGetName(const struct sMain * const pMain, /*@out@*/ char * const name)
{
	assert(NULL != pMain);

	name[NAMELEN - 1] = '\0';
	memmove(name, pMain->name, NAMELEN - 1);
	return true;
}

/**
 * Function to set the name of a main signal
 * @param pMain:	pointer to the main signal
 * @param name:		pointer to the string with the name 
 * @exception		assert on NULL pointer
 */ 
void mainSetName(struct sMain * const pMain, const char * const name)
{
	assert(NULL != pMain);

	memmove(pMain->name, name, NAMELEN);
}

#ifdef SPDR60
	/**
	 * Function to get the register of the green signal LED
	 * @param pMain		pointer to the main signal
	 * @return		register handle of green signal LED
	 * @exception		assert on NULL pointer
	 */
	unsigned short mainGetBO_Green(const struct sMain * const pMain)
	{
		assert(NULL != pMain);

		return pMain->BO_Green;
	}

	/**
	 * Function to set the register of the green signal LED
	 * @param pMain		pointer to the main signal 
	 * @param handle	register handle of the green signal LED
	 * @exception		assert on NULL pointer
	 */
	void mainSetBO_Green(struct sMain * const pMain, const unsigned short handle)
	{
		assert(NULL != pMain);

		pMain->BO_Green = handle;
	}

	/**
	 * Function to get the register of the red signal LED
	 * @param pMain		pointer to the main signal
	 * @return		register handle of red signal LED
	 * @exception		assert on NULL pointer
	 */
	unsigned short mainGetBO_Red(const struct sMain * const pMain)
	{
		assert(NULL != pMain);

		return pMain->BO_Red;
	}

	/**
	 * Function to set the register of the red signal LED
	 * @param pMain		pointer to the main signal 
	 * @param handle	register handle of the red signal LED
	 * @exception		assert on NULL pointer
	 */
	void mainSetBO_Red(struct sMain * const pMain, const unsigned short handle)
	{
		assert(NULL != pMain);

		pMain->BO_Red = handle;
	}

	/**
	 * Function to get the register of the yellow signal LED
	 * @param pMain		pointer to the main signal
	 * @return		register handle of yellow signal LED
	 * @exception		assert on NULL pointer
	 */
	unsigned short mainGetBO_Yellow(const struct sMain * const pMain)
	{
		assert(NULL != pMain);

		return pMain->BO_Yellow;
	}

	/**
	 * Function to set the register of the yellow signal LED
	 * @param pMain		pointer to the main signal 
	 * @param handle	register handle of the yellow signal LED
	 * @exception		assert on NULL pointer
	 */
	void mainSetBO_Yellow(struct sMain * const pMain, const unsigned short handle)
	{
		assert(NULL != pMain);

		pMain->BO_Yellow = handle;
	}

	/**
	 * Function to get the register of the Zs3 LED
	 * @param pMain		pointer to the main signal
	 * @return		register handle of Zs3 LED
	 * @exception		assert on NULL pointer
	 */
	unsigned short mainGetBO_Zs3(const struct sMain * const pMain)
	{
		assert(NULL != pMain);

		return pMain->BO_Zs3;
	}

	/**
	 * Function to set the register of the Zs3 LED
	 * @param pMain		pointer to the main signal 
	 * @param handle	register handle of the Zs3 LED
	 * @exception		assert on NULL pointer
	 */
	void mainSetBO_Zs3(struct sMain * const pMain, const unsigned short handle)
	{
		assert(NULL != pMain);

		pMain->BO_Zs3 = handle;
	}

	/**
	 * Function to get the register of the Zp9 LED
	 * @param pMain		pointer to the main signal
	 * @return		register handle of Zp9 LED
	 * @exception		assert on NULL pointer
	 */
	unsigned short mainGetBO_Zp9(const struct sMain * const pMain)
	{
		assert(NULL != pMain);

		return pMain->BO_Zp9;
	}

	/**
	 * Function to set the register of the Zp9 LED
	 * @param pMain		pointer to the main signal 
	 * @param handle	register handle of the Zp9 LED
	 * @exception		assert on NULL pointer
	 */
	void mainSetBO_Zp9(struct sMain * const pMain, const unsigned short handle)
	{
		assert(NULL != pMain);

		pMain->BO_Zp9 = handle;
	}

#endif

#ifdef DOMINO55
	/** 
	 * Function to get the register of the green 1 LED
	 * @param pMain		pointer to the main signal
	 * @return		register handle of green 1 LED
	 * @exception		assert on NULL pointer
	 */
	unsigned short mainGetBO_Green1(const struct sMain * const pMain)
	{
		assert(NULL != pMain);

		return pMain->BO_Green1;
	}

	/**
	 * Function to set the register of the green 1 LED
	 * @param pMain		pointer to the main signal 
	 * @param handle	register handle of the green 1 LED
	 * @exception		assert on NULL pointer
	 */
	void mainSetBO_Green1(struct sMain * const pMain, const unsigned short handle)
	{
		assert(NULL != pMain);

		pMain->BO_Green1 = handle;
	}

	/** 
	 * Function to get the register of the green 2 LED
	 * @param pMain		pointer to the main signal
	 * @return		register handle of the green 2 LED
	 * @exception		assert on NULL pointer
	 */
	unsigned short mainGetBO_Green2(const struct sMain * const pMain)
	{
		assert(NULL != pMain);

		return pMain->BO_Green2;
	}

	/**
	 * Function to set the register of the green 2 LED
	 * @param pMain		pointer to the main signal
	 * @param handle	register handle of the green 2 LED
	 * @exception		assert on NULL pointer
	 */
	void mainSetBO_Green2(struct sMain * const pMain, const unsigned short handle)
	{
		assert(NULL != pMain);

		pMain->BO_Green2 = handle;
	}

	/**
	 * Function to get the register of the green 3 LED
	 * @param pMain		pointer to the main signal
	 * @return		register handle of the green 3 LED
	 * @exception		assert on NULL pointer
	 */
	unsigned short mainGetBO_Green3(const struct sMain * const pMain)
	{
		assert(NULL != pMain);

		return pMain->BO_Green3;
	}

	/**
	 * Function to set the register of the green 3 LED
	 * @param pMain		pointer to the main signal
	 * @param handle	register handle of the green 3 LED
	 * @exception		assert on NULL pointer
	 */
	void mainSetBO_Green3(struct sMain * const pMain, const unsigned short handle)
	{
		assert(NULL != pMain);

		pMain->BO_Green3 = handle;
	}

	/**
	 * Function to get the register of the yellow 1 LED
	 * @param pMain		pointer to the main signal
	 * @return		register handle of the yellow 1 LED
	 * @exception		assert on NULL pointer
	 */
	unsigned short mainGetBO_Yellow1(const struct sMain * const pMain)
	{
		assert(NULL != pMain);

		return pMain->BO_Yellow1;
	}

	/**
	 * Function to set the register of the yellow 1 LED
	 * @param pMain		pointer to the main signal
	 * @param handle	register handle of the yellow 1 LED
	 * @exception		assert on NULL pointer
	 */
	void mainSetBO_Yellow1(struct sMain * const pMain, const unsigned short handle)
	{
		assert(NULL != pMain);

		pMain->BO_Yellow1 = handle;
	}

	/**
	 * Function to get the register of the yellow 2 LED
	 * @param pMain		pointer to the main signal
	 * @return		register handle of the yellow 2 LED
	 * @exception		assert on NULL pointer
	 */
	unsigned short mainGetBO_Yellow2(const struct sMain * const pMain)
	{
		assert(NULL != pMain);

		return pMain->BO_Yellow2;
	}

	/**
	 * Function to set the register of the yellow 2 LED
	 * @param pMain		pointer to the main signal
	 * @param handle	register handle of the yellow 2 LED
	 * @exception		assert on NULL pointer
	 */
	void mainSetBO_Yellow2(struct sMain * const pMain, const unsigned short handle)
	{
		assert(NULL != pMain);

		pMain->BO_Yellow2 = handle;
	}

	/**
	 * Function to get the register of the red 1 LED
	 * @param pMain		pointer to the main signal
	 * @return		register handle of the red 1 LED
	 * @exception		assert on NULL pointer
	 */
	unsigned short mainGetBO_Red1(const struct sMain * const pMain)
	{
		assert(NULL != pMain);

		return pMain->BO_Red1;
	}

	/**
	 * Function to set the register of the red 1 LED
	 * @param pMain		pointer to the main signal
	 * @param handle	register handle of the red 1 LED
	 * @exception		assert on NULL pointer
	 */
	void mainSetBO_Red1(struct sMain * const pMain, const unsigned short handle)
	{
		assert(NULL != pMain);

		pMain->BO_Red1 = handle;
	}

	/**
	 * Function to get the register of the red 2 LED
	 * @param pMain		pointer to the main signal
	 * @return		register handle of the red 2 LED
	 * @exception		assert on NULL pointer
	 */
	unsigned short mainGetBO_Depart(const struct sMain * const pMain)
	{
		assert(NULL != pMain);

		return pMain->BO_Depart;
	}

	/**
	 * Function to set the register of the red 2 LED
	 * @param pMain		pointer to the main signal
	 * @param handle	register handle of the red 2 LED
	 * @exception		assert on NULL pointer
	 */
	void mainSetBO_Depart(struct sMain * const pMain, const unsigned short handle)
	{
		assert(NULL != pMain);

		pMain->BO_Depart = handle;
	}
#endif

/**
 * Function to get the register of the red LED on the switchboard
 * @param pMain		pointer to the main signal
 * @return		register handle of the red LED on the switchboard
 * @exception		assert on NULL pointer
 */
unsigned short mainGetBO_Desk_Red(const struct sMain * const pMain)
{
	assert(NULL != pMain);

	return pMain->BO_Desk_Red;
}

/**
 * Function to set the register of the red LED on the switchboard
 * @param pMain		pointer to the main signal
 * @param handle	register handle of the red LED on the switchboard
 * @exception		assert on NULL pointer
 */
void mainSetBO_Desk_Red(struct sMain * const pMain, const unsigned short handle)
{
	assert(NULL != pMain);

	pMain->BO_Desk_Red = handle;
}

/**
 * Function to get the register of the green LED on the switchboard
 * @param pMain		pointer to the main signal
 * @return		register handle of the green LED on the switchboard
 * @exception		assert on NULL pointer
 */
unsigned short mainGetBO_Desk_Green(const struct sMain * const pMain)
{
	assert(NULL != pMain);

	return pMain->BO_Desk_Green;
}

/**
 * Function to set the register of the green LED on the switchboard
 * @param pMain		pointer to the main signal
 * @param handle	register handle of the green LED on the switchboard
 * @exception		assert on NULL pointer
 */
void mainSetBO_Desk_Green(struct sMain * const pMain, const unsigned short handle)
{
	assert(NULL != pMain);

	pMain->BO_Desk_Green = handle;
}

/**
 * Function to get the register of the button for the main signal
 * @param pMain		pointer to the main signal
 * @return		pointer to the button
 * @exception		assert on NULL pointer
 */
struct sButton * mainGetBI_Button(const struct sMain * const pMain)
{
	assert(NULL != pMain);

	return pMain->pButton;
}

/**
 * Function to set the register of the button of the main signal
 * @param pMain		pointer to the main signal
 * @param pButton	pointer to the button
 * @exception		assert on NULL pointer
 */
void mainSetBI_Button(struct sMain * const pMain, struct sButton *  const pButton)
{
	assert(NULL != pMain);

	pMain->pButton = pButton;
}

/**
 * Function to get the name of a distant signal attached to the mast of the main signal
 * @param pMain:	pointer to the main signal
 * @param name:		pointer to the string filled out with the name
 * 			of the distant signal. Must be at least NAMELEN long
 * @return		true, if successful
 * @exception		assert on NULL pointer
 */ 
_Bool mainGetDistantName(const struct sMain * const pMain, /*@out@*/ char * const name)
{
	assert(NULL != pMain);

	name[NAMELEN - 1] = '\0';
	memmove(name, pMain->distantName, NAMELEN - 1);
	return true;
}

/**
 * Function to set the name of a distant signal attached to the mast of the main signal
 * @param pMain:	pointer to the main signal
 * @param name:		pointer to the string with the name 
 * @exception		assert on NULL pointer
 */ 
void mainSetDistantName(struct sMain * const pMain, const char * const name)
{
	assert(NULL != pMain);

	memmove(pMain->distantName, name, NAMELEN);
}

/**
 * Function to get the the distant signal of the main
 * @param pMain		pointer to the main signal
 * @return		aspect of the distant signal
 * @exception		assert on NULL pointer
 */
struct sDistant * mainGetDistant(const struct sMain * const pMain)
{
	assert(NULL != pMain);

	return pMain->pDistant;
}

/**
 * Function to set the the aspect of the main signal
 * @param pMain		pointer to the main signal
 * @param pDistant	pointer to the distant signal
 * @exception		assert on NULL pointer
 */
void mainSetDistant(struct sMain * const pMain, struct sDistant * const pDistant)
{
	assert(NULL != pMain);
	
	pMain->pDistant = pDistant;
}

/**
 * Function to get the the aspect of the main
 * @param pMain		pointer to the aspect
 * @return		aspect of the main
 * @exception		assert on NULL pointer
 */
enum eMainAspect mainGetAspect(const struct sMain * const pMain)
{
	assert(NULL != pMain);

	return pMain->eRequestedAspect;
}

/**
 * Function to set the the aspect of the main signal
 * @param pMain			pointer to the main signal
 * @param eRequestedAspect	aspect of the main signal
 * @exception			assert on NULL pointer
 */
void mainSetAspect(struct sMain * const pMain, const enum eMainAspect eRequestedAspect)
{
	assert(NULL != pMain);
	
	if(pMain->eRequestedAspect != eRequestedAspect)
	{
		pMain->eRequestedAspect = eRequestedAspect;
		pMain->ToDo = true;
		m_mainsTodo = true;
	}
}

#ifdef SPDR60
	/**
	 * state machine function
	 * provides the function when being inside the state given in pMain->eState
	 * @param pMain		pointer to the main signal leaving the state
	 */
	static enum eMainAspect mainStateBehaviour(struct sMain * const pMain);
	static enum eMainAspect mainStateBehaviour(struct sMain * const pMain)
	{
		assert(NULL != pMain);

		/* actually no state behaviour. Could be made later to make a nicer 
		 * slow change between aspects
		 */
		return pMain->eRequestedAspect;
	}
#endif

#ifdef DOMINO55
	/**
	 * state machine function
	 * provides the function when being inside the state given in pMain->eState
	 * @param pMain		pointer to the main signal leaving the state
	 */
	static enum eMainAspect mainStateBehaviour(struct sMain * const pMain);
	static enum eMainAspect mainStateBehaviour(struct sMain * const pMain)
	{
		assert(NULL != pMain);

		/* actually no state behaviour. Could be made later to make a nicer 
		 * slow change between aspects
		 */
		return pMain->eRequestedAspect;
	}
#endif

#ifdef SPDR60
	/**
	 * state machine entry function
	 * provides the function when entering the state given in pMain->eState
	 * @param pMain		pointer to the main signal leaving the state
	 */
	static void mainStateEntry(struct sMain * const pMain);
	static void mainStateEntry(struct sMain * const pMain)
	{
		assert(NULL != pMain);

		switch(pMain->eAspect)
		{
		case SIG_MN_UNDEF:
			LOG_STW("Hauptsignal %s ist neu undefiniert", pMain->name);
			break;

		case SIG_MN_HP0:
			LOG_STW("Hauptsignal %s ist neu auf Hp0", pMain->name);
			registerBitSet(pMain->BO_Red);
			registerBitSet(pMain->BO_Desk_Red);
			break;

		case SIG_MN_HP1:
			LOG_STW("Hauptsignal %s ist neu auf Hp1", pMain->name);
			registerBitSet(pMain->BO_Green);
			registerBitSet(pMain->BO_Desk_Green);
			break;
		
		case SIG_MN_HP1_ZS3:
			LOG_STW("Hauptsignal %s ist neu auf Hp1 mit Zs3", pMain->name);
			registerBitSet(pMain->BO_Green);
			registerBitSet(pMain->BO_Zs3);
			registerBitSet(pMain->BO_Desk_Green);
			break;
		
		case SIG_MN_HP2:
			LOG_STW("Hauptsignal %s ist neu auf Hp2", pMain->name);
			registerBitSet(pMain->BO_Green);
			registerBitSet(pMain->BO_Yellow);
			registerBitSet(pMain->BO_Desk_Green);
			break;
		
		case SIG_MN_HP2_ZS3:
			LOG_STW("Hauptsignal %s ist neu auf Hp2 mit Zs3", pMain->name);
			registerBitSet(pMain->BO_Green);
			registerBitSet(pMain->BO_Yellow);
			registerBitSet(pMain->BO_Zs3);
			registerBitSet(pMain->BO_Desk_Green);
			break;
		
		default:
			break;
		}
		/* now give the aspect to the signals that may be attached to this mast	*/
		if(NULL != pMain->pDwarf)
		{
			switch(pMain->eAspect)
			{
			case SIG_MN_UNDEF:
			case SIG_MN_HP0:
				dwarfSetMainClear(pMain->pDwarf, false);
				break;

			default:
				dwarfSetMainClear(pMain->pDwarf, true);
				break;
			}
		}

		if(NULL != pMain->pDistant)
		{
			distantSetMainAspect(pMain->pDistant, pMain->eAspect);
		}
	}
#endif

#ifdef DOMINO55
	/**
	 * state machine entry function
	 * provides the function when entering the state given in pMain->eState
	 * @param pMain		pointer to the main signal leaving the state
	 */
	static void mainStateEntry(struct sMain * const pMain);
	static void mainStateEntry(struct sMain * const pMain)
	{
		assert(NULL != pMain);

		switch(pMain->eAspect)
		{
		case SIG_MN_UNDEF:
			LOG_STW("Hauptsignal %s ist neu undefiniert", pMain->name);
			break;
		case SIG_MN_HALT:
			LOG_STW("Hauptsignal %s ist neu Halt", pMain->name);
			registerBitSet(pMain->BO_Red1);
			registerBitSet(pMain->BO_Desk_Red);
			break;
		case SIG_MN_1:
			LOG_STW("Hauptsignal %s ist neu auf Fahrbegriff 1", pMain->name);
			registerBitSet(pMain->BO_Green1);
			registerBitSet(pMain->BO_Desk_Green);
			break;
		case SIG_MN_2:
			LOG_STW("Hauptsignal %s ist neu auf Fahrbegriff 2", pMain->name);
			registerBitSet(pMain->BO_Green1);
			registerBitSet(pMain->BO_Yellow1);
			registerBitSet(pMain->BO_Desk_Green);
			break;
		case SIG_MN_3:
			LOG_STW("Hauptsignal %s ist neu auf Fahrbegriff 3", pMain->name);
			registerBitSet(pMain->BO_Green1);
			registerBitSet(pMain->BO_Green2);
			registerBitSet(pMain->BO_Desk_Green);
			break;
		case SIG_MN_5:
			LOG_STW("Hauptsignal %s ist neu auf Fahrbegriff 5", pMain->name);
			registerBitSet(pMain->BO_Green1);
			registerBitSet(pMain->BO_Green2);
			registerBitSet(pMain->BO_Green3);
			registerBitSet(pMain->BO_Desk_Green);
			break;
		case SIG_MN_6:
			LOG_STW("Hauptsignal %s ist neu auf Fahrbegriff 6", pMain->name);
			registerBitSet(pMain->BO_Yellow1);
			registerBitSet(pMain->BO_Yellow2);
			registerBitSet(pMain->BO_Desk_Green);
			break;
		default:
			break;
		}
		
		/* now set the signals that can be attached to this mast	*/

		if(NULL != pMain->pDistant)
		{
			distantSetMainAspect(pMain->pDistant, pMain->eAspect);
		}
	}
#endif

#ifdef SPDR60
	/**
	 * state machine exit function
	 * provides the function when leaving the state given in pMain->eState
	 * @param pMain		pointer to the main signal leaving the state
	 */
	static void mainStateExit(struct sMain * const pMain);
	static void mainStateExit(struct sMain * const pMain)
	{
		assert(NULL != pMain);

		registerBitClr(pMain->BO_Green);
		registerBitClr(pMain->BO_Red);
		registerBitClr(pMain->BO_Yellow);
		registerBitClr(pMain->BO_Zs3);
		registerBitClr(pMain->BO_Desk_Red);
		registerBitClr(pMain->BO_Desk_Green);
						 
		return;	/* temporary switched off this comments	*/
		switch(pMain->eAspect)
		{
		case SIG_MN_UNDEF:
			LOG_STW("Hauptsignal %s ist nicht mehr undefiniert", pMain->name);
			break;
		case SIG_MN_HP0:
			LOG_STW("Hauptsignal %s ist nicht mehr auf Hp0", pMain->name);
			break;
		case SIG_MN_HP1:
			LOG_STW("Hauptsignal %s ist nicht mehr auf Hp1", pMain->name);
			break;
		case SIG_MN_HP1_ZS3:
			LOG_STW("Hauptsignal %s ist nicht mehr auf Hp1 mit Zs3", pMain->name);
			break;
		case SIG_MN_HP2:
			LOG_STW("Hauptsignal %s ist nicht mehr auf Hp2", pMain->name);
			break;
		case SIG_MN_HP2_ZS3:
			LOG_STW("Hauptsignal %s ist nicht mehr auf Hp2 mit Zs3", pMain->name);
			break;
		default:
			break;
		}
	}
#endif

#ifdef DOMINO55
	/**
	 * state machine exit function
	 * provides the function when leaving the state given in pMain->eState
	 * @param pMain		pointer to the main signal leaving the state
	 */
	static void mainStateExit(struct sMain * const pMain);
	static void mainStateExit(struct sMain * const pMain)
	{
	//	return;	/* temporary switched off this comments	*/
		assert(NULL != pMain);

		registerBitClr(pMain->BO_Green1);
		registerBitClr(pMain->BO_Green2);
		registerBitClr(pMain->BO_Green3);
		registerBitClr(pMain->BO_Yellow1);
		registerBitClr(pMain->BO_Yellow2);
		registerBitClr(pMain->BO_Red1);
		registerBitClr(pMain->BO_Desk_Red);
		registerBitClr(pMain->BO_Desk_Green);
						 
		switch(pMain->eAspect)
		{
		case SIG_MN_UNDEF:
			LOG_STW("Hauptsignal %s ist nicht mehr undefiniert", pMain->name);
			break;
		case SIG_MN_HALT:
			LOG_STW("Hauptsignal %s ist nicht mehr Halt", pMain->name);
			break;
		case SIG_MN_1:
			LOG_STW("Hauptsignal %s ist nicht mehr auf Fahrbegriff 1", pMain->name);
			break;
		case SIG_MN_2:
			LOG_STW("Hauptsignal %s ist nicht mehr auf Fahrbegriff 2", pMain->name);
			break;
		case SIG_MN_3:
			LOG_STW("Hauptsignal %s ist nicht mehr auf Fahrbegriff 3", pMain->name);
			break;
		case SIG_MN_5:
			LOG_STW("Hauptsignal %s ist nicht mehr auf Fahrbegriff 5", pMain->name);
			break;
		case SIG_MN_6:
			LOG_STW("Hauptsignal %s ist nicht mehr auf Fahrbegriff 6", pMain->name);
			break;
		default:
			break;
		}
	}
#endif

/**
 * Function to let all main signals switch to the desired state
 */
void mainsProcess(void)
{
	struct sMain *		pMain = m_pMain;
	enum eMainAspect	resultAspect;
	int 	i;

	assert(NULL != pMain);

	for(i = 0; i < m_nrMains; i++)
	{
		resultAspect = mainStateBehaviour(pMain);
		if(pMain->eAspect != resultAspect)
		{
			mainStateExit(pMain);
			pMain->eAspect = resultAspect;
			mainStateEntry(pMain);
		}
		pMain++;		
	}	
}
