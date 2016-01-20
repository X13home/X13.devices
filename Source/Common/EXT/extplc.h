/*
Copyright (c) 2011-2016 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _EXTPLC_H
#define _EXTPLC_H

#ifdef __cplusplus
extern "C" {
#endif


typedef enum
{
    PLC_ANSWER_OK               = 0,
    
    PLC_ANSWER_ERROR_UNK_COP    = 0x80,     // Unknown operation
    PLC_ANSWER_ERROR_AV_PRG     = 0x81,     // Access violation programm
    PLC_ANSWER_ERROR_AV_RAM     = 0x82,     // Access violation RAM
    
    PLC_ANSWER_ERROR_WRS        = 0xFA,     // Wrong State
    PLC_ANSWER_ERROR_CRC        = 0xFB,     // CRC Error
    PLC_ANSWER_ERROR_OFR        = 0xFC,     // Out of range
    PLC_ANSWER_ERROR_FMT        = 0xFD,     // Format Error
    PLC_ANSWER_ERROR_CMD        = 0xFE,     // Unknown command
    PLC_ANSWER_IDLE             = 0xFF
}ePLC_ANSWER_t;

void plcInit(void);
bool plcCheckSubidx(subidx_t * pSubidx);
e_MQTTSN_RETURNS_t plcRegisterOD(indextable_t *pIdx);

void plcProc(void);

// Procedure defined in extplc_vm.c
ePLC_ANSWER_t plcvm(void);
void plcvm_reset_cache(void);

#ifdef __cplusplus
}
#endif

#endif  //  _EXTPLC_H