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

#define SMART_MAX_DEV           2         //  2/4/8

#define SMART_DATA_SIZE         (MQTTS_MSG_SIZE-5)

extern volatile uint8_t twim_access;      // access mode & busy flag

typedef struct
{
  uint8_t   state;
  uint16_t  status;
  MQ_t *    mBuf;
}SMART_BUF_t;

enum
{
  SMART_STATE_WAIT = 0x7F,
  SMART_STATE_WRITE_DATA_READY = 0x81,
  SMART_STATE_WRITE_DATA_WRITES,
  SMART_STATE_STATUS_READY,
  SMART_STATE_READ_DATA_READY,
}e_POOL_STATE_SMART_DRV;

static SMART_BUF_t smart_buf[SMART_MAX_DEV];

uint8_t twi_smart_pool(subidx_t * pSubidx, uint8_t sleep)
{
  uint8_t pos = pSubidx->Base & (SMART_MAX_DEV - 1);
  uint8_t addr = pSubidx->Base>>8;
  uint8_t state = smart_buf[pos].state;
  uint8_t len;

  if((state > 0) && (state <= SMART_STATE_WAIT))
  {
    smart_buf[pos].state--;
    return 0;
  }

  if(twim_access != 0)
  {
    if((twim_access & TWIM_ERROR) && (state != 0))
      state = SMART_STATE_WAIT;

    return 0;
  }

  switch(state)
  {
    case SMART_STATE_WRITE_DATA_READY:    // Data ready to write
      smart_buf[pos].state = SMART_STATE_WRITE_DATA_WRITES;
      twimExch_ISR(addr, TWIM_WRITE, smart_buf[pos].mBuf->mq.Length, 0, smart_buf[pos].mBuf->mq.m.raw, NULL);
      break;
    case SMART_STATE_STATUS_READY:        //  Status ready
        if(smart_buf[pos].status == 0x00FF)
        smart_buf[pos].state = SMART_STATE_WAIT;
      else if((len = smart_buf[pos].status>>8) > 0)
      {
        // Read data
        smart_buf[pos].mBuf = mqAssert();
        if(smart_buf[pos].mBuf == NULL)   // No Memory
          state = SMART_STATE_WAIT;
        else
        {
          smart_buf[pos].state = SMART_STATE_READ_DATA_READY;
          smart_buf[pos].mBuf->mq.m.raw[0] = smart_buf[pos].status & 0xFF;
          if(len > SMART_DATA_SIZE)
            len = SMART_DATA_SIZE;
          twimExch_ISR(addr, (TWIM_WRITE | TWIM_READ), 1, len, smart_buf[pos].mBuf->mq.m.raw, NULL);
        }
      }
      else
      {
        smart_buf[pos].state = SMART_STATE_WAIT;
        return 1;
      }
      break;
    case SMART_STATE_READ_DATA_READY:     //  Read data ready
      smart_buf[pos].state = SMART_STATE_WAIT;
      return 1;
    case SMART_STATE_WRITE_DATA_WRITES:   // Data writes
      mqRelease(smart_buf[pos].mBuf);
      smart_buf[pos].mBuf = NULL;
    default:                              // No info, read actual status
      smart_buf[pos].state = SMART_STATE_STATUS_READY;
      smart_buf[pos].status = 0x00F0;
      twimExch_ISR(addr, (TWIM_WRITE | TWIM_READ), 1, 2, (uint8_t *)&smart_buf[pos].status, NULL);
      break;
  }

  return 0;
}

uint8_t twi_smart_write(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf)
{
  uint8_t pos = pSubidx->Base & (SMART_MAX_DEV - 1);

  if((smart_buf[pos].mBuf != NULL) || ((smart_buf[pos].mBuf = mqAssert()) == NULL))
    return MQTTS_RET_REJ_CONG;

  smart_buf[pos].mBuf->mq.Length = Len;
  smart_buf[pos].state = SMART_STATE_WRITE_DATA_READY;
  memcpy(smart_buf[pos].mBuf->mq.m.raw, pBuf, Len);
  return MQTTS_RET_ACCEPTED;
}

uint8_t twi_smart_read(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
  uint8_t pos = pSubidx->Base & (SMART_MAX_DEV - 1);
  uint8_t len = smart_buf[pos].status>>8;
  if(len >= (SMART_DATA_SIZE - 1))
    len = (SMART_DATA_SIZE - 1);

  *pLen = len + 1;
  pBuf[0] = smart_buf[pos].status & 0xFF;

  if(len > 0)
  {
    memcpy(&pBuf[1], smart_buf[pos].mBuf->mq.m.raw, len);
    mqRelease(smart_buf[pos].mBuf);
    smart_buf[pos].mBuf = NULL;
  }

  return MQTTS_RET_ACCEPTED;
}

uint8_t twi_Smart_Config(void)
{
  uint8_t addr;
  uint8_t buf[2];
  uint8_t pos = 0;

  indextable_t * pIndex;

  for(addr = SMART_START_ADDR; (addr <= SMART_STOP_ADDR) && (pos < SMART_MAX_DEV); addr++)
  {
    buf[0] = 0xF0;    // Control Register
    buf[1] = 'R';     // Command - Reset
    if((twimExch(addr, TWIM_WRITE, 2, 0, buf) != TW_SUCCESS) ||     // Is Device Present ?
       (twimExch(addr, TWIM_READ,  0, 2, buf) != TW_SUCCESS) ||     // Read Status Register
       (buf[0] != 0xB0) || (buf[1] < 6))                            // Status  != DeviceId Reg or Length < 6
      continue;

    pIndex = getFreeIdxOD();
    if(pIndex == NULL)
      break;

    pIndex->cbRead  =  &twi_smart_read;
    pIndex->cbWrite =  &twi_smart_write;
    pIndex->cbPool  =  &twi_smart_pool;

    pIndex->sidx.Place = objSmart;             // Object TWI
    pIndex->sidx.Type =  objArray;             // Variable Type -  Byte Array
    pIndex->sidx.Base = (addr<<8);             // Device address

    smart_buf[pos++].state = SMART_STATE_WAIT;
    break;
  }

  return pos;
}

#endif  //  EXTDIO_USED &  TWI_USED & TWI_USE_SMARTDRV
