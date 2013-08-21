/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

// Extensions prototypes

#include "config.h"
#include "ext.h"
#include "util.h"

#ifdef EXTAI_USED
#include "ext/extai.h"
#endif  //  EXTAI_USED

#ifdef EXTDIO_USED
#include "ext/extdio.h"
#endif  //  EXTDIO_USED

#ifdef TWI_USED
#include "ext/twim.h"
#endif  //  TWI_USED

#if (defined EXTSER_TX_USED) || (defined EXTSER_RX_USED)
#include "ext/extSer.h"
#endif  //  EXTSER_TX_USED || EXTSER_RX_USED

// Delete All objects, & init hardware
void extClean(void)
{
#ifdef EXTAI_USED
    aiClean();
#endif  //  EXTAI_USED
#ifdef EXTDIO_USED
    dioClean();
#endif  //  EXTDIO_USED
#if (defined EXTSER_TX_USED) || (defined EXTSER_RX_USED)
    serClean();
#endif  //  EXTSER_TX_USED || EXTSER_RX_USED
#ifdef TWI_USED
    twiClean();
#endif  //  TWI_USED
}

// Configure & Register PnP devices
void extConfig(void)
{
#ifdef TWI_USED
    twiConfig();
#endif  //  TWI_USED
}

uint8_t extRegisterOD(indextable_t * pIdx)
{
    switch(pIdx->sidx.Place)
    {
#ifdef EXTAI_USED
        case objAin:        // Analogue Input's(unsigned word)
            return aiRegisterOD(pIdx);
#endif  //  EXTAI_USED
#ifdef EXTDIO_USED
        case objDin:        // Digital(bool) Input's
        case objDout:       // Digital(bool) Output's
#ifdef EXTPWM_USED
        case objPWM:        // LED HW PWM
#endif  //  EXTPWM_USED
            return dioRegisterOD(pIdx);
#endif  //  EXTDIO_USED
#if (defined EXTSER_TX_USED) || (defined EXTSER_RX_USED)
        case objSer:        // Serial IO
            return serRegisterOD(pIdx);
#endif  //  EXTSER_TX_USED || EXTSER_RX_USED
    }
    return MQTTS_RET_REJ_NOT_SUPP;
}

uint8_t extCheckIdx(subidx_t * pSubidx)
{
    switch(pSubidx->Place)
    {
#ifdef EXTAI_USED
        case objAin:
            return aiCheckIdx(pSubidx);
#endif  //  EXTAI_USED
#ifdef EXTDIO_USED
        case objDin:
        case objDout:
#ifdef EXTPWM_USED
        case objPWM:        // LED HW PWM
#endif  //  EXTPWM_USED
            return dioCheckIdx(pSubidx);
#endif  //  EXTDIO_USED
#if (defined EXTSER_TX_USED) || (defined EXTSER_RX_USED)
        case objSer:        // Serial IO
            return serCheckIdx(pSubidx);
#endif  //  EXTSER_TX_USED || EXTSER_RX_USED
    }
    return MQTTS_RET_REJ_NOT_SUPP;
}

void extDeleteOD(subidx_t * pSubidx)
{
    // Delete Objects
    switch(pSubidx->Place)
    {
#ifdef EXTAI_USED
        case objAin:
            aiDeleteOD(pSubidx);
            break;
#endif  //  EXTAI_USED
#ifdef EXTDIO_USED
        case objDin:
        case objDout:
#ifdef EXTPWM_USED
        case objPWM:        // LED HW PWM
#endif  //  EXTPWM_USED
            dioDeleteOD(pSubidx);
            break;
#endif  //  EXTDIO_USED
#if (defined EXTSER_TX_USED) || (defined EXTSER_RX_USED)
        case objSer:        // Serial IO
            serDeleteOD(pSubidx);
            break;
#endif  //  EXTSER_TX_USED || EXTSER_RX_USED
    }
}

// Convert Index to Topic ID
uint8_t extCvtIdx2TopicId(subidx_t * pSubidx, uint8_t * pPnt)
{
    uint8_t size;
    *(uint8_t*)(pPnt++) = pSubidx->Place;
    *(uint8_t*)(pPnt++) = pSubidx->Type;
    uint16_t addr = pSubidx->Base;
#ifdef EXTAI_USED
    if(pSubidx->Place == objAin)
        addr &= EXTAI_CHN_MASK;
#endif  // EXTAI_USED
    size = sprintdec((uint8_t*)pPnt, addr);
    return (size + 2);
}
