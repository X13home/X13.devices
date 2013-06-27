/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

// BlinkM driver, Prototype, !! Not Tested

#include "../../config.h"

#if (defined EXTDIO_USED) && (defined TWI_USED) && (defined TWI_USE_BLINKM)

#include "../twim.h"
#include "twiDriver_BlinkM.h"

extern volatile uint8_t twim_access;           // access mode & busy flag

// Process variables
static uint8_t  blinkm_buf[BLINKM_MAX_DEV];
static uint8_t blinkm_state[BLINKM_MAX_DEV];

uint8_t twi_BlinkM_Write(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf)
{
  uint8_t pos = pSubidx->Base & (BLINKM_MAX_DEV - 1);
  blinkm_buf[pos] = *(uint32_t *)pBuf;
  blinkm_state[pos] = 1;
  return MQTTS_RET_ACCEPTED;
}

uint8_t twi_BlinkM_Pool(subidx_t * pSubidx, uint8_t sleep)
{
#ifdef ASLEEP
  if(sleep != 0)
    return 0;
#endif  //  ASLEEP

  uint8_t pos, tmp;

  pos = pSubidx->Base & (BLINKM_MAX_DEV - 1);
  tmp = blinkm_state[pos];

  if(tmp == 2)
  {
    blinkm_state[pos] = 0;
    twim_access = 0;
  }
  else if((tmp == 1) && (twim_access == 0))
  {
    blinkm_state[pos] = 2;
    twimExch_ISR(pSubidx->Base>>8, (TWIM_BUSY | TWIM_WRITE), 4, 0, (uint8_t *)&blinkm_buf[pos], NULL);
  }

  return 0;
}

uint8_t twi_BlinkM_Config(void)
{
  uint8_t reg, addr, cnt = 0;
  
  indextable_t * pIndex;

  for(addr = BLINKM_START_ADDR; (addr < BLINKM_STOP_ADDR) && (cnt < BLINKM_MAX_DEV); addr++)
  {
    reg = 'a';  // Get BlinkM Address
    if((twimExch(addr, (TWIM_READ | TWIM_WRITE), 1, 1, &reg) == TW_SUCCESS)  &&    // Device Present
        (reg == addr))
    {
      pIndex = getFreeIdxOD();
      if(pIndex == NULL)
        break;

      pIndex->cbRead  =  NULL;
      pIndex->cbWrite =  &twi_BlinkM_Write;
      pIndex->cbPool  =  &twi_BlinkM_Pool;
      pIndex->sidx.Place = objTWI;               // Object TWI
      pIndex->sidx.Type =  objUInt32;            // Variables Type -  UInt32
      pIndex->sidx.Base = (addr<<8) + cnt;       // Device addr
      
      blinkm_buf[cnt] = 0;
      blinkm_state[cnt] = 1;

      cnt++;
    }
  }

  return cnt;
}

#endif  //  (defined EXTDIO_USED) && (defined TWI_USED) && (defined TWI_USE_BLINKM)
