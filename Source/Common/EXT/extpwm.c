/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// Extensions PWM

#include "../config.h"

#if ((defined EXTDIO_USED) && (defined EXTPWM_USED))
    
/////////////////////////////////////////////////////
// PWM Section ,   depended from extdio.c

bool pwmCheckSubidx(subidx_t * pSubidx)
{
    eObjTyp_t type = pSubidx->Type;
    
    if(((type != objPinNPN) && (type != objPinPNP)) || 
        (hal_pwm_base2dio(pSubidx->Base) == 0xFF))
        return false;

    return true;
}

#if defined ( __GNUC__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
static e_MQTTSN_RETURNS_t pwmWriteOD(subidx_t * pSubidx, uint8_t Unused, uint8_t *pBuf)
{
    // Prevent hard fault on ARM
    uint16_t value = pBuf[1];
    value <<= 8;
    value |= pBuf[0];
    hal_pwm_write(pSubidx->Base, value);

    return MQTTSN_RET_ACCEPTED;
}
#if defined ( __GNUC__ )
#pragma GCC diagnostic pop
#endif

e_MQTTSN_RETURNS_t pwmRegisterOD(indextable_t *pIdx)
{
    uint16_t base = pIdx->sidx.Base;
    uint8_t dio = hal_pwm_base2dio(base);
    
    if(dioCheckBase(dio) != 0)
        return MQTTSN_RET_REJ_INV_ID;
    
    dioTake(dio);
    hal_pwm_configure(base, (pIdx->sidx.Type == objPinNPN));

    pIdx->cbWrite = &pwmWriteOD;
    
    return MQTTSN_RET_ACCEPTED;
}

void pwmDeleteOD(subidx_t * pSubidx)
{
    uint16_t base = pSubidx->Base;

    hal_pwm_delete(base);
    dioRelease(hal_pwm_base2dio(base));
}

#ifdef EXTPLC_USED
void pwmWrite(subidx_t *pSubidx, uint16_t val)
{
    uint16_t base = pSubidx->Base;
    if(hal_pwm_base2dio(base) == 0xFF)
        return;

    hal_pwm_write(pSubidx->Base, val);
}
#endif  //  EXTPLC_USED

#endif  // ((defined EXTDIO_USED) && (defined EXTPWM_USED))
