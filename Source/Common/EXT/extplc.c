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
    
    PLC_ANSWER_ERROR_WRS    = 0xFA,     // Wrong State
    PLC_ANSWER_ERROR_CRC    = 0xFB,     // CRC Error
    PLC_ANSWER_ERROR_OFR    = 0xFC,     // Out of range
    PLC_ANSWER_ERROR_FMT    = 0xFD,     // Format Error
    PLC_ANSWER_ERROR_CMD    = 0xFE,     // Unknown command
    PLC_ANSWER_IDLE         = 0xFF
}ePLC_ANSWER_t;

typedef enum
{
    PLC_STAT_STOP           = 0,
    PLC_STAT_RUN            = 0x7F,
    
    PLC_STAT_ERR_UNK_COP    = 0x80,     // Unknown operation
    PLC_STAT_ERR_AV_PRG     = 0x81,     // Access violation programm
    PLC_STAT_ERR_AV_RAM     = 0x82,     // Access violation RAM
}ePLC_STAT_t;


typedef enum
{
    COP_NOP     = 0,
    COP_NOT     = 0x28,
    COP_INC     = 0x2E,
    COP_DEC     = 0x2F,
    COP_NEG     = 0x30,
    // 0x80 - 0x9F  Load data
    // 0xA0 - 0xBF  Store Data
    COP_RET     = 0xFF
}ePLC_COP_t;

typedef struct 
{
    uint8_t     len;
    uint8_t     data[];
} sPLCexch_t;

// Local Variables
// Control
static ePLC_STAT_t      plc_stat    = PLC_STAT_RUN;
static bool             plc_answer  = false;
static sPLCexch_t     * plc_exchg   = NULL;
// PLC Intern
static uint32_t         plc_ram[EXTPLC_SIZEOF_RAM];
static uint32_t         plc_old[EXTPLC_SIZEOF_RW];

static uint16_t plc_calc_crc(uint16_t crc_in, uint16_t len, uint8_t *pBuf)
{
    while(len != 0)
    {
        uint8_t data = *(pBuf++);
        uint8_t i;
        for (i = 0; i < 8; i++)
        {
            if(((crc_in & 0x8000) >> 8) ^ (data & 0x80))
                crc_in = (crc_in<<1) ^ 0x8005;
            else
                crc_in = (crc_in<<1);
            data <<= 1;
        }
        len--;
    }
    
    return crc_in;
}

static uint8_t plc_get_cop(uint32_t pc)
{
    static uint32_t page = 0xFFFFFFFF;
    static uint8_t  plc_prg_cache[EXTPLC_SIZEOF_PRG_CACHE];
    
    if(pc >= EXTPLC_SIZEOF_PRG)
    {
        page = 0xFFFFFFFF;
        plc_stat = PLC_STAT_ERR_AV_PRG;
        return COP_RET;
    }
    
    if((pc & ~(EXTPLC_SIZEOF_PRG_CACHE - 1UL)) != page)
    {
        page = (pc & ~(EXTPLC_SIZEOF_PRG_CACHE - 1UL));
        eeprom_read(plc_prg_cache, eePLCprogram + page, EXTPLC_SIZEOF_PRG_CACHE);
    }
    
    return plc_prg_cache[pc & (EXTPLC_SIZEOF_PRG_CACHE - 1UL)];
}

