/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// TWI Driver Sensirion  SHT21, Temperature & Humidity

// Outs
// TW16384      Temperature counter (TC)
// A*175.72/65536-46.85
// TW16385      Relative Humidity Counter(RH)
// A*125/65536-6

#include "../../config.h"

#if (defined EXTDIO_USED) && (defined TWI_USED) && (defined TWI_USE_SHT21)

#include "../twim.h"
#include "twiDriver_SHT21.h"

#define SHT21_ADDR              0x40

#define SHT_CMD_START_T_HOLD    0xE3    // Trigger T measurement  with Hold
#define SHT_CMD_START_RH_HOLD   0xE5    // Trigger RH measurement  with Hold 
#define SHT_CMD_START_T         0xF3    // Trigger T measurement  without Hold
#define SHT_CMD_START_RH        0xF5    // Trigger RH measurement  without Hold 
#define SHT_WRITE               0xE6    // Write to User Register
#define SHT_READ                0xE7    // Read from user Register
#define SHT_RESET               0xEE    // Soft Reset

extern volatile uint8_t twim_access;           // access mode & busy flag

// Process variables
static uint8_t  sht21_stat;
static uint16_t sht21_oldTemp;
static uint16_t sht21_oldHumi;
uint8_t         sht21_buf[2];

void sht21_cb(void)
{
  if(twim_access & TWIM_READ)
    twim_access = 0;
  else
  {
    twim_access &= ~TWIM_BUSY;
    if(sht21_stat < 7)
      sht21_stat = 7;
    else
      sht21_stat = 13;
  }
}

uint8_t twi_sht21_Read(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
  *pLen = 2;
  if(pSubidx->Base & 1)   // Read Humidity Counter
    *(uint16_t *)pBuf = sht21_oldHumi;
  else                    // Read Temperature counter TC
    *(uint16_t *)pBuf = sht21_oldTemp;

  return MQTTS_RET_ACCEPTED;
}

uint8_t twi_sht21_Poll1(subidx_t * pSubidx, uint8_t sleep)
{
  uint16_t sht21_tmp;
#ifdef ASLEEP
  if(sleep != 0)
  {
    sht21_stat = (0xFF-(POLL_TMR_FREQ/2));
    return 0;
  }
#endif  //  ASLEEP
  if(twim_access & TWIM_ERROR)      // Bus Error, or request to release bus
  {
    if((sht21_stat <= 11) && (sht21_stat > 0))
      sht21_stat = 12;
    return 0;
  }

  switch(sht21_stat)
  {
    case 0:   // Start Temperature Conversion
      if(twim_access != 0)
        return 0;
      sht21_buf[0] = SHT_CMD_START_T;
      twimExch_ISR(SHT21_ADDR, TWIM_WRITE, 1, 0, sht21_buf, NULL);
      break;
    case 1:   // Temperature - 11bit
    case 2:   // Temperature - 12bit
    case 3:   // Temperature - 13 bit
    case 5:   // Temperature - 14 bit
    case 9:   // Humidity 8/10 bit
    case 10:  // Humidity 11/12 bit
      if(twim_access != 0)
        return 0;
      twimExch_ISR(SHT21_ADDR, TWIM_READ, 0, 2, sht21_buf, sht21_cb);
      break;
    case 6:   // Get Temperature - Error
    case 11:  // Get Humidity - Error
      sht21_stat = 16;
      break;
    case 7:   // Read Temperature
      sht21_tmp = ((uint16_t)sht21_buf[0]<<8) | (sht21_buf[1] & 0xFC);
      if(sht21_oldTemp != sht21_tmp)
      {
        sht21_oldTemp = sht21_tmp;
        sht21_stat++;
        return 1;
      }
      break;
    case 8:   // Start humidity conversion
      if(twim_access != 0)
        return 0;

      sht21_buf[0] = SHT_CMD_START_RH;
      twimExch_ISR(SHT21_ADDR, TWIM_WRITE, 1, 0, sht21_buf, NULL);
      break;
  }
  sht21_stat++;
  return 0;
}

uint8_t twi_sht21_Poll2(subidx_t * pSubidx, uint8_t _unused)
{
  uint16_t sht21_tmp;
  if(sht21_stat == 15)
  {
    sht21_stat++;
    sht21_tmp = ((uint16_t)sht21_buf[0]<<8) | (sht21_buf[1] & 0xFC);
    if(sht21_tmp != sht21_oldHumi)
    {
      sht21_oldHumi = sht21_tmp;
      return 1;
    }
  }
  return 0;
}

uint8_t twi_SHT21_Config(void)
{
  uint8_t tmp;
  tmp = SHT_RESET;
  if(twimExch(SHT21_ADDR, TWIM_WRITE, 1, 0, &tmp) != TW_SUCCESS)     // Communication error
    return 0;

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
  pIndex1->cbPoll  =  &twi_sht21_Poll1;
  pIndex1->sidx.Place = objTWI;                   // Object TWI
  pIndex1->sidx.Type =  objUInt16;                // Variables Type -  UInt16
  pIndex1->sidx.Base = (SHT21_ADDR<<8);           // Variable address

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
  pIndex2->cbPoll  =  &twi_sht21_Poll2;
  pIndex2->sidx.Place = objTWI;                   // Object TWI
  pIndex2->sidx.Type =  objUInt16;
  pIndex2->sidx.Base = (SHT21_ADDR<<8) + 1;       // Device address

  return 2;
}

#endif  //  (defined EXTDIO_USED) && (defined TWI_USED) && (defined TWI_USE_SHT21)