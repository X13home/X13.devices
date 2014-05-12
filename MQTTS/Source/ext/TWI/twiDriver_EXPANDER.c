/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// TWI Driver, 16bit Bus Expander, PCA9535/TCA9535/PCA9555/MCP23016

#include "../../config.h"

#if (defined EXTDIO_USED) && (defined TWI_USED) && (defined TWI_USE_EXPANDER)

#include "../twim.h"
#include "twiDriver_expander.h"

#define EXPANDER_START_ADDR         0x20
#define EXPANDER_STOP_ADDR          0x26

#define EXPANDER_MAX_DEV            4           // 8/4/2/1

//#define EXPANDER_WRITE_ONLY         1           // Write only access

// Commands List
#define EXP_CMD_INP0                0
#define EXP_CMD_INP1                1
#define EXP_CMD_OUT0                2
#define EXP_CMD_OUT1                3
#define EXP_CMD_INV0                4
#define EXP_CMD_INV1                5
#define EXP_CMD_CFG0                6
#define EXP_CMD_CFG1                7

extern volatile uint8_t twim_access;           // access mode & busy flag

// Process variables
static uint16_t   expander_buf[EXPANDER_MAX_DEV];
static uint8_t    expander_state[EXPANDER_MAX_DEV];
uint8_t           expander_tmp[3], expander_cb_pos;

void twi_expander_cb(void)
{
  twim_access &= ~TWIM_BUSY;          // Bus Free
  
#ifndef EXPANDER_WRITE_ONLY
  uint16_t uiTmp;

  if(twim_access == 0)
  {
    if(expander_state[expander_cb_pos] == 9)
    {
      uiTmp = ((uint16_t)expander_tmp[1]<<8) | expander_tmp[0];
      if(uiTmp != expander_buf[expander_cb_pos])
      {
        expander_state[expander_cb_pos] = 10;
        expander_buf[expander_cb_pos] = uiTmp;
        return;
      }
    }
  }
#endif  // EXPANDER_WRITE_ONLY
  expander_state[expander_cb_pos] = 0;
}

#ifndef EXPANDER_WRITE_ONLY
uint8_t twi_expander_read(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
  *pLen = 2;
  *(uint16_t *)pBuf = expander_buf[(pSubidx->Base>>1) & (EXPANDER_MAX_DEV - 1)];

  return MQTTS_RET_ACCEPTED;
}
#endif  //  EXPANDER_WRITE_ONLY

uint8_t twi_expander_write(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf)
{
  uint8_t pos = (pSubidx->Base>>1) & (EXPANDER_MAX_DEV - 1);
  expander_buf[pos] = *(uint16_t *)pBuf;

  if(pSubidx->Base & 1)   //  DDR
    expander_state[pos] = 12;
  else
    expander_state[pos] = 11;

  return MQTTS_RET_ACCEPTED;
}

uint8_t twi_expander_poll(subidx_t * pSubidx, uint8_t sleep)
{
#ifdef ASLEEP
  if(sleep != 0)
    return 0;
#endif  //  ASLEEP

  uint8_t pos, tmp;

  pos = (pSubidx->Base>>1) & (EXPANDER_MAX_DEV - 1);
  tmp = expander_state[pos];

#ifndef EXPANDER_WRITE_ONLY
  if(tmp < 8)
  {
    expander_state[pos]++;
    return 0;
  }

  if(tmp == 10)
  {
    expander_state[pos] = 0;
    return 1;
  }
#endif  //  EXPANDER_WRITE_ONLY

  if(twim_access == 0)
  {
    if(tmp == 11)        //  Write Output Latch
    {
      expander_tmp[0] = EXP_CMD_OUT0;
      expander_tmp[1] = expander_buf[pos] & 0xFF;
      expander_tmp[2] = expander_buf[pos]>>8;
      twimExch_ISR(pSubidx->Base>>8, TWIM_WRITE, 3, 0, expander_tmp, &twi_expander_cb);
    }
    else if(tmp == 12)   // Write to Direction Register
    {
      expander_tmp[0] = EXP_CMD_CFG0;
      expander_tmp[1] = ~(expander_buf[pos] & 0xFF);
      expander_tmp[2] = ~(expander_buf[pos]>>8);
      twimExch_ISR(pSubidx->Base>>8, TWIM_WRITE, 3, 0, expander_tmp, &twi_expander_cb);
    }
#ifndef EXPANDER_WRITE_ONLY
    else                // Read Status
    {
      expander_tmp[0] = EXP_CMD_INP0;
      expander_state[pos] = 9;
      expander_cb_pos = pos;
      twimExch_ISR(pSubidx->Base>>8, (TWIM_WRITE | TWIM_READ), 1, 2, expander_tmp, &twi_expander_cb);
    }
#endif  //  EXPANDER_WRITE_ONLY
  }
  return 0;
}

uint8_t twi_Expander_Config(void)
{
  uint8_t buf[3], addr, cnt = 0;
  
  expander_cb_pos = 0;

  for(addr = EXPANDER_START_ADDR; (addr <= EXPANDER_STOP_ADDR) && (cnt < EXPANDER_MAX_DEV); addr++)
  {
    buf[0] = EXP_CMD_CFG0;
#ifndef EXPANDER_WRITE_ONLY
    buf[1] = 0xFF;
    buf[2] = 0xFF;
#else   //  EXPANDER_WRITE_ONLY
    buf[1] = 0x00;
    buf[2] = 0x00;
#endif  //  EXPANDER_WRITE_ONLY
    if(twimExch(addr, TWIM_WRITE, 3, 0, buf) == TW_SUCCESS)  // Device Present
    {
      buf[0] = EXP_CMD_OUT0;
      buf[1] = 0x00;
      buf[2] = 0x00;
      twimExch(addr, TWIM_WRITE, 3, 0, buf);

      indextable_t * pIndexIO;

      pIndexIO = getFreeIdxOD();
      if(pIndexIO == NULL)
        break;

#ifndef EXPANDER_WRITE_ONLY
      indextable_t * pIndexD;
      
      pIndexD = getFreeIdxOD();
      if(pIndexD == NULL)
      {
        pIndexIO->Index = 0xFFFF;
        break;
      }

      pIndexIO->cbRead  =  &twi_expander_read;
#else
      pIndexIO->cbRead  =  NULL;
#endif
      pIndexIO->cbWrite =  &twi_expander_write;
      pIndexIO->cbPoll  =  &twi_expander_poll;
      pIndexIO->sidx.Place = objTWI;               // Object TWI
      pIndexIO->sidx.Type =  objUInt16;            // Variables Type -  UInt16
      pIndexIO->sidx.Base = (addr<<8) + (cnt<<1);  // Device address

#ifndef EXPANDER_WRITE_ONLY
      pIndexD->cbRead  =  NULL;
      pIndexD->cbWrite =  &twi_expander_write;
      pIndexD->cbPoll  =  NULL;
      pIndexD->sidx.Place = objTWI;               // Object TWI
      pIndexD->sidx.Type =  objUInt16;            // Variables Type -  UInt16
      pIndexD->sidx.Base = (addr<<8) + (cnt<<1) + 1;  // Device address
#endif

      expander_buf[cnt] = 0;
      expander_state[cnt] = 0;

      cnt++;
    }
  }

#ifndef EXPANDER_WRITE_ONLY
  return (cnt<<1);
#else
  return cnt;
#endif
}

#endif  //  (defined EXTDIO_USED) && (defined TWI_USED) && (defined TWI_USE_EXPANDER)
