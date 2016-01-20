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
#include "extplc_vm.h"

#define EXTPLC_SIZEOF_PRG_CACHE         16      // Must be 2^n
#define EXTPLC_SIZEOF_RAM               32      // size in uint32_t

typedef void (*cbCOP_t)(void);

uint32_t                plc_ram[EXTPLC_SIZEOF_RAM];         // PLC data + merkers + stack
static ePLC_ANSWER_t    plcvm_stat = PLC_ANSWER_OK;
static uint32_t         plcvm_page = 0xFFFFFFFF;

static uint32_t         plcvm_sp = (EXTPLC_SIZEOF_RAM - 1); // Stack pointer
static uint32_t         plcvm_sfp = EXTPLC_SIZEOF_RAM;      // Stack frame pointer
static uint32_t         plcvm_pc = 0;                       // program counter
static uint8_t          plcvm_cop = 0;                      // operator

// Read programm memory
// return LPM8[plcvm_pc++]
static uint8_t plcvm_get_cop(void)
{
    static uint8_t  plc_prg_cache[EXTPLC_SIZEOF_PRG_CACHE];
    
    if(plcvm_pc >= EXTPLC_SIZEOF_PRG)
    {
        plcvm_stat = PLC_ANSWER_ERROR_AV_PRG;
        return 0xFF;
    }
    
    if((plcvm_pc & ~(EXTPLC_SIZEOF_PRG_CACHE - 1UL)) != plcvm_page)
    {
        plcvm_page = (plcvm_pc & ~(EXTPLC_SIZEOF_PRG_CACHE - 1UL));
        eeprom_read(plc_prg_cache, eePLCprogram + plcvm_page, EXTPLC_SIZEOF_PRG_CACHE);
    }
    
    uint8_t retval = plc_prg_cache[plcvm_pc & (EXTPLC_SIZEOF_PRG_CACHE - 1UL)];
    plcvm_pc++;
    return retval;
}

// Get uint32_t from program memory
static uint32_t plcvm_lpm_u32(void)
{
    uint32_t acc = plcvm_get_cop();
    acc |= (uint32_t)(plcvm_get_cop())<<8;
    acc |= (uint32_t)(plcvm_get_cop())<<16;
    acc |= (uint32_t)(plcvm_get_cop())<<24;
    return acc;
}

// return bool->uint32 RAM[addr'bool]
static uint32_t plcvm_get_bool(uint32_t addr)
{
    if(addr < (EXTPLC_SIZEOF_RAM * 32))
    {
        uint32_t acc = plc_ram[addr >> 5];
        return ((acc & (1UL<<(addr & 0x1F))) != 0) ? 1 : 0;
    }
    
    plcvm_stat = PLC_ANSWER_ERROR_AV_RAM;
    return 0;
}

// return uint8->uint32 RAM[addr'uint8]
static uint32_t plcvm_get_u8(uint32_t addr)
{
    if(addr < (EXTPLC_SIZEOF_RAM * 4))
    {
        uint32_t acc = plc_ram[addr >> 2];
        acc >>= ((addr & 0x03) * 8);
        return (acc & 0x000000FF);
    }
    
    plcvm_stat = PLC_ANSWER_ERROR_AV_RAM;
    return 0;
}

// return int8->uint32 RAM[addr'int8]
static uint32_t plcvm_get_s8(uint32_t addr)
{
    if(addr < (EXTPLC_SIZEOF_RAM * 4))
    {
        uint32_t acc = plc_ram[addr >> 2];
        acc >>= ((addr & 0x03) * 8);

        if(acc & 0x80)
            acc |= 0xFFFFFF00;
        else
            acc &= 0x000000FF;
        return acc;
    }
    
    plcvm_stat = PLC_ANSWER_ERROR_AV_RAM;
    return 0;
}

// return uint16->uint32 RAM[addr'uint16]
static uint32_t plcvm_get_u16(uint32_t addr)
{
    if(addr < (EXTPLC_SIZEOF_RAM * 4))
    {
        uint32_t acc = plc_ram[addr >> 2];
        acc >>= ((addr & 0x01) * 16);
        return (acc & 0x0000FFFF);
    }
    
    plcvm_stat = PLC_ANSWER_ERROR_AV_RAM;
    return 0;
}

