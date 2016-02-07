/*
Copyright (c) 2011-2016 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _EXTPLC_VM_H
#define _EXTPLC_VM_H

#ifdef __cplusplus
extern "C" {
#endif

uint8_t plcvm_get_state(uint8_t *pBuf);
ePLC_ANSWER_t plcvm(void);
void plcvm_reset_cache(void);
void plcvm_load_config(void);

#ifdef __cplusplus
}
#endif

#endif  //  _EXTPLC_VM_H

