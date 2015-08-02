/* 
 * This file is part of Raspberry Pi steuert Modelleisenbahn
 * Copyright(C)  2009 - 2015 Kurt Zerzawy www.zerzawy.ch
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
 * Buttons and standard buttons of the desk
 * \author
 * Kurt Zerzawy
 */

/*
 * $Author: kurt $
 * $Date: 2015-05-18 22:31:02 +0200 (Mon, 18. Mai 2015) $
 * $Revision: 2464 $
 */

#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "log.h"
#include "registers.h"
#include "turnout.h"
#include "dwarf.h"
#include "mains.h"
#include "button.h"


static _Bool			m_buttonToDo = true;		/**< true if something to do */
static int			m_buttonNrOfPressed = 0;
static int			m_stdButtonNrOfPressed = 0;
static struct sDwarf *		m_pDwarf1  = NULL;
static struct sDwarf *		m_pDwarf2  = NULL;
static struct sMain *		m_pMains1  = NULL;
static struct sMain *		m_pMains2  = NULL;
static struct sTurnout *	m_pTurnout1 = NULL;
static struct sButton *		m_pButton1 = NULL;
static struct sButton *		m_pButton2 = NULL;

static struct sStandardButton m_StandardButton[TASTE_ANZ_SPEZIAL];

static unsigned short m_nrButton = 0;	/**< Number of pushbuttons	*/
#ifdef SPDR60
	static const unsigned short 	FRTTIME = 50;		/**< active time of the FRT key 50cyles = 5s	*/
	static const unsigned short 	FHTTIME = 50;		/**< active time of the FHT key 50cyles = 5s	*/
	static const unsigned short 	UFGTTIME = 50;		/**< active time of the UfGT key 50cyles = 5s	*/
	static unsigned short 		m_FRTTimer = 0;		/**< timer of the FRT key	*/
	static unsigned short 		m_FHTTimer = 0;		/**< timer of the FHT key	*/
	static unsigned short 		m_UfGTTimer = 0;	/**< timer of the UfGT key	*/
#endif

/*@owned@*/ /*@null@*/ struct sButton * m_pButton = NULL;	/**< Pointer to the first pushbotton structure	*/

/**
 * Constructor for the pushbuttons.
 *
 * Some of the pushbuttons are already predefined. This are always needed. 
 *
 * In the logic it is controlled if two or less pushbuttons are pushed 
 * simultaneously. Then the tasks are given to the respective parts of the 
 * switchboard.
 *
 * @param nrButton	Number of the pushbuttons to be allocated
 * @exception           exit if no RAM available
 */
/*@maynotreturn@*/
void button(const unsigned short nrButton) 
{
	struct sButton *	ptr;
	unsigned short		i;

	LOG_INF("started");
	m_nrButton = nrButton;

	m_pButton = malloc(nrButton * sizeof(struct sButton));
	if(NULL == m_pButton) 
	{
		/* error allocating */
		LOG_ERR("cannot allocate RAM");
		exit(EXIT_FAILURE);
	}

	m_buttonNrOfPressed	= 0;
	m_stdButtonNrOfPressed 	= 0;

	for(i = 0; i < TASTE_ANZ_SPEZIAL; i++) 
	{
		m_StandardButton[i].name[0]	= '\0';
		m_StandardButton[i].token	= tokNone;
		m_StandardButton[i].BI_Button	= NONEVALUE;
		m_StandardButton[i].pressed	= false;
		m_StandardButton[i].wasPressed	= false;
	}
	
	#ifdef SPDR60
		m_FRTTimer = 0;
	#endif
	ptr = m_pButton;
	for (i = 0; i < m_nrButton; i++)
	{
		ptr->name[0]	= '\0';
		ptr->BI_Button	= NONEVALUE;
		ptr->pressed	= false;
		ptr->wasPressed	= false;
		ptr++;
	}
	LOG_INF("ended");
}

/**
 * Destruktor for buttons
 */
void buttonDestroy(void) 
{
	LOG_INF("started");
	free(m_pButton);
	m_pButton = NULL;
	LOG_INF("ended");
}

/**
 * function to read a single pushbutton from HW
 *
 * the function is also checking for changes int the state of
 * the buttons and gives a log message if something has changed
 * the button member variable m_buttonToDo is set if change happened
 *
 * @param ptr	pointer to the button
 * @return	true if pressed
 * @exception	on NULL pointer
 */
