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
    
#include "extdio.h"
#include "extpwm.h"

//////////////////////////////////////////////////////////////
// PWM HAL
uint8_t hal_pwm_base2dio(uint16_t base);
void hal_pwm_configure(uint16_t base);
void hal_pwm_delete(uint16_t base);
void hal_pwm_write(uint16_t base, uint16_t value);
// PWM HAL
//////////////////////////////////////////////////////////////

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

static e_MQTTSN_RETURNS_t pwmWriteOD(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf)
{
    // Prevent hard fault on ARM
    uint16_t value = pBuf[1];
    value <<= 8;
    value |= pBuf[0];
    if(pSubidx->Type == objPinNPN)
        value = ~value;
    
    hal_pwm_write(pSubidx->Base, value);

    return MQTTSN_RET_ACCEPTED;
}

e_MQTTSN_RETURNS_t pwmRegisterOD(indextable_t *pIdx)
{
    uint16_t base = pIdx->sidx.Base;
    uint8_t dio = hal_pwm_base2dio(base);
    
    if(dioCheckBase(dio) != 0)
        return MQTTSN_RET_REJ_INV_ID;
    
    dioTake(dio);
    hal_pwm_configure(base);
    
    if(pIdx->sidx.Type == objPinNPN)
        hal_pwm_write(base, 0xFFFF);

    pIdx->cbWrite = &pwmWriteOD;
    
    return MQTTSN_RET_ACCEPTED;
}

void pwmDeleteOD(subidx_t * pSubidx)
{
    uint16_t base = pSubidx->Base;

    hal_pwm_delete(base);
    dioRelease(hal_pwm_base2dio(base));
}

#endif  // ((defined EXTDIO_USED) && (defined EXTPWM_USED))


