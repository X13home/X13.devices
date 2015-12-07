/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#include "../config.h"

#ifdef EXTPLC_USED

#include "extplc.h"

#if ((PLC_SIZEOF_MEMORY & 0x03) != 0)
#error  PLC_SIZEOF_MEMORY MUST be alligned to 4
#endif

#if ((PLC_SIZEOF_EEPROM & 31) != 0)
#error  PLC_SIZEOF_EEPROM MUST be alligned to 32
#endif

typedef struct
{
    uint32_t    sp;                         // Stack pointer
    uint32_t    spf;                        // Stack frame pointer
    uint16_t    pc;                         // program counter
    uint8_t     prg[32];                    // Actual program page
    uint8_t     mem[PLC_SIZEOF_MEMORY];     // Bottom variables, from top - stack
    uint8_t     old_mem[PLC_SIZEOF_RW];     // old values for public variables
}sPLC_t;

typedef enum
{
    PLC_GET_SIG     = 0,    // Get CRC + Counter, req: not used, resp: CRC16 + counter 2bytes
    PLC_GET_CRC     = 1,    // get Block CRC , req: block_u16, resp: CRC16
    PLC_GET_BLK     = 2,    // get block, req: block_u16, resp: block_array
    PLC_DEL_BLK     = 3,    // delete block, req: block_16 +  CRC16, resp: 0 - ok, 1-255 errors
    PLC_WRT_BLK     = 4,    // write block, req: block_u16 + data 1-32 bytes + CRC16, resp: 0 - ok, 1-255 errors
}ePLCcmd_t;

/*
static sPLC_t vPLC;

// Load program from EEPROM to program page
static void plcReadPrg(void)
{
    uint16_t base = eePLCprogram;
    base += vPLC.pc & 0xFFE0;
    eeprom_read(&vPLC.prg, base, 32);
}

static uint8_t plcType2Lenght(uint8_t type)
{
    switch(type)
    {
        case objBool:
        case objInt8:
        case objUInt8:
            return 1;
        case objInt16:
        case objUInt16:
            return 2;
        case objInt32:
        case objUInt32:
            return 4;
        case objInt64:
            return 8;
        case objArray:
            return 0;
        default:
            break;
    }
    return 0xFF;
}

// Write program to program memory
static e_MQTTSN_RETURNS_t plcWriteOD(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf)
{
    uint16_t base = pSubidx->Base;
    base *= 32;

    if((Len > 31) || ((base + Len) >= PLC_SIZEOF_EEPROM))
        return MQTTSN_RET_REJ_NOT_SUPP;

    base += eePLCprogram;
    eeprom_write(pBuf, base, Len);
    return MQTTSN_RET_ACCEPTED;
}

static e_MQTTSN_RETURNS_t merkerWriteOD(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf)
{
    uint16_t base = pSubidx->Base;
    uint8_t len = plcType2Lenght(pSubidx->Type);
    if(len == 0)
    {
        len = Len;
        vPLC.mem[base] = len;
        vPLC.old_mem[base] = len;
        base++;
    }

    memcpy(&vPLC.mem[base], pBuf, len);
    memcpy(&vPLC.old_mem[base], &vPLC.mem[base], len);

    return MQTTSN_RET_ACCEPTED;
}

e_MQTTSN_RETURNS_t merkerReadOD(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
    uint16_t base = pSubidx->Base;
    uint8_t len = plcType2Lenght(pSubidx->Type);
    if(len == 0)
        len = vPLC.mem[base];

    memcpy(pBuf, &vPLC.mem[base], len);
    memcpy(&vPLC.old_mem[base], &vPLC.mem[base], len);
    *pLen = len;

    return MQTTSN_RET_ACCEPTED;
}

uint8_t merkerPollOD(subidx_t * pSubidx, uint8_t sleep)
{
    uint16_t base = pSubidx->Base;
    uint8_t len = plcType2Lenght(pSubidx->Type);

    if(len == 0)
        len = vPLC.old_mem[base];

    return memcmp(&vPLC.old_mem[base], &vPLC.mem[base], len);
}

void plcInit(void)
{
    PLCexchgOD.Index = 0;
    PLCexchgOD.cbRead = NULL;
    PLCexchgOD.cbWrite = &plcWriteOD;
    PLCexchgOD.cbPoll = NULL;

    vPLC.sp = (PLC_SIZEOF_MEMORY - 4);
    vPLC.spf = PLC_SIZEOF_MEMORY;
    vPLC.pc = 0;

    plcReadPrg();
}

// Check Index
bool plcCheckSubidx(subidx_t * pSubidx)
{
    uint8_t len = plcType2Lenght(pSubidx->Type);
    uint16_t base = pSubidx->Base;

    if((len == 0xFF) || ((base + len) >= PLC_SIZEOF_MEMORY))
        return 2;

    return 0;
}

// Register merker
e_MQTTSN_RETURNS_t plcRegisterOD(indextable_t *pIdx)
{
    uint16_t base = pIdx->sidx.Base;

    if(base < PLC_SIZEOF_RW)
    {
        pIdx->cbRead  = &merkerReadOD;
        pIdx->cbPoll  = &merkerPollOD;
    }
    else
    {
        pIdx->cbRead  = NULL;
        pIdx->cbPoll  = NULL;
    }
    pIdx->cbWrite = &merkerWriteOD;

    return MQTTSN_RET_ACCEPTED;
}

void plcProc(void)
{
    vPLC.mem[0]++;
    if(vPLC.mem[0] == 0)
    {
        vPLC.mem[1]++;
        if(vPLC.mem[1] == 0)
        {
            vPLC.mem[2]++;
        }
    }
}
*/

#endif  //  EXTPLC_USED
