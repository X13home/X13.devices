/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _EXTAIN_H
#define _EXTAIN_H

#ifdef __cplusplus
extern "C" {
#endif

void ainInit(void);
bool ainCheckSubidx(subidx_t * pSubidx);
e_MQTTSN_RETURNS_t ainRegisterOD(indextable_t *pIdx);
void ainDeleteOD(subidx_t * pSubidx);
void ainProc(void);

int16_t ainGet(uint8_t apin);
#ifdef __cplusplus
}
#endif

#endif  //  _EXTAIN_H
