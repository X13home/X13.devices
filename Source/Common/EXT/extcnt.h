/*
Copyright (c) 2011-2020 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _EXTCNT_H
#define _EXTCNT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

// Counter API
bool  cntCheckSubidx(subidx_t * pSubidx);
e_MQTTSN_RETURNS_t cntRegisterOD(indextable_t *pIdx);
void  cntDeleteOD(subidx_t * pSubidx);

// PLC Subroutines
uint16_t    cntRead(subidx_t * pSubidx);
void        cntClear(subidx_t * pSubidx);
cbPoll_t    cntGetPoll(void);

#ifdef __cplusplus
}
#endif

#endif  //  _EXTCNT_H

