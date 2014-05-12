/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _EXTPWM_H
#define _EXTPWM_H

void pwmClean(void);
uint8_t pwmCheckIdx(subidx_t * pSubidx);
uint8_t pwmRegisterOD(indextable_t *pIdx);
void pwmDeleteOD(subidx_t * pSubidx);

#endif