_Bool buttonSingleReadHW(struct sButton * const ptr)
{
	assert(NULL != ptr);

	ptr->pressed = registerBitGet(ptr->BI_Button);
	if(ptr->pressed) 
	{
		if(! ptr->wasPressed) 
		{
			LOG_STW("Taste %s wurde gedrueckt", ptr->name);
			m_buttonToDo = true;
		}
	}
	else 
	{
		if(ptr->wasPressed) 
		{
			LOG_STW("Taste %s wurde losgelassen", ptr->name);
			m_buttonToDo = true;
		}
	}
	ptr->wasPressed = ptr->pressed;
	return ptr->pressed;
}
	
/**
 * HW read function for pushbuttons.
 *
 * The function is checking for changes in the state of the pushbuttons
 * and returns true if something has changed.
 * @return	true if new pushbutton state
 */
_Bool buttonReadHW(void) 
{
	struct sButton *	ptr = m_pButton;
	unsigned short	 	i;
	_Bool			pressed;
	
	m_buttonToDo = false;
	m_stdButtonNrOfPressed = 0;
	for(i = 0; i < TASTE_ANZ_SPEZIAL; i++) 
	{
		pressed = registerBitGet(m_StandardButton[i].BI_Button);
		if(pressed) 
		{
			m_stdButtonNrOfPressed++;
			if(! m_StandardButton[i].wasPressed) 
			{
				LOG_STW("Taste %s wurde gedrueckt", m_StandardButton[i].name);
				m_buttonToDo = true;
			}
		}
		else 
		{
			if(m_StandardButton[i].wasPressed) 
			{
				LOG_STW("Taste %s wurde losgelassen", m_StandardButton[i].name);
				m_buttonToDo = true;
			}
		} 
		m_StandardButton[i].pressed = pressed;
		m_StandardButton[i].wasPressed = pressed;
	}

	/* read the normal keys */
	assert (NULL != ptr);

	m_buttonNrOfPressed	= 0;
	m_pButton1		= NULL;
	m_pButton2		= NULL;


	for (i = 0; i < m_nrButton; i++) 
	{
		if(buttonSingleReadHW(ptr))
		{
			m_buttonNrOfPressed++;
			m_pButton2 = m_pButton1;
			m_pButton1 = ptr;
		}
		ptr++;
	}
	return m_buttonToDo;
}

/**
 * Initialiser function for the pushbuttons.
 *
 * Also sets the correct historic values.
 */
void buttonInit(void) 
{
	;
}

/**
 * Processes the pushbuttons.
 */
