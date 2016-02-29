/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// Extensions analogue inputs

#include "../config.h"

#ifdef EXTAIN_USED

/////////////////////////////////////////////////////
// AIn Section ,   depended from extdio.c

#define AIN_MASK_SIZE   (uint8_t)((EXTAIN_MAXPORT_NR/8) + 1)

// AIn Variables

// Local Variables
static uint8_t  ain_mask[AIN_MASK_SIZE];
static uint8_t  ain_ref[EXTAIN_MAXPORT_NR];
static uint16_t ain_average;
static int16_t  ain_act_val[EXTAIN_MAXPORT_NR];

// AIn local subroutines
static uint8_t ainCheckBase(uint16_t base)
{
    uint8_t apin = hal_ain_base2apin(base);
    if(apin == 0xFF)
        return 2;

    if(ain_ref[apin] != 0xFF)
        return 1;

    uint8_t dpin = hal_ain_apin2dio(apin);

    if(dpin == 0xFE)                            // Analog Inputs without digital functions
        return 0;

    return dioCheckBase(dpin);
}

static uint8_t ainSubidx2Ref(subidx_t * pSubidx)
{
    switch(pSubidx->Type)
    {
#if ((EXTAIN_REF & 0x01) != 0)
        case objArefExt:
            return 0;
#endif
#if ((EXTAIN_REF & 0x02) != 0)
        case objArefVcc:
            return 1;
#endif
#if ((EXTAIN_REF & 0x04) != 0)
        case objArefInt1:
            return 2;
#endif
#if ((EXTAIN_REF & 0x08) != 0)
        case objArefInt2:
            return 3;
#endif
        default:
            return 0xFF;
    }
}

static void ain_mask_rs(uint8_t apin, uint8_t set)
{
    uint8_t mask = 1 << (apin & 7);
    uint8_t pos = (apin >> 3);

    if(set)
        ain_mask[pos] |= mask;
    else
        ain_mask[pos] &= ~mask;
}

static uint8_t ain_check_mask(uint8_t apin)
{
    uint8_t mask = 1 << (apin & 7);
    uint8_t pos = (apin >> 3);

    return ((ain_mask[pos] & mask) != 0);
}

// AIn specific subroutine
void ainLoadAverage(void)
{
    uint8_t len = sizeof(ain_average);
    ReadOD(objADCaverage, MQTTSN_FL_TOPICID_PREDEF, &len, (uint8_t *)&ain_average);
    
    if(ain_average > 65000)
        ain_average = OD_DEF_ADC_AVERAGE;
}

// Preinit AIn section
void ainInit()
{
    uint8_t apin;
    for(apin = 0; apin < EXTAIN_MAXPORT_NR; apin++)
    {
        ain_ref[apin] = 0xFF;
        ain_act_val[apin] = 0;
    }
    
    for(apin = 0; apin < AIN_MASK_SIZE; apin++)
        ain_mask[apin] = 0;

    ainLoadAverage();
}

// Check index analog input
bool ainCheckSubidx(subidx_t * pSubidx)
{
    return ((ainSubidx2Ref(pSubidx) != 0xFF) && (ainCheckBase(pSubidx->Base) != 2));
}

// Read analog inputs
static e_MQTTSN_RETURNS_t ainReadOD(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
    uint8_t apin = hal_ain_base2apin(pSubidx->Base);

    ain_mask_rs(apin, 0);   // Reset change mask

    *pLen = 2;
    // Prevent hard fault on ARM
    pBuf[0] = (uint8_t)(ain_act_val[apin] & 0xFF);
    pBuf[1] = (uint8_t)(ain_act_val[apin] >> 8);
    return MQTTSN_RET_ACCEPTED;
}

static e_MQTTSN_RETURNS_t ainWriteOD(subidx_t * pSubidx, uint8_t len __attribute__ ((unused)), uint8_t *pBuf)
{
    uint8_t apin = hal_ain_base2apin(pSubidx->Base);
    ain_mask_rs(apin, 0);   // Reset change mask
    ain_act_val[apin] = (pBuf[1]<<8) | (pBuf[0]);
    return MQTTSN_RET_ACCEPTED;
}

// Poll Procedure
static uint8_t ainPollOD(subidx_t * pSubidx)
{
    return ain_check_mask(hal_ain_base2apin(pSubidx->Base));
}

// Register analogue input
e_MQTTSN_RETURNS_t ainRegisterOD(indextable_t *pIdx)
{
    uint16_t base = pIdx->sidx.Base;
    if(ainCheckBase(base) != 0)
        return MQTTSN_RET_REJ_INV_ID;

    uint8_t apin = hal_ain_base2apin(base);
    ain_ref[apin] = ainSubidx2Ref(&pIdx->sidx);

    uint8_t dpin = hal_ain_apin2dio(apin);
    if(dpin < 0xFE)
        dioTake(dpin);

    // Configure PIN to Analog input
    hal_ain_configure(apin, ain_ref[apin]);

    pIdx->cbRead  = &ainReadOD;
    pIdx->cbWrite = &ainWriteOD;
    pIdx->cbPoll  = &ainPollOD;

    return MQTTSN_RET_ACCEPTED;
}

void ainDeleteOD(subidx_t * pSubidx)
{
    uint16_t base = pSubidx->Base;
    if(ainCheckBase(base) != 1)
        return;
    
    uint8_t apin = hal_ain_base2apin(base);
    ain_ref[apin] = 0xFF;

    // Release PIN
    hal_ain_configure(apin, 0xFF);
    uint8_t dpin = hal_ain_apin2dio(apin);
    if(dpin < 0xFE)
        dioRelease(dpin);
}

void ainProc(void)
{
    static int32_t ain_val;
    static uint16_t ain_cnt = 0;
    static uint8_t ain_pos = 0;

    if(ain_cnt == 0)
    {
        if(ain_ref[ain_pos] == 0xFF)
        {
            ain_pos++;
            if(ain_pos == EXTAIN_MAXPORT_NR)
                ain_pos = 0;
            return;
        }

        hal_ain_select(ain_pos, ain_ref[ain_pos]);
        ain_cnt = 1;
        ain_val = 0;
    }
    else if(ain_cnt < (ain_average + 5))
    {
        int16_t ain_tmp = hal_ain_get();
    
        ain_cnt++;
        if(ain_cnt > 4)
            ain_val += ain_tmp;
    }
    else
    {
        ain_val /= (ain_average + 1);

        if(ain_act_val[ain_pos] != ain_val)
        {
            ain_act_val[ain_pos] = ain_val;
            ain_mask_rs(ain_pos, 1);    // Set change mask
        }
        
        ain_cnt = 0;
        ain_pos++;
        if(ain_pos == EXTAIN_MAXPORT_NR)
            ain_pos = 0;
    }
}

#ifdef EXTPLC_USED
int16_t ainRead(subidx_t * pSubidx)
{
    uint16_t apin = hal_ain_base2apin(pSubidx->Base);
    if(apin < EXTAIN_MAXPORT_NR)
        return ain_act_val[apin];
    return 0;
}
#endif  //  EXTPLC_USED

// AIn Section
/////////////////////////////////////////////////////
#endif  //  EXTAIN_USED
