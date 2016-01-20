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

typedef void (*cbCOP_t)(void);

uint32_t                plc_ram[EXTPLC_SIZEOF_RAM];         // PLC data + merkers + stack
static ePLC_ANSWER_t    plcvm_stat = PLC_ANSWER_OK;
static uint32_t         plcvm_page = 0xFFFFFFFF;

static uint32_t         plcvm_sp = (EXTPLC_SIZEOF_RAM - 1); // Stack pointer
static uint32_t         plcvm_sfp = EXTPLC_SIZEOF_RAM;      // Stack frame pointer
static uint32_t         plcvm_pc = 0;                       // program counter
static uint8_t          plcvm_cop = 0;                      // operator

static uint32_t plcvm_get_mem(uint32_t addr)
{
    if(addr < EXTPLC_SIZEOF_RAM)
        return plc_ram[addr];

    plcvm_stat = PLC_ANSWER_ERROR_AV_RAM;
    return 0;
}

static void plcvm_set_mem(uint32_t addr, uint32_t val)
{
    if(addr < EXTPLC_SIZEOF_RAM)
        plc_ram[addr] = val;
    else
        plcvm_stat = PLC_ANSWER_ERROR_AV_RAM;
}

static uint8_t plcvm_get_cop(uint32_t pc)
{
    static uint8_t  plc_prg_cache[EXTPLC_SIZEOF_PRG_CACHE];
    
    if(pc >= EXTPLC_SIZEOF_PRG)
    {
        plcvm_stat = PLC_ANSWER_ERROR_AV_PRG;
        return 0xFF;
    }
    
    if((pc & ~(EXTPLC_SIZEOF_PRG_CACHE - 1UL)) != plcvm_page)
    {
        plcvm_page = (pc & ~(EXTPLC_SIZEOF_PRG_CACHE - 1UL));
        eeprom_read(plc_prg_cache, eePLCprogram + plcvm_page, EXTPLC_SIZEOF_PRG_CACHE);
    }
    
    return plc_prg_cache[pc & (EXTPLC_SIZEOF_PRG_CACHE - 1UL)];
}

// Get uint32_t from program memory
static uint32_t plcvm_get_u32_pPC(void)
{
    uint32_t acc = plcvm_get_cop(plcvm_pc++);
    acc |= (uint32_t)(plcvm_get_cop(plcvm_pc++))<<8;
    acc |= (uint32_t)(plcvm_get_cop(plcvm_pc++))<<16;
    acc |= (uint32_t)(plcvm_get_cop(plcvm_pc++))<<24;
    return acc;
}


// Operators callback

static void cb_nop(void){}
static void cb_not(void){plcvm_set_mem(plcvm_sp, ~plcvm_get_mem(plcvm_sp));}
static void cb_inc(void){plcvm_set_mem(plcvm_sp, plcvm_get_mem(plcvm_sp) + 1);}
static void cb_dec(void){plcvm_set_mem(plcvm_sp, plcvm_get_mem(plcvm_sp) - 1);}
static void cb_neg(void){plcvm_set_mem(plcvm_sp, -plcvm_get_mem(plcvm_sp));}

// Load from memory
static void cb_ldm__s(void)
{
    uint8_t type = plcvm_cop & 7;
    uint32_t acc;
    uint32_t addr = plcvm_get_mem(plcvm_sp);
    if(type == 1)    // Bool
    {
        acc = plcvm_get_mem(addr >> 5);
        acc = ((acc & (1UL<<(addr & 0x1F))) != 0);
    }
    else if((type == 2) || (type == 3))     // [u]int8
    {
        acc = plcvm_get_mem(addr >> 2);
        acc = acc >> ((addr & 0x03) * 8);
                    
        if((type == 2) && (acc & 0x80))
            acc |= 0xFFFFFF00;
        else
            acc &= 0x000000FF;
    }
    else if((type == 4) || (type == 5))
    {
        acc = plcvm_get_mem(addr >> 1);
        acc = acc >> ((addr & 0x01) * 8);
                    
        if((type == 4) && (acc & 0x8000))
            acc |= 0xFFFF0000;
        else
            acc &= 0x0000FFFF;
    }
    else
        acc = plcvm_get_mem(addr);

    plcvm_set_mem(plcvm_sp, acc);
}

