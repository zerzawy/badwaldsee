/*
 * This file is part of Raspberry Pi steuert Modelleisenbahn
 * Copyright(C)  2015 Kurt Zerzawy www.zerzawy.ch
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
 * Distant signals (Hauptsignale)
 * \author
 * Kurt Zerzawy
 */

/*
 * $Author: kurt $
 * $Date: 2015-05-14 20:52:29 +0200 (Don, 14. Mai 2015) $
 * $Revision: 2451 $
 */

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "log.h"
#include "registers.h"
#include "button.h"
#include "distant.h"


/*@only@*/ /*@null@*/ static struct sDistant * m_pDistants = NULL;	/**< Pointer to the first distant signal	*/
static unsigned short 		m_nrDistants = 0;			/**< Number of distant signals	*/
static _Bool			m_distantsTodo = true;			/**< something to do for distant signals	*/


/**
 * Constructor for the distant
 * @param nr		number of distant signals to be created
 * @exception       	Calls exit if no RAM available
 */
void distants(const unsigned short nr) 
{
	LOG_INF("started");
	
	m_nrDistants = nr;
	m_pDistants = malloc(m_nrDistants * sizeof(struct sDistant));
	if(NULL == m_pDistants) 
	{
		/* error allocating */
		LOG_ERR("cannot allocate RAM for distant signals");
		exit(EXIT_FAILURE);
	}
	distantsInit();

	LOG_INF("ended");
}

/**
 * Destructor for the distant signals
 */
void distantsDestroy(void) 
{
	LOG_INF("started");
	if(NULL == m_pDistants) 
	{
		LOG_INF("zero pointer, beende");
		return;
	}
	LOG_INF("release RAM of distant signals");
	free(m_pDistants);
	m_pDistants = NULL;
	LOG_INF("ended");
}

void distantsInit(void)
{
	struct sDistant * pDistant = m_pDistants;
	unsigned short  i;

	LOG_INF("started");

	m_distantsTodo = true;
	for(i = 0; i < m_nrDistants; i++)
	{
		pDistant->aspect	= SIG_DI_UNDEF;
		pDistant->state		= SIG_DI_UNDEF;

		pDistant->name[0] = '\0';
		#ifdef SPDR60
			pDistant->aspect		= SIG_DI_VR0;
			pDistant->BO_Yellow13		= NONEVALUE;
			pDistant->BO_Yellow11		= NONEVALUE;
			pDistant->BO_Green14		= NONEVALUE;
			pDistant->BO_Green12		= NONEVALUE;
			pDistant->BO_Zs3v		= NONEVALUE;

		#endif

		#ifdef DOMINO55
			pDistant->aspect		= SIG_DI_WARN;
			pDistant->BO_Yellow11		= NONEVALUE;
			pDistant->BO_Yellow13		= NONEVALUE;
			pDistant->BO_Green14		= NONEVALUE;
			pDistant->BO_Green12		= NONEVALUE;
			pDistant->BO_Green15		= NONEVALUE;
		#endif

		pDistant->pMain		= NULL;
		pDistant->mainAspect		= SIG_MN_UNDEF;

		pDistant->BO_Desk_Yellow	= NONEVALUE;
		pDistant->BO_Desk_Green		= NONEVALUE;

		pDistant->Todo	= true;
		pDistant++;
	}		
	LOG_INF("ended");
}

/**
 * Function to find the pointer of a distant by giving the name
 * @param name		name of the distant to be locked for
 * @return		pointer of the found distant
 * @exception		a parser assert is made if not found
 */
struct sDistant * distantFind(const char * const name) 
{
        unsigned short		i = 0;
	struct sDistant *	pDistants = NULL;
	char			temp[128];

	while(NULL != (pDistants = distantGet(i)))
	{ 
		if(0 == (strcmp(pDistants->name, name))) 
		{
			return pDistants;
		}
		i++;
        }
        sprintf(temp, "Vorsignal %s nicht gefunden", name);
        parserExit(temp);
        return NULL;
}

