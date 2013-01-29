/*
Copyright (c) 2011-2012 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com/

BSD License
See LICENSE.txt file for license details.
*/

// TWI Driver LM75, Temperature

// Outs
// Tw(addr) ADC Register

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
static uint8_t  lm75_addr[LM75_MAX_DEV];
static int16_t  lm75_oldVal[LM75_MAX_DEV];

static uint8_t twi_lm75_Read(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
    *pLen = 2;
    *(uint16_t *)pBuf = lm75_oldVal[pSubidx->Base & (LM75_MAX_DEV - 1)];
    return MQTTS_RET_ACCEPTED;
}

static uint8_t twi_lm75_Pool(subidx_t * pSubidx)
{
    uint8_t  * pStat;
    pStat = &lm75_stat[pSubidx->Base & (LM75_MAX_DEV - 1)];

    switch(*pStat)
    {
        case 0:
            if(twim_access)
                return 0;
            *pStat = 1;
            break;
    }

    *pStat++;
    return 0;

/*
    switch(si7005_stat)
    {
        case 0:
            if(twim_access)
                return 0;
            si7005_stat = 1;
        case 1:             // Start Conversion, Temperature
        case 6:
            twim_buf[0] = SI7005_REG_CONFIG;
            twim_buf[1] = (SI7005_CONFIG_START | SI7005_CONFIG_TEMPERATURE);
            twimExch_ISR(SI7005_ADDR, (TWIM_BUSY | TWIM_WRITE), 2, 0, (uint8_t *)twim_buf);
            break;
        // !! Conversion Time 35mS - Normal / 18 mS - Fast
        case 4:     // Read Busy Flag
        case 9:
        case 14:
            twim_buf[0] = SI7005_REG_STATUS;
            twimExch_ISR(SI7005_ADDR, (TWIM_BUSY | TWIM_SEQ | TWIM_WRITE | TWIM_READ), 1, 1, 
                         (uint8_t *)twim_buf);
            break;
        case 5:
        case 10:
        case 15:
            if(twim_buf[0] & SI7005_STATUS_NOT_READY)           // Busy
            {
                si7005_stat--;
                return 0;
            }
            // Read Data
            twim_buf[0] = SI7005_REG_DATA;
            twimExch_ISR(SI7005_ADDR, (TWIM_BUSY | TWIM_SEQ | TWIM_WRITE | TWIM_READ), 1, 2, 
                         (uint8_t *)twim_buf);
            break;
        case 11:                                                // Calculate & test temperature
            {
            uint16_t val = ((uint16_t)twim_buf[0]<<5) | (twim_buf[1]>>3);

            // Start Conversion, Humidity
            twim_buf[0] = SI7005_REG_CONFIG;
            twim_buf[1] = (SI7005_CONFIG_START | SI7005_CONFIG_HUMIDITY);
            twimExch_ISR(SI7005_ADDR, (TWIM_BUSY | TWIM_WRITE), 2, 0, (uint8_t *)twim_buf);

            val *= 5;
            val >>=3;
            val -= 500;
            if(val != si7005_oldTemp)
            {
                si7005_oldTemp = val;
                si7005_stat++;
                return 1;
            }
            }
            break;
    }

    twim_access = 0;        // Bus Free
*/
}

static uint8_t twi_LM75_Config(void)
{
    uint8_t addr = LM75_START_ADDR;
    uint8_t pos = 0;

    indextable_t index;
    
    index.cbRead  =  &twi_lm75_Read;
    index.cbWrite =  NULL;
    index.cbPool  =  &twi_lm75_Pool;

    index.sidx.Place = objTWI;                      // Object TWI
    index.sidx.Type =  objInt16;                    // Variables Type -  Int16
    
    while((addr <= LM75_STOP_ADDR) && (pos < LM75_MAX_DEV))
    {
        if(twimExch(addr, TWIM_WRITE, 0, 0, NULL) == TW_SUCCESS)
        {
            lm75_stat[pos] = 0x80;
            lm75_addr[pos] = addr;
            lm75_oldVal[pos] = 0;
            
            // Register variable
            index.sidx.Base = ((uint16_t)addr<<8);  // Device addr
            if(RegistIntOD(&index) != MQTTS_RET_ACCEPTED)
                break;
            pos++;
        }
        addr++;
    }

    return pos;
}