static void cb_ldm__cs8(void)
{
    uint8_t type = plcvm_cop & 7;
    uint32_t acc;
    uint32_t addr = plcvm_get_mem(plcvm_sp) + plcvm_get_cop(plcvm_pc++);

    if(type == 1)    // Bool
    {
        acc = plcvm_get_mem(addr >> 5);
        acc = ((acc & (1UL<<(addr & 0x1F))) != 0);
    }
    else if((type == 2) || (type == 3))     // [u]int8
    {
        acc = plcvm_get_mem(addr >> 2);
        acc = acc >> ((addr & 0x03) * 8);
                    
        if((type == 2) && (acc & 0x80))
            acc |= 0xFFFFFF00;
        else
            acc &= 0x000000FF;
    }
    else if((type == 4) || (type == 5))
    {
        acc = plcvm_get_mem(addr >> 1);
        acc = acc >> ((addr & 0x01) * 8);
                    
        if((type == 4) && (acc & 0x8000))
            acc |= 0xFFFF0000;
        else
            acc &= 0x0000FFFF;
    }
    else
        acc = plcvm_get_mem(addr);

    plcvm_set_mem(plcvm_sp, acc);
}

static void cb_ldm__cs16(void)
{
    uint8_t type = plcvm_cop & 7;
    uint32_t acc;
    
    uint16_t offs_pc = plcvm_get_cop(plcvm_pc++);
    offs_pc |= (uint16_t)(plcvm_get_cop(plcvm_pc++))<<8;
    uint32_t addr = plcvm_get_mem(plcvm_sp) + offs_pc;

    if(type == 1)    // Bool
    {
        acc = plcvm_get_mem(addr >> 5);
        acc = ((acc & (1UL<<(addr & 0x1F))) != 0);
    }
    else if((type == 2) || (type == 3))     // [u]int8
    {
        acc = plcvm_get_mem(addr >> 2);
        acc = acc >> ((addr & 0x03) * 8);
                    
        if((type == 2) && (acc & 0x80))
            acc |= 0xFFFFFF00;
        else
            acc &= 0x000000FF;
    }
    else if((type == 4) || (type == 5))
    {
        acc = plcvm_get_mem(addr >> 1);
        acc = acc >> ((addr & 0x01) * 8);
                    
        if((type == 4) && (acc & 0x8000))
            acc |= 0xFFFF0000;
        else
            acc &= 0x0000FFFF;
    }
    else
        acc = plcvm_get_mem(addr);

    plcvm_set_mem(plcvm_sp, acc);
}

static void cb_ldm__c16(void)
{
    uint8_t type = plcvm_cop & 7;
    uint32_t acc;
    
    uint16_t addr = plcvm_get_cop(plcvm_pc++);
    addr |= (uint16_t)(plcvm_get_cop(plcvm_pc++))<<8;
    plcvm_sp--;

    if(type == 1)    // Bool
    {
        acc = plcvm_get_mem(addr >> 5);
        acc = ((acc & (1UL<<(addr & 0x1F))) != 0);
    }
    else if((type == 2) || (type == 3))     // [u]int8
    {
        acc = plcvm_get_mem(addr >> 2);
        acc = acc >> ((addr & 0x03) * 8);
                    
        if((type == 2) && (acc & 0x80))
            acc |= 0xFFFFFF00;
        else
            acc &= 0x000000FF;
    }
    else if((type == 4) || (type == 5))
    {
        acc = plcvm_get_mem(addr >> 1);
        acc = acc >> ((addr & 0x01) * 8);
                    
        if((type == 4) && (acc & 0x8000))
            acc |= 0xFFFF0000;
        else
            acc &= 0x0000FFFF;
    }
    else
        acc = plcvm_get_mem(addr);

    plcvm_set_mem(plcvm_sp, acc);
}

