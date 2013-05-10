/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

// TWI Driver Honeywell - HIH6130/HIH6131/HIH6120/HIH6121,  Humidity & Temperature

// Outs
// Tw9984       Temperature Counter(TC)
// TB9985       Humidity Counter(HC)
// T°C = (TC * 55 / 5461) - 40
// RH% = HC * 20 / 51

#define HIH61XX_TWI_ADDR            0x27

#define HIH61XX_T_MIN_DELTA         6
#define HIH61XX_H_MIN_DELTA         1

static uint8_t  hih61xx_stat;
static uint8_t  hih61xx_oldhumi;
static uint16_t hih61xx_oldtemp;

static uint8_t twi_HIH61xx_Read(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
    if(pSubidx->Base & 1)               // Read Humidity
    {
        *pLen = 1;
        *pBuf = hih61xx_oldhumi;
        // Return Humidity %
//        *pBuf = ((uint16_t)hih61xx_oldhumi*25)>>6;
    }
    else                                // Read Temperature
    {
        *pLen = 2;
        *(uint16_t *)pBuf = hih61xx_oldtemp;
/*
        // Return T 0.1°C
        uint16_t temp = ((uint32_t)hih61xx_oldtemp * 825)>>13;
        temp -= 400;
        *(uint16_t *)pBuf = temp;
*/
    }
    return MQTTS_RET_ACCEPTED;
}

static uint8_t twi_HIH61xx_Write(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf)
{
    if(pSubidx->Base & 1)               // Renew Humidity
        hih61xx_oldhumi = *pBuf;
    else                                // Renew Temperature
        hih61xx_oldtemp = *(uint16_t *)pBuf;
    return MQTTS_RET_ACCEPTED;
}

static uint8_t twi_HIH61xx_Pool1(subidx_t * pSubidx)
{
    if(twim_access & (TWIM_ERROR | TWIM_RELEASE))   // Bus Error, or request to release bus
    {
        if(hih61xx_stat != 0)
        {
            hih61xx_stat = 0x40;
            if(twim_access & TWIM_RELEASE)
                twim_access = TWIM_RELEASE;
        }
        return 0;
    }
    
    if(twim_access & (TWIM_READ | TWIM_WRITE))      // Bus Busy
        return 0;

    switch(hih61xx_stat)
    {
        case 0:         // Start Conversion
            if(twim_access & TWIM_BUSY)
                return 0;
            hih61xx_stat = 1;
        case 1:
            twimExch_ISR(HIH61XX_TWI_ADDR, (TWIM_BUSY | TWIM_WRITE), 0, 0, NULL, NULL);
            break;
        case 4:         // !! The measurement cycle duration is typically 36.65 ms
            twimExch_ISR(HIH61XX_TWI_ADDR, (TWIM_BUSY | TWIM_READ), 0, 4, (uint8_t *)twim_buf, NULL);
            break;
        case 5:
            if((twim_buf[0] & 0xC0) != 0)   // data invalid
            {
                hih61xx_stat--;
                return 0;
            }

            {
            uint16_t temp = ((((uint16_t)twim_buf[2])<<6) | (twim_buf[3]>>2)) & 0x3FFF;
            uint16_t delta;
            delta = temp > hih61xx_oldtemp ? temp - hih61xx_oldtemp : hih61xx_oldtemp - temp;
            if(delta > HIH61XX_T_MIN_DELTA)
            {
                hih61xx_oldtemp = temp;
                hih61xx_stat++;
                return 1;
            }
            }
            break;
    }

    hih61xx_stat++;
    return 0;
}

static uint8_t twi_HIH61xx_Pool2(subidx_t * pSubidx)
{
    if(hih61xx_stat == 7)
    {
        hih61xx_stat++;
        uint8_t tmp = (twim_buf[0]<<2) | (twim_buf[1]>>6);
        twim_access = 0;        // Bus Free
        uint16_t delta;
        delta = tmp > hih61xx_oldhumi ? tmp - hih61xx_oldhumi : hih61xx_oldhumi - tmp;
        if(delta > HIH61XX_H_MIN_DELTA)
        {
            hih61xx_oldhumi = tmp;
            return 1;
        }
    }
    return 0;
}

static uint8_t twi_HIH61xx_Config(void)
{
    if(twimExch(HIH61XX_TWI_ADDR, TWIM_WRITE, 0, 0, NULL) != TW_SUCCESS)    // Communication error
        return 0;

    hih61xx_stat = 0;
    hih61xx_oldhumi = 0;
    hih61xx_oldtemp = 0;

    // Register variables
    indextable_t * pIndex1;
    pIndex1 = getFreeIdxOD();
    if(pIndex1 == NULL)
        return 0;

    pIndex1->cbRead  =  &twi_HIH61xx_Read;
    pIndex1->cbWrite =  &twi_HIH61xx_Write;
    pIndex1->cbPool  =  &twi_HIH61xx_Pool1;
    pIndex1->sidx.Place = objTWI;                   // Object TWI
    pIndex1->sidx.Type =  objInt16;                 // Variables Type -  UInt16
    pIndex1->sidx.Base = (HIH61XX_TWI_ADDR<<8);     // Device addr

    indextable_t * pIndex2;
    pIndex2 = getFreeIdxOD();
    if(pIndex2 == NULL)
    {
        pIndex1->Index = 0xFFFF;                    // Free Index
        return 0;
    }

    pIndex2->cbRead  =  &twi_HIH61xx_Read;
    pIndex2->cbWrite =  &twi_HIH61xx_Write;
    pIndex2->cbPool  =  &twi_HIH61xx_Pool2;
    pIndex2->sidx.Place = objTWI;                   // Object TWI
    pIndex2->sidx.Type =  objUInt8;
    pIndex2->sidx.Base = (HIH61XX_TWI_ADDR<<8) + 1; // Device addr

    return 2;
}
