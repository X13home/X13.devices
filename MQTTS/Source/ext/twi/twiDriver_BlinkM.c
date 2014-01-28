/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

// BlinkM driver

// Tq[Addr] = MODE+256*(R+256*(G+256*B))
// MODE: 0x6E - Go to RGB Color Now, 0x63 - Fade to RGB Color

#include "../../config.h"

#if (defined EXTDIO_USED) && (defined TWI_USED) && (defined TWI_USE_BLINKM)

#include "../twim.h"
#include "twiDriver_BlinkM.h"

#define BLINKM_START_ADDR           8
#define BLINKM_STOP_ADDR            31

#define BLINKM_MAX_DEV              4           // 2/4/8

extern volatile uint8_t twim_access;           // access mode & busy flag

// Process variables
static uint8_t  blinkm_buf[BLINKM_MAX_DEV][9];

uint8_t twi_BlinkM_Write(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf)
{
  uint8_t pos = pSubidx->Base & (BLINKM_MAX_DEV - 1);
  if((Len >= 8) || (Len == 0))
    return MQTTS_RET_REJ_CONG;
  
  blinkm_buf[pos][0] = Len;
  memcpy(&blinkm_buf[pos][1], pBuf, Len);

  return MQTTS_RET_ACCEPTED;
}

uint8_t twi_BlinkM_Pool(subidx_t * pSubidx, uint8_t sleep)
{
#ifdef ASLEEP
  if(sleep != 0)
    return 0;
#endif  //  ASLEEP

  uint8_t pos, len;

  if(twim_access != 0)
    return 0;

  pos = pSubidx->Base & (BLINKM_MAX_DEV - 1);
  len = blinkm_buf[pos][0];

  if(len != 0)
  {
    blinkm_buf[pos][0] = 0;
    twimExch_ISR(pSubidx->Base>>8, TWIM_WRITE, len, 0, (uint8_t *)&blinkm_buf[pos][1], NULL);
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
      pIndex->sidx.Type =  objArray;             // Variable Type -  Byte Array
      pIndex->sidx.Base = (addr<<8) + cnt;       // Device address
      blinkm_buf[cnt][0] = 0;

      cnt++;
    }
  }

  return cnt;
}

#endif  //  (defined EXTDIO_USED) && (defined TWI_USED) && (defined TWI_USE_BLINKM)