/**
 * Function to get the pointer by giving the number of the distant
 * @param i		index, starting with 0
 * @return		pointer to this distant, NULL if not existing
 */ 
struct sDistant * distantGet(const unsigned short i)
{
	if(i >= m_nrDistants)
	{
		return NULL;
	}
	else
	{
		return m_pDistants + i;
	}
}

/**
 * Function to get the number of distant signals 
 * @return		Number of distant signals
 */
unsigned short distantsGetNr(void)
{
	return m_nrDistants;
}

/**
 * Function to get the name of a distant signal
 * @param pDistant:	pointer to the distant signal
 * @param name:		pointer to the string filled out with the name
 * 			of the distant signal. Must be at least NAMELEN long
 * @return		true, if successful
 * @exception		assert on NULL pointer
 */ 
_Bool distantGetName(const struct sDistant * const pDistant, /*@out@*/ char * const name)
{
	assert(NULL != pDistant);

	name[NAMELEN - 1] = '\0';
	memmove(name, pDistant->name, NAMELEN - 1);
	return true;
}

/**
 * Function to set the name of a distant signal
 * @param pDistant:	pointer to the distant signal
 * @param name:		pointer to the string with the name 
 * @exception		assert on NULL pointer
 */ 
void distantSetName(struct sDistant * const pDistant, const char * const name)
{
	assert(NULL != pDistant);

	memmove(pDistant->name, name, NAMELEN);
}

/**
 * Function to get the register of the yellow signal LED 13
 * @param pDistant	pointer to the distant signal
 * @return		register handle of yellow signal LED 13
 * @exception		assert on NULL pointer
 */
unsigned short distantGetBO_Yellow13(const struct sDistant * const pDistant)
{
	assert(NULL != pDistant);

	return pDistant->BO_Yellow13;
}

/**
 * Function to set the register of the yellow signal LED 13
 * @param pDistant	pointer to the distant signal 
 * @param handle	register handle of the yellow signal LED 13
 * @exception		assert on NULL pointer
 */
void distantSetBO_Yellow13(struct sDistant * const pDistant, const unsigned short handle)
{
	assert(NULL != pDistant);

	pDistant->BO_Yellow13 = handle;
}

/**
 * Function to get the register of the yellow signal LED 11
 * @param pDistant	pointer to the distant signal
 * @return		register handle of yellow signal LED 11
 * @exception		assert on NULL pointer
 */
unsigned short distantGetBO_Yellow11(const struct sDistant * const pDistant)
{
	assert(NULL != pDistant);

	return pDistant->BO_Yellow11;
}

/**
 * Function to set the register of the yellow signal LED 11
 * @param pDistant	pointer to the distant signal 
 * @param handle	register handle of the yellow signal LED 11
 * @exception		assert on NULL pointer
 */
void distantSetBO_Yellow11(struct sDistant * const pDistant, const unsigned short handle)
{
	assert(NULL != pDistant);

	pDistant->BO_Yellow11 = handle;
}

/**
 * Function to get the register of the green signal LED 14
 * @param pDistant	pointer to the distant signal
 * @return		register handle of green signal LED 14
 * @exception		assert on NULL pointer
 */
unsigned short distantGetBO_Green14(const struct sDistant * const pDistant)
{
	assert(NULL != pDistant);

	return pDistant->BO_Green14;
}

/**
 * Function to set the register of the green signal LED 14
 * @param pDistant	pointer to the distant signal 
 * @param handle	register handle of the green signal LED 14
 * @exception		assert on NULL pointer
 */
void distantSetBO_Green14(struct sDistant * const pDistant, const unsigned short handle)
{
	assert(NULL != pDistant);

	pDistant->BO_Green14 = handle;
}

