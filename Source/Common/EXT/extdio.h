/*
Copyright (c) 2011-2016 <comparator@gmx.de>

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

// DIO API
void    dioInit(void);
bool    dioCheckSubidx(subidx_t * pSubidx);
e_MQTTSN_RETURNS_t dioRegisterOD(indextable_t *pIdx);
void    dioDeleteOD(subidx_t * pSubidx);

// Shared DIO subroutines
uint8_t dioCheckBase(uint8_t base);
void    dioTake(uint8_t base);
void    dioRelease(uint8_t base);

// PLC Subroutines
bool    dioRead(subidx_t * pSubidx);
void    dioWrite(subidx_t * pSubidx, bool sr);
cbPoll_t dioGetPoll(void);

#ifdef __cplusplus
}
#endif

#endif  //  _EXTDIO_H