// Load constant
static void cb_ldi_zero(void){plcvm_set_mem(--plcvm_sp, 0);}
static void cb_ldi_true(void){plcvm_set_mem(--plcvm_sp, 1);}
static void cb_ldi_minus1(void){plcvm_set_mem(--plcvm_sp, -1);}
static void cb_ldi_s1(void)
{
    uint32_t acc = plcvm_get_cop(plcvm_pc++);
    if(acc & 0x80)
    {
        acc |= 0xFFFFFF00;
    }
    plcvm_set_mem(--plcvm_sp, acc);
}
static void cb_ldi_u1(void){plcvm_set_mem(--plcvm_sp, plcvm_get_cop(plcvm_pc++));}
static void cb_ldi_s2(void)
{
    uint32_t acc = plcvm_get_cop(plcvm_pc++);
    acc |= (uint16_t)(plcvm_get_cop(plcvm_pc++))<<8;
    if(acc & 0x8000)
    {
        acc |= 0xFFF00F00;
    }
    plcvm_set_mem(--plcvm_sp, acc);
}
static void cb_ldi_u2(void)
{
    uint32_t acc = (uint16_t)(plcvm_get_cop(plcvm_pc++));
    acc	|=  (uint16_t)(plcvm_get_cop(plcvm_pc++))<<8;
    plcvm_set_mem(--plcvm_sp, acc);
}
static void cb_ldi_s4(void){plcvm_set_mem(--plcvm_sp, plcvm_get_u32_pPC());}


// Store in memory
static void cb_stm__s(void)
{
    uint8_t type = plcvm_cop & 7;
    uint32_t addr = plcvm_get_mem(plcvm_sp++);
    uint32_t acc  = plcvm_get_mem(plcvm_sp++);
    uint32_t bcc;
    uint32_t mask;

    if(type == 1)                               // bool
    {
        mask = 1UL<<(addr & 0x1F);
        addr >>= 5;
        bcc = plcvm_get_mem(addr);
        if(acc != 0)
            bcc |= mask;
        else
            bcc &= ~mask;
        plcvm_set_mem(addr, bcc);
    }
    else if((type == 2) || (type == 3))
    {
        acc &= 0x000000FF;
        acc <<= (addr & 3)*8;
        mask = 0x000000FF << ((addr & 3)*8);
                
        addr >>= 2;
        bcc = plcvm_get_mem(addr);
        bcc &= ~mask;
        bcc |= acc;
        plcvm_set_mem(addr, bcc);
    }
    else if((type == 4) || (type == 5))
    {
        acc &= 0x0000FFFF;
        acc <<= (addr & 1)*16;
        mask = 0x0000FFFF << ((addr & 1)*16);

        addr >>= 1;
        bcc = plcvm_get_mem(addr);
        bcc &= ~mask;
        bcc |= acc;
        plcvm_set_mem(addr, bcc);
    }
    else // if(type == 6)
    {
        plcvm_set_mem(addr, acc);
    }
}

static void cb_stm_cs8(void)
{
    uint8_t type = plcvm_cop & 7;
    uint32_t addr = plcvm_get_mem(plcvm_sp++) + plcvm_get_cop(plcvm_pc++);
    uint32_t acc  = plcvm_get_mem(plcvm_sp++);
    uint32_t bcc;
    uint32_t mask;

    if(type == 1)                               // bool
    {
        mask = 1UL<<(addr & 0x1F);
        addr >>= 5;
        bcc = plcvm_get_mem(addr);
        if(acc != 0)
            bcc |= mask;
        else
            bcc &= ~mask;
        plcvm_set_mem(addr, bcc);
    }
    else if((type == 2) || (type == 3))
    {
        acc &= 0x000000FF;
        acc <<= (addr & 3)*8;
        mask = 0x000000FF << ((addr & 3)*8);
                
        addr >>= 2;
        bcc = plcvm_get_mem(addr);
        bcc &= ~mask;
        bcc |= acc;
        plcvm_set_mem(addr, bcc);
    }
    else if((type == 4) || (type == 5))
    {
        acc &= 0x0000FFFF;
        acc <<= (addr & 1)*16;
        mask = 0x0000FFFF << ((addr & 1)*16);

        addr >>= 1;
        bcc = plcvm_get_mem(addr);
        bcc &= ~mask;
        bcc |= acc;
        plcvm_set_mem(addr, bcc);
    }
    else // if(type == 6)
    {
        plcvm_set_mem(addr, acc);
    }
}