/**
 * Function to get the register of the green signal LED 12
 * @param pDistant	pointer to the distant signal
 * @return		register handle of green signal LED 12
 * @exception		assert on NULL pointer
 */
unsigned short distantGetBO_Green12(const struct sDistant * const pDistant)
{
	assert(NULL != pDistant);

	return pDistant->BO_Green12;
}

/**
 * Function to set the register of the green signal LED 12
 * @param pDistant	pointer to the distant signal 
 * @param handle	register handle of the green signal LED 12
 * @exception		assert on NULL pointer
 */
void distantSetBO_Green12(struct sDistant * const pDistant, const unsigned short handle)
{
	assert(NULL != pDistant);

	pDistant->BO_Green12 = handle;
}


#ifdef SPDR60
	/**
	 * Function to get the register of the Zs3v LED
	 * @param pDistant	pointer to the distant signal
	 * @return		register handle of Zs3v LED
	 * @exception		assert on NULL pointer
	 */
	unsigned short distantGetBO_Zs3v(const struct sDistant * const pDistant)
	{
		assert(NULL != pDistant);

		return pDistant->BO_Zs3v;
	}

	/**
	 * Function to set the register of the Zs3v LED
	 * @param pDistant	pointer to the distant signal 
	 * @param handle	register handle of the Zs3v LED
	 * @exception		assert on NULL pointer
	 */
	void distantSetBO_Zs3v(struct sDistant * const pDistant, const unsigned short handle)
	{
			assert(NULL != pDistant);

			pDistant->BO_Zs3v = handle;
	}

	/**
	 * Function to get the register of the yellow LED on the switchboard
	 * @param pDistant	pointer to the distant signal
	 * @return		register handle of the yellow LED on the switchboard
	 * @exception		assert on NULL pointer
	 */
	unsigned short distantGetBO_Desk_Yellow(const struct sDistant * const pDistant)
	{
		assert(NULL != pDistant);

		return pDistant->BO_Desk_Yellow;
	}

	/**
	 * Function to set the register of the yellow LED on the switchboard
	 * @param pDistant	pointer to the distant signal
	 * @param handle	register handle of the yellow LED on the switchboard
	 * @exception		assert on NULL pointer
	 */
	void distantSetBO_Desk_Yellow(struct sDistant * const pDistant, const unsigned short handle)
	{
		assert(NULL != pDistant);

		pDistant->BO_Desk_Yellow = handle;
	}

	/**
	 * Function to get the register of the green LED on the switchboard
	 * @param pDistant	pointer to the distant signal
	 * @return		register handle of the green LED on the switchboard
	 * @exception		assert on NULL pointer
	 */
	unsigned short distantGetBO_Desk_Green(const struct sDistant * const pDistant)
	{
		assert(NULL != pDistant);

		return pDistant->BO_Desk_Green;
	}

	/**
	 * Function to set the register of the green LED on the switchboard
	 * @param pDistant	pointer to the distant signal
	 * @param handle	register handle of the green LED on the switchboard
	 * @exception		assert on NULL pointer
	 */
	void distantSetBO_Desk_Green(struct sDistant * const pDistant, const unsigned short handle)
	{
		assert(NULL != pDistant);

		pDistant->BO_Desk_Green = handle;
	}
#endif

