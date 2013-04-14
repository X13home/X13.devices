/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

// TWI Driver SiLabs - Si114x, proximity/ambient light sensor

// Outs
// TW23040      Visible intens.
// TW23041      IR Intens.
// TW23042      PS1
// TW23043      PS2
// TW23044      PS3

#include "Si114x_defs.h"

#define SI114X_ADDR             0x5A
//#define NUM_PS_CHANNELS         3         // Number of Proximity Sense channels

typedef struct
{
    uint16_t    AlsVis;
    uint16_t    AlsIR;
#ifdef NUM_PS_CHANNELS
    uint16_t    PS1;
#if (NUM_PS_CHANNELS > 1)
    uint16_t    PS2;
#if (NUM_PS_CHANNELS > 2)
    uint16_t    PS3;
#endif  //  (NUM_PS_CHANNELS > 2)
#endif  //  (NUM_PS_CHANNELS > 1)
#endif  //  NUM_PS_CHANNELS
}s_SI114X_DATA;

static s_SI114X_DATA    si114xData;

static uint8_t twi_SI114X_Read(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
    *pLen = 2;
    switch(pSubidx->Base & 0xFF)
    {
        case 1:     // IR Intens
            *(uint16_t *)pBuf = si114xData.AlsIR;
            break;
#ifdef NUM_PS_CHANNELS
        case 2:     // IR Intens
            *(uint16_t *)pBuf = si114xData.PS1;
            break;
#if (NUM_PS_CHANNELS > 1)
        case 3:     // IR Intens
            *(uint16_t *)pBuf = si114xData.PS2;
            break;
#if (NUM_PS_CHANNELS > 2)
        case 4:     // IR Intens
            *(uint16_t *)pBuf = si114xData.PS3;
            break;
#endif  //  (NUM_PS_CHANNELS > 2)
#endif  //  (NUM_PS_CHANNELS > 1)
#endif  //  NUM_PS_CHANNELS
        default:    // Visible
            *(uint16_t *)pBuf = si114xData.AlsVis;
            break;
    }

    return MQTTS_RET_ACCEPTED;
}

static uint8_t twi_SI114X_Write(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf)
{
    return MQTTS_RET_ACCEPTED;
}

static uint8_t twi_SI114X_Pool(subidx_t * pSubidx)
{
    return 0;
}

