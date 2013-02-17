/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

// TWI Driver LM75, Temperature

// Outs
// Tw(addr | pos) ADC Register (Temp * 256)

#define LM75_START_ADDR             0x48
#define LM75_STOP_ADDR              0x4F

#define LM75_MAX_DEV                8           // 8/4/2/1

// LM75 Registers
#define LM75_REG_TEMP               0x00        // Temperature
#define LM75_REG_CONF               0x01        // Configuration
#define LM75_REG_THYST              0x02        // Hysteresis
#define LM75_REG_TOS                0x03        // Overtemperature

// Config Register
#define LM75_CONFIG_PD              0x01        // shutdown, 0 - Normal operation, 1 - shutdown
#define LM75_CONFIG_OS_MODE_INT     0x02        // OS mode 0 -  Comparator, 1 - interrupt
#define LM75_CONFIG_OS_POL_HI       0x04        // OS active level; 0 - Low, 1 - High
#define LM75_CONFIG_QS_QUE_1        0x00        // OS fault queue = 1
#define LM75_CONFIG_QS_QUE_2        0x08        // OS fault queue = 2
#define LM75_CONFIG_QS_QUE_4        0x10        // OS fault queue = 4
#define LM75_CONFIG_QS_QUE_6        0x18        // OS fault queue = 6

// Process variables
static uint8_t  lm75_stat[LM75_MAX_DEV];
static int16_t  lm75_oldVal[LM75_MAX_DEV];

static uint8_t twi_lm75_Read(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
    *pLen = 2;
    *(uint16_t *)pBuf = lm75_oldVal[pSubidx->Base & (LM75_MAX_DEV - 1)];
    return MQTTS_RET_ACCEPTED;
}

static uint8_t twi_lm75_Pool(subidx_t * pSubidx)
{
    uint8_t base = pSubidx->Base & (LM75_MAX_DEV - 1);
    uint16_t val;

    if(twim_access & TWIM_ERROR)
    {
        lm75_stat[base] = 0x80;
        return 0;
    }

    switch(lm75_stat[base])
    {
        case 0:
            if(twim_access & TWIM_BUSY)
                return 0;
            lm75_stat[base] = 1;
        case 1:
            twim_buf[0] = LM75_REG_TEMP;
            twimExch_ISR(pSubidx->Base>>8, (TWIM_BUSY | TWIM_SEQ | TWIM_WRITE | TWIM_READ), 1, 2,
                                                                    (uint8_t *)twim_buf);
            break;
        case 2:
            val = ((uint16_t)twim_buf[0]<<8) | (twim_buf[1]);
            lm75_stat[base]++;

            if(val != lm75_oldVal[base])
            {
                lm75_oldVal[base] = val;
                return 1;
            }
        case 3:
            twim_access = 0;        // Bus Free
            break;
    }

    lm75_stat[base]++;
    return 0;
}

static uint8_t twi_LM75_Config(void)
{
    uint8_t addr = LM75_START_ADDR;
    uint8_t pos = 0;
    
    indextable_t * pIndex;

    while((addr <= LM75_STOP_ADDR) && (pos < LM75_MAX_DEV))
    {
        twim_buf[0] = LM75_REG_CONF;
        twim_buf[1] = 0;    

        if(twimExch(addr, TWIM_WRITE, 2, 0, twim_buf) == TW_SUCCESS)
        {
            lm75_stat[pos] = 0x80;
            lm75_oldVal[pos] = 0;

            // Register variable
            pIndex = getFreeIdxOD();
            if(pIndex == NULL)
                break;
            
            pIndex->Index = 0;
            pIndex->cbRead  =  &twi_lm75_Read;
            pIndex->cbWrite =  NULL;
            pIndex->cbPool  =  &twi_lm75_Pool;
            pIndex->sidx.Place = objTWI;                   // Object TWI
            pIndex->sidx.Type =  objInt16;                 // Variables Type -  UInt16
            pIndex->sidx.Base = ((uint16_t)addr<<8) | pos;

            pos++;
        }
        addr++;
    }
    return pos;
}
