/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

// Dummy driver, prototypes

#include "../../config.h"

#if (defined EXTDIO_USED) && (defined TWI_USED) && (defined TWI_USE_DUMMY)

#include "../twim.h"
#include "twiDriver_Dummy.h"

extern volatile uint8_t twim_access;           // access mode & busy flag

uint8_t twi_Dummy_Read(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
  // Place Hier you read code
  
  *pLen = 2;
  *(uint16_t *)pBuf = 0x55AA;     
  return MQTTS_RET_ACCEPTED;
}

uint8_t twi_Dummy_Write(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf)
{
  // Place hier you write code

  return MQTTS_RET_ACCEPTED;
}

uint8_t twi_Dummy_Pool(subidx_t * pSubidx, uint8_t sleep)
{
  if(sleep != 0)
  {
    // Reaction on ASleep mode
    return 0;
  }

  // Place hier you Pool code

  return 0;
}

uint8_t twi_Dummy_Config(void)
{
  // place hier you config code

  return 0;
}

#endif  //  (defined EXTDIO_USED) && (defined TWI_USED) && (defined TWI_USE_DUMMY)
