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
    PLC_ANSWER_RUN              = 1,        // PLC VM in Run State 
    
    PLC_ANSWER_ERROR_UNK_COP    = 0x80,     // Unknown operation
    PLC_ANSWER_ERROR_OFR_PC     = 0x81,     // Out of Range PC
    PLC_ANSWER_ERROR_OFR_RAM    = 0x82,     // Out of Range RAM pointer
    PLC_ANSWER_ERROR_TEST       = 0x83,     // Test error
    PLC_ANSWER_ERROR_WD         = 0x84,     // Watchdog
    PLC_ANSWER_ERROR_OFR_SFP    = 0x85,     // Out of Range SFP
    PLC_ANSWER_ERROR_DIV0       = 0x86,     // division to 0
    PLC_ANSWER_ERROR_OFR_SP     = 0x87,     // Out of Range SP
    PLC_ANSWER_ERROR_UNK_API    = 0x88,     // Unknown API function
    
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

#ifdef __cplusplus
}
#endif

#endif  //  _EXTPLC_H
