/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

// BlinkM driver, Prototype

#include "../../config.h"

#if (defined EXTDIO_USED) && (defined TWI_USED) && (defined TWI_USE_BLINKM)

#include "../twim.h"
#include "twiDriver_BlinkM.h"

extern volatile uint8_t twim_access;           // access mode & busy flag

static uint32_t blinkm_buf;

uint8_t twi_BlinkM_Write(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf)
{
  blinkm_buf = *(uint32_t *)pBuf;
  twimExch_ISR(pSubidx->Base>>8, (TWIM_BUSY | TWIM_WRITE), 4, 0, (uint8_t *)&blinkm_buf, NULL);
  return MQTTS_RET_ACCEPTED;
}

uint8_t twi_BlinkM_Config(void)
{
  uint8_t reg, addr;
  
  for(addr = 1; addr < 129; addr++)
  {
    if(addr == 128)
      return 0;

    reg = 'a';  // Get BlinkM Address
    if(twimExch(addr, (TWIM_READ | TWIM_WRITE), 1, 1, &reg) == TW_SUCCESS)  // &&    // Device Present
//        (reg == addr))
      break;
  }
  
  // Register variables
  indextable_t * pIndex;
  pIndex = getFreeIdxOD();
  if(pIndex == NULL)
    return 0;

  pIndex->cbRead  =  NULL;
  pIndex->cbWrite =  &twi_BlinkM_Write;
  pIndex->cbPool  =  NULL;
  pIndex->sidx.Place = objTWI;               // Object TWI
  pIndex->sidx.Type =  objUInt32;            // Variables Type -  UInt32
  pIndex->sidx.Base = (addr<<8);             // Device addr

  return 1;
}

#endif  //  (defined EXTDIO_USED) && (defined TWI_USED) && (defined TWI_USE_BLINKM)