// return int16->uint32 RAM[addr'int16]
static uint32_t plcvm_get_s16(uint32_t addr)
{
    if(addr < (EXTPLC_SIZEOF_RAM * 2))
    {
        uint32_t acc = plc_ram[addr >> 1];
        acc >>= ((addr & 0x01) * 16);
        if(acc & 0x8000)
            acc |= 0xFFFF0000;
        else
            acc &= 0x0000FFFF;
        return acc;
    }
    
    plcvm_stat = PLC_ANSWER_ERROR_AV_RAM;
    return 0;
}

// return u32 RAM[addr'u32]
static uint32_t plcvm_get_u32(uint32_t addr)
{
    if(addr < EXTPLC_SIZEOF_RAM)
        return plc_ram[addr];

    plcvm_stat = PLC_ANSWER_ERROR_AV_RAM;
    return 0;
}

static void plcvm_set_bool(uint32_t addr, bool sr)
{
    if(addr < (EXTPLC_SIZEOF_RAM * 32))
    {
        uint32_t mask = 1UL<<(addr & 0x1F);
        addr >>= 5;
        
        if(sr)
            plc_ram[addr] |= mask;
        else
            plc_ram[addr] &= ~mask;
    }
    
    plcvm_stat = PLC_ANSWER_ERROR_AV_RAM;
}

static void plcvm_set_u8(uint32_t addr, uint8_t val)
{
    if(addr < (EXTPLC_SIZEOF_RAM * 4))
    {
        uint32_t acc = plc_ram[addr >> 2];
        uint32_t mask = 0x000000FF << ((addr & 3)*8);
        uint32_t bcc = val;
        bcc <<= ((addr & 3)*8);
        acc &= ~mask;
        acc |= bcc;
        plc_ram[addr >> 2] = acc;
    }

    plcvm_stat = PLC_ANSWER_ERROR_AV_RAM;
}

static void plcvm_set_u16(uint32_t addr, uint16_t val)
{
    if(addr < (EXTPLC_SIZEOF_RAM * 2))
    {
        uint32_t acc = plc_ram[addr >> 1];
        uint32_t mask = 0x0000FFFF << ((addr & 1)*16);
        uint32_t bcc = val;
        bcc <<= ((addr & 1)*16);
        acc &= ~mask;
        acc |= bcc;
        plc_ram[addr >> 1] = acc;
    }
    else
        plcvm_stat = PLC_ANSWER_ERROR_AV_RAM;
}

static void plcvm_set_u32(uint32_t addr, uint32_t val)
{
    if(addr < EXTPLC_SIZEOF_RAM)
        plc_ram[addr] = val;
    else
        plcvm_stat = PLC_ANSWER_ERROR_AV_RAM;
}


// Operators callback
static void cb_nop(void){}
static void cb_dup(void)
{
    uint32_t tmp = plcvm_get_u32(plcvm_sp--);
    plcvm_set_u32(plcvm_sp, tmp);
}
static void cb_drop(void){plcvm_sp++;}
static void cb_nip(void)
{
    uint32_t tmp = plcvm_get_u32(plcvm_sp++);
    plcvm_set_u32(plcvm_sp, tmp);
}
static void cb_swap(void)
{
    uint32_t tmp = plcvm_get_u32(plcvm_sp);
    plcvm_set_u32(plcvm_sp, plcvm_get_u32(plcvm_sp + 1));
    plcvm_set_u32(plcvm_sp + 1, tmp);
}
static void cb_over(void)
{
    uint32_t tmp = plcvm_get_u32(plcvm_sp + 1);
    plcvm_set_u32(--plcvm_sp, tmp);
}
static void cb_rot(void)
{
    // ( a b c -- b c a )
    uint32_t tmp = plcvm_get_u32(plcvm_sp + 2);                 // tmp = *(SP+2)
    plcvm_set_u32(plcvm_sp + 2, plcvm_get_u32(plcvm_sp + 1));   // *(SP+2) = *(SP+1)
    plcvm_set_u32(plcvm_sp + 1, plcvm_get_u32(plcvm_sp));       // *(SP+1) = *(SP)
    plcvm_set_u32(plcvm_sp, tmp);                               // *(SP) = tmp
}
static void cb_not(void){plcvm_set_u32(plcvm_sp, ~plcvm_get_u32(plcvm_sp));}

