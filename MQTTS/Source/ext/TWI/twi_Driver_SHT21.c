/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

// TWI Driver Sensirion  SHT21, Temperature & Humidity

// Outs
// TW16384      Temperature counter (TC)
// TW16385      Relative Humidity Counter(RH)

#include "../../config.h"

#if (defined EXTDIO_USED) && (defined TWI_USED) && (defined TWI_USE_SHT21)

#define SHT_CMD_START_T_HOLD    0xE3    // Trigger T measurement  with Hold
#define SHT_CMD_START_RH_HOLD   0xE5    // Trigger RH measurement  with Hold 
#define SHT_CMD_START_T         0xF3    // Trigger T measurement  without Hold
#define SHT_CMD_START_RH        0xF5    // Trigger RH measurement  without Hold 
#define SHT_WRITE               0xE6    // Write to User Register
#define SHT_READ                0xE7    // Read from user Register
#define SHT_RESET               0xEE    // Soft Reset

#include "../twim.h"
#include "twiDriver_SHT21.h"

extern volatile uint8_t twim_access;           // access mode & busy flag


// Process variables
static uint8_t  sht21_stat;
static uint16_t sht21_oldTemp;
static uint16_t sht21_oldHumi;
uint8_t         sht21_buf[2];


/*


void sht21_ReadStatus_cb(void)
{
  if(sht21_buf[0] & sht21_STATUS_NOT_READY)
    sht21_stat--;
  else
  {
    sht21_buf[0] = sht21_REG_DATA;
    twimExch_ISR(sht21_ADDR, (TWIM_BUSY | TWIM_WRITE | TWIM_READ), 1, 2, sht21_buf, NULL);
    sht21_stat++;
  }
}
*/
uint8_t twi_sht21_Read(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
    *pLen = 2;
    if(pSubidx->Base & 1)   // Read Humidity Counter
        *(uint16_t *)pBuf = sht21_oldHumi;
    else                    // Read Temperature counter TC
        *(uint16_t *)pBuf = sht21_oldTemp;

    return MQTTS_RET_ACCEPTED;
}

uint8_t twi_sht21_Pool1(subidx_t * pSubidx, uint8_t sleep)
{
  uint16_t sht21_tmp;
#ifdef ASLEEP
  if(sleep != 0)
  {
    sht21_stat = (0xFF-(POOL_TMR_FREQ/2));
    return 0;
  }
#endif  //  ASLEEP
  if(sht21_stat == 0)
  {
    if(twim_access & (TWIM_BUSY | TWIM_ERROR | TWIM_READ | TWIM_WRITE))
      return 0;
    sht21_stat = 1;
  }
  else
  {
    if(twim_access & TWIM_ERROR)      // Bus Error, or request to release bus
    {
      sht21_stat = 0x40;
      return 0;
    }

    if(twim_access & (TWIM_BUSY | TWIM_READ | TWIM_WRITE))      // Bus Busy
      return 0;
  }
  
  switch(sht21_stat)
  {
    case 1:   // Start T Conversion
      sht21_buf[0] = SHT_CMD_START_T;
      twimExch_ISR(sht21_ADDR, TWIM_WRITE, 1, 0, sht21_buf, NULL);
      break;
    case 2:
      sht21_buf[0] = SHT_CMD_START_T;
  
  
  }
  sht21_stat++;
  
/*
  switch(sht21_stat)
  {
    case 1:             // Start Dummy Conversion
      sht21_buf[0] = sht21_REG_CONFIG;
      sht21_buf[1] = (sht21_CONFIG_START | sht21_CONFIG_TEMPERATURE | sht21_CONFIG_FAST);
      twimExch_ISR(sht21_ADDR, (TWIM_BUSY | TWIM_WRITE), 2, 0, sht21_buf, NULL);
      break;
    case 3:             // Start Conversion, Humidity
      sht21_buf[0] = sht21_REG_CONFIG;
      sht21_buf[1] = (sht21_CONFIG_START | sht21_CONFIG_HUMIDITY);
      twimExch_ISR(sht21_ADDR, (TWIM_BUSY | TWIM_WRITE), 2, 0, sht21_buf, NULL);
      break;
    // !! Conversion Time 35mS - Normal / 18 mS - Fast
    case 6:             // Read Busy Flag
    case 12:
      sht21_buf[0] = sht21_REG_STATUS;
      twimExch_ISR(sht21_ADDR, (TWIM_BUSY | TWIM_WRITE | TWIM_READ), 1, 1, sht21_buf, &sht21_ReadStatus_cb);
      break;
    case 7:
    case 13:
    case 14:
      return 0;
    case 8:             // Read Humidity
      sht21_stat++;
      sht21_tmp = ((uint16_t)sht21_buf[0]<<4) | (sht21_buf[1]>>4);
#if (defined sht21_H_MIN_DELTA) && (sht21_H_MIN_DELTA > 0)
      if((sht21_tmp > sht21_oldHumi ? sht21_tmp - sht21_oldHumi : sht21_oldHumi - sht21_tmp)
                                                                             > sht21_H_MIN_DELTA)
#else
      if(sht21_tmp != sht21_oldHumi)
#endif  // sht21_H_MIN_DELTA
      {
        sht21_oldHumi = sht21_tmp;
        return 1;
      }
      return 0;
    case 9:             // Start Conversion Temperature
      sht21_buf[0] = sht21_REG_CONFIG;
      sht21_buf[1] = (sht21_CONFIG_START | sht21_CONFIG_TEMPERATURE);
      twimExch_ISR(sht21_ADDR, (TWIM_BUSY | TWIM_WRITE), 2, 0, sht21_buf, NULL);
      break;
  }
  sht21_stat++;
*/
  return 0;
}

