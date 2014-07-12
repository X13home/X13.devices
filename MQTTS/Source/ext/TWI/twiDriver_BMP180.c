/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// TWI Driver Bosh - BMP180/BMP085, Pressure & Temperature

#include "../../config.h"

#if (defined EXTDIO_USED) && (defined TWI_USED) && (defined TWI_USE_BMP180)

#include "../twim.h"
#include "twiDriver_BMP180.h"

// Outs
//  Tw30464     - Temperature       0,1°C
//  Td30464     - Pressure          0,01 hPa(mBar)

#define BMP180_ADDR                 0x77

#define BMP180_CHIP_ID              0x55
// Register definitions
#define BMP180_PROM_START__ADDR     0xAA
#define BMP180_CHIP_ID_REG          0xD0
#define BMP180_CTRL_MEAS_REG        0xF4
#define BMP180_ADC_OUT_MSB_REG      0xF6
//
#define BMP180_T_MEASURE            0x2E    // temperature measurent 
#define BMP180_P_MEASURE            0x34    // pressure measurement

#define SMD500_PARAM_MG             3038    // calibration parameter
#define SMD500_PARAM_MH            -7357    // calibration parameter
#define SMD500_PARAM_MI             3791    // calibration parameter

#define BMP180_OSS                    3       // oversampling settings [0 - Low Power / 3 - Ultra High resolution]

//#define BMP180_P_MIN_DELTA          20      // use hysteresis for pressure
//#define BMP180_T_MIN_DELTA          1       // use hysteresis for temperature

typedef struct
{
    // Calibration parameters
    int16_t  ac1;
    int16_t  ac2; 
    int16_t  ac3; 
    uint16_t ac4;
    uint16_t ac5;
    uint16_t ac6;
    int16_t  b1; 
    int16_t  b2;
    int16_t  mb;
    int16_t  mc;
    int16_t  md;
}s_bmp180_calib_t;

static s_bmp180_calib_t bmp180_calib;
static uint8_t bmp180_stat;
static int32_t  bmp180_b5;                        // Compensation parameter
static uint16_t bmp180_oldtemp = 0;
static uint32_t bmp180_oldpress = 0;
uint8_t         bmp180_Buf[3];

extern volatile uint8_t twim_access;           // access mode & busy flag

uint8_t twi_BMP180_Read(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf) 
{
  if(pSubidx->Base & 1)               // Read Pressure
  {
    *pLen = 4;
    *(uint32_t *)pBuf = bmp180_oldpress;
  }
  else                                // Read Temperature
  {
    *pLen = 2;
    *(uint16_t *)pBuf = bmp180_oldtemp;
  }
  return MQTTS_RET_ACCEPTED;
}

uint8_t twi_BMP180_Write(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf)
{
    if(pSubidx->Base & 1)               // Renew Pressure
        bmp180_oldpress = *(uint32_t *)pBuf;
    else                                // Renew Temperature
        bmp180_oldtemp = *(uint16_t *)pBuf;
    return MQTTS_RET_ACCEPTED;
}