static void cb_inc(void){plcvm_set_u32(plcvm_sp, plcvm_get_u32(plcvm_sp) + 1);}
static void cb_dec(void){plcvm_set_u32(plcvm_sp, plcvm_get_u32(plcvm_sp) - 1);}
static void cb_neg(void){plcvm_set_u32(plcvm_sp, -plcvm_get_u32(plcvm_sp));}

// Load from memory
static void cb_ldm__s(void)
{
    uint32_t addr = plcvm_get_u32(plcvm_sp);

    uint32_t acc;
    switch(plcvm_cop & 7)
    {
        case 1:
            acc = plcvm_get_bool(addr);
            break;
        case 2:
            acc = plcvm_get_s8(addr);
            break;
        case 3:
            acc = plcvm_get_u8(addr);
            break;
        case 4:
            acc = plcvm_get_s16(addr);
            break;
        case 5:
            acc = plcvm_get_u16(addr);
            break;
        default:
            acc = plcvm_get_u32(addr);
            break;
    }
    plcvm_set_u32(plcvm_sp, acc);
}

static void cb_ldm__cs8(void)
{
    uint32_t addr = plcvm_get_u32(plcvm_sp) + plcvm_get_cop();

    uint32_t acc;
    switch(plcvm_cop & 7)
    {
        case 1:
            acc = plcvm_get_bool(addr);
            break;
        case 2:
            acc = plcvm_get_s8(addr);
            break;
        case 3:
            acc = plcvm_get_u8(addr);
            break;
        case 4:
            acc = plcvm_get_s16(addr);
            break;
        case 5:
            acc = plcvm_get_u16(addr);
            break;
        default:
            acc = plcvm_get_u32(addr);
            break;
    }
    plcvm_set_u32(plcvm_sp, acc);
}

static void cb_ldm__cs16(void)
{
    uint16_t offs_pc = plcvm_get_cop();
    offs_pc |= (uint16_t)(plcvm_get_cop())<<8;
    uint32_t addr = plcvm_get_u32(plcvm_sp) + offs_pc;

    uint32_t acc;
    switch(plcvm_cop & 7)
    {
        case 1:
            acc = plcvm_get_bool(addr);
            break;
        case 2:
            acc = plcvm_get_s8(addr);
            break;
        case 3:
            acc = plcvm_get_u8(addr);
            break;
        case 4:
            acc = plcvm_get_s16(addr);
            break;
        case 5:
            acc = plcvm_get_u16(addr);
            break;
        default:
            acc = plcvm_get_u32(addr);
            break;
    }
    plcvm_set_u32(plcvm_sp, acc);
}

static void cb_ldm__c16(void)
{
    uint16_t addr = plcvm_get_cop();
    addr |= (uint16_t)(plcvm_get_cop())<<8;
    plcvm_sp--;

    uint32_t acc;
    switch(plcvm_cop & 7)
    {
        case 1:
            acc = plcvm_get_bool(addr);
            break;
        case 2:
            acc = plcvm_get_s8(addr);
            break;
        case 3:
            acc = plcvm_get_u8(addr);
            break;
        case 4:
            acc = plcvm_get_s16(addr);
            break;
        case 5:
            acc = plcvm_get_u16(addr);
            break;
        default:
            acc = plcvm_get_u32(addr);
            break;
    }
    plcvm_set_u32(plcvm_sp, acc);
}

