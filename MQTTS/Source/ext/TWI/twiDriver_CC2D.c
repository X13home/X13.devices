/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

// TWI Driver GE Sensing CC2Dxx[s], Temperature & Humidity

// Outs
// TW10240    Temperature counter (TC)
// TW10241    Relative Humidity Counter(RH)
// Temperature °C = TC*165/16384 - 40
// A*165/16384-40
// Humidity % = RH*100/16384
// A*25/4096

#include "../../config.h"

#if (defined EXTDIO_USED) && (defined TWI_USED) && (defined TWI_USE_CC2D)

#include "../twim.h"
#include "twiDriver_CC2D.h"

#define CC2D_ADDR   0x28

#define CC2D_CMD_RD_EEPROM_BASE     0x16    // EEPROM Read of addresses 0x16 to 0x1F
#define CC2D_CMD_WR_EEPROM_BASE     0x56    // Write to EEPROM addresses 0x16 to 0x1F
#define CC2D_CMD_START_NOM          0x80    // Ends Command Mode and transitions to Normal Operation Mode.
#define CC2D_CMD_START_CM           0xA0    // Start Command Mode

typedef union
{
  uint32_t  l;
  uint16_t  i[2];
  uint8_t   c[4];
}uL2I;

extern volatile uint8_t twim_access;           // access mode & busy flag

static uint8_t  cc2d_stat;
static uL2I     cc2d_exchg, cc2d_old;

uint8_t twi_CC2D_Read(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
  *pLen = 2;
  if(pSubidx->Base & 1)   // Read Humidity Counter
  {
    cc2d_old.i[1] = cc2d_exchg.i[1];
    *(uint16_t *)pBuf = cc2d_exchg.c[1] | ((uint16_t)cc2d_exchg.c[0]<<8);
  }
  else                    // Read Temperature counter TC
  {
    cc2d_old.i[0] = cc2d_exchg.i[0];
    *(uint16_t *)pBuf = (cc2d_exchg.c[3]>>2) | ((uint16_t)cc2d_exchg.c[2]<<6);
  }

  return MQTTS_RET_ACCEPTED;
}

uint8_t twi_CC2D_Pool1(subidx_t * pSubidx, uint8_t sleep)
{
#ifdef ASLEEP
  if(sleep != 0)
  {
    cc2d_stat = (0xFF-(POOL_TMR_FREQ/2));
    return 0;
  }
#endif  //  ASLEEP

  if(cc2d_stat == 0)
  {
    if(twim_access & (TWIM_ERROR | TWIM_BUSY | TWIM_WRITE | TWIM_READ))
      return 0;
    // Wake Up and Start Conversion
    twimExch_ISR(CC2D_ADDR, TWIM_WRITE, 0, 0, NULL, NULL);
    cc2d_stat = 1;
    return 0;
  }
  else
  {
    if(twim_access & TWIM_ERROR)    // Bus Error, or request to release bus
    {
      cc2d_stat = 0x40;
      return 0;
    }

    if(twim_access & (TWIM_READ | TWIM_WRITE))      // Bus Busy
      return 0;
      
    if((twim_access == 0) && (cc2d_stat < 11) && (cc2d_stat > 4))
    {
      if(cc2d_stat & 1)
        twimExch_ISR(CC2D_ADDR, TWIM_READ, 0, 4, (uint8_t *)&cc2d_exchg.l, NULL);
      else
      {
        if((cc2d_exchg.c[0] & 0xC0) == 0)
        {
          cc2d_stat = 12;
          if((cc2d_exchg.i[0] != cc2d_old.i[0]))
          {
            cc2d_stat++;
            return 1;
          }
        }
      }
    }
  }

  cc2d_stat++;
  return 0;
}

uint8_t twi_CC2D_Pool2(subidx_t * pSubidx, uint8_t sleep)
{
  if((cc2d_stat == 15) && (cc2d_exchg.i[1] != cc2d_old.i[1]))
    return 1;
  return 0;
}


uint8_t twi_CC2D_Config(void)
{
  cc2d_exchg.l = CC2D_CMD_START_NOM;   //  Ends Command Mode and transitions to Normal Operation Mode.
  if(twimExch(CC2D_ADDR, TWIM_WRITE, 3, 0, &cc2d_exchg.c[0]) == TW_SUCCESS) // Device Present
  {
    cc2d_stat = 0;
    cc2d_exchg.l = 0;
    cc2d_old.l = 0;

    // Register variable 1, Temperature counter°C
    indextable_t * pIndex1;
    pIndex1 = getFreeIdxOD();
    if(pIndex1 == NULL)
      return 0;
    
    // Register variable 2, Humidity counter
    indextable_t * pIndex2;
    pIndex2 = getFreeIdxOD();
    if(pIndex2 == NULL)
    {
      pIndex1->Index = 0xFFFF;                    // Free Index
      return 0;
    }

    pIndex1->cbRead  =  &twi_CC2D_Read;
    pIndex1->cbWrite =  NULL;
    pIndex1->cbPool  =  &twi_CC2D_Pool1;
    pIndex1->sidx.Place = objTWI;                // Object TWI
    pIndex1->sidx.Type =  objUInt16;             // Variables Type -  String
    pIndex1->sidx.Base = (CC2D_ADDR<<8);         // Device address

    pIndex2->cbRead  =  &twi_CC2D_Read;
    pIndex2->cbWrite =  NULL;
    pIndex2->cbPool  =  &twi_CC2D_Pool2;
    pIndex2->sidx.Place = objTWI;                // Object TWI
    pIndex2->sidx.Type =  objUInt16;             // Variables Type -  String
    pIndex2->sidx.Base = (CC2D_ADDR<<8) + 1;     // Device address
  }

  return 2;
}

#endif  //  (defined EXTDIO_USED) && (defined TWI_USED) && (defined TWI_USE_DUMMY)
