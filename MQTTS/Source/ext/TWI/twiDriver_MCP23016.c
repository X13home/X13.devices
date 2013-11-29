/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

// TWI Driver, Bus Expander MCP23016

#include "../../config.h"

#if (defined EXTDIO_USED) && (defined TWI_USED) && (defined TWI_USE_MCP23016)

#include "../twim.h"
#include "twiDriver_MCP23016.h"

extern volatile uint8_t twim_access;           // access mode & busy flag

// Process variables
static uint16_t   mcp23016_buf[MCP23016_MAX_DEV];
static uint8_t    mcp23016_state[MCP23016_MAX_DEV];
uint8_t           mcp23016_tmp[3], mcp23016_cb_pos;

void mcp23016_cb(void)
{
  uint8_t tmp;
  uint16_t uiTmp;

  twim_access &= ~TWIM_BUSY;          // Bus Free
  if(twim_access == 0)
  {
    tmp = mcp23016_state[mcp23016_cb_pos];

    if(tmp == 9)
    {
      uiTmp = ((uint16_t)mcp23016_tmp[1]<<8) | mcp23016_tmp[0];
      if(uiTmp != mcp23016_buf[mcp23016_cb_pos])
      {
        mcp23016_state[mcp23016_cb_pos] = 10;
        mcp23016_buf[mcp23016_cb_pos] = uiTmp;
        return;
      }
    }
  }
  mcp23016_state[mcp23016_cb_pos] = 0;
}

uint8_t twi_mcp23016_read(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
  *pLen = 2;
  *(uint16_t *)pBuf = mcp23016_buf[(pSubidx->Base>>1) & (MCP23016_MAX_DEV - 1)];

  return MQTTS_RET_ACCEPTED;
}

uint8_t twi_mcp23016_write(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf)
{
  uint8_t pos = (pSubidx->Base>>1) & (MCP23016_MAX_DEV - 1);
  mcp23016_buf[pos] = *(uint16_t *)pBuf;

  if(pSubidx->Base & 1)   //  DDR
    mcp23016_state[pos] = 12;
  else
    mcp23016_state[pos] = 11;

  return MQTTS_RET_ACCEPTED;
}

uint8_t twi_mcp23016_Pool(subidx_t * pSubidx, uint8_t sleep)
{
#ifdef ASLEEP
  if(sleep != 0)
    return 0;
#endif  //  ASLEEP

  uint8_t pos, tmp;

  pos = (pSubidx->Base>>1) & (MCP23016_MAX_DEV - 1);
  tmp = mcp23016_state[pos];

  if(tmp < 8)
  {
    mcp23016_state[pos]++;
    return 0;
  }

  if(tmp == 10)
  {
    mcp23016_state[pos] = 0;
    return 1;
  }

  if(twim_access == 0)
  {
    if(tmp == 11)        //  Write Output Latch
    {
      mcp23016_tmp[0] = 0;
      mcp23016_tmp[1] = mcp23016_buf[pos] & 0xFF;
      mcp23016_tmp[2] = mcp23016_buf[pos]>>8;
      twimExch_ISR(pSubidx->Base>>8, TWIM_WRITE, 3, 0, mcp23016_tmp, &mcp23016_cb);
    }
    else if(tmp == 12)   // Write to Direction Register
    {
      mcp23016_tmp[0] = 6;
      mcp23016_tmp[1] = ~(mcp23016_buf[pos] & 0xFF);
      mcp23016_tmp[2] = ~(mcp23016_buf[pos]>>8);
      twimExch_ISR(pSubidx->Base>>8, TWIM_WRITE, 3, 0, mcp23016_tmp, &mcp23016_cb);
    }
    else                // Read Status
    {
      mcp23016_tmp[0] = 0;
      mcp23016_state[pos] = 9;
      mcp23016_cb_pos = pos;
      twimExch_ISR(pSubidx->Base>>8, TWIM_WRITE | TWIM_READ, 1, 2, mcp23016_tmp, &mcp23016_cb);
    }
  }
  return 0;
}


uint8_t twi_MCP23016_Config(void)
{
  uint8_t buf[3], addr, cnt = 0;
  
  mcp23016_cb_pos = 0;

  for(addr = MCP23016_START_ADDR; (addr <= MCP23016_STOP_ADDR) && (cnt < MCP23016_MAX_DEV); addr++)
  {
    buf[0] = 6;
    buf[1] = 0xFF;
    buf[2] = 0xFF;
    if(twimExch(addr, TWIM_WRITE, 3, 0, buf) == TW_SUCCESS)  // Device Present
    {
      buf[0] = 0;
      buf[1] = 0;
      buf[2] = 0;
      twimExch(addr, TWIM_WRITE, 3, 0, buf);

      indextable_t * pIndexIO;
      indextable_t * pIndexD;

      pIndexIO = getFreeIdxOD();
      if(pIndexIO == NULL)
        break;

      pIndexD = getFreeIdxOD();
      if(pIndexD == NULL)
      {
        pIndexIO->Index = 0xFFFF;
        break;
      }

      pIndexIO->cbRead  =  &twi_mcp23016_read;
      pIndexIO->cbWrite =  &twi_mcp23016_write;
      pIndexIO->cbPool  =  &twi_mcp23016_Pool;
      pIndexIO->sidx.Place = objTWI;               // Object TWI
      pIndexIO->sidx.Type =  objUInt16;            // Variables Type -  UInt16
      pIndexIO->sidx.Base = (addr<<8) + (cnt<<1);  // Device address

      pIndexD->cbRead  =  NULL;
      pIndexD->cbWrite =  &twi_mcp23016_write;
      pIndexD->cbPool  =  NULL;
      pIndexD->sidx.Place = objTWI;               // Object TWI
      pIndexD->sidx.Type =  objUInt16;            // Variables Type -  UInt16
      pIndexD->sidx.Base = (addr<<8) + (cnt<<1) + 1;  // Device address

      mcp23016_buf[cnt] = 0;
      mcp23016_state[cnt] = 0;

      cnt++;
    }
  }

  return cnt<<1;
}

#endif  //  (defined EXTDIO_USED) && (defined TWI_USED) && (defined TWI_USE_BLINKM)
