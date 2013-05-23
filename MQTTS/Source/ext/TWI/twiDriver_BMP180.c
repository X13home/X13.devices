/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

// TWI Driver Bosh - BMP180/BMP085, Pressure & Temperature

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

#define BMP180_OSS                  3       // oversampling settings [0 - Low Power / 3 - Ultra High resolution]

#define BMP180_P_MIN_DELTA          20
#define BMP180_T_MIN_DELTA          1

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
}s_BMP180_CALIB_t;

static s_BMP180_CALIB_t BMP180_calib;

static uint8_t BMP180_stat;

static int32_t  BMP180_b5;              // Compensation parameter
static uint16_t BMP180_oldtemp = 0;
static uint32_t BMP180_oldpress = 0;

static uint8_t twi_BMP180_Read(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf) 
{
    if(pSubidx->Base & 1)               // Read Pressure
    {
        *pLen = 4;
        *(uint32_t *)pBuf = BMP180_oldpress;
    }
    else                                // Read Temperature
    {
        *pLen = 2;
        *(uint16_t *)pBuf = BMP180_oldtemp;
    }
    return MQTTS_RET_ACCEPTED;
}

static uint8_t twi_BMP180_Write(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf)
{
    if(pSubidx->Base & 1)               // Renew Pressure
        BMP180_oldpress = *(uint32_t *)pBuf;
    else                                // Renew Temperature
        BMP180_oldtemp = *(uint16_t *)pBuf;
    return MQTTS_RET_ACCEPTED;
}

static uint8_t twi_BMP180_Pool1(subidx_t * pSubidx)
{
    uint16_t ut;
    int32_t x1,x2;

    if(twim_access & (TWIM_ERROR | TWIM_RELEASE))   // Bus Error, or request to release bus
    {
        if(BMP180_stat != 0)
        {
            BMP180_stat = 0x40;
            if(twim_access & TWIM_RELEASE)
                twim_access = TWIM_RELEASE;
        }
        return 0;
    }

    if(twim_access & (TWIM_READ | TWIM_WRITE))      // Bus Busy
        return 0;
    
    switch(BMP180_stat)
    {
        case 0:
            if(twim_access & TWIM_BUSY)
                return 0;
            BMP180_stat = 1;
        case 1:             // Start dummy Conversion, Temperature
        case 3:             // Start Conversion, Temperature
            twim_buf[0] = BMP180_CTRL_MEAS_REG;
            twim_buf[1] = BMP180_T_MEASURE;
            twimExch_ISR(BMP180_ADDR, (TWIM_BUSY | TWIM_WRITE), 2, 0, (uint8_t *)twim_buf, NULL);
            break;
        // !! ut Conversion time 4,5 mS
        case 2:             // Get dummy ut
        case 4:             // Get ut
            twim_buf[0] = BMP180_ADC_OUT_MSB_REG;            // Select ADC out register
            twimExch_ISR(BMP180_ADDR, (TWIM_BUSY | TWIM_WRITE | TWIM_READ), 1, 2, 
                                                                        (uint8_t *)twim_buf, NULL);
            break;
        case 5:             // Get uncompensated temperature, and normalize
            ut = ((uint16_t)twim_buf[0]<<8) | twim_buf[1];

            // Calculate temperature
            x1 = (((int32_t)ut - BMP180_calib.ac6) * BMP180_calib.ac5) >> 15;
            x2 = ((int32_t)BMP180_calib.mc << 11) / (x1 + BMP180_calib.md);
            BMP180_b5 = x1 + x2;

            ut = (BMP180_b5 + 8)>>4;
            
            x1 = ut > BMP180_oldtemp ? ut - BMP180_oldtemp : BMP180_oldtemp - ut;

            if(x1 > BMP180_T_MIN_DELTA)
            {
                BMP180_stat++;
                BMP180_oldtemp = ut;
                return 1;
            }
            break;
        case 6:             // Start dummy conversion, Pressure
        case 9:             // Start conversion, Pressure
            twim_buf[0] = BMP180_CTRL_MEAS_REG;
            twim_buf[1] = BMP180_P_MEASURE + (BMP180_OSS<<6);
            twimExch_ISR(BMP180_ADDR, (TWIM_BUSY | TWIM_WRITE), 2, 0, (uint8_t *)twim_buf, NULL);
            break;
        // !! up Conversion time on ultra high resolution (BMP180_OSS = 3) 25,5 mS
        case 8:             // Get dummy up
        case 11:            // Get up
            twim_buf[0] = BMP180_ADC_OUT_MSB_REG;            // Select ADC out register
            twimExch_ISR(BMP180_ADDR, (TWIM_BUSY | TWIM_WRITE | TWIM_READ), 1, 3,
                                                                        (uint8_t *)twim_buf, NULL);
            break;
    }
    BMP180_stat++;
    return 0;
}