#ifdef DOMINO55
	/** 
	 * Function to get the register of the green 15 LED
	 * @param pDistant	pointer to the distant signal
	 * @return		register handle of green 15 LED
	 * @exception		assert on NULL pointer
	 */
	unsigned short distantGetBO_Green15(const struct sDistant * const pDistant)
	{
		assert(NULL != pDistant);

		return pDistant->BO_Green15;
	}

	/**
	 * Function to set the register of the green 15 LED
	 * @param pDistant	pointer to the distant signal 
	 * @param handle	register handle of the green 15 LED
	 * @exception		assert on NULL pointer
	 */
	void distantSetBO_Green15(struct sDistant * const pDistant, const unsigned short handle)
	{
		assert(NULL != pDistant);

		pDistant->BO_Green15 = handle;
	}

	/**
	 * Function to set the parameter telling that the distant shall be dark
	 *  on main signal on the same mast Halt
	 * @param pDistant	pointer to the distant signal
	 * @param dark		to be set true if the distant shall become dark
	 * @exception		assert on NULL pointer
	 */
	void distantSetDarkOnHalt(struct sDistant * const pDistant, const _Bool dark)
	{
		assert(NULL != pDistant);

		pDistant->darkOnMainHalt = dark;
	}

	/**
	 * Function to get the parameter telling that the distant shall be dark
	 * @param pDistant	pointer to the distant signal
	 * @return		true if the distant shall become dark
	 * @exception		assert on NULL pointer
	 */
	_Bool distantGetDarkOnHalt(const struct sDistant * const pDistant)
	{
		assert(NULL != pDistant);

		return pDistant->darkOnMainHalt;
	}
#endif

/**
 * Function to set  the aspect of the main signal on the same mast
 *  on main signal on the same mast Halt
 * @param pDistant	pointer to the distant signal
 * @param aspect	aspect of the main signal on the same mast
 * @exception		assert on NULL pointer
 */
void distantSetMainAspect(struct sDistant * const pDistant, const enum eMainAspect aspect)
{
	assert(NULL != pDistant);
	
	if(pDistant->mainAspect != aspect)
	{
		m_distantsTodo	= true;
		pDistant->Todo	= true;
		pDistant->mainAspect = aspect;
	}
}

/**
 * Function to get the aspect of the main signal on the same mast
 * @param pDistant	pointer to the distant signal
 * @return		aspect of the main signal on the same mast
 * @exception		assert on NULL pointer
 */
enum eMainAspect distantGetMainAspect(const struct sDistant * const pDistant)
{
	assert(NULL != pDistant);

	return pDistant->mainAspect;
}

/**
 * Function to set the the main signal on the same mast
 * @param pDistant	pointer to the distant signal
 * @param pMain	pointer to the main signal on the same mast
 * @exception		assert on NULL pointer
 */
void distantSetMain(struct sDistant * const pDistant, const struct sMain * const pMain)
{
	assert(NULL != pDistant);
	
	pDistant->pMain = pMain;
}

/**
 * Function to get the aspect of the main signal on the same mast
 * @param pDistant	pointer to the distant signal
 * @return		pointer to the main signal on the same mast
 * @exception		assert on NULL pointer
 */
struct sMain * distantGetMain(const struct sDistant * const pDistant)
{
	assert(NULL != pDistant);

	return pDistant->pMain;
}

/**
 * Function to get the the aspect of the distant
 * @param pDistant	pointer to the aspect
 * @return		aspect of the distant
 * @exception		assert on NULL pointer
 */
enum eDistantsAspect distantGetAspect(const struct sDistant * const pDistant)
{
	assert(NULL != pDistant);

	return pDistant->aspect;
}

/**
 * Function to set the the aspect of the distant signal
 * @param pDistant	pointer to the distant signal
 * @param aspect	aspect of the distant signal
 * @exception		assert on NULL pointer
 */
void distantSetAspect(struct sDistant * const pDistant, const enum eDistantsAspect aspect)
{
	assert(NULL != pDistant);
	
	if(pDistant->aspect != aspect)
	{
		pDistant->aspect = aspect;
		pDistant->Todo = true;
		m_distantsTodo = true;
	}
}