static void plc_control(void)
{
    plc_answer = true;

    switch(plc_exchg->data[0])
    {
        case PLC_CMD_START_REQ:
        {
            if(plc_stat != PLC_STAT_STOP)
                plc_exchg->data[1] = PLC_ANSWER_ERROR_WRS;      // Wrong State
            else
            {
                plc_exchg->data[1] = PLC_ANSWER_OK;
                plc_stat    = PLC_STAT_RUN;
            }
            plc_exchg->len = 2;
            plc_exchg->data[0] = PLC_CMD_START_RESP;
            break;
        }
        case PLC_CMD_STOP_REQ:
        {
            // Reset cache
            plc_get_cop(EXTPLC_SIZEOF_PRG);

            plc_exchg->len = 2;
            plc_exchg->data[0] = PLC_CMD_STOP_RESP;
            plc_exchg->data[1] = PLC_ANSWER_OK;

            plc_stat    = PLC_STAT_STOP;
            break;
        }
        case PLC_CMD_CRC_REQ:
        {
            plc_exchg->data[0] = PLC_CMD_CRC_RESP;
            if(plc_exchg->len != 5)
            {
                plc_exchg->len = 2;
                plc_exchg->data[1] = PLC_ANSWER_ERROR_FMT;
                break;
            }

            uint16_t addr, len;
            uint8_t *pBuf = &plc_exchg->data[1];

            addr  = *(pBuf++);
            addr |= (uint16_t)(*(pBuf++))<<8;
            len   = *(pBuf++);
            len  |= (uint16_t)(*pBuf)<<8;

            if((addr + len) > EXTPLC_SIZEOF_PRG)
            {
                plc_exchg->len = 2;
                plc_exchg->data[1] = PLC_ANSWER_ERROR_OFR;
                break;
            }

            pBuf = plc_exchg->data;
            uint16_t rdLen;
            uint16_t rCRC = 0xFFFF;
                
            while(len > 0)
            {
                if(len <= (sizeof(MQ_t) - 1))
                    rdLen = len;
                else
                    rdLen = (sizeof(MQ_t) - 1);

                eeprom_read(pBuf, eePLCprogram + addr, rdLen);
                rCRC = plc_calc_crc(rCRC, rdLen, pBuf);

                addr += rdLen;
                len -= rdLen;
                pBuf += rdLen;
            }

            plc_exchg->len = 4;
            plc_exchg->data[0] = PLC_CMD_CRC_RESP;
            plc_exchg->data[1] = PLC_ANSWER_OK;
            plc_exchg->data[2] = rCRC & 0xFF;
            plc_exchg->data[3] = rCRC >> 8;
            break;
        }
        case PLC_CMD_WRITE_REQ:
        {
            // Write Block Request: 7, addrL, addrH, data[], crcL, crcH
            uint16_t len = plc_exchg->len;

            plc_exchg->data[0] = PLC_CMD_WRITE_RESP;
            plc_exchg->len = 2;

            if(plc_stat != PLC_STAT_STOP)
            {
                plc_exchg->data[1] = PLC_ANSWER_ERROR_WRS;      // Wrong State
                break;
            }

            if(len < 6)
            {
                plc_exchg->data[1] = PLC_ANSWER_ERROR_FMT;      // Format error
                break;
            }

            uint16_t addr = ((uint16_t)(plc_exchg->data[1]) | ((uint16_t)(plc_exchg->data[2])<<8));
            if((addr + len - 5) > EXTPLC_SIZEOF_PRG)
            {
                plc_exchg->data[1] = PLC_ANSWER_ERROR_OFR;      // Out of range
                break;
            }

            uint16_t rCRC = ((uint16_t)(plc_exchg->data[len - 2]) | ((uint16_t)(plc_exchg->data[len - 1])<<8));
            if(plc_calc_crc(0xFFFF, len - 5, &plc_exchg->data[3]) != rCRC)
            {
                plc_exchg->data[1] = PLC_ANSWER_ERROR_CRC;
                break;
            }
                
            eeprom_write(&plc_exchg->data[3], eePLCprogram + addr, len - 5);
            break;
        }
        default:
        {
            plc_exchg->len = 2;
            plc_exchg->data[1] = PLC_ANSWER_ERROR_CMD;
            break;
        }
    }
}

static int32_t plc_peek(uint32_t sp)
{
    if(sp < EXTPLC_SIZEOF_RAM)
        return plc_ram[sp];

    plc_stat = PLC_STAT_ERR_AV_RAM;
    return 0;
}

static void plc_poke(uint32_t sp, uint32_t val)
{
    if(sp < EXTPLC_SIZEOF_RAM)
        plc_ram[sp] = val;
    else
        plc_stat = PLC_STAT_ERR_AV_RAM;
}

