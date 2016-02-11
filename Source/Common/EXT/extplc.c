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

// PLC_VM Section
static uint32_t         plc_ram[EXTPLC_SIZEOF_RAM];         // PLC data + merkers + stack
static ePLC_ANSWER_t    plcvm_stat = PLC_ANSWER_OK;
static uint32_t         plcvm_page = 0xFFFFFFFF;
static uint32_t         plcvm_stack_bot = 0;

static uint32_t         plcvm_sp =  EXTPLC_SIZEOF_RAM;      // Stack pointer
static uint32_t         plcvm_sfp = EXTPLC_SIZEOF_RAM;      // Stack frame pointer
static uint32_t         plcvm_pc = 0;                       // program counter
static uint8_t          plcvm_cop = 0;                      // operator

static bool             plcvm_1st_start = true;

#include "extplc_vm.h"
// End PLC_VM Section

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
                plcvm_page = 0xFFFFFFFF;  // Reset cache
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
        default:
        {
            plc_exchg->len = 2;
            plc_exchg->data[1] = PLC_ANSWER_ERROR_CMD;
            break;
        }
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

static uint8_t plcPollOD(subidx_t * pSubidx)
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

#if defined ( __GNUC__ )
#pragma GCC diagnostic pop
#endif

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

#if defined ( __GNUC__ )
#pragma GCC diagnostic pop
#endif

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
            return (plc_ram[base] != plc_old[base]);
            
        // supressed warning -Wswitch;
        default:
            return 0;
    }
    
    return (plc_ram[base] & mask) != (plc_old[base] & mask);
}

void plcvm_set_stack_bot(uint32_t val)
{
    plcvm_stack_bot = val;
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

    if(base >= EXTPLC_SIZEOF_WO)
        return false;
    return true;
}

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
}

void plcProc(void)
{
    if(plc_run)
    {
#if (defined LED_On) // && (defined DEBUG)
        LED_On();
#endif  //  LED_On
        plcvm_sfp = EXTPLC_SIZEOF_RAM;      // Stack frame pointer
        plcvm_sp = EXTPLC_SIZEOF_RAM;       // Stack pointer
        plcvm_pc = 0;                       // program counter
        plcvm_stat = PLC_ANSWER_RUN;

        uint16_t plc_wd = 0xFFFF;
        while(plcvm_stat == PLC_ANSWER_RUN)
        {
            if(plc_wd > 0)
            {    
                plc_wd--;
            }
            else
            {
                plcvm_stat = PLC_ANSWER_ERROR_WD;
                break;
            }

            plcvm_cop = plcvm_lpm_u8();
            cbCOP_t cop_proc = plcvm_cb[plcvm_cop];
            cop_proc();
        }
        
        plcvm_1st_start = false;
#if (defined LED_Off)// && (defined DEBUG)
        LED_Off();
#endif  //  LED_Off
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
    
                uint32_t acc = 0;
                if((plcvm_sp > 0) && (plcvm_sp < EXTPLC_SIZEOF_RAM))
                    acc = plc_ram[plcvm_sp - 1];
    
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

                plcvm_page = 0xFFFFFFFF;  // Reset cache
                plcvm_stat = PLC_ANSWER_OK;
                plc_answer = true;
            }
        }
    }
}

#endif  //  EXTPLC_USED
