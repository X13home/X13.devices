/*
Copyright (c) 2011-2016 <comparator@gmx.de>

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
    uint16_t base = pSubidx->Base;
    if(((type != objPinNPN) && (type != objPinPNP)) ||
        (hal_dio_base2pin(base) == 0xFF) ||
        (hal_pwm_checkbase(base) != true))
        return false;

    return true;
}

static e_MQTTSN_RETURNS_t pwmWriteOD(subidx_t * pSubidx, uint8_t len __attribute__ ((unused)), uint8_t *pBuf)
{
    // Prevent hard fault on ARM
    uint16_t value = pBuf[1];
    value <<= 8;
    value |= pBuf[0];
    hal_pwm_write(pSubidx->Base, value);

    return MQTTSN_RET_ACCEPTED;
}

e_MQTTSN_RETURNS_t pwmRegisterOD(indextable_t *pIdx)
{
    uint16_t base = pIdx->sidx.Base;
    uint8_t dio = hal_dio_base2pin(base);
    
    if((dioCheckBase(dio) != 0) || hal_pwm_busy(base))
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
    dioRelease(hal_dio_base2pin(base));
}
#endif  // ((defined EXTDIO_USED) && (defined EXTPWM_USED))