void buttonProcess(void) 
{
	/*@dependent@*/ struct sButton * buttonArray[2];

	#ifdef SPDR60
		/* now decrement the timer of FRT button	*/
		if(stdButtonFRTTimerIsRunning())
		{
			if(1 == m_FRTTimer)
			{
				LOG_STW("Zeitrelais der FRT erreicht 0");
			}
			m_FRTTimer--;
		}

		/* decrement of FHT button	*/
		if(stdButtonFHTTimerIsRunning())
		{
			if(1 == m_FHTTimer)
			{
				LOG_STW("Zeitrelais der FHT erreicht 0");
			}
			m_FHTTimer--;
		}

		/* decrement of UfGT button	*/
		if(stdButtonUfGTTimerIsRunning())
		{
			if(1 == m_UfGTTimer)
			{
				LOG_STW("Zeitrelais der UfGT erreicht 0");
			}
			m_UfGTTimer--;
		}
	#endif

	if(2 < m_stdButtonNrOfPressed + m_buttonNrOfPressed) 
	{
		/* there are more than 2 pushbuttons pushed, 
		 * do not process this error
		 */
		m_buttonToDo = false;
		return;
	}

	#ifdef SPDR60
		if((1 == m_stdButtonNrOfPressed) && + (0 == m_buttonNrOfPressed))
		{
			/* set the FRT timer if the button is pushed	*/
			if(m_StandardButton[TASTE_FRT].pressed)
			{
				m_FRTTimer = FRTTIME;
			}

			/* set the FHT timer if the button is pushed	*/
			if(m_StandardButton[TASTE_FHT].pressed)
			{
				m_FHTTimer = FHTTIME;
			}

			/* set the UfGT timer if the button is pushed	*/
			if(m_StandardButton[TASTE_UfGT].pressed)
			{
				m_UfGTTimer = UFGTTIME;
			}
			m_buttonToDo = false;
			return;
		}
	#endif
	buttonArray[0] = NULL;
	buttonArray[1] = NULL;

	if(! m_buttonToDo) 
	{
		/* nothing to do, return */
		return;
	}

	/**
	 * for all the functions following note that only 0 - 2 buttons
	 * on the desk can be pushed
	 */

	/**************************
	 * dwarfs and shuntroutes *
	 **************************/

	/* If 2 dwarf buttons of them are pressed, it could be a shuntroute.	*/
	switch(dwarfNrButtons(& m_pDwarf1, & m_pDwarf2))
	{
	case 2:
		/**
		 * all cases with 2 dwarf buttons pressed
		 */
		#ifdef SPDR60
			if(stdButtonFRTTimerIsRunning())
			{
				/* the shuntroute shall be cancelled	*/
				shuntrouteCancel(m_pDwarf1, m_pDwarf2);
			}
			else
			{
				/* the shuntroute shall be established	*/
				shuntrouteTwoButtons(m_pDwarf1, m_pDwarf2);
			}
		#endif

		#ifdef DOMINO55
			/* the shuntroute shall be established	*/
			shuntrouteTwoButtons(m_pDwarf1, m_pDwarf2);
		#endif

		m_buttonToDo = false;
		return;

	case 1:
		/**
		 * all cases with 1 dwarf button pressed
		 */
		#ifdef SPDR60
			if(m_StandardButton[TASTE_HaGT].pressed)
			{
				/* the dwarf shall be set to halt	*/
				dwarfSetAspect(m_pDwarf1, SIG_DW_SH0);
			}
			m_buttonToDo = false;
			return;
		#endif

		#ifdef DOMINO55
			if(m_StandardButton[TASTE_BETRAUFL].pressed)
			{
				/* possibly a switchroute shall be reset 	*/
				shuntrouteCancelDest(m_pDwarf1);

				m_buttonToDo = false;
				return;
			}

			if(1 ==  m_stdButtonNrOfPressed + m_buttonNrOfPressed)
			{
				/* now only this dwarf button is pressed. This is
				 * necessary for DOMINO55 to know because the shunt-
				 * route can be cancelled as long as the destination
				 * button is not released
				 */
				shuntrouteOneButton(m_pDwarf1);

				m_buttonToDo = false;
				return;
			}
		#endif

	default:
		break;
	}

	/***************************
	 * main signals and routes *
	 ***************************/

	/* If 2 main signal buttons of them are pressed, it could be a route.	*/
	switch(mainNrButtons(& m_pMains1, & m_pMains2))
	{
	case 2:
		/**
		 * all cases with 2 main signal buttons pressed
		 */
		#ifdef SPDR60
			if(stdButtonFHTTimerIsRunning())
			{
				/* the shuntroute shall be cancelled	*/
				routeCancel(m_pMains1, m_pMains2);
			}
			else
			{
				/* the shuntroute shall be established	*/
				routeTwoButtons(m_pMains1, m_pMains2, stdButtonUfGTTimerIsRunning());
			}
		#endif

		#ifdef DOMINO55
			/* the shuntroute shall be established	*/
			routeTwoButtons(m_pMains1, m_pMains2);
		#endif

		m_buttonToDo = false;
		return;

	case 1:
		/**
		 * all cases with 1 dwarf button pressed
		 */
		#ifdef SPDR60
			if(m_StandardButton[TASTE_HaGT].pressed)
			{
				/* the dwarf shall be set to halt	*/
				dwarfSetAspect(m_pDwarf1, SIG_DW_SH0);
			}
			m_buttonToDo = false;
			return;
		#endif

		#ifdef DOMINO55
			if(m_StandardButton[TASTE_BETRAUFL].pressed)
			{
				/* possibly a switchroute shall be reset 	*/
				shuntrouteCancelDest(m_pMains1);

				m_buttonToDo = false;
				return;
			}

			if(1 ==  m_stdButtonNrOfPressed + m_buttonNrOfPressed)
			{
				/* now only this dwarf button is pressed. This is
				 * necessary for DOMINO55 to know because the shunt-
				 * route can be cancelled as long as the destination
				 * button is not released
				 */
				shuntrouteOneButton(m_pMains1);

				m_buttonToDo = false;
				return;
			}
		#endif

	default:
		break;
	}

	/************
	 * turnouts *
	 ************/

	if(1 == turnoutNrButtons(& m_pTurnout1)) 
	{
		/**
		 * cases for turnouts
		 *  only a turnout button with a senseful control button is handled
		 */

		/* now one of the buttons is a turnout button	*/
		#ifdef DOMINO55
			if(m_StandardButton[TASTE_STEUERUNG].pressed)
			{
				/* now the turnout button and the button Steuerung
				 * are pushed, thus all buttons found
				 * now operate the turnout manually, if possible
				 */
				/*@void@*/ turnoutOperateManually(m_pTurnout1);
				m_buttonToDo = false;
				return;
			}
		#endif

		#ifdef SPDR60
			if(m_StandardButton[TASTE_WGT].pressed)
			{
				/* now the turnout button and the button WGT
				 * are pushed, thus all buttons found
				 * now operate the turnout manually, if possible
				 */
				/*@void@*/ turnoutOperateManually(m_pTurnout1);
				m_buttonToDo = false;
				return;
			}
			/* TODO now the cases with a turnout key together with another key
			 * like Isolierumgehung, Sperrtaste and so on
			 */
			m_buttonToDo = false;
			return;
		#endif
	}
}