#ifdef SPDR60
	/**
	 * state machine function
	 * provides the function when being inside the state given in pDistant->eState
	 * @param pDistant		pointer to the distant signal leaving the state
	 */
	static enum eDistantsAspect distantStateBehaviour(struct sDistant * const pDistant);
	static enum eDistantsAspect distantStateBehaviour(struct sDistant * const pDistant)
	{
		assert(NULL != pDistant);

		if(pDistant->Todo)
		{
			if(NULL != pDistant->pMain)
			{
				/* the main signal on the same mast exists	*/
				switch(pDistant->mainAspect)
				{
				case SIG_MN_UNDEF:
				case SIG_MN_HP0:
					pDistant->state = SIG_DI_BLANKED;
					break;

				default:
					pDistant->state = pDistant->aspect;
					break;
				}
			}
			else
			{
				pDistant->state = pDistant->aspect;
			}

			pDistant->Todo	= false;
		}
		return pDistant->state;
	}
#endif

#ifdef DOMINO55
	/**
	 * state machine function
	 * provides the function when being inside the state given in pDistant->eState
	 * @param pDistant		pointer to the distant signal leaving the state
	 */
	static enum eDistantsAspect distantStateBehaviour(struct sDistant * const pDistant);
	static enum eDistantsAspect distantStateBehaviour(struct sDistant * const pDistant)
	{
		assert(NULL != pDistant);

		if(pDistant->Todo)
		{
			if(NULL != pDistant->pMain)
			{
				/* attached main signal exists 
				 * Swiss speciality: distant signal can be parameterized so 
				 *  it is dark when main signal on same mast is Halt
				 *  the distant signal is generally dark when main signal on
				 *  same mast signals 'Fahrbegriff 6'
				 */
				switch(pDistant->mainAspect)
				{
				case SIG_MN_UNDEF:
				case SIG_MN_HALT:
					/* now check if it shall be blanked or not	*/
					/* the main signal on the same mast exists	*/
					if(pDistant->darkOnMainHalt)
					{
						pDistant->state = SIG_DI_BLANKED;
					}
					break;

				case SIG_MN_6:
					pDistant->state = SIG_DI_BLANKED;
					break;

				default:
					pDistant->state = pDistant->aspect;
					break;
				}
			}
			else
			{
				pDistant->state = pDistant->aspect;
			}

			pDistant->Todo	= false;
		}
		return pDistant->aspect;
	}
#endif

#ifdef SPDR60
	/**
	 * state machine entry function
	 * provides the function when entering the state given in pDistant->eState
	 * @param pDistant		pointer to the distant signal leaving the state
	 */
	static void distantStateEntry(struct sDistant * const pDistant);
	static void distantStateEntry(struct sDistant * const pDistant)
	{
		assert(NULL != pDistant);

		/* now the normal aspect can be set	*/
		switch(pDistant->state)
		{
		case SIG_DI_UNDEF:
			LOG_STW("Vorsignal %s ist neu undefiniert", pDistant->name);
			break;

		case SIG_DI_VR0:
			LOG_STW("Vorsignal %s ist neu auf Vr0", pDistant->name);
			registerBitSet(pDistant->BO_Yellow13);
			registerBitSet(pDistant->BO_Yellow11);
			registerBitSet(pDistant->BO_Desk_Yellow);
			break;

		case SIG_DI_VR1:
			LOG_STW("Vorsignal %s ist neu auf Vr1", pDistant->name);
			registerBitSet(pDistant->BO_Green14);
			registerBitSet(pDistant->BO_Green12);
			registerBitSet(pDistant->BO_Desk_Green);
			break;

		case SIG_DI_VR1_ZS3V:
			LOG_STW("Vorsignal %s ist neu auf Vr1 mit Zs3v", pDistant->name);
			registerBitSet(pDistant->BO_Green14);
			registerBitSet(pDistant->BO_Green12);
			registerBitSet(pDistant->BO_Zs3v);
			registerBitSet(pDistant->BO_Desk_Green);
			break;

		case SIG_DI_VR2:
			LOG_STW("Vorsignal %s ist neu auf Vr2", pDistant->name);
			registerBitSet(pDistant->BO_Green14);
			registerBitSet(pDistant->BO_Yellow11);
			registerBitSet(pDistant->BO_Desk_Green);
			break;

		case SIG_DI_VR2_ZS3V:
			LOG_STW("Vorsignal %s ist neu auf Hp2 mit Zs3v", pDistant->name);
			registerBitSet(pDistant->BO_Green14);
			registerBitSet(pDistant->BO_Yellow11);
			registerBitSet(pDistant->BO_Zs3v);
			registerBitSet(pDistant->BO_Desk_Green);
			break;

		case SIG_DI_BLANKED:
			LOG_STW("Vorsignal %s ist neu dunkel", pDistant->name);
			registerBitSet(pDistant->BO_Desk_Yellow);
			break;

		default:
			break;
		}
	}
