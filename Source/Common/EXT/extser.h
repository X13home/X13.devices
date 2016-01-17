/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _EXTSER_H
#define _EXTSER_H

#ifdef __cplusplus
extern "C" {
#endif

// UART API
void    serInit(void);
bool    serCheckSubidx(subidx_t * pSubidx);
e_MQTTSN_RETURNS_t serRegisterOD(indextable_t *pIdx);
void    serDeleteOD(subidx_t * pSubidx);
void    serProc(void);

#ifdef __cplusplus
}
#endif

#endif  //  _EXTSER_H
