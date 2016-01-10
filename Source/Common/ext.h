/*
Copyright (c) 2011-2015 <comparator@gmx.de>

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

#ifdef EXTDIO_USED
#include "EXT/extdio.h"

#ifdef EXTPWM_USED
#include "EXT/extpwm.h"
#endif  //  EXTPWM_USED

#ifdef EXTAIN_USED
#include "EXT/extain.h"
#endif  //  EXTAIN_USED

#endif  //  EXTDIO_USED

#ifdef EXTTWI_USED
#include "EXT/exttwi.h"
#endif  //  EXTTWI_USED

#ifdef EXTSER_USED
#include "EXT/extser.h"
#endif  //  EXTSER_USED

#ifdef EXTPLC_USED
#include "EXT/extplc.h"
#endif  //  EXTPLC_USED

void extInit(void);                                         // Initialise extensions
bool extCheckSubidx(subidx_t * pSubidx);                    // Check Subindex , true -> Ok
e_MQTTSN_RETURNS_t extRegisterOD(indextable_t * pIdx);      // Register Object
void extDeleteOD(subidx_t * pSubidx);                       // Delete Object
void extProc(void);                                         // Update IO's

#ifdef __cplusplus
}
#endif

#endif  //  _EXT_H_