#endif

#ifdef DOMINO55
	/**
	 * state machine entry function
	 * provides the function when entering the state given in pDistant->eState
	 * @param pDistant		pointer to the distant signal leaving the state
	 */
	static void distantStateEntry(struct sDistant * const pDistant);
	static void distantStateEntry(struct sDistant * const pDistant)
	{
		assert(NULL != pDistant);

		switch(pDistant->state)
		{
		case SIG_MN_UNDEF:
			LOG_STW("Vorsignal %s ist neu undefiniert", pDistant->name);
			break;

		case SIG_DI_WARN:
			LOG_STW("Vorsignal %s ist neu Warnung", pDistant->name);
			registerBitSet(pDistant->BO_Yellow11);
			registerBitSet(pDistant->BO_Yellow13);
			break;

		case SIG_DI_1:
			LOG_STW("Vorsignal %s ist neu auf Fahrbegriff 1", pDistant->name);
			registerBitSet(pDistant->BO_Green14);
			registerBitSet(pDistant->BO_Green12);
			break;
			
		case SIG_DI_2:
			LOG_STW("Vorsignal %s ist neu auf Fahrbegriff 2", pDistant->name);
			registerBitSet(pDistant->BO_Yellow11);
			registerBitSet(pDistant->BO_Green14);
			break;
		
		case SIG_DI_3:
			LOG_STW("Vorsignal %s ist neu auf Fahrbegriff 3", pDistant->name);
			registerBitSet(pDistant->BO_Yellow11);
			registerBitSet(pDistant->BO_Green14);
			registerBitSet(pDistant->BO_Green12);
			break;
			
		case SIG_DI_5:
			LOG_STW("Vorsignal %s ist neu auf Fahrbegriff 5", pDistant->name);
			registerBitSet(pDistant->BO_Green14);
			registerBitSet(pDistant->BO_Green12);
			registerBitSet(pDistant->BO_Green15);
			break;

		case SIG_DI_6:
			LOG_STW("Vorsignal %s ist neu auf Fahrbegriff 6", pDistant->name);
			registerBitSet(pDistant->BO_Yellow11);
			registerBitSet(pDistant->BO_Green14);
			break;

		case SIG_DI_BLANKED:
			LOG_STW("Vorsignal %s ist neu dunkel", pDistant->name);
			break;

		default:
			break;
		}
	}
#endif