// Load constant
static void cb_ldi_zero(void){plcvm_set_u32(--plcvm_sp, 0);}
static void cb_ldi_true(void){plcvm_set_u32(--plcvm_sp, 1);}
static void cb_ldi_minus1(void){plcvm_set_u32(--plcvm_sp, -1);}
static void cb_ldi_s1(void)
{
    uint32_t acc = plcvm_get_cop();
    if(acc & 0x80)
    {
        acc |= 0xFFFFFF00;
    }
    plcvm_set_u32(--plcvm_sp, acc);
}
static void cb_ldi_u1(void){plcvm_set_u32(--plcvm_sp, plcvm_get_cop());}
static void cb_ldi_s2(void)
{
    uint32_t acc = plcvm_get_cop();
    acc |= (uint16_t)(plcvm_get_cop())<<8;
    if(acc & 0x8000)
    {
        acc |= 0xFFFF0000;
    }
    plcvm_set_u32(--plcvm_sp, acc);
}
static void cb_ldi_u2(void)
{
    uint32_t acc = (uint16_t)(plcvm_get_cop());
    acc	|=  (uint16_t)(plcvm_get_cop())<<8;
    plcvm_set_u32(--plcvm_sp, acc);
}
static void cb_ldi_s4(void){plcvm_set_u32(--plcvm_sp, plcvm_lpm_u32());}

// Store in memory
static void cb_stm__s(void)
{
    uint8_t  type = plcvm_cop & 7;
    uint32_t addr = plcvm_get_u32(plcvm_sp++);
    uint32_t acc  = plcvm_get_u32(plcvm_sp++);

    if(type == 1)                               // bool
    {
        plcvm_set_bool(addr, (acc != 0) ? 1 : 0);
    }
    else if((type == 2) || (type == 3))
    {
        plcvm_set_u8(addr, acc & 0x000000FF);
    }
    else if((type == 4) || (type == 5))
    {
        plcvm_set_u16(addr, acc & 0x0000FFFF);
    }
    else // if(type == 6)
    {
        plcvm_set_u32(addr, acc);
    }
}

static void cb_stm_cs8(void)
{
    uint8_t  type = plcvm_cop & 7;
    uint32_t addr = plcvm_get_u32(plcvm_sp++) + plcvm_get_cop();
    uint32_t acc  = plcvm_get_u32(plcvm_sp++);

    if(type == 1)                               // bool
    {
        plcvm_set_bool(addr, (acc != 0) ? 1 : 0);
    }
    else if((type == 2) || (type == 3))
    {
        plcvm_set_u8(addr, acc & 0x000000FF);
    }
    else if((type == 4) || (type == 5))
    {
        plcvm_set_u16(addr, acc & 0x0000FFFF);
    }
    else // if(type == 6)
    {
        plcvm_set_u32(addr, acc);
    }
}

static void cb_stm_cs16(void)
{
    uint8_t  type = plcvm_cop & 7;
    uint16_t offs = (uint16_t)plcvm_get_cop();
    	     offs |= (uint16_t)(plcvm_get_cop())<<8;
    uint32_t addr = plcvm_get_u32(plcvm_sp++) + offs;
    uint32_t acc  = plcvm_get_u32(plcvm_sp++);

    if(type == 1)                               // bool
    {
        plcvm_set_bool(addr, (acc != 0) ? 1 : 0);
    }
    else if((type == 2) || (type == 3))
    {
        plcvm_set_u8(addr, acc & 0x000000FF);
    }
    else if((type == 4) || (type == 5))
    {
        plcvm_set_u16(addr, acc & 0x0000FFFF);
    }
    else // if(type == 6)
    {
        plcvm_set_u32(addr, acc);
    }
}

static void cb_stm_c16(void)
{
    uint8_t  type = plcvm_cop & 7;
    uint16_t addr = plcvm_get_cop();
             addr |= (uint16_t)(plcvm_get_cop())<<8;
    uint32_t acc  = plcvm_get_u32(plcvm_sp++);

    if(type == 1)                               // bool
    {
        plcvm_set_bool(addr, (acc != 0) ? 1 : 0);
    }
    else if((type == 2) || (type == 3))
    {
        plcvm_set_u8(addr, acc & 0x000000FF);
    }
    else if((type == 4) || (type == 5))
    {
        plcvm_set_u16(addr, acc & 0x0000FFFF);
    }
    else // if(type == 6)
    {
        plcvm_set_u32(addr, acc);
    }
}


