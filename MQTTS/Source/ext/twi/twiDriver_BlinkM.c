/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

// BlinkM driver

// Tq[Addr] = MODE+256*(R+256*(G+256*B))
// MODE: 110 - Go to RGB Color Now, 99 - Fade to RGB Color

#include "../../config.h"

#if (defined EXTDIO_USED) && (defined TWI_USED) && (defined TWI_USE_BLINKM)

#include "../twim.h"
#include "twiDriver_BlinkM.h"

extern volatile uint8_t twim_access;           // access mode & busy flag

// Process variables
static uint64_t  blinkm_buf[BLINKM_MAX_DEV];
static uint8_t blinkm_state[BLINKM_MAX_DEV];

uint8_t twi_BlinkM_Write(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf)
{
  uint8_t pos = pSubidx->Base & (BLINKM_MAX_DEV - 1);
  blinkm_buf[pos] = *(uint64_t *)pBuf;
  blinkm_state[pos] = 1;
  return MQTTS_RET_ACCEPTED;
}

uint8_t twi_BlinkM_Pool(subidx_t * pSubidx, uint8_t sleep)
{
#ifdef ASLEEP
  if(sleep != 0)
    return 0;
#endif  //  ASLEEP

  uint8_t pos, tmp, len;

  pos = pSubidx->Base & (BLINKM_MAX_DEV - 1);
  tmp = blinkm_state[pos];

  if(tmp == 2)
  {
    blinkm_state[pos] = 0;
    twim_access = 0;
  }
  else if((tmp == 1) && (twim_access == 0))
  {
    switch(blinkm_buf[pos] & 0xFF)
    {
      case 'W': // Write Script Line            7 0 {‘W’,n,p,...}
        len = 8;
        break;
      case 'B': // Set Startup Parameters       5 0 {‘B’,m,n,r,f,t}
        len = 6;
        break;
      case 'A': // Set BlinkM Address           4 0 {‘A’,a...}
        len = 5;
        break;
      case 'n': // Go to RGB Color Now          3 0 {‘n’,R,G,B}
      case 'c': // Fade to RGB Color            3 0 {‘c’,R,G,B}
      case 'h': // Fade to HSB Color            3 0 {‘h’,H,S,B}
      case 'C': // Fade to Random RGB Color     3 0 {‘C’,R,G,B}
      case 'H': // Fade to Random HSB Color     3 0 {‘H’,H,S,B}
      case 'p': // Play Light Script            3 0 {‘p’,n,r,p}
      case 'L': // Set Script Length & Repeats  3 0 {‘L’,n,l,r}
        len = 4;
        break;
      case 'f': // Set Fade Speed               1 0 {‘f’,f}
      case 't': // Set Time Adjust              1 0 {‘t’,t}
        len = 2;
        break;
      case 'o': // Stop Script                  0 0 {‘o’}
        len = 1;
        break;
      default:
        blinkm_state[pos] = 0;
        return 0;
    }
    blinkm_state[pos] = 2;
    
    twimExch_ISR(pSubidx->Base>>8, (TWIM_BUSY | TWIM_WRITE), len, 0, (uint8_t *)&blinkm_buf[pos], NULL);
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
      pIndex->sidx.Type =  objInt64;             // Variables Type -  Int64
      pIndex->sidx.Base = (addr<<8) + cnt;       // Device addr
      
      blinkm_buf[cnt] = 0;
      blinkm_state[cnt] = 1;

      cnt++;
    }
  }

  return cnt;
}

#endif  //  (defined EXTDIO_USED) && (defined TWI_USED) && (defined TWI_USE_BLINKM)