#ifdef SPDR60
	/**
	 * state machine exit function
	 * provides the function when leaving the state given in pDistant->eState
	 * @param pDistant		pointer to the distant signal leaving the state
	 */
	static void distantStateExit(struct sDistant * const pDistant);
	static void distantStateExit(struct sDistant * const pDistant)
	{
		assert(NULL != pDistant);

		registerBitClr(pDistant->BO_Yellow13);
		registerBitClr(pDistant->BO_Yellow11);
		registerBitClr(pDistant->BO_Green14);
		registerBitClr(pDistant->BO_Green12);
		registerBitClr(pDistant->BO_Zs3v);
		registerBitClr(pDistant->BO_Desk_Yellow);
		registerBitClr(pDistant->BO_Desk_Green);
						 
	//	return;	/* temporary switched off this comments	*/
		switch(pDistant->state)
		{
		case SIG_MN_UNDEF:
			LOG_STW("Vorsignal %s ist nicht mehr undefiniert", pDistant->name);
			break;

		case SIG_DI_VR0:
			LOG_STW("Vorsignal %s ist nicht mehr auf Vr0", pDistant->name);
			break;

		case SIG_DI_VR1:
			LOG_STW("Vorsignal %s ist nicht mehr auf Vr1", pDistant->name);
			break;
		
		case SIG_DI_VR1_ZS3V:
			LOG_STW("Vorsignal %s ist nicht mehr auf Vr1 mit Zs3v", pDistant->name);
			break;
		
		case SIG_DI_VR2:
			LOG_STW("Vorsignal %s ist nicht mehr auf Vr2", pDistant->name);
			break;
		
		case SIG_DI_VR2_ZS3V:
			LOG_STW("Vorsignal %s ist nicht mehr auf Vr2 mit Zs3v", pDistant->name);
			break;

		case SIG_DI_BLANKED:
			LOG_STW("Vorsignal %s ist nicht mehr dunkel geschaltet", pDistant->name);
			break;

		default:
			break;
		}
	}
#endif

#ifdef DOMINO55
	/**
	 * state machine exit function
	 * provides the function when leaving the state given in pDistant->eState
	 * @param pDistant		pointer to the distant signal leaving the state
	 */
	static void distantStateExit(struct sDistant * const pDistant);
	static void distantStateExit(struct sDistant * const pDistant)
	{
		assert(NULL != pDistant);

		registerBitClr(pDistant->BO_Yellow11);
		registerBitClr(pDistant->BO_Yellow13);
		registerBitClr(pDistant->BO_Green14);
		registerBitClr(pDistant->BO_Green12);
		registerBitClr(pDistant->BO_Green15);
		registerBitClr(pDistant->BO_Desk_Yellow);
		registerBitClr(pDistant->BO_Desk_Green);
						 
	//	return;	/* temporary switched off this comments	*/
		switch(pDistant->state)
		{
		case SIG_DI_UNDEF:
			LOG_STW("Vorsignal %s ist nicht mehr undefiniert", pDistant->name);
			break;

		case SIG_DI_WARN:
			LOG_STW("Vorsignal %s ist nicht mehr Warnung", pDistant->name);
			break;

		case SIG_DI_1:
			LOG_STW("Vorsignal %s ist nicht mehr auf Fahrbegriff 1", pDistant->name);
			break;

		case SIG_DI_2:
			LOG_STW("Vorsignal %s ist nicht mehr auf Fahrbegriff 2", pDistant->name);
			break;

		case SIG_DI_3:
			LOG_STW("Vorsignal %s ist nicht mehr auf Fahrbegriff 3", pDistant->name);
			break;
		
		case SIG_DI_5:
			LOG_STW("Vorsignal %s ist nicht mehr auf Fahrbegriff 5", pDistant->name);
			break;
		
		case SIG_DI_6:
			LOG_STW("Vorsignal %s ist nicht mehr auf Fahrbegriff 6", pDistant->name);
			break;
		
		case SIG_DI_BLANKED:
			LOG_STW("Vorsignal %s ist nicht mehr dunkel geschaltet", pDistant->name);
			break;

		default:
			break;
		}
	}
#endif

/**
 * Function to let all distant signals switch to the desired state
 */
void distantsProcess(void)
{
	struct sDistant *	pDistant = m_pDistants;
	enum eDistantsAspect	resultAspect;
	int 	i;

	assert(NULL != pDistant);

	for(i = 0; i < m_nrDistants; i++)
	{
		resultAspect = distantStateBehaviour(pDistant);
		if(pDistant->aspect != resultAspect)
		{
			distantStateExit(pDistant);
			pDistant->aspect = resultAspect;
			distantStateEntry(pDistant);
		}
		pDistant++;		
	}	
}