static const cbCOP_t plcmv_cb[] =
{
    // 0x00 - 0x1F
    
    cb_nop,     NULL,       cb_dup,     cb_drop,    cb_nip,     cb_swap,    cb_over,    cb_rot,
    cb_not,     NULL,       NULL,       NULL,       NULL,       NULL,       cb_inc,     cb_dec,
    cb_neg,     NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,
    NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,
    // 0x20 - 0x3F 
    NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,
    NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,
    NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,
    NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,
    NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,
    NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,
    NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,
    NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,
    // 0x60 - 0x9F
    cb_ldi_zero, cb_ldm__s, cb_ldm__s,  cb_ldm__s,  cb_ldm__s,  cb_ldm__s,  cb_ldm__s,  cb_ldi_true,
    cb_ldi_s1, cb_ldm__cs8, cb_ldm__cs8,cb_ldm__cs8,cb_ldm__cs8,cb_ldm__cs8,cb_ldm__cs8,cb_ldi_u1,
    cb_ldi_s2, cb_ldm__cs16, cb_ldm__cs16,cb_ldm__cs16,cb_ldm__cs16,cb_ldm__cs16,cb_ldm__cs16,cb_ldi_u2,
    cb_ldi_s4, cb_ldm__c16, cb_ldm__c16,cb_ldm__c16,cb_ldm__c16,cb_ldm__c16,cb_ldm__c16,cb_ldi_minus1,
    NULL, cb_stm__s, cb_stm__s, cb_stm__s,cb_stm__s, cb_stm__s, cb_stm__s, NULL,
    NULL, cb_stm_cs8, cb_stm_cs8, cb_stm_cs8, cb_stm_cs8, cb_stm_cs8, cb_stm_cs8, NULL,
    NULL, cb_stm_cs16, cb_stm_cs16, cb_stm_cs16, cb_stm_cs16, cb_stm_cs16, cb_stm_cs16, NULL,
    NULL, cb_stm_c16, cb_stm_c16, cb_stm_c16, cb_stm_c16, cb_stm_c16, cb_stm_c16,  NULL,
};

uint8_t plcvm_get_state(uint8_t *pBuf)
{
    *(pBuf++) = plcvm_sp & 0xFF;
    *(pBuf++) = (plcvm_sp>>8) & 0xFF;
    *(pBuf++) = (plcvm_sp>>16) & 0xFF;
    *(pBuf++) = (plcvm_sp>>24) & 0xFF;
    
    uint32_t acc = 0;
    if(plcvm_sp < EXTPLC_SIZEOF_RAM)
        acc = plc_ram[plcvm_sp];
    
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
    
    return 16;
}

void plcvm_reset_cache(void)
{
    plcvm_page = 0xFFFFFFFF;
}

ePLC_ANSWER_t plcvm(void)
{
    plcvm_sp = (EXTPLC_SIZEOF_RAM - 1); // Stack pointer
    plcvm_sfp = EXTPLC_SIZEOF_RAM;      // Stack frame pointer
    plcvm_pc = 0;                       // program counter
    plcvm_stat = PLC_ANSWER_OK;
    
    uint16_t plc_wd = 0xFFFF;

    while(plcvm_stat == PLC_ANSWER_OK)
    {
        if(plc_wd > 0)
            plc_wd--;
        else
            return PLC_ANSWER_ERROR_WD;

        plcvm_cop = plcvm_get_cop();

        // Standard operators
        if(plcvm_cop <= 0x9F)
        {
            cbCOP_t cop_proc = plcmv_cb[plcvm_cop];
            if(cop_proc == NULL)
                plcvm_stat = PLC_ANSWER_ERROR_UNK_COP;
            else
                cop_proc();
        }
        // Special Operators
        else
        {
            switch(plcvm_cop)
            {
                case 0xFF:                              // RET
                    if(plcvm_sfp == EXTPLC_SIZEOF_RAM)
                        return PLC_ANSWER_OK;
                    plcvm_sp  = plcvm_sfp - 1;
                    plcvm_sfp = plcvm_get_u32(plcvm_sp++);
                    plcvm_pc  = plcvm_get_u32(plcvm_sp++);
                    break;

                case 0xFE:                              // TEST
                    if(plcvm_get_u32(plcvm_sp++) != plcvm_lpm_u32())
                        return PLC_ANSWER_ERROR_TEST;
                    break;

                default:
                    return PLC_ANSWER_ERROR_UNK_COP;        // Error unknown COP
            }
        }
    }
    return plcvm_stat;
}

#endif  //  EXTPLC_USED
