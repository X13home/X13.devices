/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _EXTDIO_H
#define _EXTDIO_H

#ifdef __cplusplus
extern "C" {
#endif

void dioInit(void);
bool dioCheckSubidx(subidx_t * pSubidx);
e_MQTTSN_RETURNS_t dioRegisterOD(indextable_t *pIdx);
void dioDeleteOD(subidx_t * pSubidx);
void dioProc(void);

uint8_t dioCheckBase(uint8_t base);
void dioTake(uint8_t base);
void dioRelease(uint8_t base);

bool dioGet(uint16_t base);
void dioSet(uint16_t base);
void dioReset(uint16_t base);

#ifdef __cplusplus
}
#endif

#endif  //  _EXTDIO_H
