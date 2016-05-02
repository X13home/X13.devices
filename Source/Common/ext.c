/*
Copyright (c) 2011-2016 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#include "config.h"

#define EXT_MAX_PROC    8

typedef void (*cbEXT_t)(void);

static void * extProcCB[EXT_MAX_PROC];

// Initialise extensions
void extInit(void)
{
    uint8_t pos;
    for(pos = 0; pos < EXT_MAX_PROC; pos++)
        extProcCB[pos] = NULL;
    
#ifdef EXTDIO_USED
    dioInit();
#ifdef EXTAIN_USED
    ainInit();
#endif  //  EXTAIN_USED
#endif  //  EXTDIO_USED

#ifdef EXTTWI_USED
    twiInit();
#endif

#ifdef EXTSER_USED
    serInit();
#endif  //  EXTSER_USED

#ifdef EXTPLC_USED
    plcInit();
#endif  //  EXTPLC_USED
}

// Check Subindex: 0 - free / 1 - busy / 2 - invalid
bool extCheckSubidx(subidx_t * pSubidx)
{
    switch(pSubidx->Place)
    {
#ifdef EXTDIO_USED
        case objDin:
        case objDout:
            return dioCheckSubidx(pSubidx);

#ifdef EXTPWM_USED
        case objPWM:
            return pwmCheckSubidx(pSubidx);
#endif  //  EXTPWM_USED

#ifdef EXTAIN_USED
        case objAin:
            return ainCheckSubidx(pSubidx);
#endif  //  EXTAIN_USED
#endif  //  EXTDIO_USED

#ifdef EXTSER_USED
        case objSer:
            return serCheckSubidx(pSubidx);
#endif  //  EXTSER_USED

#ifdef EXTPLC_USED
        case objMerker:
            return plcCheckSubidx(pSubidx);
#endif  //  EXTPLC_USED

        default:
            break;
  }

  return false;
}

// Register Object
e_MQTTSN_RETURNS_t extRegisterOD(indextable_t * pIdx)
{
    pIdx->cbRead = NULL;
    pIdx->cbWrite = NULL;
    pIdx->cbPoll = NULL;

    switch(pIdx->sidx.Place)
    {
#ifdef EXTDIO_USED
        case objDin:        // Digital(bool) Input's
        case objDout:       // Digital(bool) Output's
            return dioRegisterOD(pIdx);

#ifdef EXTPWM_USED
        case objPWM:        // PWM
            return pwmRegisterOD(pIdx);
#endif  //  EXTPWM_USED

#ifdef EXTAIN_USED
        case objAin:        // Analog(int16_t) Input's
            return ainRegisterOD(pIdx);
#endif  //  EXTAIN_USED

#endif  //  EXTDIO_USED

#ifdef EXTSER_USED
        case objSer:        // User Serial I/O
            return serRegisterOD(pIdx);
#endif  //  EXTSER_USED

#ifdef EXTPLC_USED
        case objMerker:
            return plcRegisterOD(pIdx);
#endif  //  EXTPLC_USED

        default:
            break;
    }
    return MQTTSN_RET_REJ_NOT_SUPP;
}

// Delete Object
void extDeleteOD(subidx_t * pSubidx)
{
    // Delete Objects
    switch(pSubidx->Place)
    {
#ifdef EXTDIO_USED
        case objDin:
        case objDout:
            dioDeleteOD(pSubidx);
            break;

#ifdef EXTPWM_USED
        case objPWM:        // PWM
            pwmDeleteOD(pSubidx);
            break;
#endif  //  EXTPWM_USED
            
#ifdef EXTAIN_USED
        case objAin:
            ainDeleteOD(pSubidx);
            break;
#endif  //  EXTAIN_USED
#endif  //  EXTDIO_USED

#ifdef EXTSER_USED
        case objSer:
            serDeleteOD(pSubidx);
            break;
#endif  //  EXTSER_USED

        default:
            break;
    }
}


void extRegProc(void * cb)
{
    uint8_t pos;
    for(pos = 0; pos < EXT_MAX_PROC; pos++)
    {
        if(extProcCB[pos] == cb)
        {
            return;
        }
    }

    for(pos = 0; pos < EXT_MAX_PROC; pos++)
    {
        if(extProcCB[pos] == NULL)
        {
            extProcCB[pos] = cb;
            return;
        }
    }
}

void extProc(void)
{
    uint8_t pos;
    for(pos = 0; pos < EXT_MAX_PROC; pos++)
    {
        cbEXT_t cb = extProcCB[pos];
        if(cb != NULL)
        {
            cb();
        }
    }
}

#ifdef EXTPLC_USED
uint32_t ext_in(subidx_t * pSubidx)
{
    switch(pSubidx->Place)
    {
#ifdef EXTDIO_USED
        case objDin:
        case objDout:
            return dioRead(pSubidx);
#endif  //  EXTDIO_USED
#ifdef EXTAIN_USED
        case objAin:
            return ainRead(pSubidx);
#endif  //  EXTAIN_USED
        default:
            break;
    }

    return 0;
}

void ext_out(subidx_t * pSubidx, uint32_t val)
{
    switch(pSubidx->Place)
    {
#ifdef EXTDIO_USED
        case objDout:
            dioWrite(pSubidx, val != 0);
            break;
#endif  //  EXTDIO_USED
#ifdef EXTPWM_USED
        case objPWM:        // PWM
            if(val > 0xFFFF)
                val = 0xFFFF;
            hal_pwm_write(pSubidx->Base, val);
            break;
#endif  //  EXTPWM_USED
        default:
            break;
    }
}
#endif  // EXTPLC_USED