static uint8_t twi_SI114X_Config(void)
{
    uint8_t reg = REG_PART_ID;
    if((twimExch(SI114X_ADDR, (TWIM_READ | TWIM_WRITE), 1, 1, &reg) != TW_SUCCESS) ||
       (reg < PART_ID_SI41) || (reg > PART_ID_SI43))                        // Bad device ID
        return 0;

    // Configure device
    // Send Hardware Key
    twim_buf[0] = REG_HW_KEY;
    twim_buf[1] = HW_KEY_VAL0;
    twimExch(SI114X_ADDR, TWIM_WRITE, 2, 0, twim_buf);
#ifdef NUM_PS_CHANNELS
    // Initialize LED Current
    twim_buf[0] = REG_PS_LED21;
    twim_buf[1] = ((MAX_LED_CURRENT<<4) | MAX_LED_CURRENT);     // Set MAX Current for LED 1 & 2
    twim_buf[2] = MAX_LED_CURRENT;                              // Set MAX Current for LED 3
    twimExch(SI114X_ADDR, TWIM_WRITE, 3, 0, twim_buf);
#endif  //  NUM_PS_CHANNELS

    // Initialize Task List
    uint8_t tmp;
    tmp = ALS_IR_TASK + ALS_VIS_TASK;
#ifdef NUM_PS_CHANNELS
    tmp += PS1_TASK;            // Si1141 - 1 PS LED
#if (NUM_PS_CHANNELS > 1)
    if(reg > PART_ID_SI41)
        tmp += PS2_TASK;        // Si1142 - 2 PS LEDs
#if (NUM_PS_CHANNELS > 2)
    if(reg > PART_ID_SI42)
        tmp += PS3_TASK;        // Si1143 - 3 PS LEDs
#endif  //  (NUM_PS_CHANNELS > 2)
#endif  //  (NUM_PS_CHANNELS > 1)
#endif  //  NUM_PS_CHANNELS
    // Write Value
    twim_buf[0] = REG_PARAM_WR;
    twim_buf[1] = tmp;
    twimExch(SI114X_ADDR, TWIM_WRITE, 2, 0, twim_buf);
    // Write Command
    twim_buf[0] = REG_COMMAND;
    twim_buf[1] = CMD_PARAM_SET | PARAM_CH_LIST;
    twimExch(SI114X_ADDR, TWIM_WRITE, 2, 0, twim_buf);
    
    // Recovery period the ADC takes before making a PS measurement.
    // Write Value
    twim_buf[0] = REG_PARAM_WR;
    twim_buf[1] = RECCNT_001;                               // 50 nS * 2^ADC_GAIN
    twimExch(SI114X_ADDR, TWIM_WRITE, 2, 0, twim_buf);
    // Write Command
    twim_buf[0] = REG_COMMAND;
    twim_buf[1] = CMD_PARAM_SET | PARAM_PS_ADC_COUNTER;
    twimExch(SI114X_ADDR, TWIM_WRITE, 2, 0, twim_buf);

    // Recovery period the ADC takes before making a ALS-VIS measurement.
    // Write Value
    twim_buf[0] = REG_PARAM_WR;
    twim_buf[1] = RECCNT_001;                               // 50 nS * 2^ADC_GAIN
    twimExch(SI114X_ADDR, TWIM_WRITE, 2, 0, twim_buf);
    // Write Command
    twim_buf[0] = REG_COMMAND;
    twim_buf[1] = CMD_PARAM_SET | PARAM_ALSVIS_ADC_COUNTER;
    twimExch(SI114X_ADDR, TWIM_WRITE, 2, 0, twim_buf);
    
    // Recovery period the ADC takes before making a ALS-IR measurement.
    // Write Value
    twim_buf[0] = REG_PARAM_WR;
    twim_buf[1] = RECCNT_001;                               // 50 nS * 2^ADC_GAIN
    twimExch(SI114X_ADDR, TWIM_WRITE, 2, 0, twim_buf);
    // Write Command
    twim_buf[0] = REG_COMMAND;
    twim_buf[1] = CMD_PARAM_SET | PARAM_ALSIR_ADC_COUNTER;
    twimExch(SI114X_ADDR, TWIM_WRITE, 2, 0, twim_buf);

    // Register variables
    indextable_t * pIndex1;
	indextable_t * pIndex2;
    pIndex1 = getFreeIdxOD();
    if(pIndex1 == NULL)
        return 0;
        
    pIndex2 = getFreeIdxOD();
    if(pIndex2 == NULL)
    {
        pIndex1->Index = 0xFFFF;
        return 0;
    }

    // Variable 1 - Visible Light
    pIndex1->cbRead  =  &twi_SI114X_Read;
    pIndex1->cbWrite =  &twi_SI114X_Write;
    pIndex1->cbPool  =  &twi_SI114X_Pool;
    pIndex1->sidx.Place = objTWI;               // Object TWI
    pIndex1->sidx.Type =  objUInt16;            // Variables Type -  UInt16
    pIndex1->sidx.Base = (SI114X_ADDR<<8);      // Device addr
    
    // Variable 1 - Visible Light
    pIndex2->cbRead  =  &twi_SI114X_Read;
    pIndex2->cbWrite =  &twi_SI114X_Write;
    pIndex2->cbPool  =  &twi_SI114X_Pool;
    pIndex2->sidx.Place = objTWI;               // Object TWI
    pIndex2->sidx.Type =  objUInt16;            // Variables Type -  UInt16
    pIndex2->sidx.Base = ((SI114X_ADDR<<8) + 1);  // Device addr

    return 1;
}
