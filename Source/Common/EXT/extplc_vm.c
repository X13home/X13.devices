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

#include "extplc.h"


#define EXTPLC_SIZEOF_PRG_CACHE         16      // Must be 2^n
#define EXTPLC_SIZEOF_RAM               32      // size in uint32_t

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
}ePLCVM_COP_t;

uint32_t                plc_ram[EXTPLC_SIZEOF_RAM];     // PLC data + merkers + stack
static ePLC_ANSWER_t    plcvm_stat = PLC_ANSWER_OK;
static uint32_t         plcvm_page = 0xFFFFFFFF;

static int32_t plcvm_peek(uint32_t sp)
{
    if(sp < EXTPLC_SIZEOF_RAM)
        return plc_ram[sp];

    plcvm_stat = PLC_ANSWER_ERROR_AV_RAM;
    return 0;
}

static void plcvm_poke(uint32_t sp, uint32_t val)
{
    if(sp < EXTPLC_SIZEOF_RAM)
        plc_ram[sp] = val;
    else
        plcvm_stat = PLC_ANSWER_ERROR_AV_RAM;
}

static uint8_t plcvm_get_cop(uint32_t pc)
{
    static uint8_t  plc_prg_cache[EXTPLC_SIZEOF_PRG_CACHE];
    
    if(pc >= EXTPLC_SIZEOF_PRG)
    {
        plcvm_page = 0xFFFFFFFF;
        plcvm_stat = PLC_ANSWER_ERROR_AV_PRG;
        return COP_RET;
    }
    
    if((pc & ~(EXTPLC_SIZEOF_PRG_CACHE - 1UL)) != plcvm_page)
    {
        plcvm_page = (pc & ~(EXTPLC_SIZEOF_PRG_CACHE - 1UL));
        eeprom_read(plc_prg_cache, eePLCprogram + plcvm_page, EXTPLC_SIZEOF_PRG_CACHE);
    }
    
    return plc_prg_cache[pc & (EXTPLC_SIZEOF_PRG_CACHE - 1UL)];
}

void plcvm_reset_cache(void)
{
    plcvm_page = 0xFFFFFFFF;
}

