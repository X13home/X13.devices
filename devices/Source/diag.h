/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// Optional Diagnostic

#ifndef _DIAG_H
#define _DIAG_H

#ifdef __cplusplus
extern "C" {
#endif

void DIAG_Init(void);
void DIAG_Poll(void);

#ifdef __cplusplus
}
#endif

#endif  //  _DIAG_H