uint8_t twi_sht21_Pool2(subidx_t * pSubidx, uint8_t _unused)
{
/*
  uint16_t sht21_tmp;

  if(sht21_stat == 14)
  {
    sht21_stat++;
    twim_access = 0;        // Bus Free

    sht21_tmp = ((uint16_t)sht21_buf[0]<<6) | (sht21_buf[1]>>2);
#if (defined sht21_T_MIN_DELTA) && (sht21_T_MIN_DELTA > 0)
    if((sht21_tmp > sht21_oldTemp ? sht21_tmp - sht21_oldTemp : sht21_oldTemp - sht21_tmp)
                                                                             > sht21_T_MIN_DELTA)
#else
    if(sht21_tmp != sht21_oldTemp)
#endif  // sht21_T_MIN_DELTA
    {
      sht21_oldTemp = sht21_tmp;
      return 1;
    }
  }
*/
  return 0;
}

uint8_t twi_SHT21_Config(void)
{
  uint8_t tmp;
  tmp = SHT_READ;

  if((twimExch(SHT21_ADDR, (TWIM_READ | TWIM_WRITE), 1, 1, &tmp) != TW_SUCCESS)     // Communication error
    return 0;

  sht21_buf[0] = SHT_WRITE;
  tmp &= 0x7A;  // Disable Chip Heater
  tmp |= 0x83;  // Disable OTP Reload, resolution 11/11 bit.
  sht21_buf[1] = tmp;
  twimExch(SHT21_ADDR, TWIM_WRITE, 2, 0, sht21_buf);

  sht21_stat = 0;
  sht21_oldTemp = 0;
  sht21_oldHumi = 0;

  // Register variable 1, Temperature Counter
  indextable_t * pIndex1;
  pIndex1 = getFreeIdxOD();
  if(pIndex1 == NULL)
    return 0;

  pIndex1->cbRead  =  &twi_sht21_Read;
  pIndex1->cbWrite =  NULL;
  pIndex1->cbPool  =  &twi_sht21_Pool1;
  pIndex1->sidx.Place = objTWI;                   // Object TWI
  pIndex1->sidx.Type =  objUInt16;                // Variables Type -  UInt16
  pIndex1->sidx.Base = (sht21_ADDR<<8);           // Variable address

  // Register variable 2, Humidity COunter
  indextable_t * pIndex2;
  pIndex2 = getFreeIdxOD();
  if(pIndex2 == NULL)                             // No Memory
  {
    pIndex1->Index = 0xFFFF;                      // Free Index
    return 0;
  }

  pIndex2->cbRead  =  &twi_sht21_Read;
  pIndex2->cbWrite =  NULL;
  pIndex2->cbPool  =  &twi_sht21_Pool2;
  pIndex2->sidx.Place = objTWI;                   // Object TWI
  pIndex2->sidx.Type =  objUInt16;
  pIndex2->sidx.Base = (sht21_ADDR<<8) + 1;       // Device address

  return 2;
}

#endif // (defined EXTDIO_USED) && (defined TWI_USED) && (defined TWI_USE_SHT21)