uint8_t twi_BMP180_Poll1(subidx_t * pSubidx, uint8_t sleep)
{
  uint16_t ut;
  int32_t x1,x2;
#ifdef ASLEEP
  if(sleep != 0)
  {
    bmp180_stat = (0xFF-(POLL_TMR_FREQ/2));
    return 0;
  }
#endif  //  ASLEEP

  if(bmp180_stat > 4)
    return 0;

  if(bmp180_stat == 4) // Get uncompensated temperature, and normalize
  {
    ut = ((uint16_t)bmp180_Buf[0]<<8) | bmp180_Buf[1];

    // Calculate temperature
    x1 = (((int32_t)ut - bmp180_calib.ac6) * bmp180_calib.ac5) >> 15;
    x2 = ((int32_t)bmp180_calib.mc << 11) / (x1 + bmp180_calib.md);
    bmp180_b5 = x1 + x2;

    ut = (bmp180_b5 + 8)>>4;

#if (defined BMP180_T_MIN_DELTA) && (BMP180_T_MIN_DELTA > 0)
    x1 = ut > bmp180_oldtemp ? ut - bmp180_oldtemp : bmp180_oldtemp - ut;
    if(x1 > BMP180_T_MIN_DELTA)
#else
    if(ut != bmp180_oldtemp)
#endif  //  BMP180_T_MIN_DELTA
    {
      bmp180_stat++;
      bmp180_oldtemp = ut;
      return 1;
    }
  }
  else
  {
    if(twim_access != 0)              // Bus busy
    {
      if(twim_access & TWIM_ERROR)    // Bus Error
        bmp180_stat = 12;
      return 0;
    }

    if(bmp180_stat & 1)   // Get ut
    {
      bmp180_Buf[0] = BMP180_ADC_OUT_MSB_REG;            // Select ADC out register
      twimExch_ISR(BMP180_ADDR, (TWIM_WRITE | TWIM_READ), 1, 2, bmp180_Buf, NULL);
    }
    else  // Start Conversion, Temperature
    {
      bmp180_Buf[0] = BMP180_CTRL_MEAS_REG;
      bmp180_Buf[1] = BMP180_T_MEASURE;
      twimExch_ISR(BMP180_ADDR, TWIM_WRITE, 2, 0, bmp180_Buf, NULL);
      // !! ut Conversion time 4,5 mS
    }
  }
  bmp180_stat++;
  return 0;
}

uint8_t twi_BMP180_Poll2(subidx_t * pSubidx, uint8_t _unused)
{
  uint32_t up, b4, b7;
  int32_t b6, x1, x2, x3, b3, p;

  if(bmp180_stat < 5)
    return 0;

  if(bmp180_stat == 11)
  {
    bmp180_stat++;
    up = (((uint32_t)bmp180_Buf[0]<<16) | ((uint32_t)bmp180_Buf[1]<<8) |
          ((uint32_t)bmp180_Buf[2]))>>(8-BMP180_OSS);
    b6 = bmp180_b5 - 4000;
    //  calculate B3
    x1 = (b6 * b6)>>12;
    x1 *= bmp180_calib.b2;
    x1 >>= 11;
    x2 = bmp180_calib.ac2 * b6;
    x2 >>= 11;
    x3 = x1 + x2;
    b3 = (((((int32_t)bmp180_calib.ac1) * 4 + x3) << BMP180_OSS) + 2) >> 2;
    // calculate B4
    x1 = (bmp180_calib.ac3 * b6) >> 13;
    x2 = (bmp180_calib.b1 * ((b6*b6) >> 12)) >> 16;
    x3 = ((x1 + x2) + 2) >> 2;
    b4 = (bmp180_calib.ac4 * (uint32_t)(x3 + 32768)) >> 15;
    b7 = ((uint32_t)(up - b3) * (50000>>BMP180_OSS));
    if(b7 < 0x80000000)
      p = (b7 << 1) / b4;
    else
      p = (b7 / b4) << 1;
    x1 = (p >> 8);
    x1 *= x1;
    x1 = (x1 * SMD500_PARAM_MG) >> 16;
    x2 = (p * SMD500_PARAM_MH) >> 16;
    p += (x1 + x2 + SMD500_PARAM_MI) >> 4;

#if (defined BMP180_P_MIN_DELTA) && (BMP180_P_MIN_DELTA > 0)
    up = p > bmp180_oldpress ? p - bmp180_oldpress : bmp180_oldpress - p;
    if(up > BMP180_P_MIN_DELTA)
#else
    if(p != bmp180_oldpress)
#endif  //  BMP180_P_MIN_DELTA
    {
      bmp180_oldpress = p;
      return 1;
    }
  }
  else if(bmp180_stat < 11)
  {
    if(twim_access != 0)              // Bus busy
    {
      if(twim_access & TWIM_ERROR)    // Bus Error
        bmp180_stat = 12;
      return 0;
    }

    if((bmp180_stat & 5) == 5)        // Start conversion, Pressure
    {
      bmp180_Buf[0] = BMP180_CTRL_MEAS_REG;
      bmp180_Buf[1] = BMP180_P_MEASURE + (BMP180_OSS<<6);
      twimExch_ISR(BMP180_ADDR, TWIM_WRITE, 2, 0, bmp180_Buf, NULL);
    }
    else if((bmp180_stat & 5) == 0)   // Get up
    {
      // !! up Conversion time on ultra high resolution (BMP180_OSS = 3) 25,5 mS
      bmp180_Buf[0] = BMP180_ADC_OUT_MSB_REG;            // Select ADC out register
      twimExch_ISR(BMP180_ADDR, (TWIM_WRITE | TWIM_READ), 1, 3, bmp180_Buf, NULL);
    }
  }

  bmp180_stat++;
  return 0;
}