static void plc_vm(void)
{
    uint32_t sp = (EXTPLC_SIZEOF_RAM - 1);          // Stack pointer
    uint32_t spf = EXTPLC_SIZEOF_RAM;               // Stack frame pointer
    uint32_t pc = 0;                                // program counter

    while(plc_stat == PLC_STAT_RUN)
    {
        uint8_t cop = plc_get_cop(pc++);

        // Load data to stack
        if(cop >= 0x80 && cop <= 0x9F)
        {
            uint8_t type = cop & 7;
            uint8_t place = cop & 0x18;
            uint32_t acc;
            
            if((type == 0) || (type == 7))          // Load constants
            {
                if(place == 0)                      // bool
                {
                    acc = (type != 0);
                }
                else if(place == 0x08)              // int8/uint8
                {
                    acc = plc_get_cop(pc++);
                    if((type == 0) && (acc & 0x80))
                        acc |= 0xFFFFFF00;
                }
                else if(place == 0x10)              // int16/uint16
                {
                    acc = plc_get_cop(pc++);
                    acc |= (int32_t)(plc_get_cop(pc++))<<8;
                    if((type == 0) && (acc & 0x8000))
                        acc |= 0xFFFF0000;
                }
                else if(type == 0)                  // int32
                {
                    acc = plc_get_cop(pc++);
                    acc |= (int32_t)(plc_get_cop(pc++))<<8;
                    acc |= (int32_t)(plc_get_cop(pc++))<<16;
                    acc |= (int32_t)(plc_get_cop(pc++))<<24;
                }
                else                                // 0xFFFFFFFF
                {
                    acc = 0xFFFFFFFF;
                }

                plc_poke(--sp, acc);
            }
            else
            {
                uint32_t addr;
                if(place == 0)
                    addr = plc_peek(sp);
                else if(place == 0x08)
                    addr = plc_peek(sp) + plc_get_cop(pc++);
                else if(place == 0x10)
                {
                    int32_t adr1 = plc_get_cop(pc++);
                    adr1 |= (int32_t)(plc_get_cop(pc++))<<8;
                    addr = plc_peek(sp) + adr1;
                }
                else
                {
                    addr = plc_get_cop(pc++);
                    addr |= (int32_t)(plc_get_cop(pc++))<<8;
                    sp--;
                }

                if(type == 1)    // Bool
                {
                    acc = plc_peek(addr >> 5);
                    acc = ((acc & (1UL<<(addr & 0x1F))) != 0);
                }
                else if((type == 2) || (type == 3))     // [u]int8
                {
                    acc = plc_peek(addr >> 2);
                    acc = acc >> ((addr & 0x03) * 8);
                    
                    if((type == 2) && (acc & 0x80))
                        acc |= 0xFFFFFF00;
                    else
                        acc &= 0x000000FF;
                }
                else if((type == 4) || (type == 5))
                {
                    acc = plc_peek(addr >> 1);
                    acc = acc >> ((addr & 0x01) * 8);
                    
                    if((type == 4) && (acc & 0x8000))
                        acc |= 0xFFFF0000;
                    else
                        acc &= 0x0000FFFF;
                }
                else
                    acc = plc_peek(addr);

                plc_poke(sp, acc);
            }
        }
        else if(cop >= 0xA0 && cop <= 0xBF)
        {
            uint8_t type = cop & 7;
            uint8_t place = cop & 0x18;
            
            uint32_t addr;
            if(place == 0)
                addr = plc_peek(sp++);
            else if(place == 0x08)
                addr = plc_peek(sp++) + plc_get_cop(pc++);
            else if(place == 0x10)
            {
                int32_t adr1 = plc_get_cop(pc++);
                adr1 |= (int32_t)(plc_get_cop(pc++))<<8;
                addr = plc_peek(sp++) + adr1;
            }
            else
            {
                addr = plc_get_cop(pc++);
                addr |= (int32_t)(plc_get_cop(pc++))<<8;
            }

            uint32_t acc  = plc_peek(sp++);
            uint32_t bcc;
            uint32_t mask;
            

            if(type == 1)                               // bool
            {
                mask = 1UL<<(addr & 0x1F);
                addr >>= 5;
                bcc = plc_peek(addr);
                if(acc != 0)
                    bcc |= mask;
                else
                    bcc &= ~mask;
                plc_poke(addr, bcc);
            }
            else if((type == 2) || (type == 3))
            {
                acc &= 0x000000FF;
                acc <<= (addr & 3)*8;
                mask = 0x000000FF << ((addr & 3)*8);
                
                addr >>= 2;
                bcc = plc_peek(addr);
                bcc &= ~mask;
                bcc |= acc;
                plc_poke(addr, bcc);
            }
            else if((type == 4) || (type == 5))
            {
                acc &= 0x0000FFFF;
                acc <<= (addr & 1)*16;
                mask = 0x0000FFFF << ((addr & 1)*16);
                
                addr >>= 1;
                bcc = plc_peek(addr);
                bcc &= ~mask;
                bcc |= acc;
                plc_poke(addr, bcc);
            }
            else if(type == 6)
            {
                plc_poke(addr, acc);
            }
            else    // Type 0,7
            {
                plc_stat = PLC_STAT_ERR_UNK_COP;        // Error unknown COP
                return;
            }
        }
        else
        switch(cop)
        {
            case COP_NOP:
                break;

            // 0x22 - 0x3F, arithmetic and logic operations
            case COP_NOT:
                plc_poke(sp, ~plc_peek(sp));
                break;
            case COP_INC:
                plc_poke(sp, plc_peek(sp) + 1);
                break;
            case COP_DEC:
                plc_poke(sp, plc_peek(sp) - 1);
                break;
            case COP_NEG:
                plc_poke(sp, -plc_peek(sp));
                break;

            case COP_RET:
                if(spf == EXTPLC_SIZEOF_RAM)
                    return;
                sp  = spf - 1;
                spf = plc_peek(sp++);
                pc  = plc_peek(sp++);
                break;

            default:
                plc_stat = PLC_STAT_ERR_UNK_COP;        // Error unknown COP
                return;
        }
    }
}

