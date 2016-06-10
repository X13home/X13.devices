/*
Copyright (c) 2011-2016 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _EXTPLC_VM_H_
#define _EXTPLC_VM_H_

typedef void (*cbCOP_t)(void);

// PLC_VM Section
static uint32_t         plcvm_ram[EXTPLC_SIZEOF_RAM];       // PLC data + merkers + stack
static ePLC_ANSWER_t    plcvm_stat = PLC_ANSWER_OK;
static uint32_t         plcvm_stack_bot = 0;

static uint32_t         plcvm_sp =  EXTPLC_SIZEOF_RAM;      // Stack pointer
static uint32_t         plcvm_sfp = EXTPLC_SIZEOF_RAM;      // Stack frame pointer
static uint32_t         plcvm_pc = 0;                       // program counter

static bool             plcvm_1st_start = true;

static uint8_t          plcvm_cache[EXTPLC_SIZEOF_PRG_CACHE];
static uint32_t         plcvm_page = 0xFFFFFFFF;

// return LPM8[pc++]
static uint8_t plcvm_lpm_u8(void){
    uint32_t act_page = plcvm_pc & (const uint32_t)(~(EXTPLC_SIZEOF_PRG_CACHE - 1UL));
    uint32_t offset = plcvm_pc - act_page;
    plcvm_pc++;

    if(act_page != plcvm_page)
    {
        if(plcvm_pc >= EXTPLC_SIZEOF_PRG)
        {
            plcvm_stat = PLC_ANSWER_ERROR_OFR_PC;
            return 0xFF;
        }

        plcvm_page = act_page;
        eeprom_read(plcvm_cache, eePLCprogram + act_page, EXTPLC_SIZEOF_PRG_CACHE);
    }

    return plcvm_cache[offset];
}

// return LPM16[pc+=2]
static uint16_t plcvm_lpm_u16(void){
    uint16_t retval;
    uint32_t act_page = plcvm_pc & (const uint32_t)(~(EXTPLC_SIZEOF_PRG_CACHE - 1UL));
    uint32_t offset = plcvm_pc - act_page;
    plcvm_pc += 2;

    if(act_page != plcvm_page)
    {
        if(plcvm_pc > EXTPLC_SIZEOF_PRG)
        {
            plcvm_stat = PLC_ANSWER_ERROR_OFR_PC;
            return 0xFFFF;
        }
        plcvm_page = act_page;
        eeprom_read(plcvm_cache, eePLCprogram + act_page, EXTPLC_SIZEOF_PRG_CACHE);
    }
    retval = plcvm_cache[offset];

    offset++;
    if(offset == EXTPLC_SIZEOF_PRG_CACHE)
    {
        if(plcvm_pc > EXTPLC_SIZEOF_PRG)
        {
            plcvm_stat = PLC_ANSWER_ERROR_OFR_PC;
            return 0xFFFF;
        }
        offset = 0;
        plcvm_page += EXTPLC_SIZEOF_PRG_CACHE;
        eeprom_read(plcvm_cache, eePLCprogram + plcvm_page, EXTPLC_SIZEOF_PRG_CACHE);
    }
    retval |= plcvm_cache[offset] << 8;
    return retval;
}

// return LPM32[pc+=4]
static uint32_t plcvm_lpm_u32(void){
    uint32_t retval;
    uint32_t act_page = plcvm_pc & (const uint32_t)(~(EXTPLC_SIZEOF_PRG_CACHE - 1UL));
    uint32_t offset = plcvm_pc - act_page;
    plcvm_pc += 4;

    if(act_page != plcvm_page)
    {
        if(plcvm_pc > EXTPLC_SIZEOF_PRG)
        {
            plcvm_stat = PLC_ANSWER_ERROR_OFR_PC;
            return 0xFFFFFFFF;
        }
        plcvm_page = act_page;
        eeprom_read(plcvm_cache, eePLCprogram + act_page, EXTPLC_SIZEOF_PRG_CACHE);
    }
    retval = plcvm_cache[offset];

    offset++;
    if(offset == EXTPLC_SIZEOF_PRG_CACHE)
    {
        if(plcvm_pc > EXTPLC_SIZEOF_PRG)
        {
            plcvm_stat = PLC_ANSWER_ERROR_OFR_PC;
            return 0xFFFFFFFF;
        }
        offset = 0;
        plcvm_page += EXTPLC_SIZEOF_PRG_CACHE;
        eeprom_read(plcvm_cache, eePLCprogram + plcvm_page, EXTPLC_SIZEOF_PRG_CACHE);
    }
    retval |= plcvm_cache[offset] << 8;

    offset++;
    if(offset == EXTPLC_SIZEOF_PRG_CACHE)
    {
        if(plcvm_pc > EXTPLC_SIZEOF_PRG)
        {
            plcvm_stat = PLC_ANSWER_ERROR_OFR_PC;
            return 0xFFFFFFFF;
        }
        offset = 0;
        plcvm_page += EXTPLC_SIZEOF_PRG_CACHE;
        eeprom_read(plcvm_cache, eePLCprogram + plcvm_page, EXTPLC_SIZEOF_PRG_CACHE);
    }
    retval |= (uint32_t)plcvm_cache[offset] << 16;

    offset++;
    if(offset == EXTPLC_SIZEOF_PRG_CACHE)
    {
        if(plcvm_pc > EXTPLC_SIZEOF_PRG)
        {
            plcvm_stat = PLC_ANSWER_ERROR_OFR_PC;
            return 0xFFFFFFFF;
        }
        offset = 0;
        plcvm_page += EXTPLC_SIZEOF_PRG_CACHE;
        eeprom_read(plcvm_cache, eePLCprogram + plcvm_page, EXTPLC_SIZEOF_PRG_CACHE);
    }
    retval |= (uint32_t)plcvm_cache[offset] << 24;

    return retval;
}

// return bool->uint32 RAM[addr'bool]
static uint32_t plcvm_get_bool(uint32_t addr){
    if(addr < (EXTPLC_SIZEOF_RAM * 32))
    {
        uint32_t acc = plcvm_ram[addr >> 5];
        return ((acc & (1UL<<(addr & 0x1F))) != 0) ? 1 : 0;
    }
    plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;
    return 0;
}

// return uint8->uint32 RAM[addr'uint8]
static uint32_t plcvm_get_u8(uint32_t addr){
    if(addr < (EXTPLC_SIZEOF_RAM * 4))
    {
        uint8_t *pDat = (uint8_t *)&plcvm_ram[addr >> 2];
        uint32_t acc = *(pDat + (addr & 0x03));
        return acc;
    }
    plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;
    return 0;
}

// return int8->uint32 RAM[addr'int8]
static uint32_t plcvm_get_s8(uint32_t addr){
    if(addr < (EXTPLC_SIZEOF_RAM * 4))
    {
        uint8_t *pDat = (uint8_t *)&plcvm_ram[addr >> 2];
        uint32_t acc = *(pDat + (addr & 0x03));
        if(acc & 0x80)
            acc |= 0xFFFFFF00;
        return acc;
    }
    plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;
    return 0;
}

// return uint16->uint32 RAM[addr'uint16]
static uint32_t plcvm_get_u16(uint32_t addr){
    if(addr < (EXTPLC_SIZEOF_RAM * 2))
    {
        uint16_t *pDat = (uint16_t *)&plcvm_ram[addr >> 1];
        uint32_t acc = *(pDat + (addr & 0x01));
        return acc;
    }
    plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;
    return 0;
}

// return int16->uint32 RAM[addr'int16]
static uint32_t plcvm_get_s16(uint32_t addr){
    if(addr < (EXTPLC_SIZEOF_RAM * 2))
    {
        uint16_t *pDat = (uint16_t *)&plcvm_ram[addr >> 1];
        uint32_t acc = *(pDat + (addr & 0x01));
        if(acc & 0x8000)
            acc |= 0xFFFF0000;
        return acc;
    }
    plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;
    return 0;
}

// return u32 RAM[addr'u32]
static uint32_t plcvm_get_u32(uint32_t addr){
    if(addr < EXTPLC_SIZEOF_RAM)
        return plcvm_ram[addr];
    plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;
    return 0;
}

// return u32 RAM[plcvm_sp++]
static uint32_t plcvm_pop(void){
    if(plcvm_sp < plcvm_sfp)
        return plcvm_ram[plcvm_sp++];
    plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;
    return 0;
}

static void plcvm_set_bool(uint32_t addr, bool sr){
    if(addr < (EXTPLC_SIZEOF_RAM * 32))
    {
        uint32_t mask = 1UL<<(addr & 0x1F);
        addr >>= 5;
        
        if(sr)
            plcvm_ram[addr] |= mask;
        else
            plcvm_ram[addr] &= ~mask;
    }
    else
        plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;
}

static void plcvm_set_u8(uint32_t addr, uint8_t val){
    if(addr < (EXTPLC_SIZEOF_RAM * 4))
    {
        uint8_t *pDat = (uint8_t *)&plcvm_ram[addr >> 2];
        *(pDat + (addr & 3)) = val;
    }
    else
        plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;
}

static void plcvm_set_u16(uint32_t addr, uint16_t val){
    if(addr < (EXTPLC_SIZEOF_RAM * 2))
    {
        uint16_t *pDat = (uint16_t *)&plcvm_ram[addr >> 1];
        pDat += (addr & 1);
        *pDat = val;
    }
    else
        plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;
}

static void plcvm_set_u32(uint32_t addr, uint32_t val){
    if(addr < EXTPLC_SIZEOF_RAM)
        plcvm_ram[addr] = val;
    else
        plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;
}

// (u32)RAM[--plcvm_sp] = val
static void plcvm_push(uint32_t val){
    if(plcvm_sp > plcvm_stack_bot)
        plcvm_ram[--plcvm_sp] = val;
    else
        plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;
}

// Operators callback

// 0x00
static void cb_nop(void){}
// 0x02 - 0x07 - Stack manipulation
static void cb_dup(void){if((plcvm_sp < plcvm_sfp) && (plcvm_sp > plcvm_stack_bot)){uint32_t tmp = plcvm_ram[plcvm_sp];plcvm_ram[--plcvm_sp] = tmp;}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}} // *(SP) = *(SP--);  (a -- a a)
static void cb_drop(void){if(plcvm_sp < plcvm_sfp){plcvm_sp++;}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_nip(void){if((plcvm_sp + 1) < plcvm_sfp){uint32_t tmp = plcvm_ram[plcvm_sp++];plcvm_ram[plcvm_sp] = tmp;}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}} // *(SP) = *(SP++)  //( a b -- b )
static void cb_swap(void){if((plcvm_sp + 1) < plcvm_sfp){uint32_t tmp = plcvm_ram[plcvm_sp];plcvm_ram[plcvm_sp] = plcvm_ram[plcvm_sp + 1];plcvm_ram[plcvm_sp + 1] = tmp;}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}} // tmp = *(SP); *(SP) = *(SP+1); *(SP+1) = tmp;  // (a b -- b a)
static void cb_over(void){uint32_t c1 = plcvm_pop(); uint32_t c2 = plcvm_pop(); plcvm_push(c2); plcvm_push(c1); plcvm_push(c2);}    // *(--SP) = *(SP+1);  //( a b -- a b a )
static void cb_rot(void){uint32_t c1 = plcvm_pop(); uint32_t c2 = plcvm_pop(); uint32_t c3 = plcvm_pop(); plcvm_push(c2); plcvm_push(c1); plcvm_push(c3); }    // ( a b c -- b c a )
// 0x08 - 0x0F - Bitwise and Bit Shift Operators
static void cb_not(void){if(plcvm_sp < plcvm_sfp){plcvm_ram[plcvm_sp] = ~plcvm_ram[plcvm_sp];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_and(void){if((plcvm_sp + 1) < plcvm_sfp){uint32_t v1 = plcvm_ram[plcvm_sp++];uint32_t v2 = plcvm_ram[plcvm_sp];plcvm_ram[plcvm_sp] = v1 & v2;}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_or (void){if((plcvm_sp + 1) < plcvm_sfp){uint32_t v1 = plcvm_ram[plcvm_sp++];uint32_t v2 = plcvm_ram[plcvm_sp];plcvm_ram[plcvm_sp] = v1 | v2;}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_xor(void){if((plcvm_sp + 1) < plcvm_sfp){uint32_t v1 = plcvm_ram[plcvm_sp++];uint32_t v2 = plcvm_ram[plcvm_sp];plcvm_ram[plcvm_sp] = v1 ^ v2;}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_lsl(void){if(plcvm_sp < plcvm_sfp){uint32_t v1 = plcvm_ram[plcvm_sp];v1 <<= plcvm_lpm_u8();plcvm_ram[plcvm_sp] = v1;}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_lsr(void){if(plcvm_sp < plcvm_sfp){uint32_t v1 = plcvm_ram[plcvm_sp];v1 >>= plcvm_lpm_u8();plcvm_ram[plcvm_sp] = v1;}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_asr(void){if(plcvm_sp < plcvm_sfp){int32_t  v1 = plcvm_ram[plcvm_sp];v1 >>= plcvm_lpm_u8();plcvm_ram[plcvm_sp] = v1;}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
// 0x10 - 0x17 - Arithmetic Operators
static void cb_add(void){if((plcvm_sp + 1) < plcvm_sfp){int32_t v1 = plcvm_ram[plcvm_sp++];int32_t v2 = plcvm_ram[plcvm_sp];plcvm_ram[plcvm_sp] = v2 + v1;}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_sub(void){if((plcvm_sp + 1) < plcvm_sfp){int32_t v1 = plcvm_ram[plcvm_sp++];int32_t v2 = plcvm_ram[plcvm_sp];plcvm_ram[plcvm_sp] = v2 - v1;}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_mul(void){if((plcvm_sp + 1) < plcvm_sfp){int32_t v1 = plcvm_ram[plcvm_sp++];int32_t v2 = plcvm_ram[plcvm_sp];plcvm_ram[plcvm_sp] = v2 * v1;}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_div(void){if((plcvm_sp + 1) < plcvm_sfp){int32_t v1 = plcvm_ram[plcvm_sp++];if(v1 != 0){int32_t v2 = plcvm_ram[plcvm_sp];plcvm_ram[plcvm_sp] = v2 / v1;}else{plcvm_stat = PLC_ANSWER_ERROR_DIV0;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_mod(void){if((plcvm_sp + 1) < plcvm_sfp){int32_t v1 = plcvm_ram[plcvm_sp++];int32_t v2 = plcvm_ram[plcvm_sp];plcvm_ram[plcvm_sp] = v2 % v1;}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_inc(void){if(plcvm_sp < plcvm_sfp){plcvm_ram[plcvm_sp]++;}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_dec(void){if(plcvm_sp < plcvm_sfp){plcvm_ram[plcvm_sp]--;}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_neg(void){if(plcvm_sp < plcvm_sfp){plcvm_ram[plcvm_sp] = -plcvm_ram[plcvm_sp];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
// 0x18 - 0x1F - Reserved
// 0x20 - 0x27 - Compare
static void cb_ceq(void){if((plcvm_sp + 1) < plcvm_sfp){int32_t v1 = plcvm_ram[plcvm_sp++];int32_t v2 = plcvm_ram[plcvm_sp];plcvm_ram[plcvm_sp] = (v2 == v1);}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_cne(void){if((plcvm_sp + 1) < plcvm_sfp){int32_t v1 = plcvm_ram[plcvm_sp++];int32_t v2 = plcvm_ram[plcvm_sp];plcvm_ram[plcvm_sp] = (v2 != v1);}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_cgt(void){if((plcvm_sp + 1) < plcvm_sfp){int32_t v1 = plcvm_ram[plcvm_sp++];int32_t v2 = plcvm_ram[plcvm_sp];plcvm_ram[plcvm_sp] = (v2 >  v1);}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_cge(void){if((plcvm_sp + 1) < plcvm_sfp){int32_t v1 = plcvm_ram[plcvm_sp++];int32_t v2 = plcvm_ram[plcvm_sp];plcvm_ram[plcvm_sp] = (v2 >= v1);}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_clt(void){if((plcvm_sp + 1) < plcvm_sfp){int32_t v1 = plcvm_ram[plcvm_sp++];int32_t v2 = plcvm_ram[plcvm_sp];plcvm_ram[plcvm_sp] = (v2 <  v1);}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_cle(void){if((plcvm_sp + 1) < plcvm_sfp){int32_t v1 = plcvm_ram[plcvm_sp++];int32_t v2 = plcvm_ram[plcvm_sp];plcvm_ram[plcvm_sp] = (v2 <= v1);}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_cze(void){if(plcvm_sp < plcvm_sfp){plcvm_ram[plcvm_sp] = (plcvm_ram[plcvm_sp] == 0);}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
// 0x28 - 0x2F - Reserverd
// 0x30 - 0x37 - Reserved 
// 0x38 - 0x3F - Load Constant
static void cb_ldi_0(void) {if(plcvm_sp > plcvm_stack_bot){plcvm_ram[--plcvm_sp] = 0;}                             else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ldi_s1(void){if(plcvm_sp > plcvm_stack_bot){plcvm_ram[--plcvm_sp] = (plcvm_lpm_u8() | 0xFFFFFF00);} else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ldi_s2(void){if(plcvm_sp > plcvm_stack_bot){plcvm_ram[--plcvm_sp] = (plcvm_lpm_u16() | 0xFFFF0000);}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ldi_s4(void){if(plcvm_sp > plcvm_stack_bot){plcvm_ram[--plcvm_sp] = plcvm_lpm_u32();}               else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ldi_u1(void){if(plcvm_sp > plcvm_stack_bot){plcvm_ram[--plcvm_sp] = plcvm_lpm_u8();}                else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ldi_u2(void){if(plcvm_sp > plcvm_stack_bot){plcvm_ram[--plcvm_sp] = plcvm_lpm_u16();}               else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ldi_1(void) {if(plcvm_sp > plcvm_stack_bot){plcvm_ram[--plcvm_sp] = 1UL;}                           else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ldi_m1(void){if(plcvm_sp > plcvm_stack_bot){plcvm_ram[--plcvm_sp] = ~0UL;}                          else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ldi_min(void){if(plcvm_sp > plcvm_stack_bot){plcvm_ram[--plcvm_sp] = 0x80000000UL;}                 else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
// 0x40 - 0x5F - Load with SFP based offset
static void cb_ld_p0(void){if(plcvm_sp > plcvm_stack_bot){uint32_t addr = plcvm_sfp + 0x02;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[--plcvm_sp] = plcvm_ram[addr];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ld_p1(void){if(plcvm_sp > plcvm_stack_bot){uint32_t addr = plcvm_sfp + 0x03;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[--plcvm_sp] = plcvm_ram[addr];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ld_p2(void){if(plcvm_sp > plcvm_stack_bot){uint32_t addr = plcvm_sfp + 0x04;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[--plcvm_sp] = plcvm_ram[addr];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ld_p3(void){if(plcvm_sp > plcvm_stack_bot){uint32_t addr = plcvm_sfp + 0x05;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[--plcvm_sp] = plcvm_ram[addr];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ld_p4(void){if(plcvm_sp > plcvm_stack_bot){uint32_t addr = plcvm_sfp + 0x06;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[--plcvm_sp] = plcvm_ram[addr];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ld_p5(void){if(plcvm_sp > plcvm_stack_bot){uint32_t addr = plcvm_sfp + 0x07;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[--plcvm_sp] = plcvm_ram[addr];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ld_p6(void){if(plcvm_sp > plcvm_stack_bot){uint32_t addr = plcvm_sfp + 0x08;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[--plcvm_sp] = plcvm_ram[addr];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ld_p7(void){if(plcvm_sp > plcvm_stack_bot){uint32_t addr = plcvm_sfp + 0x09;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[--plcvm_sp] = plcvm_ram[addr];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ld_p8(void){if(plcvm_sp > plcvm_stack_bot){uint32_t addr = plcvm_sfp + 0x0A;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[--plcvm_sp] = plcvm_ram[addr];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ld_p9(void){if(plcvm_sp > plcvm_stack_bot){uint32_t addr = plcvm_sfp + 0x0B;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[--plcvm_sp] = plcvm_ram[addr];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ld_pA(void){if(plcvm_sp > plcvm_stack_bot){uint32_t addr = plcvm_sfp + 0x0C;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[--plcvm_sp] = plcvm_ram[addr];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ld_pB(void){if(plcvm_sp > plcvm_stack_bot){uint32_t addr = plcvm_sfp + 0x0D;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[--plcvm_sp] = plcvm_ram[addr];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ld_pC(void){if(plcvm_sp > plcvm_stack_bot){uint32_t addr = plcvm_sfp + 0x0E;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[--plcvm_sp] = plcvm_ram[addr];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ld_pD(void){if(plcvm_sp > plcvm_stack_bot){uint32_t addr = plcvm_sfp + 0x0F;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[--plcvm_sp] = plcvm_ram[addr];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ld_pE(void){if(plcvm_sp > plcvm_stack_bot){uint32_t addr = plcvm_sfp + 0x10;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[--plcvm_sp] = plcvm_ram[addr];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ld_pF(void){if(plcvm_sp > plcvm_stack_bot){uint32_t addr = plcvm_sfp + 0x11;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[--plcvm_sp] = plcvm_ram[addr];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ld_l0(void){if(plcvm_sp > plcvm_stack_bot){uint32_t addr = plcvm_sfp - 0x01;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[--plcvm_sp] = plcvm_ram[addr];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ld_l1(void){if(plcvm_sp > plcvm_stack_bot){uint32_t addr = plcvm_sfp - 0x02;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[--plcvm_sp] = plcvm_ram[addr];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ld_l2(void){if(plcvm_sp > plcvm_stack_bot){uint32_t addr = plcvm_sfp - 0x03;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[--plcvm_sp] = plcvm_ram[addr];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ld_l3(void){if(plcvm_sp > plcvm_stack_bot){uint32_t addr = plcvm_sfp - 0x04;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[--plcvm_sp] = plcvm_ram[addr];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ld_l4(void){if(plcvm_sp > plcvm_stack_bot){uint32_t addr = plcvm_sfp - 0x05;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[--plcvm_sp] = plcvm_ram[addr];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ld_l5(void){if(plcvm_sp > plcvm_stack_bot){uint32_t addr = plcvm_sfp - 0x06;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[--plcvm_sp] = plcvm_ram[addr];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ld_l6(void){if(plcvm_sp > plcvm_stack_bot){uint32_t addr = plcvm_sfp - 0x07;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[--plcvm_sp] = plcvm_ram[addr];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ld_l7(void){if(plcvm_sp > plcvm_stack_bot){uint32_t addr = plcvm_sfp - 0x08;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[--plcvm_sp] = plcvm_ram[addr];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ld_l8(void){if(plcvm_sp > plcvm_stack_bot){uint32_t addr = plcvm_sfp - 0x09;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[--plcvm_sp] = plcvm_ram[addr];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ld_l9(void){if(plcvm_sp > plcvm_stack_bot){uint32_t addr = plcvm_sfp - 0x0A;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[--plcvm_sp] = plcvm_ram[addr];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ld_lA(void){if(plcvm_sp > plcvm_stack_bot){uint32_t addr = plcvm_sfp - 0x0B;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[--plcvm_sp] = plcvm_ram[addr];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ld_lB(void){if(plcvm_sp > plcvm_stack_bot){uint32_t addr = plcvm_sfp - 0x0C;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[--plcvm_sp] = plcvm_ram[addr];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ld_lC(void){if(plcvm_sp > plcvm_stack_bot){uint32_t addr = plcvm_sfp - 0x0D;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[--plcvm_sp] = plcvm_ram[addr];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ld_lD(void){if(plcvm_sp > plcvm_stack_bot){uint32_t addr = plcvm_sfp - 0x0E;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[--plcvm_sp] = plcvm_ram[addr];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ld_lE(void){if(plcvm_sp > plcvm_stack_bot){uint32_t addr = plcvm_sfp - 0x0F;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[--plcvm_sp] = plcvm_ram[addr];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ld_lF(void){if(plcvm_sp > plcvm_stack_bot){uint32_t addr = plcvm_sfp - 0x10;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[--plcvm_sp] = plcvm_ram[addr];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
// 0x60 - 0x7F - Store with SFP based offset
static void cb_st_p0(void){if(plcvm_sp < plcvm_sfp){uint32_t addr = plcvm_sfp + 0x02;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[addr] = plcvm_ram[plcvm_sp++];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_st_p1(void){if(plcvm_sp < plcvm_sfp){uint32_t addr = plcvm_sfp + 0x03;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[addr] = plcvm_ram[plcvm_sp++];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_st_p2(void){if(plcvm_sp < plcvm_sfp){uint32_t addr = plcvm_sfp + 0x04;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[addr] = plcvm_ram[plcvm_sp++];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_st_p3(void){if(plcvm_sp < plcvm_sfp){uint32_t addr = plcvm_sfp + 0x05;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[addr] = plcvm_ram[plcvm_sp++];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_st_p4(void){if(plcvm_sp < plcvm_sfp){uint32_t addr = plcvm_sfp + 0x06;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[addr] = plcvm_ram[plcvm_sp++];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_st_p5(void){if(plcvm_sp < plcvm_sfp){uint32_t addr = plcvm_sfp + 0x07;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[addr] = plcvm_ram[plcvm_sp++];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_st_p6(void){if(plcvm_sp < plcvm_sfp){uint32_t addr = plcvm_sfp + 0x08;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[addr] = plcvm_ram[plcvm_sp++];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_st_p7(void){if(plcvm_sp < plcvm_sfp){uint32_t addr = plcvm_sfp + 0x09;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[addr] = plcvm_ram[plcvm_sp++];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_st_p8(void){if(plcvm_sp < plcvm_sfp){uint32_t addr = plcvm_sfp + 0x0A;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[addr] = plcvm_ram[plcvm_sp++];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_st_p9(void){if(plcvm_sp < plcvm_sfp){uint32_t addr = plcvm_sfp + 0x0B;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[addr] = plcvm_ram[plcvm_sp++];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_st_pA(void){if(plcvm_sp < plcvm_sfp){uint32_t addr = plcvm_sfp + 0x0C;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[addr] = plcvm_ram[plcvm_sp++];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_st_pB(void){if(plcvm_sp < plcvm_sfp){uint32_t addr = plcvm_sfp + 0x0D;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[addr] = plcvm_ram[plcvm_sp++];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_st_pC(void){if(plcvm_sp < plcvm_sfp){uint32_t addr = plcvm_sfp + 0x0E;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[addr] = plcvm_ram[plcvm_sp++];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_st_pD(void){if(plcvm_sp < plcvm_sfp){uint32_t addr = plcvm_sfp + 0x0F;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[addr] = plcvm_ram[plcvm_sp++];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_st_pE(void){if(plcvm_sp < plcvm_sfp){uint32_t addr = plcvm_sfp + 0x10;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[addr] = plcvm_ram[plcvm_sp++];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_st_pF(void){if(plcvm_sp < plcvm_sfp){uint32_t addr = plcvm_sfp + 0x11;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[addr] = plcvm_ram[plcvm_sp++];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_st_l0(void){if(plcvm_sp < plcvm_sfp){uint32_t addr = plcvm_sfp - 0x01;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[addr] = plcvm_ram[plcvm_sp++];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_st_l1(void){if(plcvm_sp < plcvm_sfp){uint32_t addr = plcvm_sfp - 0x02;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[addr] = plcvm_ram[plcvm_sp++];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_st_l2(void){if(plcvm_sp < plcvm_sfp){uint32_t addr = plcvm_sfp - 0x03;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[addr] = plcvm_ram[plcvm_sp++];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_st_l3(void){if(plcvm_sp < plcvm_sfp){uint32_t addr = plcvm_sfp - 0x04;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[addr] = plcvm_ram[plcvm_sp++];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_st_l4(void){if(plcvm_sp < plcvm_sfp){uint32_t addr = plcvm_sfp - 0x05;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[addr] = plcvm_ram[plcvm_sp++];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_st_l5(void){if(plcvm_sp < plcvm_sfp){uint32_t addr = plcvm_sfp - 0x06;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[addr] = plcvm_ram[plcvm_sp++];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_st_l6(void){if(plcvm_sp < plcvm_sfp){uint32_t addr = plcvm_sfp - 0x07;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[addr] = plcvm_ram[plcvm_sp++];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_st_l7(void){if(plcvm_sp < plcvm_sfp){uint32_t addr = plcvm_sfp - 0x08;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[addr] = plcvm_ram[plcvm_sp++];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_st_l8(void){if(plcvm_sp < plcvm_sfp){uint32_t addr = plcvm_sfp - 0x09;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[addr] = plcvm_ram[plcvm_sp++];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_st_l9(void){if(plcvm_sp < plcvm_sfp){uint32_t addr = plcvm_sfp - 0x0A;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[addr] = plcvm_ram[plcvm_sp++];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_st_lA(void){if(plcvm_sp < plcvm_sfp){uint32_t addr = plcvm_sfp - 0x0B;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[addr] = plcvm_ram[plcvm_sp++];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_st_lB(void){if(plcvm_sp < plcvm_sfp){uint32_t addr = plcvm_sfp - 0x0C;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[addr] = plcvm_ram[plcvm_sp++];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_st_lC(void){if(plcvm_sp < plcvm_sfp){uint32_t addr = plcvm_sfp - 0x0D;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[addr] = plcvm_ram[plcvm_sp++];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_st_lD(void){if(plcvm_sp < plcvm_sfp){uint32_t addr = plcvm_sfp - 0x0E;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[addr] = plcvm_ram[plcvm_sp++];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_st_lE(void){if(plcvm_sp < plcvm_sfp){uint32_t addr = plcvm_sfp - 0x0F;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[addr] = plcvm_ram[plcvm_sp++];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_st_lF(void){if(plcvm_sp < plcvm_sfp){uint32_t addr = plcvm_sfp - 0x10;if(addr < EXTPLC_SIZEOF_RAM){plcvm_ram[addr] = plcvm_ram[plcvm_sp++];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_RAM;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
// 0x80 - 0x9F - Load from memory
// *(SP) = (type)*(*(SP));
static void cb_ldm_b1_s(void){plcvm_push(plcvm_get_bool(plcvm_pop()));}
static void cb_ldm_s1_s(void){plcvm_push(plcvm_get_s8(  plcvm_pop()));}
static void cb_ldm_u1_s(void){plcvm_push(plcvm_get_u8(  plcvm_pop()));}
static void cb_ldm_s2_s(void){plcvm_push(plcvm_get_s16( plcvm_pop()));}
static void cb_ldm_u2_s(void){plcvm_push(plcvm_get_u16( plcvm_pop()));}
static void cb_ldm_s4_s(void){plcvm_push(plcvm_get_u32( plcvm_pop()));}
//*(SP) = (type)*(*(SP)+*(uint8*)(PC++));
static void cb_ldm_b1_cs8(void){plcvm_push(plcvm_get_bool(plcvm_pop()*32  + plcvm_lpm_u8()));}
static void cb_ldm_s1_cs8(void){plcvm_push(plcvm_get_s8(  plcvm_pop()*4   + plcvm_lpm_u8()));}
static void cb_ldm_u1_cs8(void){plcvm_push(plcvm_get_u8(  plcvm_pop()*4   + plcvm_lpm_u8()));}
static void cb_ldm_s2_cs8(void){plcvm_push(plcvm_get_s16( plcvm_pop()*2   + plcvm_lpm_u8()));}
static void cb_ldm_u2_cs8(void){plcvm_push(plcvm_get_u16( plcvm_pop()*2   + plcvm_lpm_u8()));}
static void cb_ldm_s4_cs8(void){plcvm_push(plcvm_get_u32( plcvm_pop()     + plcvm_lpm_u8()));}
// *(SP) = (type)*(*(SP)+*(uint16*)(PC+=2));
static void cb_ldm_b1_cs16(void){plcvm_push(plcvm_get_bool(plcvm_pop()*32 + plcvm_lpm_u16()));}
static void cb_ldm_s1_cs16(void){plcvm_push(plcvm_get_s8(  plcvm_pop()*4  + plcvm_lpm_u16()));}
static void cb_ldm_u1_cs16(void){plcvm_push(plcvm_get_u8(  plcvm_pop()*4  + plcvm_lpm_u16()));}
static void cb_ldm_s2_cs16(void){plcvm_push(plcvm_get_s16( plcvm_pop()*2  + plcvm_lpm_u16()));}
static void cb_ldm_u2_cs16(void){plcvm_push(plcvm_get_u16( plcvm_pop()*2  + plcvm_lpm_u16()));}
static void cb_ldm_s4_cs16(void){plcvm_push(plcvm_get_u32( plcvm_pop()    + plcvm_lpm_u16()));}
// *(--SP) = (type)*(*(uint16*)(PC+=2)); // bool LPM(addr)
static void cb_ldm_b1_c16(void){plcvm_push(plcvm_get_bool(plcvm_lpm_u16()));}
static void cb_ldm_s1_c16(void){plcvm_push(plcvm_get_s8(  plcvm_lpm_u16()));}
static void cb_ldm_u1_c16(void){plcvm_push(plcvm_get_u8(  plcvm_lpm_u16()));}
static void cb_ldm_s2_c16(void){plcvm_push(plcvm_get_s16( plcvm_lpm_u16()));}
static void cb_ldm_u2_c16(void){plcvm_push(plcvm_get_u16( plcvm_lpm_u16()));}
static void cb_ldm_s4_c16(void){plcvm_push(plcvm_get_u32( plcvm_lpm_u16()));}
// 0xA0 - 0xBF - Store in memory
// (type)*(SP++) = *(SP++);
static void cb_stm_b1_s(void){uint32_t addr = plcvm_pop();uint32_t acc  = plcvm_pop();plcvm_set_bool(addr, (acc != 0) ? 1 : 0);}
static void cb_stm_s1_s(void){uint32_t addr = plcvm_pop();uint32_t acc  = plcvm_pop();plcvm_set_u8(addr, acc & 0x000000FF);}
static void cb_stm_s2_s(void){uint32_t addr = plcvm_pop();uint32_t acc  = plcvm_pop();plcvm_set_u16(addr, acc & 0x0000FFFF);}
static void cb_stm_s4_s(void){uint32_t addr = plcvm_pop();uint32_t acc  = plcvm_pop();plcvm_set_u32(addr, acc);}
// (type)*(*(SP++) + *(uint8*)(PC++)) = *(SP++);
static void cb_stm_b1_cs8(void){uint32_t addr = plcvm_pop()*32  + plcvm_lpm_u8();uint32_t acc  = plcvm_pop();plcvm_set_bool(addr, (acc != 0) ? 1 : 0);}
static void cb_stm_s1_cs8(void){uint32_t addr = plcvm_pop()*4   + plcvm_lpm_u8();uint32_t acc  = plcvm_pop();plcvm_set_u8(addr, acc & 0x000000FF);}
static void cb_stm_s2_cs8(void){uint32_t addr = plcvm_pop()*2   + plcvm_lpm_u8();uint32_t acc  = plcvm_pop();plcvm_set_u16(addr, acc & 0x0000FFFF);}
static void cb_stm_s4_cs8(void){uint32_t addr = plcvm_pop()     + plcvm_lpm_u8();uint32_t acc  = plcvm_pop();plcvm_set_u32(addr, acc);}
//(type)*(*(SP++) + *(uint16*)(PC+=2)) = *(SP++);
static void cb_stm_b1_cs16(void){uint32_t addr = plcvm_pop()*32 + plcvm_lpm_u16();uint32_t acc  = plcvm_pop();plcvm_set_bool(addr, (acc != 0) ? 1 : 0);}
static void cb_stm_s1_cs16(void){uint32_t addr = plcvm_pop()*4  + plcvm_lpm_u16();uint32_t acc  = plcvm_pop();plcvm_set_u8(addr, acc & 0x000000FF);}
static void cb_stm_s2_cs16(void){uint32_t addr = plcvm_pop()*2  + plcvm_lpm_u16();uint32_t acc  = plcvm_pop();plcvm_set_u16(addr, acc & 0x0000FFFF);}
static void cb_stm_s4_cs16(void){uint32_t addr = plcvm_pop()    + plcvm_lpm_u16();uint32_t acc  = plcvm_pop();plcvm_set_u32(addr, acc);}
//(type)*((uint16*)(PC+=2)) = *(SP++);
static void cb_stm_b1_c16(void){uint16_t addr = plcvm_lpm_u16();uint32_t acc  = plcvm_pop();plcvm_set_bool(addr, (acc != 0) ? 1 : 0);}
static void cb_stm_s1_c16(void){uint16_t addr = plcvm_lpm_u16();uint32_t acc  = plcvm_pop();plcvm_set_u8(addr, acc & 0x000000FF);}
static void cb_stm_s2_c16(void){uint16_t addr = plcvm_lpm_u16();uint32_t acc  = plcvm_pop();plcvm_set_u16(addr, acc & 0x0000FFFF);}
static void cb_stm_s4_c16(void){uint16_t addr = plcvm_lpm_u16();uint32_t acc  = plcvm_pop();plcvm_set_u32(addr, acc);}
// 0xC0 - 0xEF
static void cb_in(void){if(plcvm_sp > plcvm_stack_bot){uint32_t sidx = plcvm_lpm_u32();plcvm_ram[--plcvm_sp] = ext_in((subidx_t *)&sidx);}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
//subidx_t IO; IO.Base  = plcvm_lpm_u16(); IO.Type  = plcvm_lpm_u8(); IO.Place = plcvm_lpm_u8(); plcvm_push(ext_in(&IO));}
static void cb_out(void){if(plcvm_sp < plcvm_sfp){uint32_t sidx = plcvm_lpm_u32();ext_out((subidx_t *)&sidx, plcvm_ram[plcvm_sp++]);}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
//subidx_t IO; IO.Base  = plcvm_lpm_u16(); IO.Type  = plcvm_lpm_u8(); IO.Place = plcvm_lpm_u8(); ext_out(&IO, plcvm_pop());}
static void cb_api(void);
// 0xF0 - 0xFF
static void cb_sjmp(void){if(plcvm_sp < plcvm_sfp){plcvm_pc = plcvm_ram[plcvm_sp++];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_jz(void){if(plcvm_sp < plcvm_sfp){if(plcvm_ram[plcvm_sp++] == 0){plcvm_pc = plcvm_lpm_u16();}else{plcvm_pc += 2;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_jnz(void){if(plcvm_sp < plcvm_sfp){if(plcvm_ram[plcvm_sp++] != 0){plcvm_pc = plcvm_lpm_u16();}else{plcvm_pc += 2;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_jmp(void){plcvm_pc = plcvm_lpm_u16();}
static void cb_scall(void){
    if((plcvm_sp < plcvm_sfp) && (plcvm_sp > plcvm_stack_bot))
    {
        uint32_t tmp = plcvm_ram[plcvm_sp];     // tmp = *(SP++)
        plcvm_ram[plcvm_sp] = plcvm_pc;         // *(--SP) = PC
        plcvm_ram[--plcvm_sp] = plcvm_sfp;      // *(--SP) = SFP
        plcvm_sfp = plcvm_sp;                   // SFP = SP
        plcvm_pc = tmp;                         // PC = tmp
    }
    else
    {
        plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;
    }
}
static void cb_call(void){
    if((plcvm_sp - 1) > plcvm_stack_bot)
    {
        uint32_t tmp = plcvm_lpm_u16();
        plcvm_ram[--plcvm_sp] = plcvm_pc;       // *(--SP) = PC
        plcvm_ram[--plcvm_sp] = plcvm_sfp;      // *(--SP) = SFP
        plcvm_sfp = plcvm_sp;                   // SFP = SP
        plcvm_pc = tmp;                         // PC = *(uint16_t *)(PC)  //LPM
    }
    else
    {
        plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;
    }
}
static void cb_test_eq(void){if(plcvm_sp < plcvm_sfp){if(plcvm_ram[plcvm_sp] != plcvm_lpm_u32()){plcvm_stat = PLC_ANSWER_ERROR_TEST;}else{plcvm_sp++;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_ret(void){
    if(plcvm_sfp < (const uint32_t)(EXTPLC_SIZEOF_RAM - 1))
    {
        plcvm_sp  = plcvm_sfp;
        plcvm_sfp = plcvm_ram[plcvm_sp++];
        plcvm_pc  = plcvm_ram[plcvm_sp++];
    }
    else if(plcvm_sfp == (const uint32_t)EXTPLC_SIZEOF_RAM) // Last RET
    {
        plcvm_stat = PLC_ANSWER_OK;
    }
    else
    {
        plcvm_stat = PLC_ANSWER_ERROR_OFR_SFP;
    }
}

// End API functions
static void cb_unk(void){plcvm_stat = PLC_ANSWER_ERROR_UNK_COP;}        // Unknown COP

// Operators table
static const cbCOP_t plcvm_cb[] = {
    // 0x00 - 0x1F
    //              //              //              //              //              //              //              //
    cb_nop,         cb_unk,         cb_dup,         cb_drop,        cb_nip,         cb_swap,        cb_over,        cb_rot,
    cb_not,         cb_and,         cb_or,          cb_xor,         cb_lsl,         cb_lsr,         cb_asr,         cb_unk,
    cb_add,         cb_sub,         cb_mul,         cb_div,         cb_mod,         cb_inc,         cb_dec,         cb_neg,
    cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,
    // 0x20 - 0x3F 
    cb_ceq,         cb_cne,         cb_cgt,         cb_cge,         cb_clt,         cb_cle,         cb_cze,         cb_unk,
    cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,
    cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,
    cb_ldi_0,       cb_ldi_s1,      cb_ldi_s2,      cb_ldi_s4,      cb_ldi_u1,      cb_ldi_u2,      cb_ldi_1,       cb_ldi_m1,
    // 0x40 - 0x5F 
    cb_ld_p0,       cb_ld_p1,       cb_ld_p2,       cb_ld_p3,       cb_ld_p4,       cb_ld_p5,       cb_ld_p6,       cb_ld_p7,
    cb_ld_p8,       cb_ld_p9,       cb_ld_pA,       cb_ld_pB,       cb_ld_pC,       cb_ld_pD,       cb_ld_pE,       cb_ld_pF,
    cb_ld_l0,       cb_ld_l1,       cb_ld_l2,       cb_ld_l3,       cb_ld_l4,       cb_ld_l5,       cb_ld_l6,       cb_ld_l7,
    cb_ld_l8,       cb_ld_l9,       cb_ld_lA,       cb_ld_lB,       cb_ld_lC,       cb_ld_lD,       cb_ld_lE,       cb_ld_lF,
    // 0x60 - 0x7F
    cb_st_p0,       cb_st_p1,       cb_st_p2,       cb_st_p3,       cb_st_p4,       cb_st_p5,       cb_st_p6,       cb_st_p7,
    cb_st_p8,       cb_st_p9,       cb_st_pA,       cb_st_pB,       cb_st_pC,       cb_st_pD,       cb_st_pE,       cb_st_pF,
    cb_st_l0,       cb_st_l1,       cb_st_l2,       cb_st_l3,       cb_st_l4,       cb_st_l5,       cb_st_l6,       cb_st_l7,
    cb_st_l8,       cb_st_l9,       cb_st_lA,       cb_st_lB,       cb_st_lC,       cb_st_lD,       cb_st_lE,       cb_st_lF,
    // 0x80 - 0x9F
    cb_ldm_b1_s,    cb_ldm_s1_s,    cb_ldm_s2_s,    cb_ldm_s4_s,    cb_ldm_u1_s,    cb_ldm_u2_s,    cb_unk,         cb_ldi_min,
    cb_ldm_b1_cs8,  cb_ldm_s1_cs8,  cb_ldm_s2_cs8,  cb_ldm_s4_cs8,  cb_ldm_u1_cs8,  cb_ldm_u2_cs8,  cb_unk,         cb_unk,
    cb_ldm_b1_cs16, cb_ldm_s1_cs16, cb_ldm_s2_cs16, cb_ldm_s4_cs16, cb_ldm_u1_cs16, cb_ldm_u2_cs16, cb_unk,         cb_unk,
    cb_ldm_b1_c16,  cb_ldm_s1_c16,  cb_ldm_s2_c16,  cb_ldm_s4_c16,  cb_ldm_u1_c16,  cb_ldm_u2_c16,  cb_unk,         cb_unk,
    // 0xA0 - 0xBF
    cb_stm_b1_s,    cb_stm_s1_s,    cb_stm_s2_s,    cb_stm_s4_s,    cb_unk,         cb_unk,         cb_unk,         cb_unk,
    cb_stm_b1_cs8,  cb_stm_s1_cs8,  cb_stm_s2_cs8,  cb_stm_s4_cs8,  cb_unk,         cb_unk,         cb_unk,         cb_unk,
    cb_stm_b1_cs16, cb_stm_s1_cs16, cb_stm_s2_cs16, cb_stm_s4_cs16, cb_unk,         cb_unk,         cb_unk,         cb_unk,
    cb_stm_b1_c16,  cb_stm_s1_c16,  cb_stm_s2_c16,  cb_stm_s4_c16,  cb_unk,         cb_unk,         cb_unk,         cb_unk,
    // 0xC0 - 0xDF
    cb_in,          cb_out,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_api,         cb_unk,
    cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,
    cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,
    cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,
    // 0xE0 - 0xFF
    cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,
    cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,
    cb_sjmp,        cb_jz,          cb_jnz,         cb_jmp,         cb_scall,       cb_unk,         cb_unk,         cb_call,
    cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_test_eq,     cb_ret
};


// 0xC6 - API functions
static void cb_api(void){
    switch(plcvm_lpm_u8())
    {
#ifdef EXTTWI_USED
        case 1: // Write TWI Control
            twiControl(plcvm_pop());
            break;
        case 2: // Read TWI Status
            plcvm_push(twiStat());
            break;
        case 3: // Write TWI Data
            twiWr(plcvm_pop() & 0x000000FF);
            break;
        case 4: // Read TWI Data
            plcvm_push(twiRd());
            break;
#endif  //  EXTTWI_USED
        case 5: // Return status, 0: disconnected, 1: connected
            if(MQTTSN_GetStatus() == MQTTSN_STATUS_CONNECT)
                cb_ldi_1();
            else
                cb_ldi_0();
            break;
        case 6:
            plcvm_push(HAL_get_ms());
            break;
        case 7:
            plcvm_push(HAL_get_sec());
            break;
        case 8:
            plcvm_push(HAL_RNG32());
            break;
#if (defined HAL_USE_RTC)
        case 9:     // Get Now Seconds from 00:00:00
            plcvm_push(HAL_RTC_SecNow());
            break;
        case 10:    // Get Date 4 bytes: YRS [31-24], MNT[23-16], DAY[15-8], WDU[7-0]
                    // Years
                    // Month:    1 - Jan, 12 - Dez
                    // Day:      1 - 31
                    // Week day: 1 - Monday, 7 - Sunday
            plcvm_push(HAL_RTC_DateNow());
            break;
#endif  //  HAL_USE_RTC
        default:
            plcvm_stat = PLC_ANSWER_ERROR_UNK_API;
            break;
    }
}

#endif  //  _EXTPLC_VM_H_