ePLC_ANSWER_t plcvm(void)
{
    uint32_t sp = (EXTPLC_SIZEOF_RAM - 1);          // Stack pointer
    uint32_t spf = EXTPLC_SIZEOF_RAM;               // Stack frame pointer
    uint32_t pc = 0;                                // program counter

    plcvm_stat = PLC_ANSWER_OK;

    while(plcvm_stat == PLC_ANSWER_OK)
    {
        uint8_t cop = plcvm_get_cop(pc++);

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
                    acc = plcvm_get_cop(pc++);
                    if((type == 0) && (acc & 0x80))
                        acc |= 0xFFFFFF00;
                }
                else if(place == 0x10)              // int16/uint16
                {
                    acc = plcvm_get_cop(pc++);
                    acc |= (int32_t)(plcvm_get_cop(pc++))<<8;
                    if((type == 0) && (acc & 0x8000))
                        acc |= 0xFFFF0000;
                }
                else if(type == 0)                  // int32
                {
                    acc = plcvm_get_cop(pc++);
                    acc |= (int32_t)(plcvm_get_cop(pc++))<<8;
                    acc |= (int32_t)(plcvm_get_cop(pc++))<<16;
                    acc |= (int32_t)(plcvm_get_cop(pc++))<<24;
                }
                else                                // 0xFFFFFFFF
                {
                    acc = 0xFFFFFFFF;
                }

                plcvm_poke(--sp, acc);
            }
            else
            {
                uint32_t addr;
                if(place == 0)
                    addr = plcvm_peek(sp);
                else if(place == 0x08)
                    addr = plcvm_peek(sp) + plcvm_get_cop(pc++);
                else if(place == 0x10)
                {
                    int32_t adr1 = plcvm_get_cop(pc++);
                    adr1 |= (int32_t)(plcvm_get_cop(pc++))<<8;
                    addr = plcvm_peek(sp) + adr1;
                }
                else
                {
                    addr = plcvm_get_cop(pc++);
                    addr |= (int32_t)(plcvm_get_cop(pc++))<<8;
                    sp--;
                }

                if(type == 1)    // Bool
                {
                    acc = plcvm_peek(addr >> 5);
                    acc = ((acc & (1UL<<(addr & 0x1F))) != 0);
                }
                else if((type == 2) || (type == 3))     // [u]int8
                {
                    acc = plcvm_peek(addr >> 2);
                    acc = acc >> ((addr & 0x03) * 8);
                    
                    if((type == 2) && (acc & 0x80))
                        acc |= 0xFFFFFF00;
                    else
                        acc &= 0x000000FF;
                }
                else if((type == 4) || (type == 5))
                {
                    acc = plcvm_peek(addr >> 1);
                    acc = acc >> ((addr & 0x01) * 8);
                    
                    if((type == 4) && (acc & 0x8000))
                        acc |= 0xFFFF0000;
                    else
                        acc &= 0x0000FFFF;
                }
                else
                    acc = plcvm_peek(addr);

                plcvm_poke(sp, acc);
            }
        }
        else if(cop >= 0xA0 && cop <= 0xBF)
        {
            uint8_t type = cop & 7;
            uint8_t place = cop & 0x18;
            
            uint32_t addr;
            if(place == 0)
                addr = plcvm_peek(sp++);
            else if(place == 0x08)
                addr = plcvm_peek(sp++) + plcvm_get_cop(pc++);
            else if(place == 0x10)
            {
                int32_t adr1 = plcvm_get_cop(pc++);
                adr1 |= (int32_t)(plcvm_get_cop(pc++))<<8;
                addr = plcvm_peek(sp++) + adr1;
            }
            else
            {
                addr = plcvm_get_cop(pc++);
                addr |= (int32_t)(plcvm_get_cop(pc++))<<8;
            }

            uint32_t acc  = plcvm_peek(sp++);
            uint32_t bcc;
            uint32_t mask;
            

            if(type == 1)                               // bool
            {
                mask = 1UL<<(addr & 0x1F);
                addr >>= 5;
                bcc = plcvm_peek(addr);
                if(acc != 0)
                    bcc |= mask;
                else
                    bcc &= ~mask;
                plcvm_poke(addr, bcc);
            }
            else if((type == 2) || (type == 3))
            {
                acc &= 0x000000FF;
                acc <<= (addr & 3)*8;
                mask = 0x000000FF << ((addr & 3)*8);
                
                addr >>= 2;
                bcc = plcvm_peek(addr);
                bcc &= ~mask;
                bcc |= acc;
                plcvm_poke(addr, bcc);
            }
            else if((type == 4) || (type == 5))
            {
                acc &= 0x0000FFFF;
                acc <<= (addr & 1)*16;
                mask = 0x0000FFFF << ((addr & 1)*16);
                
                addr >>= 1;
                bcc = plcvm_peek(addr);
                bcc &= ~mask;
                bcc |= acc;
                plcvm_poke(addr, bcc);
            }
            else if(type == 6)
            {
                plcvm_poke(addr, acc);
            }
            else    // Type 0,7
            {
                plcvm_stat = PLC_ANSWER_ERROR_UNK_COP;        // Error unknown COP
                break;
            }
        }
        else
        switch(cop)
        {
            case COP_NOP:
                break;

            // 0x22 - 0x3F, arithmetic and logic operations
            case COP_NOT:
                plcvm_poke(sp, ~plcvm_peek(sp));
                break;
            case COP_INC:
                plcvm_poke(sp, plcvm_peek(sp) + 1);
                break;
            case COP_DEC:
                plcvm_poke(sp, plcvm_peek(sp) - 1);
                break;
            case COP_NEG:
                plcvm_poke(sp, -plcvm_peek(sp));
                break;

            case COP_RET:
                if(spf == EXTPLC_SIZEOF_RAM)
                    break;
                sp  = spf - 1;
                spf = plcvm_peek(sp++);
                pc  = plcvm_peek(sp++);
                break;

            default:
                plcvm_stat = PLC_ANSWER_ERROR_UNK_COP;        // Error unknown COP
                break;
        }
    }
    return plcvm_stat;
}

#endif  //  EXTPLC_USED