/**
 * Function to get the state of a key.
 *  the values are valid after calling buttonReadHW() 
 * @param ptr		pointer to the button
 * @return		true if pushed
 * @exception		Assert on NULL pointer
 */
_Bool buttonIsPressed(const struct sButton * const ptr)
{
	assert(NULL != ptr);

	return(ptr->pressed);
}

/**
 * Function to find the pointer of a button by giving the name
 * @param name		name of the button to be locked for
 * @return		pointer of the found button
 * @exception		A parser assert is made if not found
 */
struct sButton * buttonFind(const char * const name) 
{
	unsigned short		i = 0;
	struct sButton *	ptr = NULL;
	char			temp[128];

	i = 0;
	while(NULL != (ptr = buttonGet(i)))
	{ 	
		if(0 == (strcmp(ptr->name, name))) 
			return ptr;
		i++;
	}
	sprintf(temp, "Taste %s nicht gefunden", name);
	parserExit(temp);
	return NULL;
}

/**
 * Function to get the name of a button
 * @param ptr		pointer to the button
 * @param name		pointer to the string filled out with the name
 * 			of the button. Must be at least NAMELEN long
 * @return		true, if successful
 * @exception		Assert on NULL pointer
 */ 
_Bool buttonGetName(const struct sButton * const ptr, char * const name)
{
	assert(NULL != ptr);

	name[NAMELEN - 1] = '\0';
	memmove(name, ptr->name, NAMELEN - 1);
	return true;
}

/**
 * Function to set the name of a button
 * @param ptr		pointer to the button
 * @param name		pointer to the string with the name 
 * @exception		Assert on NULL pointer
 */ 
void buttonSetName(struct sButton * const ptr, const char * const name)
{
	assert(NULL != ptr);

	memmove(ptr->name, name, NAMELEN);
}

/**
 * Function to get the pointer by giving the number of the button
 * @param i		index, starting with 0
 * @return		Pointer to this button, NULL if not existing
 */ 
struct sButton * buttonGet(const unsigned short i)
{
	if(i >= m_nrButton)
	{
		return NULL;
	}
	else
	{
		return m_pButton + i;
	}
}

/**
 * Function to get the number of buttons 
 * @return			Number of buttons
 */
unsigned short buttonGetNr(void)
{
	return m_nrButton;
}

/**
 * Function to get the register of the button
 * @param ptr		pointer to the button
 * @return		Register handle of the button
 * @exception	Assert on NULL pointer
 */
unsigned short buttonGetBI_Button(const struct sButton * const ptr)
{
	assert(NULL != ptr);

	return ptr->BI_Button;
}

/**
 * Function to set the register of the button
 * @param ptr		pointer to the button
 * @param handle	register handle of the button
 * @exception		Assert on NULL pointer
 */
void buttonSetBI_Button(struct sButton * const ptr, const unsigned short handle)
{
	assert(NULL != ptr);

	ptr->BI_Button = handle;
}

/**
 * Function to get the state of a key
 *
 * The values are valid after calling buttonReadHW() 
 * @param index:	index of the stdButton
 * @return		true if pushed
 * @exception		Assert on NULL pointer
 */
_Bool stdButtonIsPressed(const unsigned short index)
{
	return(m_StandardButton[index].pressed);
}