void plcProc(void)
{
    if((plc_answer == false) && (plc_exchg != NULL))
        plc_control();

    if(plc_stat == PLC_STAT_RUN)
        plc_vm();
    else if(plc_stat != PLC_STAT_STOP)
    {
        if(plc_exchg == NULL)
            plc_exchg = mqAlloc(sizeof(MQ_t));
        if(plc_exchg != NULL)
        {
            plc_exchg->len = 2;
            plc_exchg->data[0] = PLC_CMD_STOP_RESP;
            plc_exchg->data[1] = plc_stat;
        }
        plc_stat = PLC_STAT_STOP;
        plc_answer = true;
    }
}

// ignore some GCC warnings
#if defined ( __GNUC__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

static e_MQTTSN_RETURNS_t plcReadOD(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
    *pLen = plc_exchg->len;
    memcpy(pBuf, plc_exchg->data, plc_exchg->len);
    mqFree(plc_exchg);
    plc_exchg = NULL;
    plc_answer = false;

    return MQTTSN_RET_ACCEPTED;
}

static e_MQTTSN_RETURNS_t plcWriteOD(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf)
{
    if(plc_exchg != NULL)
        return MQTTSN_RET_REJ_CONG;

    plc_exchg = mqAlloc(sizeof(MQ_t));
    if(plc_exchg == NULL)
        return MQTTSN_RET_REJ_CONG;

    plc_exchg->len = Len;
    memcpy(plc_exchg->data, pBuf, Len);

    return MQTTSN_RET_ACCEPTED;
}

static uint8_t plcPollOD(subidx_t * pSubidx, uint8_t unused)
{
    if(plc_answer)
        return 1;
    return 0;
}

#if defined ( __GNUC__ )
#pragma GCC diagnostic pop
#endif

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
    plc_answer      = false;
    plc_exchg       = NULL;
}

// Merkers
// Check Index
bool plcCheckSubidx(subidx_t * pSubidx)
{
    uint16_t base = pSubidx->Base;
    switch(pSubidx->Type)
    {
        case objBool:
            base >>= 5;
            break;
        case objInt8:
        case objUInt8:
            base >>= 2;
            break;
        case objInt16:
        case objUInt16:
            base >>= 1;
            break;
        case objInt32:
            break;
        default:
            return false;
    }

    if(base >= EXTPLC_SIZEOF_WO)
        return false;
    return true;
}

static e_MQTTSN_RETURNS_t merkerReadOD(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
    uint16_t    base = pSubidx->Base;
    uint32_t    mask;
    uint32_t    data;
    uint8_t     offset;

    switch(pSubidx->Type)
    {
        case objBool:
            *pLen = 1;
            mask = 1UL << (base & 0x001F);
            base >>= 5;
            if((plc_ram[base] & mask) != 0)
            {
                *pBuf = 1;
                if(base < EXTPLC_SIZEOF_RW)
                    plc_old[base] |= mask;
            }
            else
            {
                *pBuf = 0;
                if(base < EXTPLC_SIZEOF_RW)
                    plc_old[base] &= ~mask;
            }
            break;

        case objInt8:
        case objUInt8:
            offset = ((base & 0x003) * 8);
            mask = 0x000000FFUL << offset;
            base >>= 2;
            data = (plc_ram[base] & mask);
            if(base < EXTPLC_SIZEOF_RW)
            {
                plc_old[base] &= ~mask;
                plc_old[base] |= data;
            }
            *pBuf = data >> offset;
            *pLen = 1;
            break;
            
        case objInt16:
        case objUInt16:
            offset = ((base & 0x001) * 16);
            mask = 0x0000FFFFUL << offset;
            base >>= 1;
            data = (plc_ram[base] & mask);
            if(base < EXTPLC_SIZEOF_RW)
            {
                plc_old[base] &= ~mask;
                plc_old[base] |= data;
            }
            data >>= offset;
            *(pBuf++) = data & 0xFF;
            data >>= 8;
            *pBuf = data;
            *pLen = 2;
            break;

        case objInt32:
            data = plc_ram[base];
            if(base < EXTPLC_SIZEOF_RW)
                plc_old[base] = data;

            *(pBuf++) = data & 0xFF;
            data >>= 8;
            *(pBuf++) = data & 0xFF;
            data >>= 8;
            *(pBuf++) = data & 0xFF;
            data >>= 8;
            *pBuf = data & 0xFF;
            *pLen = 4;
            break;

        // supressed warning -Wswitch;
        default:
            return MQTTSN_RET_REJ_INV_ID;
    }
    
    return MQTTSN_RET_ACCEPTED;
}

