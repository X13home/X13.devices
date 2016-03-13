/*
Copyright (c) 2011-2016 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#include "../config.h"

#ifdef EXTPLC_USED

// PLC Return status
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

// PLC Control, command and response
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
    PLC_CMD_SET_STACK_REQ   = 0x0B, // Set Stack Bottom, 0x08, (uint32_t)StackBot
    PLC_CMD_SET_STACK_RESP  = 0x0C  // Set Stack Bottom Response, 0 - Ok, / 1-255 error + [optional info]
}ePLC_CMD_t;

typedef struct 
{
    uint8_t     len;
    uint8_t     data[];
} sPLCexch_t;

// Local Variables

// Control
static bool             plc_run     = true;
static bool             plc_answer  = false;
static sPLCexch_t     * plc_exchg   = NULL;
static uint32_t         plc_old[EXTPLC_SIZEOF_RW];          // PLC RW Data

#include "extplc_vm.h"

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

static void plc_control(void)
{
    switch(plc_exchg->data[0])
    {
        case PLC_CMD_START_REQ:
        {
            if(plc_run)
                plc_exchg->data[1] = PLC_ANSWER_ERROR_WRS;      // Wrong State
            else
            {
                plc_exchg->data[1] = PLC_ANSWER_OK;
                plcvm_1st_start = true;
                plc_run = true;
            }
            plc_exchg->len = 2;
            plc_exchg->data[0] = PLC_CMD_START_RESP;
            break;
        }
        case PLC_CMD_STOP_REQ:
        {
            plc_run = false;
            plc_exchg->len = 2;
            plc_exchg->data[0] = PLC_CMD_STOP_RESP;
            plc_exchg->data[1] = PLC_ANSWER_OK;
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

            if(plc_run)
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
            
            plc_exchg->data[1] = PLC_ANSWER_OK;
            eeprom_write(&plc_exchg->data[3], eePLCprogram + addr, len - 5);
            break;
        }
        
        case PLC_CMD_SET_STACK_REQ:
        {
            if(plc_exchg->len != 5)
            {
                plc_exchg->len = 2;
                plc_exchg->data[0] = PLC_CMD_SET_STACK_RESP;
                plc_exchg->data[1] = PLC_ANSWER_ERROR_FMT;
                break;
            }

            plc_exchg->len = 2;
            plc_exchg->data[0] = PLC_CMD_SET_STACK_RESP;

            if(plc_run)
            {
                plc_exchg->data[1] = PLC_ANSWER_ERROR_WRS;      // Wrong State
                break;
            }

            uint32_t val;
            memcpy(&val, &plc_exchg->data[1], 4);
            if(val >= (EXTPLC_SIZEOF_RAM - 2))
            {
                plc_exchg->data[1] = PLC_ANSWER_ERROR_OFR_SP;
                break;
            }

            if(val != plcvm_stack_bot)
            {
                plcvm_stack_bot = val;
                eeprom_write(&plc_exchg->data[1], eePLCStackBot, 4);
            }
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

// ignore some GCC warnings
static e_MQTTSN_RETURNS_t plcReadOD(subidx_t * pSubidx __attribute__ ((unused)), uint8_t *pLen, uint8_t *pBuf)
{
    *pLen = plc_exchg->len;
    memcpy(pBuf, plc_exchg->data, plc_exchg->len);
    mqFree(plc_exchg);
    plc_exchg = NULL;
    plc_answer = false;

    return MQTTSN_RET_ACCEPTED;
}

static e_MQTTSN_RETURNS_t plcWriteOD(subidx_t * pSubidx __attribute__ ((unused)), uint8_t Len, uint8_t *pBuf)
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

static uint8_t plcPollOD(subidx_t * pSubidx __attribute__ ((unused)))
{
    if(plc_answer)
        return 1;

    if(plc_exchg != NULL)
    {
        plc_control();
        plc_answer = true;
        return 1;
    }

    return 0;
}

static e_MQTTSN_RETURNS_t merkerReadOD(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
    uint16_t    base = pSubidx->Base;

    switch(pSubidx->Type)
    {
        case objBool:
            {
            uint32_t mask = 1UL << (base & 0x001F);
            base >>= 5;
            if((plcvm_ram[base] & mask) != 0)
            {
                *pBuf = 1;
                plc_old[base] |= mask;
            }
            else
            {
                *pBuf = 0;
                plc_old[base] &= ~mask;
            }
            }
            *pLen = 1;
            break;

        case objInt8:
        case objUInt8:
            {
            uint8_t * pDat  = (uint8_t *)&plcvm_ram[0];
            uint8_t * pDat1 = (uint8_t *)&plc_old[0];
            uint8_t data = *(pDat + base);
            *(pDat1 + base) = data;
            *pBuf = data;
            }
            *pLen = 1;
            break;
            
        case objInt16:
        case objUInt16:
            {
            uint8_t * pDat  = (uint8_t *)&plcvm_ram[0];
            uint8_t * pDat1 = (uint8_t *)&plc_old[0];
            base <<= 1;
            pDat  += base;
            pDat1 += base;
            memcpy(pBuf,  pDat, 2);
            memcpy(pDat1, pDat, 2);
            }
            *pLen = 2;
            break;

        case objInt32:
            {
            uint32_t data = plcvm_ram[base];
            plc_old[base] = data;
            memcpy(pBuf, &data, 4);
            }
            *pLen = 4;
            break;

        // supressed warning -Wswitch;
        default:
            return MQTTSN_RET_REJ_INV_ID;
    }
    
    return MQTTSN_RET_ACCEPTED;
}

static e_MQTTSN_RETURNS_t merkerWriteOD(subidx_t * pSubidx, uint8_t len __attribute__ ((unused)), uint8_t *pBuf)
{
    uint16_t    base = pSubidx->Base;
    
    switch(pSubidx->Type)
    {
        case objBool:
            {
            uint32_t mask = 1UL << (base & 0x001F);
            base >>= 5;
            if(*pBuf == 0)
            {
                plcvm_ram[base] &= ~mask;
                plc_old[base] &= ~mask;
            }
            else
            {
                plcvm_ram[base] |= mask;
                plc_old[base] |= mask;
            }
            break;
            }
        case objInt8:
        case objUInt8:
            {
            uint8_t data = *pBuf;
            uint8_t * pDat =  (uint8_t *)&plcvm_ram[0];
            uint8_t * pDat1 = (uint8_t *)&plc_old[0];
            *(pDat + base) = data;
            *(pDat1 + base) = data;
            break;
            }
        case objInt16:
        case objUInt16:
            {
            uint8_t * pDat  = (uint8_t *)&plcvm_ram[0];
            uint8_t * pDat1 = (uint8_t *)&plc_old[0];
            base <<= 1;
            memcpy(pDat  + base, pBuf, 2);
            memcpy(pDat1 + base, pBuf, 2);
            }
            break;
        case objInt32:
            memcpy(&plcvm_ram[base], pBuf, 4);
            memcpy(&plc_old[base], pBuf, 4);
            break;
        // supressed warning -Wswitch;
        default:
            break;
    }

    return MQTTSN_RET_ACCEPTED;
}

static uint8_t merkerPollOD(subidx_t * pSubidx)
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
            return (plcvm_ram[base] != plc_old[base]);
            
        // supressed warning -Wswitch;
        default:
            return 0;
    }
    
    return (plcvm_ram[base] & mask) != (plc_old[base] & mask);
}

static void plcProc(void)
{
    if(plc_run)
    {
        plcvm_sfp = EXTPLC_SIZEOF_RAM;      // Stack frame pointer
        plcvm_sp = EXTPLC_SIZEOF_RAM;       // Stack pointer
        
        if(plcvm_1st_start)                 // program counter
        {
            plcvm_1st_start = false;
            plcvm_page = 0xFFFFFFFF;        // Reset cache
            plcvm_pc = 0;
        }
        else
            plcvm_pc = 4;

        plcvm_stat = PLC_ANSWER_RUN;

        uint16_t plc_wd = 0xFFFF;
        while(plcvm_stat == PLC_ANSWER_RUN)
        {
            if(--plc_wd == 0)
            {
                plcvm_stat = PLC_ANSWER_ERROR_WD;
                break;
            }
            
            uint32_t act_page = plcvm_pc & (const uint32_t)(~(EXTPLC_SIZEOF_PRG_CACHE - 1UL));
            uint32_t offset = plcvm_pc - act_page;
            plcvm_pc++;

            if(act_page != plcvm_page)
            {
                if(plcvm_pc >= EXTPLC_SIZEOF_PRG)
                {
                    plcvm_stat = PLC_ANSWER_ERROR_OFR_PC;
                    break;
                }

                plcvm_page = act_page;
                eeprom_read(plcvm_cache, eePLCprogram + act_page, EXTPLC_SIZEOF_PRG_CACHE);
            }
            
            uint8_t cop = plcvm_cache[offset];
            cbCOP_t cop_proc = plcvm_cb[cop];
            cop_proc();
        }
    }
    
    if(plcvm_stat != PLC_ANSWER_OK)
    {
        // PLC_VM Blue Screen :(
        plc_run = false;

        if(plc_exchg == NULL)
        {
            plc_exchg = mqAlloc(sizeof(MQ_t));
            if(plc_exchg != NULL)
            {
                uint8_t * pBuf = plc_exchg->data;
                
                *(pBuf++) = PLC_CMD_STOP_RESP;
                *(pBuf++) = plcvm_stat;

                *(pBuf++) = plcvm_sp & 0xFF;
                *(pBuf++) = (plcvm_sp>>8) & 0xFF;
                *(pBuf++) = (plcvm_sp>>16) & 0xFF;
                *(pBuf++) = (plcvm_sp>>24) & 0xFF;
    
                uint32_t acc = 0xFFFFFFFF;
                if((plcvm_sp > 0) && (plcvm_sp < EXTPLC_SIZEOF_RAM))
                    acc = plcvm_ram[plcvm_sp - 1];
    
                *(pBuf++) = acc & 0xFF;
                *(pBuf++) = (acc>>8) & 0xFF;
                *(pBuf++) = (acc>>16) & 0xFF;
                *(pBuf++) = (acc>>24) & 0xFF;
    
                *(pBuf++) = plcvm_sfp & 0xFF;
                *(pBuf++) = (plcvm_sfp>>8) & 0xFF;
                *(pBuf++) = (plcvm_sfp>>16) & 0xFF;
                *(pBuf++) = (plcvm_sfp>>24) & 0xFF;
    
                *(pBuf++) = plcvm_pc & 0xFF;
                *(pBuf++) = (plcvm_pc>>8) & 0xFF;
                *(pBuf++) = (plcvm_pc>>16) & 0xFF;
                *(pBuf)   = (plcvm_pc>>24) & 0xFF;

                plc_exchg->len = 18;

                plcvm_stat = PLC_ANSWER_OK;
                plc_answer = true;
            }
        }
    }
}

// Check Merkers Index
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

    if(base >= EXTPLC_SIZEOF_RW)
        return false;
    return true;
}

// Register merker
e_MQTTSN_RETURNS_t plcRegisterOD(indextable_t *pIdx)
{
    pIdx->cbRead    = &merkerReadOD;
    pIdx->cbPoll    = &merkerPollOD;
    pIdx->cbWrite   = &merkerWriteOD;
    return MQTTSN_RET_ACCEPTED;
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
    plc_run     = true;
    plc_answer  = false;
    plc_exchg   = NULL;
    
    // Read Config
    eeprom_read((uint8_t *)&plcvm_stack_bot, eePLCStackBot, 4);
    if(plcvm_stack_bot == 0xFFFFFFFF)
        plcvm_stack_bot = 0;
    
    extRegProc(&plcProc);
}
#endif  //  EXTPLC_USED
