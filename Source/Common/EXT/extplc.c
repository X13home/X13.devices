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

// Response format: [cmd] [stat] [optional]
typedef enum
{
    PLC_CMD_IDLE            = 0x00, // Idle
    PLC_CMD_START_REQ       = 0x01, // PLC Start request
    PLC_CMD_START_RESP      = 0x02, // PLC Start response: 2, 0 - Ok, 1-255 error + [optional info]
    PLC_CMD_STOP_REQ        = 0x03, // PLC Stop request
    PLC_CMD_STOP_RESP       = 0x04, // PLC Stop response: 4, 0 - Ok / 1-255 error + [optional info]
    PLC_CMD_CRC_REQ         = 0x05, // CRC Request: 5, addrL, addrH, lenL, lenH
    PLC_CMD_CRC_RESP        = 0x06, // CRC Response: 6, 0, crcL, crcH; On error: 6, 1-255, [optional info]
    PLC_CMD_WRITE_REQ       = 0x07, // Write Block Request: 7, addrL, addrH, data[], crcL, crcH
    PLC_CMD_WRITE_RESP      = 0x08, // Write Block Response: 8, 0 - Ok / 1-255 error + [optional info]
    PLC_CMD_ERASE_REQ       = 0x09, // Erase Block Request: 9, addrL, addrH, lenL, lenH
    PLC_CMD_ERASE_RESP      = 0x0A, // Erase Block Response: 0x0a, 0  - Ok / 1-255 error + [optional info]
}ePLC_CMD_t;

typedef enum
{
    PLC_ANSWER_OK           = 0,
    
    PLC_ANSWER_ERROR_CMD    = 0xFE,
    PLC_ANSWER_IDLE         = 0xFF
}ePLC_ANSWER_t;

typedef enum
{
    PLC_STAT_STOP           = 0,

    PLC_STAT_NO_PRG         = 0x7E,
    PLC_STAT_RUN            = 0x7F,
}ePLC_STAT_t

static ePLC_STAT_t      plc_stat        = PLC_STAT_RUN;
static ePLC_CMD_t       plc_command     = PLC_CMD_IDLE;
static ePLC_ANSWER_t    plc_answer      = PLC_ANSWER_IDLE;
static uint16_t         plc_answer_ext  = 0x0000;

static void plc_parse_cmd(uint8_t Len, uint8_t *pBuf)
{
    ePLC_CMD_t cmd = pBuf[0];
    switch(cmd)
    {
        case PLC_CMD_START_REQ:
            // ToDo procedure Start PLC
            if(plc_stat != PLC_STAT_STOP)
            {
                plc_answer = plc_stat;
            }
            else
            {
                plc_answer = PLC_ANSWER_OK;
                plc_stat = PLC_STAT_STOP;
            }
            plc_command = PLC_CMD_START_RESP;
            break;

        case PLC_CMD_STOP_REQ:
            plc_stat = PLC_STAT_STOP;
            plc_command = PLC_CMD_STOP_RESP;
            plc_answer = PLC_ANSWER_OK;
            break;

        default:
            plc_command = cmd;
            plc_answer  = PLC_ANSWER_ERROR_CMD;
            break;
    }
}

static e_MQTTSN_RETURNS_t plcReadOD(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
    if(pSubidx->Place == objPLCctrl)
    {
        *(pBuf++) = plc_command;
        *(pBuf++) = plc_answer;
        
        if((plc_command == PLC_CMD_CRC_RESP) || (plc_answer_ext != 0))
        {
            *(pBuf++) = plc_answer_ext & 0x00FF;
            *(pBuf++) = plc_answer_ext >> 8;
            *pLen = 4;
        }
        else
            *pLen = 2;
        
        plc_command     = PLC_CMD_IDLE;
        plc_answer      = PLC_ANSWER_IDLE;
        plc_answer_ext  = 0x0000;
    }
    // ToDo else merker

    return MQTTSN_RET_ACCEPTED;
}

static e_MQTTSN_RETURNS_t plcWriteOD(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf)
{
    if(pSubidx->Place == objPLCctrl)
        plc_parse_cmd(Len, pBuf);
    // ToDo else merker

    return MQTTSN_RET_ACCEPTED;
}

static uint8_t plcPollOD(subidx_t * pSubidx, uint8_t sleep)
{
    if(pSubidx->Place == objPLCctrl)
    {
       if(plc_answer != PLC_ANSWER_IDLE)
            return 1;
    }
    // ToDo else merker

    return 0;
}

void plcInit(void)
{
   // Register variable pa0
    indextable_t * pIndex = getFreeIdxOD();
    if(pIndex == NULL)
        return;
    
    pIndex->cbRead     = &plcReadOD;
    pIndex->cbWrite    = &plcWriteOD;
    pIndex->cbPoll     = &plcPollOD;
    pIndex->sidx.Place = objPLCctrl;    // PLC Control
    pIndex->sidx.Type  = objArray;      // Variable Type - Byte Array
    pIndex->sidx.Base  = 0;             // not used
    
    // Init variables
    plc_stat        = PLC_STAT_RUN;
    plc_command     = PLC_CMD_IDLE;
    plc_answer      = PLC_ANSWER_IDLE;
    plc_answer_ext  = 0x0000;

/*
    vPLC.sp = (PLC_SIZEOF_MEMORY - 4);
    vPLC.spf = PLC_SIZEOF_MEMORY;
    vPLC.pc = 0;

    plcReadPrg();
*/
}

void plcProc(void)
{

}

/*
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
*/



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


// Merkers
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
