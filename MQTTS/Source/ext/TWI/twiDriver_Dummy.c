/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// Dummy driver, prototypes

#include "../../config.h"

#if (defined EXTDIO_USED) && (defined TWI_USED) && (defined TWI_USE_DUMMY)

#include "../twim.h"
#include "twiDriver_Dummy.h"

extern volatile uint8_t twim_access;           // access mode & busy flag

uint8_t twi_Dummy_Read(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
  // Place here your read code
  
  // Begins dummy code, remove it
  *pLen = 2;
  *(uint16_t *)pBuf = 0x55AA;
  return MQTTS_RET_ACCEPTED;
  // Ended dummy code
}

uint8_t twi_Dummy_Write(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf)
{
  // Place here your write code

  return MQTTS_RET_ACCEPTED;
}

uint8_t twi_Dummy_Poll(subidx_t * pSubidx, uint8_t sleep)
{
#ifdef ASLEEP
  if(sleep != 0)
  {
    // Reaction on ASleep mode
    return 0;
  }
#endif  //  ASLEEP
  // Place here your Polling code

  return 0;
}

uint8_t twi_Dummy_Config(void)
{
  // Remove dummy Code and place here your Configuration code
  
  // Begins dummy code, remove it
  uint8_t addr, counter = 0;
  indextable_t * pIndex;
  
  for(addr = 1; addr < 128; addr++)
  {
    if(twimExch(addr, TWIM_READ, 0, 0, NULL) == TW_SUCCESS)     // Device Present
    {
      pIndex = getFreeIdxOD();
      if(pIndex == NULL)
        break;

      counter++;

      pIndex->cbRead  =  &twi_Dummy_Read;
      pIndex->cbWrite =  &twi_Dummy_Write;
      pIndex->cbPoll  =  &twi_Dummy_Poll;
      pIndex->sidx.Place = objTWI;                // Object TWI
      pIndex->sidx.Type =  objString;             // Variables Type -  String
      pIndex->sidx.Base = (addr<<8);              // Device address
    }
  }

  return counter;
  // Ended dummy code
}

#endif  //  (defined EXTDIO_USED) && (defined TWI_USED) && (defined TWI_USE_DUMMY)