// ignore some GCC warnings
#if defined ( __GNUC__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

static e_MQTTSN_RETURNS_t merkerWriteOD(subidx_t * pSubidx, uint8_t unused, uint8_t *pBuf)
{
    uint16_t    base = pSubidx->Base;
    uint32_t    mask;
    uint32_t    data;
    
    switch(pSubidx->Type)
    {
        case objBool:
            mask = 1UL << (base & 0x001F);
            base >>= 5;
            if(*pBuf == 0)
            {
                plc_ram[base] &= ~mask;
                if(base < EXTPLC_SIZEOF_RW)
                    plc_old[base] &= ~mask;
            }
            else
            {
                plc_ram[base] |= mask;
                if(base < EXTPLC_SIZEOF_RW)
                    plc_old[base] |= mask;
            }
            break;
        case objInt8:
        case objUInt8:
            mask = 0x000000FFUL << ((base & 0x0003) * 8);
            data = *pBuf;
            data <<= (base & 0x0003) * 8;
            base >>= 2;
            plc_ram[base] &= ~mask;
            plc_ram[base] |= data;
            if(base < EXTPLC_SIZEOF_RW)
            {
                plc_old[base] &= ~mask;
                plc_old[base] |= data;
            }
            break;
        case objInt16:
        case objUInt16:
            mask = 0x0000FFFFUL << ((base & 0x0001) * 16);
            data = *(pBuf++);
            data |= (uint16_t)(*pBuf)<<8;
            data <<= (base & 0x0001) * 16;
            base >>= 1;
            plc_ram[base] &= ~mask;
            plc_ram[base] |= data;
            if(base < EXTPLC_SIZEOF_RW)
            {
                plc_old[base] &= ~mask;
                plc_old[base] |= data;
            }
            break;
        case objInt32:
            data = *(pBuf++);
            data |= (uint32_t)(*(pBuf++))<<8;
            data |= (uint32_t)(*(pBuf++))<<16;
            data |= (uint32_t)(*pBuf)<<24;
            plc_ram[base] = data;
            if(base < EXTPLC_SIZEOF_RW)
                plc_old[base] = data;
            break;
        // supressed warning -Wswitch;
        default:
            break;
    }

    return MQTTSN_RET_ACCEPTED;
}

static uint8_t merkerPollOD(subidx_t * pSubidx, uint8_t unused)
{
    uint16_t    base = pSubidx->Base;
    uint32_t    mask;
    
    switch(pSubidx->Type)
    {
        case objBool:
            mask = 1UL << (base & 0x001F);
            base >>= 5;
            break;
        case objInt8:
        case objUInt8:
            mask = 0x000000FFUL << ((base & 0x0003) * 8);
            base >>= 2;
            break;
        case objInt16:
        case objUInt16:
            mask = 0x0000FFFFUL << ((base & 0x0001) * 16);
            base >>= 1;
            break;
        case objInt32:
            return (plc_ram[base] != plc_old[base]);
            
        // supressed warning -Wswitch;
        default:
            return 0;
    }
    
    return (plc_ram[base] & mask) != (plc_old[base] & mask);
}

#if defined ( __GNUC__ )
#pragma GCC diagnostic pop
#endif

// Register merker
e_MQTTSN_RETURNS_t plcRegisterOD(indextable_t *pIdx)
{
    uint16_t base = pIdx->sidx.Base;
    switch(pIdx->sidx.Type)
    {
        case objBool:
            base >>= 5;
            break;
        case objInt8:
        case objUInt8:
            base >>= 2;
            break;
        case objInt16:
        case objUInt16:
            base >>= 1;
            break;
        case objInt32:
            break;

        // supressed warning -Wswitch;
        default:
            return MQTTSN_RET_REJ_NOT_SUPP;
    }

    if(base < EXTPLC_SIZEOF_RW)
    {
        pIdx->cbRead    = &merkerReadOD;
        pIdx->cbPoll    = &merkerPollOD;
    }

    pIdx->cbWrite   = &merkerWriteOD;
    return MQTTSN_RET_ACCEPTED;
}

#endif  //  EXTPLC_USED
