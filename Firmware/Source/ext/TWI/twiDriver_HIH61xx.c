/*
Copyright (c) 2011-2012 <comparator@gmx.de>

This file is part of the X13.Home project.
https://github.com/X13home

BSD License
See LICENSE.txt file for license details.
*/

// TWI Driver Honeywell - HIH6130/HIH6131/HIH6120/HIH6121,  Humidity & Temperature

// Outs
// TW(addr)   Temperature 0.1°C
//  TB(add+1)  Relative Humidity %

#define HIH61XX_TWI_ADDR            0x27

static uint8_t  hih61xx_stat;
static uint8_t  hih61xx_oldhumi;
static uint16_t hih61xx_oldtemp;

static uint8_t twi_HIH61xx_Read(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
    if(pSubidx->Base & 1)               // Read Humidity
    {
        *pLen = 1;
        *pBuf = hih61xx_oldhumi;
    }
    else                                // Read Temperature
    {
        *pLen = 2;
        *(uint16_t *)pBuf = hih61xx_oldtemp;
    }
    return MQTTS_RET_ACCEPTED;
}

static uint8_t twi_HIH61xx_Pool1(subidx_t * pSubidx)
{
    if(hih61xx_stat == 0)  // Start Conversion
    {
        if(twi_bus_busy)
            return 0;
        hih61xx_stat = 1;
        twi_bus_busy = 1;
        twiExch_ISR(HIH61XX_TWI_ADDR, TW_WRITE, 0);
    }
    // !!Conversion Time > 35 mS
    else if(hih61xx_stat == 4)
        twiExch_ISR(HIH61XX_TWI_ADDR, TW_READ, 4);
    else if(hih61xx_stat == 5)
    {
        if((twi_trv_buf[0] & 0xC0) != 0)   // data invalid
        {
            hih61xx_stat--;
            return 0;
        }
        
        uint16_t temp = ((((uint16_t)twi_trv_buf[2])<<6) | (twi_trv_buf[3]>>2)) & 0x3FFF;
        temp = ((uint32_t)temp * 825)>>13;
        temp -= 400;
        if(temp != hih61xx_oldtemp)
        {
            hih61xx_oldtemp = temp;
            hih61xx_stat++;
            return 1;
        }
    }

    hih61xx_stat++;
    return 0;
}

static uint8_t twi_HIH61xx_Pool2(subidx_t * pSubidx)
{
    if(hih61xx_stat == 7)
    {
        twi_bus_busy = 0;
        hih61xx_stat++;
        uint16_t humi = (((uint16_t)twi_trv_buf[0]<<4) | (twi_trv_buf[1]>>4)) & 0x3FF;
        humi++;
        humi *= 25;
        uint8_t tmp = humi>>8;
        if(tmp != hih61xx_oldhumi)
        {
            hih61xx_oldhumi = tmp;
            return 1;
        }
    }
    return 0;
}

static uint8_t twi_HIH61xx_Config(void)
{
    if(twiExch(HIH61XX_TWI_ADDR, TW_WRITE, 0, NULL) != TW_SUCCESS)
        return 0;

    hih61xx_stat = 0;
    hih61xx_oldhumi = 0;
    hih61xx_oldtemp = 0;

    indextable_t index;
    // Register variables
    index.cbRead  =  &twi_HIH61xx_Read;
    index.cbWrite =  NULL;
    index.cbPool  =  &twi_HIH61xx_Pool1;

    index.sidx.Place = objTWI;                  // Object TWI
    index.sidx.Type =  objInt16;                // Variables Type -  UInt16
    index.sidx.Base = (HIH61XX_TWI_ADDR<<8);    // Device addr
    
    if(RegistIntOD(&index) != MQTTS_RET_ACCEPTED)
        return 0;

    // Register variable 2
    index.cbPool  =  &twi_HIH61xx_Pool2;
    index.sidx.Type =  objUInt8;                // Variables Type -  UInt32
    index.sidx.Base++;

    if(RegistIntOD(&index) != MQTTS_RET_ACCEPTED)
        return 0;

    return 2;
}
