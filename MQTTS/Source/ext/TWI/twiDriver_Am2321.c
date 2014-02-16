/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org

BSD New License
See LICENSE.txt file for license details.
*/

// TWI Driver Aosong AM2321, Temperature & Humidity
// Prototype, !! NOT Tested

// Outs
// TW23552      Temperature counter (TC)
// T°C= TC/10
// TW23553      Relative Humidity Counter(RHC)
// RH% = RHC/10

#include "../../config.h"

#if (defined EXTDIO_USED) && (defined TWI_USED) && (defined TWI_USE_AM2321)

#include "../twim.h"
#include "twiDriver_Am2321.h"

#define AM2321_ADDR             0x5C

#define AM2321_PARAM_READ                0x03
#define AM2321_REG_HUMIDITY_MSB          0x00
#define AM2321_REG_HUMIDITY_LSB          0x01
#define AM2321_REG_TEMPERATURE_MSB       0x02
#define AM2321_REG_TEMPERATURE_LSB       0x03
#define AM2321_REG_DEVICE_ID_BIT_24_31   0x0B

//#define AM2321_USE_CALLBACK   1

extern volatile uint8_t twim_access;           // access mode & busy flag

// Process variables
uint8_t         am2321_buf[8];
static uint8_t  am2321_stat = 0;
static uint16_t am2321_oldTemp = 0;
static uint16_t am2321_oldHumi = 0;

#ifdef AM2321_USE_CALLBACK
void am2321_cb(void)
{
  am2321_buf[0] = AM2321_PARAM_READ;
  am2321_buf[1] = AM2321_REG_HUMIDITY_MSB;
  am2321_buf[2] = 4;
  twimExch_ISR(AM2321_ADDR, TWIM_WRITE, 3, 0, am2321_buf, NULL);
}
#endif  //  AM2321_USE_CALLBACK

uint8_t twi_am2321_Read(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
  *pLen = 2;
  if(pSubidx->Base & 1)   // Read Humidity Counter
    *(uint16_t *)pBuf = am2321_oldHumi;
  else                    // Read Temperature counter TC
    *(uint16_t *)pBuf = am2321_oldTemp;

  return MQTTS_RET_ACCEPTED;
}

uint8_t twi_am2321_Pool1(subidx_t * pSubidx, uint8_t sleep)
{
  uint16_t temp;

#ifdef ASLEEP
  if(sleep != 0)
  {
    am2321_stat = (0xFF-(POOL_TMR_FREQ/2));
    return 0;
  }
#endif  //  ASLEEP
  if(twim_access & TWIM_ERROR)      // Bus Error
  {
    if((am2321_stat <= 3) && (am2321_stat > 0))
      am2321_stat = 7;
    return 0;
  }
  
  switch(am2321_stat)
  {
    case 0:   // wakeup
      if(twim_access != 0)
        return 0;
#ifndef AM2321_USE_CALLBACK
      twimExch_ISR(AM2321_ADDR, TWIM_WRITE, 0, 0, NULL, NULL);
#else
      twimExch_ISR(AM2321_ADDR, TWIM_WRITE, 0, 0, NULL, am2321_cb);
#endif
      break;
    case 1:   // Read Command
      if(twim_access != 0)
        return 0;
#ifndef AM2321_USE_CALLBACK
      am2321_buf[0] = AM2321_PARAM_READ;
      am2321_buf[1] = AM2321_REG_HUMIDITY_MSB;
      am2321_buf[2] = 4;
      twimExch_ISR(AM2321_ADDR, TWIM_WRITE, 3, 0, am2321_buf, NULL);
      break;
    case 2:
      if(twim_access != 0)
        return 0;
#endif  //  !AM2321_USE_CALLBACK
      twimExch_ISR(AM2321_ADDR, TWIM_READ, 0, 8, am2321_buf, NULL);
      break;
    case 3:
      temp = am2321_buf[5] | ((uint16_t)am2321_buf[5]<<8);
      if(am2321_oldTemp != temp)
      {
        am2321_oldTemp = temp;
        am2321_stat++;
        return 1;
      }
      break;
  }

  am2321_stat++;
  return 0;
}

uint8_t twi_am2321_Pool2(subidx_t * pSubidx, uint8_t sleep)
{
  uint16_t humi;

  if(am2321_stat == 5)
  {
    am2321_stat++;
    humi = am2321_buf[3] | ((uint16_t)am2321_buf[2]<<8);
    
    if(am2321_oldHumi != humi)
    {
      am2321_oldHumi = humi;
      return 1;
    }
  }
  
  return 0;
}

uint8_t twi_AM2321_Config(void)
{
  // Wakeup
  if(twimExch(AM2321_ADDR, TWIM_WRITE, 0, 0, NULL) != TW_SUCCESS)     // Communication error
    return 0;

  am2321_stat = 0;
  am2321_oldTemp = 0;
  am2321_oldHumi = 0;

  // Register variable 1, Temperature Counter
  indextable_t * pIndex1;
  pIndex1 = getFreeIdxOD();
  if(pIndex1 == NULL)
    return 0;

  pIndex1->cbRead  =  &twi_am2321_Read;
  pIndex1->cbWrite =  NULL;
  pIndex1->cbPool  =  &twi_am2321_Pool1;
  pIndex1->sidx.Place = objTWI;                   // Object TWI
  pIndex1->sidx.Type =  objInt16;                 // Variables Type -  Int16
  pIndex1->sidx.Base = (AM2321_ADDR<<8);          // Variable address

  // Register variable 2, Humidity Counter
  indextable_t * pIndex2;
  pIndex2 = getFreeIdxOD();
  if(pIndex2 == NULL)                             // No Memory
  {
    pIndex1->Index = 0xFFFF;                      // Free Index
    return 0;
  }

  pIndex2->cbRead  =  &twi_am2321_Read;
  pIndex2->cbWrite =  NULL;
  pIndex2->cbPool  =  &twi_am2321_Pool2;
  pIndex2->sidx.Place = objTWI;                   // Object TWI
  pIndex2->sidx.Type =  objUInt16;
  pIndex2->sidx.Base = (AM2321_ADDR<<8) + 1;      // Device address

  return 2;
}

#endif // (defined EXTDIO_USED) && (defined TWI_USED) && (defined TWI_USE_SHT21)