#ifdef SPDR60
	/**
	 * Function to read the state of the timer (monoflop). After the
	 * FRT key is pushed, this timer is running for FRTTIME cycles.
	 * with stdButtonFRTTimerReset this timer can be reset.
	 * @return	true if timer is still running
	 */
	_Bool stdButtonFRTTimerIsRunning(void)
	{
		return (m_FRTTimer > 0);
	}

	/**
	 * Function to reset and stop the FRT key timer.
	 */
	void stdButtonFRTTimerReset(void)
	{
		LOG_STW("Zeitrelais der FRT wird auf 0 gesetzt");

		m_FRTTimer = 0;
	}

	/**
	 * Function to read the state of the timer (monoflop). After the
	 * FHT key is pushed, this timer is running for FHTTIME cycles.
	 * with stdButtonFHTTimerReset this timer can be reset.
	 * @return	true if timer is still running
	 */
	_Bool stdButtonFHTTimerIsRunning(void)
	{
		return (m_FHTTimer > 0);
	}

	/**
	 * Function to reset and stop the FHT key timer.
	 */
	void stdButtonFHTTimerReset(void)
	{
		LOG_STW("Zeitrelais der FHT wird auf 0 gesetzt");

		m_FHTTimer = 0;
	}

	/**
	 * Function to read the state of the timer (monoflop). After the
	 * UfGT key is pushed, this timer is running for UFGTTIME cycles.
	 * with stdButtonUfGTTimerReset this timer can be reset.
	 * @return	true if timer is still running
	 */
	_Bool stdButtonUfGTTimerIsRunning(void)
	{
		return (m_UfGTTimer > 0);
	}

	/**
	 * Function to reset and stop the UfGT key timer.
	 */
	void stdButtonUfGTTimerReset(void)
	{
		LOG_STW("Zeitrelais der UfGT wird auf 0 gesetzt");

		m_UfGTTimer = 0;
	}
#endif

/**
 * Function to find the pointer of a stdButton by giving the name
 * @param name		name of the stdButton to be locked for
 * @return		Index of the found stdButton
 * @exception		A parser assert is made if not found
 */
unsigned short stdButtonFind(const char * const name) 
{
        unsigned short		i;
		short				button;
        char				temp[128];

		i = 0;
		while(0 <= (button = stdButtonGet(i)))
		{ 
			if(0 == (strcmp(m_StandardButton[button].name, name))) 
				return button; 
			i++;
        }
        sprintf(temp, "%s nicht gefunden", name);
        parserExit(temp);
        return 0;
}

/**
 * Function to get the name of a stdButton
 * @param button	index of the stdButton
 * @param name		pointer to the string filled out with the name
 * 			of the stdButton. Must be at least NAMELEN long
 * @return		true, if successful
 * @exception		Assert on invalid range
 */ 
_Bool stdButtonGetName(const unsigned short button, char * const name)
{
	assert(TASTE_ANZ_SPEZIAL > button);

	name[NAMELEN - 1] = '\0';
	memmove(name, m_StandardButton[button].name, NAMELEN - 1);
	return true;
}

/**
 * Function to set the name of a stdButton
 * @param button	index of the stdButton
 * @param name		pointer to the string with the name 
 * @exception		Assert on invalid range
 */ 
void stdButtonSetName(const unsigned short button, const char * const name)
{
	assert(TASTE_ANZ_SPEZIAL > button);

	memmove(m_StandardButton[button].name, name, NAMELEN);
}

/**
 * Function to get the pointer by giving the number of the stdButton
 * @param i		index, starting with 0
 * @return		Index of this stdButton, NULL if not existing
 */ 
short stdButtonGet(const unsigned short i)
{
	if(i >= TASTE_ANZ_SPEZIAL)
	{
		return -1;
	}
	else
	{
		return i;
	}
}

/**
 * Function to get the number of stdButtons 
 * @return			Number of stdButtons
 */
unsigned short stdButtonsGetNr(void)
{
	return TASTE_ANZ_SPEZIAL;
}

/**
 * Function to get the register of the stdButton
 * @param button	index of the stdButton
 * @return		Register handle of the stdButton
 * @exception		Assert on invalid range
 */
unsigned short stdButtonGetBI_Button(const unsigned short button)
{
	assert(TASTE_ANZ_SPEZIAL > button);

	return m_StandardButton[button].BI_Button;
}

/**
 * Function to set the register of the stdButton
 * @param button	index of the stdButton
 * @param handle	register handle of the stdButton
 * @exception		Assert on invalid range
 */
void stdButtonSetBI_Button(const unsigned short button, const unsigned short handle)
{
	assert(TASTE_ANZ_SPEZIAL > button);

	m_StandardButton[button].BI_Button = handle;
}

/**
 * Function to set the token of the stdButton
 * @param button	index of the stdButton
 * @param token		token of the stdButton
 * @exception		Assert on invalid range
 */
void stdButtonSetToken(const unsigned short button, const enum eToken token)
{
	assert(TASTE_ANZ_SPEZIAL > button);

	m_StandardButton[button].token = token;
}

/**
 * Function to get the token of the stdButton
 * @param button	index of the stdButton
 * @return		Token of the stdButton
 * @exception		Assert on invalid range
 */
enum eToken stdButtonGetToken(const unsigned short button)
{
	assert(TASTE_ANZ_SPEZIAL > button);

	return m_StandardButton[button].token;
}

