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

typedef void (*cbCOP_t)(void);

uint32_t                plc_ram[EXTPLC_SIZEOF_RAM];         // PLC data + merkers + stack
static ePLC_ANSWER_t    plcvm_stat = PLC_ANSWER_OK;
static uint32_t         plcvm_page = 0xFFFFFFFF;

static uint32_t         plcvm_sp = (EXTPLC_SIZEOF_RAM - 1); // Stack pointer
static uint32_t         plcvm_sfp = EXTPLC_SIZEOF_RAM;      // Stack frame pointer
static uint32_t         plcvm_pc = 0;                       // program counter
static uint8_t          plcvm_cop = 0;                      // operator


// return LPM8[plcvm_pc++]
static uint8_t plcvm_lpm_u8(void)
{
    static uint8_t  plc_prg_cache[EXTPLC_SIZEOF_PRG_CACHE];
    
    if(plcvm_pc >= EXTPLC_SIZEOF_PRG)
    {
        plcvm_stat = PLC_ANSWER_ERROR_OFR_PC;
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

// return LPM32[plcvm_pc+=4]
static uint32_t plcvm_lpm_u32(void)
{
    uint32_t acc = plcvm_lpm_u8();
    acc |= (uint32_t)(plcvm_lpm_u8())<<8;
    acc |= (uint32_t)(plcvm_lpm_u8())<<16;
    acc |= (uint32_t)(plcvm_lpm_u8())<<24;
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
    
    plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;
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
    
    plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;
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
    
    plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;
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
    
    plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;
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
    
    plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;
    return 0;
}

// return u32 RAM[addr'u32]
static uint32_t plcvm_get_u32(uint32_t addr)
{
    if(addr < EXTPLC_SIZEOF_RAM)
        return plc_ram[addr];

    plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;
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
    
    plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;
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

    plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;
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
        plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;
}

static void plcvm_set_u32(uint32_t addr, uint32_t val)
{
    if(addr < EXTPLC_SIZEOF_RAM)
        plc_ram[addr] = val;
    else
        plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;
}

// Operators callback

// 0x00
static void cb_nop(void){}
// 0x02 - 0x07 - Stack manipulation
static void cb_dup(void){uint32_t tmp = plcvm_get_u32(plcvm_sp--); plcvm_set_u32(plcvm_sp, tmp);}   // *(SP) = *(SP--);  (a -- a a)
static void cb_drop(void){plcvm_sp++;}                                                              // SP++;  (a - )
static void cb_nip(void){uint32_t tmp = plcvm_get_u32(plcvm_sp++); plcvm_set_u32(plcvm_sp, tmp);}   // *(SP) = *(SP++)  //( a b -- b )
static void cb_swap(void){uint32_t tmp = plcvm_get_u32(plcvm_sp); plcvm_set_u32(plcvm_sp, plcvm_get_u32(plcvm_sp + 1)); plcvm_set_u32(plcvm_sp + 1, tmp);}  // tmp = *(SP); *(SP) = *(SP+1); *(SP+1) = tmp;  // (a b -- b a)
static void cb_over(void){uint32_t tmp = plcvm_get_u32(plcvm_sp + 1); plcvm_set_u32(--plcvm_sp, tmp);} // *(--SP) = *(SP+1);  //( a b -- a b a )
static void cb_rot(void)    // ( a b c -- b c a )
{
    uint32_t tmp = plcvm_get_u32(plcvm_sp + 2);                 // tmp = *(SP+2)
    plcvm_set_u32(plcvm_sp + 2, plcvm_get_u32(plcvm_sp + 1));   // *(SP+2) = *(SP+1)
    plcvm_set_u32(plcvm_sp + 1, plcvm_get_u32(plcvm_sp));       // *(SP+1) = *(SP)
    plcvm_set_u32(plcvm_sp, tmp);                               // *(SP) = tmp
}

// 0x08 - 0x0F - Bitwise and Bit Shift Operators
static void cb_not(void){plcvm_set_u32(plcvm_sp, ~plcvm_get_u32(plcvm_sp));}
static void cb_and(void){uint32_t tmp = plcvm_get_u32(plcvm_sp++); tmp &= plcvm_get_u32(plcvm_sp); plcvm_set_u32(plcvm_sp, tmp);}
static void cb_or(void){uint32_t tmp = plcvm_get_u32(plcvm_sp++); tmp |= plcvm_get_u32(plcvm_sp); plcvm_set_u32(plcvm_sp, tmp);}
static void cb_xor(void){uint32_t tmp = plcvm_get_u32(plcvm_sp++); tmp ^= plcvm_get_u32(plcvm_sp); plcvm_set_u32(plcvm_sp, tmp);}
static void cb_shl(void){uint32_t tmp = plcvm_get_u32(plcvm_sp++); tmp <<= (plcvm_get_u32(plcvm_sp) & 0x1F); plcvm_set_u32(plcvm_sp, tmp);}
static void cb_shr(void){uint32_t tmp = plcvm_get_u32(plcvm_sp++); tmp >>= (plcvm_get_u32(plcvm_sp) & 0x1F); plcvm_set_u32(plcvm_sp, tmp);}
static void cb_srs(void)
{
    uint32_t tmp = plcvm_get_u32(plcvm_sp++); 
    uint8_t shift = plcvm_get_u32(plcvm_sp) & 0x1F;
    if(tmp & 0x8000000)
    {
        while(shift > 0)
        {
            tmp >>= 1;
            tmp |= 0x80000000;
            shift--;
        }
    }
    else
        tmp <<= shift;
    plcvm_set_u32(plcvm_sp, tmp);
}
static void cb_shift(void)
{
    uint8_t cmd = plcvm_lpm_u8();
    int32_t tmp = (int32_t)plcvm_get_u32(plcvm_sp);

    if(cmd & 0x80)      // Shift Right
    {
        if(cmd & 0x40)
        {
            int32_t s4 = tmp;
            s4 >>= (cmd & 0x1F);
            tmp = s4;
        }
        else
            tmp >>= (cmd & 0x1F);
    }
    else
        tmp <<= (cmd & 0x1F);

    plcvm_set_u32(plcvm_sp, tmp);
}

// 0x10 - 0x17 - Arithmetic Operators
static void cb_add(void)
{
    int32_t r1 = (int32_t)plcvm_get_u32(plcvm_sp++);
    int32_t r2 = (int32_t)plcvm_get_u32(plcvm_sp);
    plcvm_set_u32(plcvm_sp, r1 + r2);
}
static void cb_sub(void)
{
    int32_t r1 = (int32_t)plcvm_get_u32(plcvm_sp++);
    int32_t r2 = (int32_t)plcvm_get_u32(plcvm_sp);
    plcvm_set_u32(plcvm_sp, r1 - r2);
}
static void cb_mul(void)
{
    int32_t r1 = (int32_t)plcvm_get_u32(plcvm_sp++);
    int32_t r2 = (int32_t)plcvm_get_u32(plcvm_sp);
    plcvm_set_u32(plcvm_sp, r1 - r2);
}
static void cb_div(void)
{
    int32_t r2 = (int32_t)plcvm_get_u32(plcvm_sp + 1);
    if(r2 == 0)
    {
        plcvm_stat = PLC_ANSWER_ERROR_DIV0;
    }    
    else
    {
        int32_t r1 = (int32_t)plcvm_get_u32(plcvm_sp++);
        plcvm_set_u32(plcvm_sp, r1/r2);
    }
}
static void cb_mod(void)
{
    int32_t r1 = (int32_t)plcvm_get_u32(plcvm_sp++);
    int32_t r2 = (int32_t)plcvm_get_u32(plcvm_sp);
    plcvm_set_u32(plcvm_sp, r1 % r2);
}
static void cb_inc(void){plcvm_set_u32(plcvm_sp, plcvm_get_u32(plcvm_sp) + 1);}
static void cb_dec(void){plcvm_set_u32(plcvm_sp, plcvm_get_u32(plcvm_sp) - 1);}
static void cb_neg(void){plcvm_set_u32(plcvm_sp, -plcvm_get_u32(plcvm_sp));}

// 0x18 - 0x1F - Reserved
// 0x20 - 0x27 - Compare
// 0x30 - 0x3F - Reserved
// 0x40 - 0x5F - Load with SFP based offset
// 0x60 - 0x7F - Store with SFP based offset

// 0x80 - 0x9F - Load from memory

// *(SP) = (type)*(*(SP));
static void cb_ldm_b1_s(void)
{
    uint32_t addr = plcvm_get_u32(plcvm_sp);
    uint32_t acc = plcvm_get_bool(addr);
    plcvm_set_u32(plcvm_sp, acc);
}
static void cb_ldm_s1_s(void)
{
    uint32_t addr = plcvm_get_u32(plcvm_sp);
    uint32_t acc = plcvm_get_s8(addr);
    plcvm_set_u32(plcvm_sp, acc);
}
static void cb_ldm_u1_s(void)
{
    uint32_t addr = plcvm_get_u32(plcvm_sp);
    uint32_t acc = plcvm_get_u8(addr);
    plcvm_set_u32(plcvm_sp, acc);    
}
static void cb_ldm_s2_s(void)
{
    uint32_t addr = plcvm_get_u32(plcvm_sp);
    uint32_t acc = plcvm_get_s16(addr);
    plcvm_set_u32(plcvm_sp, acc);
}
static void cb_ldm_u2_s(void)
{
    uint32_t addr = plcvm_get_u32(plcvm_sp);
    uint32_t acc = plcvm_get_u16(addr);
    plcvm_set_u32(plcvm_sp, acc);
}
static void cb_ldm_s4_s(void)
{
    uint32_t addr = plcvm_get_u32(plcvm_sp);
    uint32_t acc = plcvm_get_u32(addr);
    plcvm_set_u32(plcvm_sp, acc);
}
//*(SP) = (type)*(*(SP)+*(uint8*)(PC++));
static void cb_ldm_b1_cs8(void)
{
    uint32_t addr = plcvm_get_u32(plcvm_sp) + plcvm_lpm_u8();
    uint32_t acc = plcvm_get_bool(addr);
    plcvm_set_u32(plcvm_sp, acc);
}
static void cb_ldm_s1_cs8(void)
{
    uint32_t addr = plcvm_get_u32(plcvm_sp) + plcvm_lpm_u8();
    uint32_t acc = plcvm_get_s8(addr);
    plcvm_set_u32(plcvm_sp, acc);
}
static void cb_ldm_u1_cs8(void)
{
    uint32_t addr = plcvm_get_u32(plcvm_sp) + plcvm_lpm_u8();
    uint32_t acc = plcvm_get_u8(addr);
    plcvm_set_u32(plcvm_sp, acc);    
}
static void cb_ldm_s2_cs8(void)
{
    uint32_t addr = plcvm_get_u32(plcvm_sp) + plcvm_lpm_u8();
    uint32_t acc = plcvm_get_s16(addr);
    plcvm_set_u32(plcvm_sp, acc);
}
static void cb_ldm_u2_cs8(void)
{
    uint32_t addr = plcvm_get_u32(plcvm_sp) + plcvm_lpm_u8();
    uint32_t acc = plcvm_get_u16(addr);
    plcvm_set_u32(plcvm_sp, acc);
}
static void cb_ldm_s4_cs8(void)
{
    uint32_t addr = plcvm_get_u32(plcvm_sp) + plcvm_lpm_u8();
    uint32_t acc = plcvm_get_u32(addr);
    plcvm_set_u32(plcvm_sp, acc);
}
// *(SP) = (type)*(*(SP)+*(uint16*)(PC+=2));
static void cb_ldm_b1_cs16(void)
{
    uint16_t offs_pc = plcvm_lpm_u8();
    offs_pc |= (uint16_t)(plcvm_lpm_u8())<<8;
    uint32_t addr = plcvm_get_u32(plcvm_sp) + offs_pc;
    uint32_t acc = plcvm_get_bool(addr);
    plcvm_set_u32(plcvm_sp, acc);
}
static void cb_ldm_s1_cs16(void)
{
    uint16_t offs_pc = plcvm_lpm_u8();
    offs_pc |= (uint16_t)(plcvm_lpm_u8())<<8;
    uint32_t addr = plcvm_get_u32(plcvm_sp) + offs_pc;
    uint32_t acc = plcvm_get_s8(addr);
    plcvm_set_u32(plcvm_sp, acc);
}
static void cb_ldm_u1_cs16(void)
{
    uint16_t offs_pc = plcvm_lpm_u8();
    offs_pc |= (uint16_t)(plcvm_lpm_u8())<<8;
    uint32_t addr = plcvm_get_u32(plcvm_sp) + offs_pc;
    uint32_t acc = plcvm_get_u8(addr);
    plcvm_set_u32(plcvm_sp, acc);    
}
static void cb_ldm_s2_cs16(void)
{
    uint16_t offs_pc = plcvm_lpm_u8();
    offs_pc |= (uint16_t)(plcvm_lpm_u8())<<8;
    uint32_t addr = plcvm_get_u32(plcvm_sp) + offs_pc;
    uint32_t acc = plcvm_get_s16(addr);
    plcvm_set_u32(plcvm_sp, acc);
}
static void cb_ldm_u2_cs16(void)
{
    uint16_t offs_pc = plcvm_lpm_u8();
    offs_pc |= (uint16_t)(plcvm_lpm_u8())<<8;
    uint32_t addr = plcvm_get_u32(plcvm_sp) + offs_pc;
    uint32_t acc = plcvm_get_u16(addr);
    plcvm_set_u32(plcvm_sp, acc);
}
static void cb_ldm_s4_cs16(void)
{
    uint16_t offs_pc = plcvm_lpm_u8();
    offs_pc |= (uint16_t)(plcvm_lpm_u8())<<8;
    uint32_t addr = plcvm_get_u32(plcvm_sp) + offs_pc;
    uint32_t acc = plcvm_get_u32(addr);
    plcvm_set_u32(plcvm_sp, acc);
}
// *(--SP) = (type)*(*(uint16*)(PC+=2)); // bool LPM(addr)
static void cb_ldm_b1_c16(void)
{
    uint16_t addr = plcvm_lpm_u8();
    addr |= (uint16_t)(plcvm_lpm_u8())<<8;
    plcvm_sp--;
    uint32_t acc = plcvm_get_bool(addr);
    plcvm_set_u32(plcvm_sp, acc);
}
static void cb_ldm_s1_c16(void)
{
    uint16_t addr = plcvm_lpm_u8();
    addr |= (uint16_t)(plcvm_lpm_u8())<<8;
    plcvm_sp--;
    uint32_t acc = plcvm_get_s8(addr);
    plcvm_set_u32(plcvm_sp, acc);
}
static void cb_ldm_u1_c16(void)
{
    uint16_t addr = plcvm_lpm_u8();
    addr |= (uint16_t)(plcvm_lpm_u8())<<8;
    plcvm_sp--;
    uint32_t acc = plcvm_get_u8(addr);
    plcvm_set_u32(plcvm_sp, acc);    
}
static void cb_ldm_s2_c16(void)
{
    uint16_t addr = plcvm_lpm_u8();
    addr |= (uint16_t)(plcvm_lpm_u8())<<8;
    plcvm_sp--;
    uint32_t acc = plcvm_get_s16(addr);
    plcvm_set_u32(plcvm_sp, acc);
}
static void cb_ldm_u2_c16(void)
{
    uint16_t addr = plcvm_lpm_u8();
    addr |= (uint16_t)(plcvm_lpm_u8())<<8;
    plcvm_sp--;
    uint32_t acc = plcvm_get_u16(addr);
    plcvm_set_u32(plcvm_sp, acc);
}
static void cb_ldm_s4_c16(void)
{
    uint16_t addr = plcvm_lpm_u8();
    addr |= (uint16_t)(plcvm_lpm_u8())<<8;
    plcvm_sp--;
    uint32_t acc = plcvm_get_u32(addr);
    plcvm_set_u32(plcvm_sp, acc);
}

// Load constant
static void cb_ldi_zero(void){plcvm_set_u32(--plcvm_sp, 0);}
static void cb_ldi_true(void){plcvm_set_u32(--plcvm_sp, 1);}
static void cb_ldi_minus1(void){plcvm_set_u32(--plcvm_sp, -1);}
static void cb_ldi_s1(void)
{
    uint32_t acc = plcvm_lpm_u8();
    if(acc & 0x80)
    {
        acc |= 0xFFFFFF00;
    }
    plcvm_set_u32(--plcvm_sp, acc);
}
static void cb_ldi_u1(void){plcvm_set_u32(--plcvm_sp, plcvm_lpm_u8());}
static void cb_ldi_s2(void)
{
    int32_t acc = plcvm_lpm_u8();
    acc |= (int16_t)(plcvm_lpm_u8())<<8;
    if(acc & 0x8000)
    {
        acc |= (int32_t)0xFFFF0000;
    }
    plcvm_set_u32(--plcvm_sp, acc);
}
static void cb_ldi_u2(void)
{
    int32_t acc = (uint16_t)(plcvm_lpm_u8());
    acc	|=  (uint16_t)(plcvm_lpm_u8())<<8;
    plcvm_set_u32(--plcvm_sp, acc);
}
static void cb_ldi_s4(void){plcvm_set_u32(--plcvm_sp, plcvm_lpm_u32());}

// 0xA0 - 0xBF - Store in memory
// (type)*(SP++) = *(SP++);
static void cb_stm_b_s(void)
{
    uint32_t addr = plcvm_get_u32(plcvm_sp++);
    uint32_t acc  = plcvm_get_u32(plcvm_sp++);
    plcvm_set_bool(addr, (acc != 0) ? 1 : 0);
}
static void cb_stm_1_s(void)
{
    uint32_t addr = plcvm_get_u32(plcvm_sp++);
    uint32_t acc  = plcvm_get_u32(plcvm_sp++);
    plcvm_set_u8(addr, acc & 0x000000FF);
}
static void cb_stm_2_s(void)
{
    uint32_t addr = plcvm_get_u32(plcvm_sp++);
    uint32_t acc  = plcvm_get_u32(plcvm_sp++);
    plcvm_set_u16(addr, acc & 0x0000FFFF);
}
static void cb_stm_4_s(void)
{
    uint32_t addr = plcvm_get_u32(plcvm_sp++);
    uint32_t acc  = plcvm_get_u32(plcvm_sp++);
    plcvm_set_u32(addr, acc);
}
// (type)*(*(SP++) + *(uint8*)(PC++)) = *(SP++);
static void cb_stm_b_cs8(void)
{
    uint32_t addr = plcvm_get_u32(plcvm_sp++) + plcvm_lpm_u8();
    uint32_t acc  = plcvm_get_u32(plcvm_sp++);
    plcvm_set_bool(addr, (acc != 0) ? 1 : 0);
}
static void cb_stm_1_cs8(void)
{
    uint32_t addr = plcvm_get_u32(plcvm_sp++);
    uint32_t acc  = plcvm_get_u32(plcvm_sp++);
    plcvm_set_u8(addr, acc & 0x000000FF);
}
static void cb_stm_2_cs8(void)
{
    uint32_t addr = plcvm_get_u32(plcvm_sp++) + plcvm_lpm_u8();
    uint32_t acc  = plcvm_get_u32(plcvm_sp++);
    plcvm_set_u16(addr, acc & 0x0000FFFF);
}
static void cb_stm_4_cs8(void)
{
    uint32_t addr = plcvm_get_u32(plcvm_sp++) + plcvm_lpm_u8();
    uint32_t acc  = plcvm_get_u32(plcvm_sp++);
    plcvm_set_u32(addr, acc);
}
//(type)*(*(SP++) + *(uint16*)(PC+=2)) = *(SP++);
static void cb_stm_b_cs16(void)
{
    uint16_t offs = (uint16_t)plcvm_lpm_u8();
    	     offs |= (uint16_t)(plcvm_lpm_u8())<<8;
    uint32_t addr = plcvm_get_u32(plcvm_sp++) + offs;
    uint32_t acc  = plcvm_get_u32(plcvm_sp++);
    plcvm_set_bool(addr, (acc != 0) ? 1 : 0);
}
static void cb_stm_1_cs16(void)
{
    uint16_t offs = (uint16_t)plcvm_lpm_u8();
    	     offs |= (uint16_t)(plcvm_lpm_u8())<<8;
    uint32_t addr = plcvm_get_u32(plcvm_sp++) + offs;
    uint32_t acc  = plcvm_get_u32(plcvm_sp++);
    plcvm_set_u8(addr, acc & 0x000000FF);
}
static void cb_stm_2_cs16(void)
{
    uint16_t offs = (uint16_t)plcvm_lpm_u8();
    	     offs |= (uint16_t)(plcvm_lpm_u8())<<8;
    uint32_t addr = plcvm_get_u32(plcvm_sp++) + offs;
    uint32_t acc  = plcvm_get_u32(plcvm_sp++);
    plcvm_set_u16(addr, acc & 0x0000FFFF);
}
static void cb_stm_4_cs16(void)
{
    uint16_t offs = (uint16_t)plcvm_lpm_u8();
    	     offs |= (uint16_t)(plcvm_lpm_u8())<<8;
    uint32_t addr = plcvm_get_u32(plcvm_sp++) + offs;
    uint32_t acc  = plcvm_get_u32(plcvm_sp++);
    plcvm_set_u32(addr, acc);
}
//(type)*((uint16*)(PC+=2)) = *(SP++);
static void cb_stm_b_c16(void)
{
    uint16_t addr = plcvm_lpm_u8();
             addr |= (uint16_t)(plcvm_lpm_u8())<<8;
    uint32_t acc  = plcvm_get_u32(plcvm_sp++);
    plcvm_set_bool(addr, (acc != 0) ? 1 : 0);
}
static void cb_stm_1_c16(void)
{
    uint16_t addr = plcvm_lpm_u8();
             addr |= (uint16_t)(plcvm_lpm_u8())<<8;
    uint32_t acc  = plcvm_get_u32(plcvm_sp++);
    plcvm_set_u8(addr, acc & 0x000000FF);
}
static void cb_stm_2_c16(void)
{
    uint16_t addr = plcvm_lpm_u8();
             addr |= (uint16_t)(plcvm_lpm_u8())<<8;
    uint32_t acc  = plcvm_get_u32(plcvm_sp++);
    plcvm_set_u16(addr, acc & 0x0000FFFF);
}
static void cb_stm_4_c16(void)
{
    uint16_t addr = plcvm_lpm_u8();
             addr |= (uint16_t)(plcvm_lpm_u8())<<8;
    uint32_t acc  = plcvm_get_u32(plcvm_sp++);
    plcvm_set_u32(addr, acc);
}

// 0xE0 - 0xFF
static void cb_test_eq(void){if(plcvm_get_u32(plcvm_sp++) != plcvm_lpm_u32()) {plcvm_stat = PLC_ANSWER_ERROR_TEST;}}
static void cb_ret(void)
{
    if(plcvm_sfp == EXTPLC_SIZEOF_RAM)
    {
        plcvm_stat = PLC_ANSWER_OK;
    }
    else if((plcvm_sfp < (EXTPLC_SIZEOF_RAM - 2)) && (plcvm_sfp > 0))
    {
        plcvm_sp  = plcvm_sfp - 1;
        plcvm_sfp = plcvm_get_u32(plcvm_sp++);
        plcvm_pc  = plcvm_get_u32(plcvm_sp++);
    }
    else
    {
        plcvm_stat = PLC_ANSWER_ERROR_OFR_SFP;
    }
}

static const cbCOP_t plcmv_cb[] =
{
    // 0x00 - 0x1F
    cb_nop,     NULL,       cb_dup,     cb_drop,    cb_nip,     cb_swap,    cb_over,    cb_rot,
    // Arithmetic function
    cb_not,     cb_and,     cb_or,      cb_xor,     cb_shl,     cb_shr,     cb_srs,     cb_shift,
    cb_add,     cb_sub,     cb_mul,     cb_div,     cb_mod,     cb_inc,     cb_dec,     cb_neg,
    NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,
    // 0x20 - 0x3F 
    NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,
    NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,
    NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,
    NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,
    // 0x40 - 0x5F 
    NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,
    NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,
    NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,
    NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,
    // 0x60 - 0x7F
    NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,
    NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,
    NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,
    NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,
    // 0x80 - 0x9F
    cb_ldi_zero, cb_ldm_b1_s,    cb_ldm_s1_s,    cb_ldm_u1_s,    cb_ldm_s2_s,    cb_ldm_u2_s,    cb_ldm_s4_s,    cb_ldi_true,
    cb_ldi_s1,   cb_ldm_b1_cs8,  cb_ldm_s1_cs8,  cb_ldm_u1_cs8,  cb_ldm_s2_cs8,  cb_ldm_u2_cs8,  cb_ldm_s4_cs8,  cb_ldi_u1,
    cb_ldi_s2,   cb_ldm_b1_cs16, cb_ldm_s1_cs16, cb_ldm_u1_cs16, cb_ldm_s2_cs16, cb_ldm_u2_cs16, cb_ldm_s4_cs16, cb_ldi_u2,
    cb_ldi_s4,   cb_ldm_b1_c16,  cb_ldm_s1_c16,  cb_ldm_u1_c16,  cb_ldm_s2_c16,  cb_ldm_u2_c16,  cb_ldm_s4_c16,  cb_ldi_minus1,
    // 0xA0 - 0xBF
    cb_stm_b_s,    cb_stm_1_s,    cb_stm_2_s,    cb_stm_4_s,    NULL,       NULL,       NULL,       NULL,
    cb_stm_b_cs8,  cb_stm_1_cs8,  cb_stm_2_cs8,  cb_stm_4_cs8,  NULL,       NULL,       NULL,       NULL,
    cb_stm_b_cs16, cb_stm_1_cs16, cb_stm_2_cs16, cb_stm_4_cs16, NULL,       NULL,       NULL,       NULL,
    cb_stm_b_c16,  cb_stm_1_c16,  cb_stm_2_c16,  cb_stm_4_c16,  NULL,       NULL,       NULL,       NULL,
    // 0xC0 - 0xDF
    NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,
    NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,
    NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,
    NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,
    // 0xE0 - 0xFF
    NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,
    NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,
    NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,
    NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       cb_test_eq, cb_ret
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
    plcvm_stat = PLC_ANSWER_RUN;
    
    uint16_t plc_wd = 0xFFFF;

    while(plcvm_stat == PLC_ANSWER_RUN)
    {
        if(plc_wd > 0)
            plc_wd--;
        else
            return PLC_ANSWER_ERROR_WD;

        plcvm_cop = plcvm_lpm_u8();
        cbCOP_t cop_proc = plcmv_cb[plcvm_cop];
        if(cop_proc == NULL)
            return PLC_ANSWER_ERROR_UNK_COP;
        else
            cop_proc();
    }
    return plcvm_stat;
}

#endif  //  EXTPLC_USED
