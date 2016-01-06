/*
Copyright (c) 2011-2015 <comparator@gmx.de>

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

// HAL Section
uint8_t hal_ain_apin2dio(uint8_t apin);
void    hal_ain_configure(uint8_t apin, uint8_t aref);
void    hal_ain_select(uint8_t apin, uint8_t aref);
int16_t hal_ain_get(void);

// AIn API
void    ainInit(void);
bool    ainCheckSubidx(subidx_t * pSubidx);
e_MQTTSN_RETURNS_t ainRegisterOD(indextable_t *pIdx);
void    ainDeleteOD(subidx_t * pSubidx);
void    ainProc(void);

#ifdef __cplusplus
}
#endif

#endif  //  _EXTAIN_H
