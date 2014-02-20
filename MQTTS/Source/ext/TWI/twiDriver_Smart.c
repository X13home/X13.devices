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

static uint8_t smart_state[SMART_MAX_DEV];

uint8_t twi_Smart_Config(void)
{
  uint8_t addr, cnt = 0;
  uint8_t buf[2];

  indextable_t * pIndex;

  for(addr = SMART_START_ADDR; (addr <= SMART_STOP_ADDR) && (cnt < SMART_MAX_DEV); addr++)
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
    pIndex->cbWrite =  NULL;
    pIndex->cbPool  =  NULL;
    pIndex->sidx.Place = objSmart;             // Object TWI
    pIndex->sidx.Type =  objArray;             // Variable Type -  Byte Array
    pIndex->sidx.Base = (addr<<8) + cnt;       // Device address
    cnt++;
  }

  return cnt;
}

#endif  //  EXTDIO_USED &  TWI_USED & TWI_USE_SMARTDRV
