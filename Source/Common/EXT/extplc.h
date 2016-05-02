/*
Copyright (c) 2011-2016 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _EXTPLC_H
#define _EXTPLC_H

#ifdef __cplusplus
extern "C" {
#endif

void plcInit(void);
bool plcCheckSubidx(subidx_t * pSubidx);
e_MQTTSN_RETURNS_t plcRegisterOD(indextable_t *pIdx);

#ifdef __cplusplus
}
#endif

#endif  //  _EXTPLC_H