static uint8_t twi_BMP180_Pool2(subidx_t * pSubidx)
{
    uint32_t up, b4, b7;
    int32_t b6, x1, x2, x3, b3, p;

    if(BMP180_stat == 13)
    {
        BMP180_stat++;
        up = (((uint32_t)twim_buf[0]<<16) | ((uint32_t)twim_buf[1]<<8) |
                       ((uint32_t)twim_buf[2]))>>(8-BMP180_OSS);
        twim_access = 0;    // Bus Free
        b6 = BMP180_b5 - 4000;
        //  calculate B3
        x1 = (b6 * b6)>>12;
        x1 *= BMP180_calib.b2;
        x1 >>= 11;
        x2 = BMP180_calib.ac2 * b6;
        x2 >>= 11;
        x3 = x1 + x2;
        b3 = (((((int32_t)BMP180_calib.ac1) * 4 + x3) << BMP180_OSS) + 2) >> 2;
        // calculate B4
        x1 = (BMP180_calib.ac3 * b6) >> 13;
        x2 = (BMP180_calib.b1 * ((b6*b6) >> 12)) >> 16;
        x3 = ((x1 + x2) + 2) >> 2;
        b4 = (BMP180_calib.ac4 * (uint32_t)(x3 + 32768)) >> 15;
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

        up = p > BMP180_oldpress ? p - BMP180_oldpress : BMP180_oldpress - p;
        if(up > BMP180_P_MIN_DELTA)
        {
            BMP180_oldpress = p;
            return 1;
        }
    }
    return 0;
}

static uint8_t twi_BMP180_Config(void)
{
    uint8_t reg = BMP180_CHIP_ID_REG;
    if((twimExch(BMP180_ADDR, (TWIM_READ | TWIM_WRITE), 1, 1, &reg) != 
                                                                TW_SUCCESS) ||  // Communication error
       (reg != BMP180_CHIP_ID))                                                 // Bad device ID
        return 0;

    // readout BMP180 calibparam structure
    BMP180_calib.ac1 = BMP180_PROM_START__ADDR;
    twimExch(BMP180_ADDR, (TWIM_READ | TWIM_WRITE), 1, 22, (uint8_t *)&BMP180_calib);

    BMP180_stat = 0;
    BMP180_oldtemp = 0;
    BMP180_oldpress = 0;

    //parameters AC1-AC6
    BMP180_calib.ac1 = SWAPWORD(BMP180_calib.ac1);
    BMP180_calib.ac2 = SWAPWORD(BMP180_calib.ac2);
    BMP180_calib.ac3 = SWAPWORD(BMP180_calib.ac3);
    BMP180_calib.ac4 = SWAPWORD(BMP180_calib.ac4);
    BMP180_calib.ac5 = SWAPWORD(BMP180_calib.ac5);
    BMP180_calib.ac6 = SWAPWORD(BMP180_calib.ac6);
    //parameters B1,B2
    BMP180_calib.b1 =  SWAPWORD(BMP180_calib.b1);
    BMP180_calib.b2 =  SWAPWORD(BMP180_calib.b2);
    //parameters MB,MC,MD
    BMP180_calib.mb =  SWAPWORD(BMP180_calib.mb);
    BMP180_calib.mc =  SWAPWORD(BMP180_calib.mc);
    BMP180_calib.md =  SWAPWORD(BMP180_calib.md);

    // Register variables
    indextable_t * pIndex1;
    pIndex1 = getFreeIdxOD();
    if(pIndex1 == NULL)
        return 0;

    pIndex1->cbRead  =  &twi_BMP180_Read;
    pIndex1->cbWrite =  &twi_BMP180_Write;
    pIndex1->cbPool  =  &twi_BMP180_Pool1;
    pIndex1->sidx.Place = objTWI;               // Object TWI
    pIndex1->sidx.Type =  objInt16;             // Variables Type -  UInt16
    pIndex1->sidx.Base = (BMP180_ADDR<<8);      // Device addr

    indextable_t * pIndex2;
    pIndex2 = getFreeIdxOD();
    if(pIndex2 == NULL)
    {
        pIndex1->Index = 0xFFFF;                // Free Index
        return 0;
    }

    pIndex2->cbRead  =  &twi_BMP180_Read;
    pIndex2->cbWrite =  &twi_BMP180_Write;
    pIndex2->cbPool  =  &twi_BMP180_Pool2;
    pIndex2->sidx.Place = objTWI;               // Object TWI
    pIndex2->sidx.Type =  objUInt32;            // Variables Type -  UInt32
    pIndex2->sidx.Base = (BMP180_ADDR<<8) + 1;  // Device addr

    return 2;
}