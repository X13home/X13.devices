/*
Copyright (c) 2011-2016 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _EXTPWM_H
#define _EXTPWM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

// HAL Section
bool  hal_pwm_checkbase(uint16_t base);
bool  hal_pwm_busy(uint16_t base);
void  hal_pwm_configure(uint16_t base, bool inv);
void  hal_pwm_delete(uint16_t base);
void  hal_pwm_write(uint16_t base, uint16_t value);

// PWM API
bool  pwmCheckSubidx(subidx_t * pSubidx);
e_MQTTSN_RETURNS_t pwmRegisterOD(indextable_t *pIdx);
void  pwmDeleteOD(subidx_t * pSubidx);


#ifdef __cplusplus
}
#endif

#endif  //  _EXTPWM_H





