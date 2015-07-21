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
 * blocks (Bloecke)
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
#include <string.h>
#include "log.h"
#include "block.h"
#include "line.h"


/*@only@*/ /*@null@*/ static struct sBlock * m_pBlocks = NULL;	/**< Pointer to the first block	*/
static unsigned short 		m_nrBlocks = 0;			/**< Number of blocks	*/
static _Bool			m_blocksTodo = true;			/**< something to do for blocks	*/

void blocksProcess(void);


/**
 * Constructor for the block
 * @param nr		number of blocks to be created
 * @exception       	Calls exit if no RAM available
 */
void blocks(const unsigned short nr) 
{
	LOG_INF("started");
	
	m_nrBlocks = nr;
	m_pBlocks = malloc(m_nrBlocks * sizeof(struct sBlock));
	if(NULL == m_pBlocks) 
	{
		/* error allocating */
		LOG_ERR("cannot allocate RAM for blocks");
		exit(EXIT_FAILURE);
	}
	blocksInit();

	LOG_INF("ended");
}

/**
 * Destructor for the blocks
 */
void blocksDestroy(void) 
{
	LOG_INF("started");
	if(NULL == m_pBlocks) 
	{
		LOG_INF("zero pointer, beende");
		return;
	}
	LOG_INF("release RAM of blocks");
	free(m_pBlocks);
	m_pBlocks = NULL;
	LOG_INF("ended");
}

void blocksInit(void)
{
	struct sBlock *	pBlock = m_pBlocks;
	unsigned short  	i, j;

	LOG_INF("started");

	for(i = 0; i < m_nrBlocks; i++)
	{
		pBlock->name[0] = '\0';
		pBlock->state	= BlockNeutral;
		pBlock->pMain	= NULL;

		pBlock->nrSections	= 0;
		for(j = 0; j < BLOCKNRSECTIONS; j++)
		{
			pBlock->pSection[j] = NULL;
		}

		pBlock->Todo	= true;
		pBlock++;
	}		
	m_blocksTodo = true;
	LOG_INF("ended");
}

/**
 * Function to find the pointer of a block by giving the name
 * @param name		name of the block to be locked for
 * @return		pointer of the found block
 * @exception		a parser assert is made if not found
 */
struct sBlock * blockFind(const char * const name) 
{
        unsigned short		i = 0;
	struct sBlock *	pBlocks = NULL;
	char			temp[128];

	while(NULL != (pBlocks = blockGet(i)))
	{ 
		if(0 == (strcmp(pBlocks->name, name))) 
		{
			return pBlocks;
		}
		i++;
        }
        sprintf(temp, "Block %s nicht gefunden", name);
        parserExit(temp);
        return NULL;
}

/**
 * Function to get the pointer by giving the number of the block
 * @param i		index, starting with 0
 * @return		pointer to this block, NULL if not existing
 */ 
struct sBlock * blockGet(const unsigned short i)
{
	if(i >= m_nrBlocks)
	{
		return NULL;
	}
	else
	{
		return m_pBlocks + i;
	}
}

/**
 * Function to get the number of blocks
 * @return		Number of blocks
 */
unsigned short blocksGetNr(void)
{
	return m_nrBlocks;
}

/**
 * Function to get the name of a block
 * @param pBlock:	pointer to the block
 * @param name:		pointer to the string filled out with the name
 * 			of the block. Must be at least NAMELEN long
 * @return		true, if successful
 * @exception		assert on NULL pointer
 */ 
_Bool blockGetName(const struct sBlock * const pBlock, /*@out@*/ char * const name)
{
	assert(NULL != pBlock);

	name[NAMELEN - 1] = '\0';
	memmove(name, pBlock->name, NAMELEN - 1);
	return true;
}

/**
 * Function to set the name of a block
 * @param pBlock:	pointer to the block
 * @param name:		pointer to the string with the name 
 * @exception		assert on NULL pointer
 */ 
void blockSetName(struct sBlock * const pBlock, const char * const name)
{
	assert(NULL != pBlock);

	memmove(pBlock->name, name, NAMELEN);
}

/**
 * Function to see if a block is free
 * @param pBlock	pointer to the block
 * @exception		assert to NULL pointer
 */
const _Bool blockIsFree(const struct sBlock * const pBlock)
{
	assert(NULL != pBlock);

	if(BlockFree == pBlock->state)
	{
		return true;
	}
	/* else	*/
	return false;
}

/**
 * Function to get the handle of the red LED showing occupancy of the block in the desk
 * @param pBlock	pointer to the block
 * @return 		handle of the desk led
 * @exception		assert on NULL pointer
 */
const unsigned short blockGet_BO_DeskLED(const struct sBlock * const pBlock)
{
	assert(NULL != pBlock);

	return pBlock->BO_Desk_Red;
}

/**
 * Function to set the handle of the red LED showing occupancy of the block in the desk
 * @param pBlock	pointer to the block
 * @param pMain		handle of the desk led
 * @exception		assert on NULL pointer
 */
void blockSet_BO_DeskLED(struct sBlock * const pBlock, const unsigned short handle)
{
	assert(NULL != pBlock);

	pBlock->BO_Desk_Red = handle;
}