uint8_t twi_BMP180_Config(void)
{
  uint8_t reg = BMP180_CHIP_ID_REG;
  if((twimExch(BMP180_ADDR, (TWIM_READ | TWIM_WRITE), 1, 1, &reg) !=
                                                 TW_SUCCESS) ||             // Communication error
                                                (reg != BMP180_CHIP_ID))    // Bad device ID
    return 0;

  bmp180_calib.ac1 = BMP180_PROM_START__ADDR;
  twimExch(BMP180_ADDR, (TWIM_READ | TWIM_WRITE), 1, 22, (uint8_t *)&bmp180_calib);

  bmp180_stat = 0;
  bmp180_oldtemp = 0;
  bmp180_oldpress = 0;

  //parameters AC1-AC6
  bmp180_calib.ac1 = SWAPWORD(bmp180_calib.ac1);
  bmp180_calib.ac2 = SWAPWORD(bmp180_calib.ac2);
  bmp180_calib.ac3 = SWAPWORD(bmp180_calib.ac3);
  bmp180_calib.ac4 = SWAPWORD(bmp180_calib.ac4);
  bmp180_calib.ac5 = SWAPWORD(bmp180_calib.ac5);
  bmp180_calib.ac6 = SWAPWORD(bmp180_calib.ac6);
  //parameters B1,B2
  bmp180_calib.b1 =  SWAPWORD(bmp180_calib.b1);
  bmp180_calib.b2 =  SWAPWORD(bmp180_calib.b2);
  //parameters MB,MC,MD
  bmp180_calib.mb =  SWAPWORD(bmp180_calib.mb);
  bmp180_calib.mc =  SWAPWORD(bmp180_calib.mc);
  bmp180_calib.md =  SWAPWORD(bmp180_calib.md);

  // Register variables
  indextable_t * pIndex1;
  pIndex1 = getFreeIdxOD();
  if(pIndex1 == NULL)
    return 0;

  indextable_t * pIndex2;
  pIndex2 = getFreeIdxOD();
  if(pIndex2 == NULL)
  {
    pIndex1->Index = 0xFFFF;                // Free Index
    return 0;
  }

  pIndex1->cbRead  =  &twi_BMP180_Read;
  pIndex1->cbWrite =  &twi_BMP180_Write;
  pIndex1->cbPoll  =  &twi_BMP180_Poll1;
  pIndex1->sidx.Place = objTWI;               // Object TWI
  pIndex1->sidx.Type =  objInt16;             // Variables Type -  UInt16
  pIndex1->sidx.Base = (BMP180_ADDR<<8);      // Device address

  pIndex2->cbRead  =  &twi_BMP180_Read;
  pIndex2->cbWrite =  &twi_BMP180_Write;
  pIndex2->cbPoll  =  &twi_BMP180_Poll2;
  pIndex2->sidx.Place = objTWI;               // Object TWI
  pIndex2->sidx.Type =  objUInt32;            // Variables Type -  UInt32
  pIndex2->sidx.Base = (BMP180_ADDR<<8) + 1;  // Device address

  return 2;
}

#endif  //  (defined EXTDIO_USED) && (defined TWI_USED) && (defined TWI_USE_BMP180)