static void cb_stm_cs16(void)
{
    uint8_t type = plcvm_cop & 7;
    uint16_t offs = (uint16_t)plcvm_get_cop(plcvm_pc++);
    	    offs |= (uint16_t)(plcvm_get_cop(plcvm_pc++))<<8;
    uint32_t addr = plcvm_get_mem(plcvm_sp++) + offs;
    uint32_t acc  = plcvm_get_mem(plcvm_sp++);
    uint32_t bcc;
    uint32_t mask;

    if(type == 1)                               // bool
    {
        mask = 1UL<<(addr & 0x1F);
        addr >>= 5;
        bcc = plcvm_get_mem(addr);
        if(acc != 0)
            bcc |= mask;
        else
            bcc &= ~mask;
        plcvm_set_mem(addr, bcc);
    }
    else if((type == 2) || (type == 3))
    {
        acc &= 0x000000FF;
        acc <<= (addr & 3)*8;
        mask = 0x000000FF << ((addr & 3)*8);
                
        addr >>= 2;
        bcc = plcvm_get_mem(addr);
        bcc &= ~mask;
        bcc |= acc;
        plcvm_set_mem(addr, bcc);
    }
    else if((type == 4) || (type == 5))
    {
        acc &= 0x0000FFFF;
        acc <<= (addr & 1)*16;
        mask = 0x0000FFFF << ((addr & 1)*16);

        addr >>= 1;
        bcc = plcvm_get_mem(addr);
        bcc &= ~mask;
        bcc |= acc;
        plcvm_set_mem(addr, bcc);
    }
    else // if(type == 6)
    {
        plcvm_set_mem(addr, acc);
    }
}

static void cb_stm_c16(void)
{
    uint8_t type = plcvm_cop & 7;
    uint16_t addr = plcvm_get_cop(plcvm_pc++);
    addr |= (uint16_t)(plcvm_get_cop(plcvm_pc++))<<8;
    uint32_t acc  = plcvm_get_mem(plcvm_sp++);
    uint32_t bcc;
    uint32_t mask;

    if(type == 1)                               // bool
    {
        mask = 1UL<<(addr & 0x1F);
        addr >>= 5;
        bcc = plcvm_get_mem(addr);
        if(acc != 0)
            bcc |= mask;
        else
            bcc &= ~mask;
        plcvm_set_mem(addr, bcc);
    }
    else if((type == 2) || (type == 3))
    {
        acc &= 0x000000FF;
        acc <<= (addr & 3)*8;
        mask = 0x000000FF << ((addr & 3)*8);
                
        addr >>= 2;
        bcc = plcvm_get_mem(addr);
        bcc &= ~mask;
        bcc |= acc;
        plcvm_set_mem(addr, bcc);
    }
    else if((type == 4) || (type == 5))
    {
        acc &= 0x0000FFFF;
        acc <<= (addr & 1)*16;
        mask = 0x0000FFFF << ((addr & 1)*16);

        addr >>= 1;
        bcc = plcvm_get_mem(addr);
        bcc &= ~mask;
        bcc |= acc;
        plcvm_set_mem(addr, bcc);
    }
    else // if(type == 6)
    {
        plcvm_set_mem(addr, acc);
    }
}


static const cbCOP_t plcmv_cb[] =
{
    // 0x00 - 0x1F
    
    cb_nop,     NULL,       NULL,       NULL,       NULL,       NULL,       NULL,       NULL,
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

    while(plcvm_stat == PLC_ANSWER_OK)
    {
        plcvm_cop = plcvm_get_cop(plcvm_pc++);

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
                    plcvm_sfp = plcvm_get_mem(plcvm_sp++);
                    plcvm_pc  = plcvm_get_mem(plcvm_sp++);
                    break;

                case 0xFE:                              // TEST
                    if(plcvm_get_mem(plcvm_sp) != plcvm_get_u32_pPC())
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