/**
 * Function to get the pointer to the entry signal of the block
 * @param pBlock	pointer to the block
 * @return 		pointer to the main signal
 * @exception		assert on NULL pointer
 */
const struct sMain * blockGetMain(const struct sBlock * const pBlock)
{
	assert(NULL != pBlock);

	return pBlock->pMain;
}

/**
 * Function to set the pointer to the entry signal of the block to the side of my station
 * @param pBlock	pointer to the block
 * @param pMain		pointer to the main signal
 * @exception		assert on NULL pointer
 */
void blockSetMain(struct sBlock * const pBlock, struct sMain * const pMain)
{
	assert(NULL != pBlock);

	pBlock->pMain = pMain;
}

/**
 * Function to add a section to the block plate, containing the sections between block and entry signal
 * @param pBlock	pointer to the block
 * @param pSection	pointer to the section
 * @return		index, where the section has been inserted
 * @exception		Assertion on NULL pointer or too many sections
 */
unsigned short blockAddSection(struct sBlock * const pBlock, struct sSection * const pSection)
{
	assert(NULL != pBlock);
	assert(LINENRSECTIONS > pBlock->nrSections);

	pBlock->pSection[pBlock->nrSections] = pSection;
	pBlock->nrSections++;
	return pBlock->nrSections - 1;
}

/**
 * Function to get the section to the block plate, containing the sections between block and entry signal
 * @param pBlock	pointer to the block
 * @param nr		number of the train route part section
 * @return		pointer of the section, if index is too big, NULL is returned
 * @exception		Assertion on NULL pointer
 */
struct sSection * blockGetSection(const struct sBlock * const pBlock, const unsigned short index)
{
	assert(NULL != pBlock);
	if(BLOCKNRSECTIONS <= index)
	{
		return NULL;
	}
	return pBlock->pSection[index];
}

/**
 * state machine function
 * provides the function when being inside the state given in pBlock->eState
 * @param pBlock		pointer to the block leaving the state
 */
static enum eBlockState blockStateBehaviour(struct sBlock * const pBlock);
static enum eBlockState blockStateBehaviour(struct sBlock * const pBlock)
{	
	unsigned short		i;
	struct sSection *	pSection = NULL;
	_Bool			free = false;
	enum eBlockState	blockState;

	assert(NULL != pBlock);

	if(pBlock->Todo)
	{
		switch(pBlock->state)
		{
		case BlockNeutral:
			/* check if jump to Free or Occupied state	*/

			i = 0;
			pBlock->state = BlockFree;	/* suppose free	*/
			free = true;
			while(NULL != (pSection	= blockGetSection(pBlock, i)))
			{
				if(! sectionIsFree(pSection))
				{
					blockState = BlockOccupied;
				}
				i++;
			}
			break;

		case BlockFree:
			/* this state can be left by block getting occupied	*/

			i = 0;
			while(NULL != (pSection = blockGetSection(pBlock, i)))
			{
				if(! sectionIsFree(pSection))
				{
					blockState = BlockOccupied;
				}
				i++;
			}
			break;

		case BlockOccupied:
			/* this state can be left by block getting free	*/

			i = 0;
			free = true;
			while(NULL != (pSection = blockGetSection(pBlock, i)))
			{
				if(! sectionIsFree(pSection))
				{
					free = false;
				}
				i++;
			}
			if(free) 
			{
				blockState = BlockFree;
			}

		/* TODO here the behaviour setting the signals to be implemented	*/
		default:
			blockState = BlockNeutral;
			break;
		}
		pBlock->Todo	= false;
		return blockState;
	}
}

/**
 * state machine entry function
 * provides the function when entering the state given in pBlock->eState
 * @param pBlock	pointer to the block leaving the state
 */
static void blockStateEntry(struct sBlock * const pBlock);
static void blockStateEntry(struct sBlock * const pBlock)
{
	assert(NULL != pBlock);

	switch(pBlock->state)
	{
	case BlockNeutral:
		LOG_STW("Block %s ist neu undefiniert", pBlock->name);
		break;

	case BlockOccupied:
		LOG_STW("Block %s ist belegt", pBlock->name);
		break;

	case BlockFree:
		LOG_STW("Block %s ist frei", pBlock->name);
		break;

	default:
		break;
	}
}

/**
 * state machine exit function
 * provides the function when leaving the state given in pBlock->eState
 * @param pBlock	pointer to the block leaving the state
 */
static void blockStateExit(struct sBlock * const pBlock);
static void blockStateExit(struct sBlock * const pBlock)
{
	assert(NULL != pBlock);
}

/**
* Function to let all blocks switch to the desired state
*/
void blocksProcess(void)
{
	struct sBlock *		pBlock = m_pBlocks;
	enum eBlockState	resultState;
	int 			i;

	assert(NULL != pBlock);

	for(i = 0; i < m_nrBlocks; i++)
	{
		resultState = blockStateBehaviour(pBlock);
		if(pBlock->state != resultState)
		{
			blockStateExit(pBlock);
			pBlock->state = resultState;
			blockStateEntry(pBlock);
			pBlock++;		
		}	
	}
}
