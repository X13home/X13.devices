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

static uint8_t plcvm_lpm_u8_i(uint32_t addr)
{
    uint32_t act_page = addr & (const uint32_t)(~(EXTPLC_SIZEOF_PRG_CACHE - 1UL));
    uint32_t offset = addr - act_page;

    if(act_page != plcvm_page)
    {
        if(addr >= EXTPLC_SIZEOF_PRG)
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

static uint16_t plcvm_lpm_u16_i(uint32_t addr)
{
    uint16_t retval;
    uint32_t act_page = addr & (const uint32_t)(~(EXTPLC_SIZEOF_PRG_CACHE - 1UL));
    uint32_t offset = addr - act_page;

    if(act_page != plcvm_page)
    {
        if(addr >= EXTPLC_SIZEOF_PRG)
        {
            plcvm_stat = PLC_ANSWER_ERROR_OFR_PC;
            return 0xFF;
        }

        plcvm_page = act_page;
        eeprom_read(plcvm_cache, eePLCprogram + act_page, EXTPLC_SIZEOF_PRG_CACHE);
    }
    retval = plcvm_cache[offset];

    offset++;
    if(offset == EXTPLC_SIZEOF_PRG_CACHE)
    {
        addr++;
        if(addr > EXTPLC_SIZEOF_PRG)
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

static int32_t plcvm_lpm_s32_i(uint32_t addr)
{
    int32_t retval;
    uint32_t act_page = addr & (const uint32_t)(~(EXTPLC_SIZEOF_PRG_CACHE - 1UL));
    uint32_t offset = addr - act_page;

    if(act_page != plcvm_page)
    {
        if(addr >= EXTPLC_SIZEOF_PRG)
        {
            plcvm_stat = PLC_ANSWER_ERROR_OFR_PC;
            return 0xFF;
        }

        plcvm_page = act_page;
        eeprom_read(plcvm_cache, eePLCprogram + act_page, EXTPLC_SIZEOF_PRG_CACHE);
    }
    retval = plcvm_cache[offset];

    offset++;
    if(offset == EXTPLC_SIZEOF_PRG_CACHE)
    {
        addr++;
        if(addr > EXTPLC_SIZEOF_PRG)
        {
            plcvm_stat = PLC_ANSWER_ERROR_OFR_PC;
            return 0xFFFF;
        }
        offset = 0;
        plcvm_page += EXTPLC_SIZEOF_PRG_CACHE;
        eeprom_read(plcvm_cache, eePLCprogram + plcvm_page, EXTPLC_SIZEOF_PRG_CACHE);
    }
    retval |= plcvm_cache[offset] << 8;

    offset++;
    if(offset == EXTPLC_SIZEOF_PRG_CACHE)
    {
        addr++;
        if(addr > EXTPLC_SIZEOF_PRG)
        {
            plcvm_stat = PLC_ANSWER_ERROR_OFR_PC;
            return 0xFFFF;
        }
        offset = 0;
        plcvm_page += EXTPLC_SIZEOF_PRG_CACHE;
        eeprom_read(plcvm_cache, eePLCprogram + plcvm_page, EXTPLC_SIZEOF_PRG_CACHE);
    }
    retval |= plcvm_cache[offset] << 16;

    offset++;
    if(offset == EXTPLC_SIZEOF_PRG_CACHE)
    {
        addr++;
        if(addr > EXTPLC_SIZEOF_PRG)
        {
            plcvm_stat = PLC_ANSWER_ERROR_OFR_PC;
            return 0xFFFF;
        }
        offset = 0;
        plcvm_page += EXTPLC_SIZEOF_PRG_CACHE;
        eeprom_read(plcvm_cache, eePLCprogram + plcvm_page, EXTPLC_SIZEOF_PRG_CACHE);
    }
    retval |= plcvm_cache[offset] << 24;

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
// Shift counts of 32 and more give 0, ASR the sign - as on ARM
static void cb_lsl(void){if(plcvm_sp < plcvm_sfp){uint32_t v1 = plcvm_ram[plcvm_sp];uint8_t n = plcvm_lpm_u8();plcvm_ram[plcvm_sp] = (n < 32) ? (v1 << n) : 0;}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_lsr(void){if(plcvm_sp < plcvm_sfp){uint32_t v1 = plcvm_ram[plcvm_sp];uint8_t n = plcvm_lpm_u8();plcvm_ram[plcvm_sp] = (n < 32) ? (v1 >> n) : 0;}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_asr(void){if(plcvm_sp < plcvm_sfp){int32_t  v1 = plcvm_ram[plcvm_sp];uint8_t n = plcvm_lpm_u8();plcvm_ram[plcvm_sp] = (n < 32) ? (uint32_t)(v1 >> n) : ((v1 < 0) ? 0xFFFFFFFFUL : 0UL);}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
// 0x10 - 0x17 - Arithmetic Operators
static void cb_add(void){if((plcvm_sp + 1) < plcvm_sfp){int32_t v1 = plcvm_ram[plcvm_sp++];int32_t v2 = plcvm_ram[plcvm_sp];plcvm_ram[plcvm_sp] = v2 + v1;}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_sub(void){if((plcvm_sp + 1) < plcvm_sfp){int32_t v1 = plcvm_ram[plcvm_sp++];int32_t v2 = plcvm_ram[plcvm_sp];plcvm_ram[plcvm_sp] = v2 - v1;}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_mul(void){if((plcvm_sp + 1) < plcvm_sfp){int32_t v1 = plcvm_ram[plcvm_sp++];int32_t v2 = plcvm_ram[plcvm_sp];plcvm_ram[plcvm_sp] = v2 * v1;}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
// v1 == -1: the quotient is computed on uint32, INT32_MIN / -1 gives INT32_MIN as on ARM
static void cb_div(void){if((plcvm_sp + 1) < plcvm_sfp){int32_t v1 = plcvm_ram[plcvm_sp++];int32_t v2 = plcvm_ram[plcvm_sp];if(v1 == 0){plcvm_stat = PLC_ANSWER_ERROR_DIV0;}else if(v1 == -1){plcvm_ram[plcvm_sp] = 0UL - (uint32_t)v2;}else{plcvm_ram[plcvm_sp] = v2 / v1;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
// MOD by 0 stops the program as DIV does; v1 == -1 gives 0 without the INT32_MIN overflow
static void cb_mod(void){if((plcvm_sp + 1) < plcvm_sfp){int32_t v1 = plcvm_ram[plcvm_sp++];int32_t v2 = plcvm_ram[plcvm_sp];if(v1 == 0){plcvm_stat = PLC_ANSWER_ERROR_DIV0;}else if(v1 == -1){plcvm_ram[plcvm_sp] = 0;}else{plcvm_ram[plcvm_sp] = v2 % v1;}}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_inc(void){if(plcvm_sp < plcvm_sfp){plcvm_ram[plcvm_sp]++;}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_dec(void){if(plcvm_sp < plcvm_sfp){plcvm_ram[plcvm_sp]--;}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_neg(void){if(plcvm_sp < plcvm_sfp){plcvm_ram[plcvm_sp] = -plcvm_ram[plcvm_sp];}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
// 0x18 - 0x1F - Float Arithmetic Operators
#ifdef EXTPLC_USE_FLOAT
// A float is the 32 bit IEEE-754 pattern in the same stack cell as an integer, so it is loaded and
// stored by the ordinary LDM_S4 / STM_S4. Results follow JavaScript, the source language of the
// programs. Ported from EspNode, components/plc/plc_vm.c
// The equality below is the semantics of the opcodes, not an accidental comparison of floats:
// FCEQ / FCNE / FCZE are the JavaScript == and !=, and Math.min / Math.max need it to tell -0
// from +0. The project builds with -Wfloat-equal, so it is turned off for these blocks.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
static float plcvm_u2f(uint32_t bits){float f;memcpy(&f, &bits, sizeof(f));return f;}
static uint32_t plcvm_f2u(float f){uint32_t bits;memcpy(&bits, &f, sizeof(bits));return bits;}

// Math.min / Math.max: NaN if either operand is NaN, -0 < +0
static float plcvm_js_min(float a, float b)
{
    if(isnan(a) || isnan(b))
    {
        return NAN;
    }
    if(a == b)
    {
        return signbit(a) ? a : b;
    }
    return (a < b) ? a : b;
}

static float plcvm_js_max(float a, float b)
{
    if(isnan(a) || isnan(b))
    {
        return NAN;
    }
    if(a == b)
    {
        return signbit(a) ? b : a;
    }
    return (a > b) ? a : b;
}

// Math.round: halves towards +Infinity, the sign of zero is kept
static float plcvm_js_round(float a)
{
    float r = floorf(a);
    if((a - r) >= 0.5f)             // Exact for |a| < 2^23, larger floats are integers
    {
        r += 1.0f;
    }
    if((r == 0.0f) && signbit(a))
    {
        r = -0.0f;
    }
    return r;
}

// ToInt32 / ToUint32 (x | 0, x >>> 0): truncate, modulo 2^32, NaN and Infinity give 0.
// Both give the same 32 bits.
static uint32_t plcvm_js_to_int32(float f)
{
    uint32_t bits = plcvm_f2u(f);
    int32_t  exp  = (int32_t)((bits >> 23) & 0xFF) - 127;

    if((exp < 0) || (exp == 128))
    {
        return 0;                   // |f| < 1, NaN, Infinity
    }

    uint32_t mant = (bits & 0x007FFFFFUL) | 0x00800000UL;
    uint32_t mag;

    if(exp <= 23)
    {
        mag = mant >> (23 - exp);
    }
    else if(exp < (23 + 32))
    {
        mag = mant << (exp - 23);   // Bits above 2^31 drop out: modulo 2^32
    }
    else
    {
        mag = 0;
    }

    return (bits & 0x80000000UL) ? (0UL - mag) : mag;
}

// Float operations: a - second, b - top of the stack, as for the integer ones
#define PLCVM_FUNARY(name, expr)    static void name(void){if(plcvm_sp < plcvm_sfp){float a = plcvm_u2f(plcvm_ram[plcvm_sp]);plcvm_ram[plcvm_sp] = (expr);}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
#define PLCVM_FBINARY(name, expr)   static void name(void){if((plcvm_sp + 1) < plcvm_sfp){float b = plcvm_u2f(plcvm_ram[plcvm_sp++]);float a = plcvm_u2f(plcvm_ram[plcvm_sp]);plcvm_ram[plcvm_sp] = (expr);}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
// Conversion from the integer in the stack cell
#define PLCVM_FCONV(name, expr)     static void name(void){if(plcvm_sp < plcvm_sfp){uint32_t v = plcvm_ram[plcvm_sp];plcvm_ram[plcvm_sp] = (expr);}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}

// Division by 0 gives +-Infinity / NaN without an error, unlike the integer DIV
PLCVM_FBINARY(cb_fadd, plcvm_f2u(a + b))
PLCVM_FBINARY(cb_fsub, plcvm_f2u(a - b))
PLCVM_FBINARY(cb_fmul, plcvm_f2u(a * b))
PLCVM_FBINARY(cb_fdiv, plcvm_f2u(a / b))
PLCVM_FUNARY (cb_fneg, plcvm_f2u(-a))
PLCVM_FUNARY (cb_fabs, plcvm_f2u(fabsf(a)))
PLCVM_FBINARY(cb_fmin, plcvm_f2u(plcvm_js_min(a, b)))
PLCVM_FBINARY(cb_fmax, plcvm_f2u(plcvm_js_max(a, b)))
#pragma GCC diagnostic pop
#endif  //  EXTPLC_USE_FLOAT
// 0x20 - 0x27 - Compare
static void cb_ceq(void){if((plcvm_sp + 1) < plcvm_sfp){int32_t v1 = plcvm_ram[plcvm_sp++];int32_t v2 = plcvm_ram[plcvm_sp];plcvm_ram[plcvm_sp] = (v2 == v1);}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_cne(void){if((plcvm_sp + 1) < plcvm_sfp){int32_t v1 = plcvm_ram[plcvm_sp++];int32_t v2 = plcvm_ram[plcvm_sp];plcvm_ram[plcvm_sp] = (v2 != v1);}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_cgt(void){if((plcvm_sp + 1) < plcvm_sfp){int32_t v1 = plcvm_ram[plcvm_sp++];int32_t v2 = plcvm_ram[plcvm_sp];plcvm_ram[plcvm_sp] = (v2 >  v1);}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_cge(void){if((plcvm_sp + 1) < plcvm_sfp){int32_t v1 = plcvm_ram[plcvm_sp++];int32_t v2 = plcvm_ram[plcvm_sp];plcvm_ram[plcvm_sp] = (v2 >= v1);}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_clt(void){if((plcvm_sp + 1) < plcvm_sfp){int32_t v1 = plcvm_ram[plcvm_sp++];int32_t v2 = plcvm_ram[plcvm_sp];plcvm_ram[plcvm_sp] = (v2 <  v1);}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_cle(void){if((plcvm_sp + 1) < plcvm_sfp){int32_t v1 = plcvm_ram[plcvm_sp++];int32_t v2 = plcvm_ram[plcvm_sp];plcvm_ram[plcvm_sp] = (v2 <= v1);}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
static void cb_cze(void){if(plcvm_sp < plcvm_sfp){plcvm_ram[plcvm_sp] = (plcvm_ram[plcvm_sp] == 0);}else{plcvm_stat = PLC_ANSWER_ERROR_OFR_SP;}}
// 0x28 - 0x2F - Float Compare, result 0/1
#ifdef EXTPLC_USE_FLOAT
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
// Any comparison with NaN is false, FCNE is the only one true
PLCVM_FBINARY(cb_fceq, (a == b))
PLCVM_FBINARY(cb_fcne, (a != b))
PLCVM_FBINARY(cb_fcgt, (a >  b))
PLCVM_FBINARY(cb_fcge, (a >= b))
PLCVM_FBINARY(cb_fclt, (a <  b))
PLCVM_FBINARY(cb_fcle, (a <= b))
PLCVM_FUNARY (cb_fcze, (a == 0.0f))                 // True for +-0
PLCVM_FUNARY (cb_fisfinite, (isfinite(a) ? 1UL : 0UL))
// 0x30 - 0x37 - Float Conversions
PLCVM_FCONV  (cb_s32tof, plcvm_f2u((float)(int32_t)v))
PLCVM_FCONV  (cb_u32tof, plcvm_f2u((float)v))
PLCVM_FCONV  (cb_ftos32, plcvm_js_to_int32(plcvm_u2f(v)))
PLCVM_FCONV  (cb_ftou32, plcvm_js_to_int32(plcvm_u2f(v)))   // The same 32 bits as FTOS32
PLCVM_FUNARY (cb_fround, plcvm_f2u(plcvm_js_round(a)))
PLCVM_FUNARY (cb_ftrunc, plcvm_f2u(truncf(a)))
PLCVM_FUNARY (cb_fisnan, (isnan(a) ? 1UL : 0UL))
PLCVM_FUNARY (cb_fisinf, (isinf(a) ? 1UL : 0UL))
#pragma GCC diagnostic pop
#endif  //  EXTPLC_USE_FLOAT
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
// 0xC9 - 0xCC, LPM - Load from Programm Memory
static void cb_lpm_s1(void){uint32_t data = plcvm_lpm_u8_i(plcvm_pop() + plcvm_lpm_u16());if(data & 0x00000080){data |= 0xFFFFFF00;}plcvm_push(data);}
static void cb_lpm_u1(void){uint32_t data = plcvm_lpm_u8_i(plcvm_pop() + plcvm_lpm_u16());plcvm_push(data);}
static void cb_lpm_s2(void){uint32_t data = plcvm_lpm_u16_i((2*plcvm_pop()) + plcvm_lpm_u16());if(data & 0x00008000){data |= 0xFFFF0000;}plcvm_push(data);}
static void cb_lpm_u2(void){uint32_t data = plcvm_lpm_u16_i((2*plcvm_pop()) + plcvm_lpm_u16());plcvm_push(data);}
static void cb_lpm_s4(void){uint32_t data = plcvm_lpm_s32_i((4*plcvm_pop()) + plcvm_lpm_u16());plcvm_push(data);}

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
    if(plcvm_sp > (plcvm_stack_bot + 1))    // (sp - 1) > stack_bot wrapped around at sp == 0
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
static void cb_check_idx(void)
{
    if(plcvm_sp < plcvm_sfp)
    {
        int32_t data = plcvm_ram[plcvm_sp];
        if((data < 0) || (data >= plcvm_lpm_u16()))
        {
            plcvm_stat = PLC_ANSWER_ERROR_OFR_IDX;
        }
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

#ifndef EXTPLC_USE_FLOAT
// Without float the opcodes stay unknown, the table keeps one shape
#define cb_fadd         cb_unk
#define cb_fsub         cb_unk
#define cb_fmul         cb_unk
#define cb_fdiv         cb_unk
#define cb_fneg         cb_unk
#define cb_fabs         cb_unk
#define cb_fmin         cb_unk
#define cb_fmax         cb_unk
#define cb_fceq         cb_unk
#define cb_fcne         cb_unk
#define cb_fcgt         cb_unk
#define cb_fcge         cb_unk
#define cb_fclt         cb_unk
#define cb_fcle         cb_unk
#define cb_fcze         cb_unk
#define cb_fisfinite    cb_unk
#define cb_s32tof       cb_unk
#define cb_u32tof       cb_unk
#define cb_ftos32       cb_unk
#define cb_ftou32       cb_unk
#define cb_fround       cb_unk
#define cb_ftrunc       cb_unk
#define cb_fisnan       cb_unk
#define cb_fisinf       cb_unk
#endif  //  EXTPLC_USE_FLOAT

// Operators table
static const cbCOP_t plcvm_cb[] = {
    // 0x00 - 0x1F
    //              //              //              //              //              //              //              //
    cb_nop,         cb_unk,         cb_dup,         cb_drop,        cb_nip,         cb_swap,        cb_over,        cb_rot,
    cb_not,         cb_and,         cb_or,          cb_xor,         cb_lsl,         cb_lsr,         cb_asr,         cb_unk,
    cb_add,         cb_sub,         cb_mul,         cb_div,         cb_mod,         cb_inc,         cb_dec,         cb_neg,
    cb_fadd,        cb_fsub,        cb_fmul,        cb_fdiv,        cb_fneg,        cb_fabs,        cb_fmin,        cb_fmax,
    // 0x20 - 0x3F
    cb_ceq,         cb_cne,         cb_cgt,         cb_cge,         cb_clt,         cb_cle,         cb_cze,         cb_unk,
    cb_fceq,        cb_fcne,        cb_fcgt,        cb_fcge,        cb_fclt,        cb_fcle,        cb_fcze,        cb_fisfinite,
    cb_s32tof,      cb_u32tof,      cb_ftos32,      cb_ftou32,      cb_fround,      cb_ftrunc,      cb_fisnan,      cb_fisinf,
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
    cb_unk,         cb_lpm_s1,      cb_lpm_s2,      cb_lpm_s4,      cb_lpm_u1,      cb_lpm_u2,      cb_unk,         cb_unk,
    cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,
    cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,
    // 0xE0 - 0xFF
    cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,
    cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,
    cb_sjmp,        cb_jz,          cb_jnz,         cb_jmp,         cb_scall,       cb_unk,         cb_unk,         cb_call,
    cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_unk,         cb_check_idx,   cb_test_eq,     cb_ret
};


// A stream of PrintF is a UART port or a log object, so the text output belongs here as soon as
// the node has one of the two: some S4 boards carry the node log and no UART at all(S4SM13 ..)
#if (defined EXTSER_USED) || (defined EXTPLC_SIZEOF_LOG)
#define PLCVM_USE_PRINT             1
#endif  //  (defined EXTSER_USED) || (defined EXTPLC_SIZEOF_LOG)

#ifdef PLCVM_USE_PRINT
// Text output of PrintF(26). The buffer of a stream belongs to its owner(extser for a UART port), so
// the text is built here and handed over whole: it is written all or not at all. Ported from EspNode,
// components/plc/plc_stream.c - the format is parsed by the same rules as the compiler(EP_PrintF),
// so a program built by something else stops instead of printing what a libc would.

// The text of one call. The transmit buffer of a port is one mqAlloc block, and a log line has to fit
// in one PUBLISH, so nothing longer than this can ever be written.
#ifndef EXTPLC_SIZEOF_PRINT
#define EXTPLC_SIZEOF_PRINT         (sizeof(MQ_t))
#endif  //  EXTPLC_SIZEOF_PRINT

#define PLCVM_SIZEOF_CONV           24      // One conversion: 10 digits, the point and 8 more

#ifdef EXTPLC_SIZEOF_LOG
// Lines of the program for the node log, the predefined objects objLogD .. objLogE. Each line goes
// out as one PUBLISH at QoS0, so it can hold at most MQTTSN_MSG_SIZE - 5 bytes; the gateway turns
// the topic into a log level(Enviriot, MsDevice.cs). Records are [level][len][bytes] one after
// another in one buffer: the levels share the memory and the lines keep their order, and only the
// first record is published. As on EspNode a 0 byte ends a line, so one PrintF can write several,
// and the last line stays open until the end of the scan - plcProc closes it.
// A full buffer neither stops the program nor is reported to it, as on EspNode(node_log.c): the
// lines already in it are kept, the new ones are counted, and the count goes out as a line of its
// own at Warning. So 0 from PrintF to a log stream means one thing only - a line over the limit.
#define PLCVM_SIZEOF_LOG_LINE       (MQTTSN_MSG_SIZE - 5)
#define PLCVM_LOG_WARN              2       // objLogW - objLogD, the level of the report

// "node_log: " and " lines dropped" around at most five digits, as EspNode words it
#define PLCVM_SIZEOF_LOG_REPORT     (10 + 5 + 14)

#if (EXTPLC_SIZEOF_LOG < 64)
#error EXTPLC_SIZEOF_LOG is too small, the report of the lost lines has to fit beside a line
#endif  //  (EXTPLC_SIZEOF_LOG < 64)

static uint8_t  plcvm_log[EXTPLC_SIZEOF_LOG];
static uint32_t plcvm_log_len;                  // Bytes used, records from the start of the buffer
static uint32_t plcvm_log_last;                 // Offset of the last record
static bool     plcvm_log_open;                 // That record can still be appended to
static uint16_t plcvm_log_dropped;              // Lines lost since the last report
static bool     plcvm_log_losing;               // The line being written is lost, not just unwritten
static uint8_t  plcvm_log_lost_lvl;             // Its level, and the length it would have had, so the
static uint8_t  plcvm_log_lost_len;             //  limit of a line holds whatever room is left

static uint32_t plcvm_fmt_int32(char * pTxt, uint32_t value);   // Below, with the conversions

// What the line of this level already holds, whether it is being written or lost
static uint32_t plcvm_log_line_len(uint8_t level)
{
    if(plcvm_log_losing)
    {
        return (plcvm_log_lost_lvl == level) ? plcvm_log_lost_len : 0;
    }

    return (plcvm_log_open && (plcvm_log[plcvm_log_last] == level))
            ? plcvm_log[plcvm_log_last + 1] : 0;
}

// The line ends, by a 0 byte or by the end of the scan. One that was lost is counted here, where its
// length is final
static void plcvm_log_close(void)
{
    plcvm_log_open = false;

    if(plcvm_log_losing)
    {
        plcvm_log_losing = false;

        if(plcvm_log_dropped != 0xFFFF)         // Saturates: it only has to say that lines were lost
        {
            plcvm_log_dropped++;
        }
    }
}

// There is no room for the line: what of it is already in the buffer goes too, since half a line
// published as a whole one is worse than none. The open record is always the last, so it truncates
static void plcvm_log_lose(uint8_t level)
{
    plcvm_log_lost_len = 0;

    if(plcvm_log_open && (plcvm_log[plcvm_log_last] == level))
    {
        plcvm_log_lost_len = plcvm_log[plcvm_log_last + 1];
        plcvm_log_len      = plcvm_log_last;
    }

    plcvm_log_open     = false;
    plcvm_log_lost_lvl = level;
    plcvm_log_losing   = true;
}

// Queues how many lines were lost, as soon as there is room for the line
static void plcvm_log_report(void)
{
    char     text[PLCVM_SIZEOF_LOG_REPORT];
    uint32_t len = 10;

    memcpy(text, "node_log: ", 10);
    len += plcvm_fmt_int32(&text[len], plcvm_log_dropped);
    memcpy(&text[len], " lines dropped", 14);
    len += 14;

    if((plcvm_log_len + len + 2) > EXTPLC_SIZEOF_LOG)
    {
        return;                                 // No room yet, the next call tries again
    }

    plcvm_log_last = plcvm_log_len;
    plcvm_log[plcvm_log_len++] = PLCVM_LOG_WARN;
    plcvm_log[plcvm_log_len++] = (uint8_t)len;
    memcpy(&plcvm_log[plcvm_log_len], text, len);
    plcvm_log_len += len;
    plcvm_log_open = false;                     // A complete line, nothing is appended to it

    plcvm_log_dropped = 0;
}

// 1 - the text was taken, 0 - a line of it is longer than one PUBLISH and nothing was written
static uint32_t plcvm_log_put(uint8_t level, const char * pText, uint32_t len)
{
    uint32_t pos;
    uint32_t line = plcvm_log_line_len(level);

    // The limit of a line first, before anything is written: it does not depend on the room left
    for(pos = 0; pos < len; pos++)
    {
        if(pText[pos] == 0)
        {
            line = 0;
            continue;
        }

        line++;

        if(line > PLCVM_SIZEOF_LOG_LINE)        // One line has to fit in one PUBLISH
        {
            return 0;
        }
    }

    for(pos = 0; pos < len; pos++)              // The same walk, now writing
    {
        if(pText[pos] == 0)
        {
            plcvm_log_close();
            continue;
        }

        if(plcvm_log_losing && (plcvm_log_lost_lvl != level))
        {
            plcvm_log_close();                  // Another level is another line
        }

        if(plcvm_log_losing)
        {
            plcvm_log_lost_len++;
            continue;
        }

        if(!plcvm_log_open || (plcvm_log[plcvm_log_last] != level))
        {
            if((plcvm_log_len + 3) > EXTPLC_SIZEOF_LOG)     // The header and one byte
            {
                plcvm_log_lose(level);
                plcvm_log_lost_len++;
                continue;
            }

            plcvm_log_last = plcvm_log_len;
            plcvm_log[plcvm_log_len++] = level;
            plcvm_log[plcvm_log_len++] = 0;
            plcvm_log_open = true;
        }
        else if(plcvm_log_len == EXTPLC_SIZEOF_LOG)
        {
            plcvm_log_lose(level);
            plcvm_log_lost_len++;
            continue;
        }

        plcvm_log[plcvm_log_len++] = (uint8_t)pText[pos];
        plcvm_log[plcvm_log_last + 1]++;
    }

    return 1;
}

// The level of the first line, or PLC_LOG_LEVELS when there is nothing to send. The report of the
// lost lines is queued here, as node_log_get does on EspNode: the scan is over by now, so no line is
// open and adding one cannot cut another in half
static uint8_t plcvm_log_level(void)
{
    if(plcvm_log_dropped != 0)
    {
        plcvm_log_report();
    }

    return (plcvm_log_len != 0) ? plcvm_log[0] : PLC_LOG_LEVELS;
}

// Takes the first line if it is of this level: its length, or 0xFF when it is not there
static uint8_t plcvm_log_take(uint8_t level, uint8_t * pBuf, uint8_t room)
{
    if((plcvm_log_len == 0) || (plcvm_log[0] != level))
    {
        return 0xFF;
    }

    uint8_t  len  = plcvm_log[1];
    uint32_t used = (uint32_t)len + 2;

    if(len > room)                  // Cannot happen: the length is checked as the line is written
    {
        len = room;
    }
    memcpy(pBuf, &plcvm_log[2], len);

    plcvm_log_len -= used;          // Drop the record, the ones behind it move up
    memmove(plcvm_log, &plcvm_log[used], plcvm_log_len);
    plcvm_log_last = (plcvm_log_last > used) ? (plcvm_log_last - used) : 0;

    return len;
}
#endif  //  EXTPLC_SIZEOF_LOG

// One stream: a UART port in the numbering of extser, or a log object objLogD .. objLogE where the
// node has them. serPutTextPlc raises PARM for a port that does not exist, as the contract asks.
static uint32_t plcvm_put(uint32_t stream, const char * pText, uint32_t len)
{
#ifdef EXTPLC_SIZEOF_LOG
    if((stream >= objLogD) && (stream <= objLogE))
    {
        return plcvm_log_put((uint8_t)(stream - objLogD), pText, len);
    }
#endif  //  EXTPLC_SIZEOF_LOG

#ifdef EXTSER_USED
    return serPutTextPlc(stream, pText, len);
#else                               // No UART here, so a stream is a log object and nothing else
    plcException(PLC_ANSWER_ERROR_PARM);
    return 0;
#endif  //  EXTSER_USED
}

// Appends to the text, counting on past the buffer as emit() of EspNode does: a text too long for
// the stream is refused whole, never cut.
static void plcvm_emit(char * pOut, uint32_t * pLen, const char * pSrc, uint32_t len)
{
    uint32_t pos;

    for(pos = 0; pos < len; pos++, (*pLen)++)
    {
        if(*pLen < EXTPLC_SIZEOF_PRINT)
        {
            pOut[*pLen] = pSrc[pos];
        }
    }
}

// Width padding, shared by the conversions: flag 0 - spaces on the left, '0' - zeros after the sign,
// '-' - spaces on the right. The width never cuts the value.
static uint32_t plcvm_pad(char * pOut, const char * pTxt, uint32_t len, uint32_t width, char flag)
{
    uint32_t olen = 0;
    uint32_t pos  = 0;
    uint32_t fill;

    if(width <= len)
    {
        while(pos < len)
        {
            pOut[olen++] = pTxt[pos++];
        }
        return olen;
    }

    fill = width - len;

    if(flag == '-')                                 // Spaces on the right
    {
        while(pos < len)
        {
            pOut[olen++] = pTxt[pos++];
        }
        while(fill-- != 0)
        {
            pOut[olen++] = ' ';
        }
        return olen;
    }

    if((flag == '0') && (pTxt[0] == '-'))           // Zeros go after the sign
    {
        pOut[olen++] = pTxt[pos++];
    }
    while(fill-- != 0)
    {
        pOut[olen++] = (flag == '0') ? '0' : ' ';
    }
    while(pos < len)
    {
        pOut[olen++] = pTxt[pos++];
    }

    return olen;
}

// Decimal with the sign, without the width: %d
static uint32_t plcvm_fmt_int32(char * pTxt, uint32_t value)
{
    char     rev[10];                               // 2147483648, the sign goes out first
    uint32_t nrev = 0;
    uint32_t len  = 0;
    uint32_t mag  = ((int32_t)value < 0) ? (0UL - value) : value;

    do
    {
        rev[nrev++] = (char)('0' + (mag % 10));
        mag /= 10;
    }
    while(mag != 0);

    if((int32_t)value < 0)
    {
        pTxt[len++] = '-';
    }
    while(nrev != 0)
    {
        pTxt[len++] = rev[--nrev];
    }

    return len;
}

// Hexadecimal, without the width: %x and %X
static uint32_t plcvm_fmt_hex(char * pTxt, uint32_t value, bool upper)
{
    char     rev[8];
    uint32_t nrev = 0;
    uint32_t len  = 0;

    do
    {
        uint32_t digit = value & 0x0F;
        rev[nrev++] = (char)((digit < 10) ? ('0' + digit) : ((upper ? 'A' : 'a') + (digit - 10)));
        value >>= 4;
    }
    while(value != 0);

    while(nrev != 0)
    {
        pTxt[len++] = rev[--nrev];
    }

    return len;
}

#ifdef EXTPLC_USE_FLOAT
static const uint32_t plcvm_pow10[9] =
    {1UL, 10UL, 100UL, 1000UL, 10000UL, 100000UL, 1000000UL, 10000000UL, 100000000UL};

// Decimal with prec digits after the point, without the width: %f. NaN and the infinities are the
// caller's, here only a finite value. A magnitude of 2^32 and above gives PLC_ANSWER_ERROR_PARM: its
// integer part does not fit uint32_t, and nothing a node measures reaches it(EspNode has snprintf).
static uint32_t plcvm_fmt_float(char * pTxt, uint32_t prec, float f)
{
    if(fabsf(f) >= 4294967296.0f)
    {
        plcException(PLC_ANSWER_ERROR_PARM);
        return 0;
    }

    // Exact and without double: m == ipart + j / 2^23, where ipart is the truncated integer part and
    // j an integer below 2^23 - the mantissa of a float below 2^32 has no step finer than 2^-23, so
    // m - ipart and its scaling by 2^23 are both exact. Then j * 10^prec stays below 2^23 * 10^8 and
    // fits uint64_t exactly: the printed digit is rounded once, from the exact value, to the nearest
    // and halves to even - the same digits printf gives.
    float    m     = fabsf(f);
    uint32_t ipart = (uint32_t)m;
    uint32_t j     = (uint32_t)((m - (float)ipart) * 8388608.0f);
    uint64_t num   = (uint64_t)j * plcvm_pow10[prec];
    uint32_t fpart = (uint32_t)(num >> 23);
    uint32_t rem   = (uint32_t)(num & 0x007FFFFFUL);
    char     rev[PLCVM_SIZEOF_CONV];
    uint32_t nrev  = 0;
    uint32_t len   = 0;
    uint32_t pos;

    // Halves to even: for prec above 0 the printed digit is the last one of fpart, for prec 0 the
    // fraction is dropped altogether and the digit rounded is the last one of ipart
    uint32_t odd = ((prec == 0) ? (ipart + fpart) : fpart) & 1;

    if((rem > 0x00400000UL) || ((rem == 0x00400000UL) && (odd != 0)))
    {
        fpart++;
    }
    if(fpart == plcvm_pow10[prec])                  // The fraction carried into the integer part
    {
        fpart = 0;
        ipart++;
    }

    for(pos = 0; pos < prec; pos++)                 // Fraction, least significant digit first
    {
        rev[nrev++] = (char)('0' + (fpart % 10));
        fpart /= 10;
    }
    if(prec != 0)
    {
        rev[nrev++] = '.';
    }
    do
    {
        rev[nrev++] = (char)('0' + (ipart % 10));
        ipart /= 10;
    }
    while(ipart != 0);

    if(signbit(f))                                  // -0.0 keeps its sign, as printf
    {
        pTxt[len++] = '-';
    }
    while(nrev != 0)
    {
        pTxt[len++] = rev[--nrev];
    }

    return len;
}
#endif  //  EXTPLC_USE_FLOAT

// One conversion with its width, as convert() of EspNode: kind - the specifier, flag 0, '0' or '-',
// width 0..9, prec -1 not set or 0..8
static uint32_t plcvm_convert(char * pOut, char kind, char flag, uint32_t width, int32_t prec,
                              uint32_t v)
{
    char     txt[PLCVM_SIZEOF_CONV];
    uint32_t len;

    switch(kind)
    {
        case 'c':
            txt[0] = (char)(uint8_t)v;
            len = 1;
            break;
        case 'd':
            len = plcvm_fmt_int32(txt, v);
            break;
        case 'x':
        case 'X':
            len = plcvm_fmt_hex(txt, v, (kind == 'X'));
            break;
#ifdef EXTPLC_USE_FLOAT
        default:                                    // 'f'
            {
            float f = plcvm_u2f(v);

            if(isnan(f) || isinf(f))                // Padded with spaces, never zeros, as printf
            {
                len = 0;
                if(isinf(f) && signbit(f))
                {
                    txt[len++] = '-';
                }
                txt[len++] = isnan(f) ? 'n' : 'i';
                txt[len++] = isnan(f) ? 'a' : 'n';
                txt[len++] = isnan(f) ? 'n' : 'f';

                if(flag == '0')
                {
                    flag = 0;
                }
            }
            else                                    // Not set is the 6 digits of "%f"
            {
                len = plcvm_fmt_float(txt, (prec < 0) ? 6 : (uint32_t)prec, f);
            }
            }
            break;
#else
        default:                                    // The parser rejects %f without the float opcodes
            (void)prec;
            len = 0;
            break;
#endif  //  EXTPLC_USE_FLOAT
    }

    return plcvm_pad(pOut, txt, len, width, flag);
}

// PrintF(stream, "format", values...). The format lies in the memory of the program and is read
// byte by byte: on this target it is the emulated EEPROM, there is no pointer to it. Its length is
// explicit, so a 0 byte is data and does not end it. 1 - written, 0 - no room and nothing written.
static uint32_t plcvm_printf(uint32_t stream, uint32_t addr, uint32_t flen, uint32_t n)
{
    char     out[EXTPLC_SIZEOF_PRINT];
    char     conv[PLCVM_SIZEOF_CONV];
    uint32_t olen = 0;                              // All of the text, counted on past the buffer
    uint32_t used = 0;
    uint32_t i    = 0;

    while(i < flen)
    {
        char c = (char)plcvm_lpm_u8_i(addr + i++);

        if(c != '%')
        {
            plcvm_emit(out, &olen, &c, 1);
            continue;
        }
        if((i < flen) && (plcvm_lpm_u8_i(addr + i) == '%'))
        {
            i++;
            plcvm_emit(out, &olen, &c, 1);
            continue;
        }

        char flag = 0;
        if(i < flen)
        {
            char f = (char)plcvm_lpm_u8_i(addr + i);
            if((f == '0') || (f == '-'))
            {
                flag = f;
                i++;
            }
        }

        uint32_t width = 0;
        if(i < flen)
        {
            char w = (char)plcvm_lpm_u8_i(addr + i);
            if((w >= '1') && (w <= '9'))
            {
                width = (uint32_t)(w - '0');
                i++;
            }
        }

        int32_t prec = -1;
        if((i < flen) && (plcvm_lpm_u8_i(addr + i) == '.'))
        {
            i++;
            char p = (i < flen) ? (char)plcvm_lpm_u8_i(addr + i) : 0;
            if((p < '0') || (p > '8'))
            {
                plcException(PLC_ANSWER_ERROR_PARM);
                return 0;
            }
            prec = p - '0';
            i++;
        }

        char kind = (i < flen) ? (char)plcvm_lpm_u8_i(addr + i++) : 0;
        bool num  = (kind == 'd') || (kind == 'x') || (kind == 'X');
#ifdef EXTPLC_USE_FLOAT
        bool fp   = (kind == 'f');
#else
        bool fp   = false;                          // Without the float opcodes there is no %f
#endif  //  EXTPLC_USE_FLOAT

        if(!(num || fp || (kind == 'c')) ||         // The rules of EP_PrintF, in its order
           ((flag != 0) && (width == 0)) ||
           ((kind == 'c') && ((width != 0) || (prec >= 0))) ||
           (num && (prec >= 0)) || (used == n))
        {
            plcException(PLC_ANSWER_ERROR_PARM);
            return 0;
        }

        uint32_t v = plcvm_pop();
        if(plcvm_stat != PLC_ANSWER_RUN)             // The value is not on the stack: OFR_SP
        {
            return 0;
        }
        used++;

        uint32_t clen = plcvm_convert(conv, kind, flag, width, prec, v);
        plcvm_emit(out, &olen, conv, clen);
    }

    if(used != n)                                   // Not as many conversions as values
    {
        plcException(PLC_ANSWER_ERROR_PARM);
        return 0;
    }
    if(olen > EXTPLC_SIZEOF_PRINT)                  // Longer than any stream can take
    {
        return 0;
    }

    return plcvm_put(stream, out, olen);
}
#endif  //  PLCVM_USE_PRINT

// 0xC6 - API functions
// Numbers are the contract with the Enviriot compiler, see EspNode docs/plc-api.md. Arguments are
// pushed last first, so the first one is popped first. Differences from EspNode: a stream here is a
// UART port in the numbering of extser, not of the chip; 23 keeps the X13 UartPutByte, which returns
// the position in the packet or -1, where EspNode returns 1/0; and the text of one PrintF has to fit
// in the transmit buffer of a port, which is smaller here. 24 and 25 (PrintInt32, PrintFloat) were
// replaced by 26 and their numbers are free, as on EspNode. 31 (ConfigPin) and 40 - 43 (TWI bus) are
// EspNode only: the input and output of X13 is addressed by objects, by the IN and OUT operands.
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

#ifdef EXTSER_USED
        case 20:    // Init UART, parm: port, speed
            {       // One pop per statement: the order of evaluating arguments in C is unspecified
            uint32_t port = plcvm_pop();
            uint32_t speed = plcvm_pop();
            serInitPlc(port, speed);
            }
            break;
        case 21:
            plcvm_push(serBytes2ReadPlc(plcvm_pop()));
            break;
        case 22:
            plcvm_push(serGetBytePlc(plcvm_pop()));
            break;
        case 23:    // UartPutByte(port, data), X13: returns the position in the packet or -1
            {
            uint32_t port = plcvm_pop();
            uint32_t data = plcvm_pop();
            plcvm_push(serPutBytePlc(port, data));
            }
            break;
#endif  //  EXTSER_USED

#ifdef PLCVM_USE_PRINT
        case 26:    // PrintF(stream, "format", values...), 1 - written, 0 - no room
            {       // The format itself stays in the program, only its address and length are here
            uint32_t stream = plcvm_pop();
            uint32_t addr   = plcvm_pop();
            uint32_t flen   = plcvm_pop();
            uint32_t n      = plcvm_pop();

            if(plcvm_stat != PLC_ANSWER_RUN)        // The four words are not on the stack: OFR_SP
            {
                break;
            }
            if((flen > EXTPLC_SIZEOF_PRG) ||        // The format must lie inside the program
               (addr > (EXTPLC_SIZEOF_PRG - flen)))
            {
                plcException(PLC_ANSWER_ERROR_PARM);
                break;
            }
            plcvm_push(plcvm_printf(stream, addr, flen, n));
            }
            break;
#endif  //  PLCVM_USE_PRINT

        default:
            plcvm_stat = PLC_ANSWER_ERROR_UNK_API;
            break;
    }
}

#endif  //  _EXTPLC_VM_H_
