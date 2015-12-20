/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _EXT_H_
#define _EXT_H_

#ifdef __cplusplus
extern "C" {
#endif

void extInit(void);                                         // Initialise extensions
bool extCheckSubidx(subidx_t * pSubidx);                    // Check Subindex , true -> Ok
e_MQTTSN_RETURNS_t extRegisterOD(indextable_t * pIdx);      // Register Object
void extDeleteOD(subidx_t * pSubidx);                       // Delete Object
void extProc(void);                                         // Update IO's

#ifdef __cplusplus
}
#endif

#endif  //  _EXT_H_
