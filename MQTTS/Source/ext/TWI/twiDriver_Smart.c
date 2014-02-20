/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org

BSD New License
See LICENSE.txt file for license details.
*/

// TWI Driver for smart devices

#include "../../config.h"

#if (defined EXTDIO_USED) && (defined TWI_USED) && (defined TWI_USE_SMARTDRV)

#include "../twim.h"
#include "twiDriver_Smart.h"

#define SMART_START_ADDR        0x08
#define SMART_STOP_ADDR         0x1F

#define SMART_MAX_DEV           2

extern volatile uint8_t twim_access;           // access mode & busy flag

typedef struct
{
  uint8_t addr;
  uint8_t state;
  uint8_t len;
  uint8_t buf[16];
}SMART_BUF_t;

static SMART_BUF_t smart_buf;



/*
void twi_smart_cb(void)
{
  twim_access &= ~TWIM_BUSY;          // Bus Free

  if(smart_buf[smart_cb_pos] != NULL)
  {
    mqRelease(smart_buf[smart_cb_pos]);
    smart_buf[smart_cb_pos] = NULL;
  }
  else if((smart_pool_status[1] != 0) && ((smart_buf[smart_cb_pos] = mqAssert()) != NULL))
  {
    smart_buf[smart_cb_pos]->mq.Length = smart_pool_status[1];
    smart_buf[smart_cb_pos]->mq.MsgType = 0;
    smart_buf[smart_cb_pos]->mq.m.raw[0] = smart_pool_status[0];

    twimExch_ISR(smart_cb_addr,
                (TWIM_WRITE | TWIM_READ),
                1,
                smart_buf[smart_cb_pos]->mq.Length,
                smart_buf[smart_cb_pos]->mq.m.raw,
                NULL);
  }

  smart_cb_pos = 0xFF;
}

uint8_t twi_smart_read(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
  uint8_t pos = (pSubidx->Base>>1) & (SMART_MAX_DEV - 1);

  *pLen = smart_buf[pos]->mq.Length;
  memcpy(pBuf, smart_buf[pos]->mq.m.raw, *pLen);
  mqRelease(smart_buf[pos]);

  return MQTTS_RET_ACCEPTED;
}


uint8_t twi_smart_pool(subidx_t * pSubidx, uint8_t sleep)
{
  uint8_t pos = (pSubidx->Base>>1) & (SMART_MAX_DEV - 1);
  uint8_t addr = (pSubidx->Base>>8);
  if(smart_state[pos] > 0)
  {
    smart_state[pos]--;
    return 0;
  }

  if((twim_access != 0) || (smart_cb_pos != 0xFF))
    return 0;

  smart_cb_pos = pos;
  smart_state[pos] = 0xFF;

  if(smart_buf[pos] != NULL)    // Ready data to write
  {
    if(smart_buf[pos]->mq.MsgType == 1)
    {

    }
    else
      return 1;
  }

  // No info, read actual status
  smart_pool_status[0] = 0xF0;
  twimExch_ISR(addr, (TWIM_WRITE | TWIM_READ), 1, 2, smart_pool_status, &twi_smart_cb);
  smart_cb_addr = addr;
  return 0;
}
*/

/*
uint8_t twi_smart_pool(subidx_t * pSubidx, uint8_t sleep)
{
  uint8_t state = smart_buf->mq.MsgType;

  if((state > 0) && (state < 0x80))
  {
    smart_buf->mq.MsgType--;
    return 0;
  }

  if(twim_access != 0)
    return 0;


  return 0;
}

*/

uint8_t twi_smart_pool(subidx_t * pSubidx, uint8_t sleep)
{
  uint8_t state = smart_buf.state;

  if((state > 0) && (state < 0x80))
  {
    smart_buf.state--;
    return 0;
  }

  if(twim_access != 0)
    return 0;

  smart_buf.state = 0x7F;

  if(state == 0x81)    // Data ready to write
  {
    twimExch_ISR(smart_buf.addr,
                 TWIM_WRITE, 
                 smart_buf.len,
                 0, 
                 smart_buf.buf,
                 NULL);
    return 0;
  }

  return 0;
}

uint8_t twi_smart_write(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf)
{
  smart_buf.len = Len;
  smart_buf.state = 0x81;
  memcpy(smart_buf.buf, pBuf, Len);
  return MQTTS_RET_ACCEPTED;
}

uint8_t twi_Smart_Config(void)
{
  uint8_t addr;
  uint8_t buf[2];

  indextable_t * pIndex;

  for(addr = SMART_START_ADDR; addr <= SMART_STOP_ADDR; addr++)
  {
    buf[0] = 0xF0;    // Control Register
    buf[1] = 'R';     // Command - Reset
    if(twimExch(addr, TWIM_WRITE, 2, 0, buf) != TW_SUCCESS)       // Is Device Present ?
      continue;

    if((twimExch(addr, (TWIM_WRITE | TWIM_READ), 1, 2, buf) != TW_SUCCESS) ||   // Is Device Present ?
        (buf[0] != 0xC0) || (buf[1] != 4))
      continue;

    pIndex = getFreeIdxOD();
    if(pIndex == NULL)
      break;

    pIndex->cbRead  =  NULL;
    pIndex->cbWrite =  &twi_smart_write;
    pIndex->cbPool  =  &twi_smart_pool;

    pIndex->sidx.Place = objSmart;             // Object TWI
    pIndex->sidx.Type =  objArray;             // Variable Type -  Byte Array
    pIndex->sidx.Base = (addr<<8);             // Device address

    smart_buf.addr = addr;
    smart_buf.state = 0x7F;
    break;
  }

  return addr < SMART_STOP_ADDR ? 1 : 0;
}

#endif  //  EXTDIO_USED &  TWI_USED & TWI_USE_SMARTDRV
