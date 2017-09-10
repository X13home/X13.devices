/*
Copyright (c) 2011-2016 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// EEPROM emulation for STM32F0/STM32F1/STM32F3

/**************************************************************************************************/
// Data placed in the FLASH from address: FEE_BASE
// User MUST check, that:
//  data aligned to page
//  enough FLASH
//  data not overlapped with program memory

/**************************************************************************************************/
// Data allocated in program memory
//static const uint8_t fee_data[] __attribute__ ((aligned(FEE_PAGE_SIZE))) =
//            { [0 ... (FEE_SIZE - 1)] = 0xFF};
//#define FEE_BASE    (uint32_t)fee_data

/**************************************************************************************************/
// Data allocated in program memory, but counted in bss Size,
//       section ".feedata" must be defined in linker script
// EEPROM Emulation  section

//MEMORY
//{
//FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = 56K
//FEEPROM (r)     : ORIGIN = 0x0800E000, LENGTH = 8K
//RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 8K
//}
//
//SECTIONS
//{
//  .feedata ALIGN(0x0400) (NOLOAD) :
//  {
//    *(.feedata)
//    *(.feedata*)
//  } >FEEPROM
//}
//static const uint8_t fee_data[FEE_SIZE] __attribute__ ((section (".feedata")));
//#define FEE_BASE    (uint32_t)fee_data

#include "config.h"

#ifdef FEE_SIZE

#define FEE_AVAIL_PAGES (uint8_t)((FEE_SIZE/FEE_PAGE_SIZE) - FEE_TRANSFER_PAGES)

// HAL FLASH
static void flash_clear_flags(void)
{
    FLASH->SR = (FLASH_SR_PGERR | FLASH_SR_WRPRTERR | FLASH_SR_EOP);
}

static bool flash_wait(uint32_t Timeout)
{
    // Wait for a FLASH operation to complete or a TIMEOUT to occur
    while(FLASH->SR & FLASH_SR_BSY)
    {
        if(Timeout > 0)
        {
            Timeout--;
        }
        else
        {
            return false;
        }
    }

    return ((FLASH->SR & (FLASH_SR_WRPRTERR | FLASH_SR_PGERR)) == 0);
}

static bool flash_erase_page(uint32_t Page_Address)
{
    bool status = flash_wait(FEE_EraseTimeout);
    if(status)
    {
        halEnterCritical();

        // Unlocking the program memory access
        FLASH->KEYR = FLASH_KEY1;
        FLASH->KEYR = FLASH_KEY2;

        // If the previous operation is completed, proceed to erase the page
        FLASH->CR |= FLASH_CR_PER;
        FLASH->AR  = Page_Address & ~(FEE_PAGE_SIZE - 1);
        FLASH->CR |= FLASH_CR_STRT;

        // Wait for last operation to be completed
        status = flash_wait(FEE_EraseTimeout);

        // Disable the PER Bit
        FLASH->CR &= ~FLASH_CR_PER;

        // Lock the program memory access
        FLASH->CR |= FLASH_CR_LOCK;

        halLeaveCritical();
    }
    return status;
}

// Programm Half Word
static bool flash_programm(uint32_t Address, uint16_t Data)
{
    bool status = flash_wait(FEE_ProgramTimeout);
    if(status)
    {
        halEnterCritical();

        // Unlocking the program memory access
        FLASH->KEYR = FLASH_KEY1;
        FLASH->KEYR = FLASH_KEY2;

        // If the previous operation is completed, proceed to program the new data
        FLASH->CR |= FLASH_CR_PG;

        *(__IO uint16_t*)Address = Data;

        // Wait for last operation to be completed
        status = flash_wait(FEE_ProgramTimeout);

        // Disable the PG Bit */
        FLASH->CR &= ~FLASH_CR_PG;

        if(*(__IO uint16_t*)Address != Data)
        {
            status = false;
        }

        // Lock the program memory access
        FLASH->CR |= FLASH_CR_LOCK;

        halLeaveCritical();
    }
    return status;
}
// End HAL Flash

// write data to FLASH, only within 1 page.
static bool fee_write_page(uint32_t fee_addr, uint32_t len, uint8_t *pData)
{
    while(len > 0)
    {
        uint16_t din  = *(pData++) | 0xFF00;
        uint16_t dout = *(uint16_t *)(fee_addr);

        if(din != dout)
        {
            if(dout != 0xFFFF)
            {
                return false;
            }

            if(flash_programm(fee_addr, din) != true)
            {
                flash_clear_flags();
                return false;
            }
        }
        fee_addr += 2;
        len -= 1;
    }

    return true;
}

// Transfer Page from old to new place

static uint32_t fee_free_page = FEE_BASE;

static bool fee_transfer(uint32_t fee_addr, uint32_t len)
{
    uint32_t dst_addr;
    uint16_t din, dout;

fee_transfer_rep1:

    if((fee_free_page < FEE_BASE) ||
       (fee_free_page >= (FEE_BASE + FEE_SIZE)) ||
      ((fee_free_page & (FEE_PAGE_SIZE - 1)) != 0))
    {
        fee_free_page = FEE_BASE;
    }

    dst_addr = 0;

    uint8_t page;
    for(page = 0; page < (FEE_SIZE/FEE_PAGE_SIZE); page++)
    {
        fee_free_page += FEE_PAGE_SIZE;

        if(fee_free_page >= (FEE_BASE + FEE_SIZE))
        {
            fee_free_page = FEE_BASE;
        }

        if(*(uint16_t *)(fee_free_page) == 0xFFFF)
        {
            dst_addr = fee_free_page;
            break;
        }
    }

    if(dst_addr == 0)                   // Error, no available free page
    {
        return false;
    }

    // Read and update version
    uint32_t src_sddr = fee_addr & ~(FEE_PAGE_SIZE - 1);

    uint16_t info = *(uint16_t *)(src_sddr);
    if(info < 0xFF00)
    {
        info += 0x100;
    }
    else
    {
        info &= 0x00FF;
        info |= 0x0100;
    }

    // Write header to new page
    if(flash_programm(dst_addr, info) != true)
    {
        flash_clear_flags();
        flash_erase_page(dst_addr);
        goto fee_transfer_rep1;
    }
    src_sddr += 2;
    dst_addr += 2;

    // Copy Head
    while(src_sddr < fee_addr)
    {
        din  = *(uint16_t *)(src_sddr);
        dout = *(uint16_t *)(dst_addr);
        if(din != dout)
        {
            if((dout != 0xFFFF) ||
               (flash_programm(dst_addr, *(uint16_t *)(src_sddr)) != true))
            {
                flash_clear_flags();
                dst_addr &= ~(FEE_PAGE_SIZE - 1);
                flash_erase_page(dst_addr);
                goto fee_transfer_rep1;
            }
        }

        src_sddr += 2;
        dst_addr += 2;
    }

    // Copy Tail
    src_sddr += len * 2;
    dst_addr += len * 2;
    len = (~src_sddr & (FEE_PAGE_SIZE - 1)) + 1;
    while(len > 0)
    {
        din  = *(uint16_t *)(src_sddr);
        dout = *(uint16_t *)(dst_addr);
        if(din != dout)
        {
            if((dout != 0xFFFF) ||
               (flash_programm(dst_addr, *(uint16_t *)(src_sddr)) != true))
            {
                flash_clear_flags();
                dst_addr &= ~(FEE_PAGE_SIZE - 1);
                flash_erase_page(dst_addr);
                goto fee_transfer_rep1;
            }
        }

        src_sddr += 2;
        dst_addr += 2;
        len -= 2;
    }

    flash_erase_page(fee_addr);
    return true;
}

static bool fee_check_blank(uint32_t addr)
{
    addr &= ~(uint32_t)(FEE_PAGE_SIZE - 1);
    uint32_t pos;
    for(pos = 0; pos < FEE_PAGE_SIZE; pos += 4)
    {
        if(*(uint32_t *)(addr) != 0xFFFFFFFF)
        {
            return false;
        }
        addr += 4;
    }
    return true;
}

// Convert virtual EEPROM address to physical FLASH address
static uint32_t fee_virt2real(uint16_t vAddr)
{
    uint32_t addr = vAddr;

    addr++;
    addr *= 2;

    uint8_t page = 0;
    while(addr >= FEE_PAGE_SIZE)
    {
        addr -= (FEE_PAGE_SIZE - 2);
        page++;
        if(page >= FEE_AVAIL_PAGES)
        {
            return 0;
        }
    }

    uint32_t base_addr;
    for(base_addr = FEE_BASE; base_addr < (FEE_BASE + FEE_SIZE); base_addr += FEE_PAGE_SIZE)
    {
        if(*(uint8_t *)(base_addr) == page)
        {
            addr += base_addr;
            return addr;
        }
    }

    // Allocate Page
    for(base_addr = FEE_BASE; base_addr < (FEE_BASE + FEE_SIZE); base_addr += FEE_PAGE_SIZE)
    {
        if(*(uint16_t *)(base_addr) == 0xFFFF)
        {
            if(flash_programm(base_addr, 0x0100 | page))
            {
                addr += base_addr;
                return addr;
            }
            flash_clear_flags();
            flash_erase_page(base_addr);    // Flash Write Error, erase Page
        }
    }

    return 0;
}

/////////////////////////////
// API

// Initialise EEPROM hardware and validate pages
void eeprom_init_hw(void)
{
    uint32_t addr = FEE_BASE;
    while(addr < (FEE_BASE + FEE_SIZE))
    {
        // Clear Flags
        flash_clear_flags();

        uint16_t info = *(uint16_t *)(addr);            // Read page Info, LSB page, MSB Version

        if(info == 0xFFFF)
        {
            if(fee_check_blank(addr) == false)          // Page marked as blank, but it's not blank
            {
                flash_erase_page(addr);
            }
        }
        else if(((info & 0xFF00) == 0) ||                   // Bad Version
                ((info & 0x00FF) >= FEE_AVAIL_PAGES))       // Bad Page Number
        {
            flash_erase_page(addr);
        }
        else
        {
            uint32_t addr1 = addr + FEE_PAGE_SIZE;
            while(addr1 < (FEE_BASE + FEE_SIZE))
            {
                uint16_t info1 = *(uint16_t *)(addr1);
                if((info1 & 0xFF) == (info & 0xFF))         // Transfer Error
                {
                    uint16_t vers1 = info >> 8;
                    uint16_t vers2 = info1 >> 8;

                    if(vers1 == 0xFF)
                    {
                        vers1 = 0;
                    }
                    if(vers2 == 0xFF)
                    {
                        vers2 = 0;
                    }

                    if(vers2 > vers1)                       // Erase New Version
                    {
                        flash_erase_page(addr1);
                    }
                    else
                    {
                        flash_erase_page(addr);
                    }
                }
                addr1 += FEE_PAGE_SIZE;
            }
        }
        addr += FEE_PAGE_SIZE;
    }
}

// Read data from EEPROM
void eeprom_read(uint8_t *pBuf, uint16_t Addr, uint8_t Len)
{
    uint32_t fee_addr = 0;
    while(Len > 0)
    {
        if((fee_addr & (const uint32_t)(FEE_PAGE_SIZE - 1)) == 0)
        {
            fee_addr = fee_virt2real(Addr);
        }

        if(fee_addr == 0)       // Page not allocated
        {
            while(Len > 0)
            {
                *(pBuf++) = 0xFF;
                Len--;
            }
            return;
        }
        else
        {
            *(pBuf++) = *(uint8_t *)(fee_addr);
            fee_addr += 2;
        }

        Addr++;
        Len--;
    }
}

// Write data to EEPROM
bool eeprom_write(uint8_t *pBuf, uint16_t Addr, uint8_t Len)
{
    while(Len > 0)
    {
        uint32_t fee_addr = fee_virt2real(Addr);
        if(fee_addr == 0)                           // Page not allocated
        {
            return false;
        }

        uint32_t length = (FEE_PAGE_SIZE - (fee_addr & (FEE_PAGE_SIZE - 1))) / 2;
        if(Len < length)
        {
            length = Len;
        }

        if(fee_write_page(fee_addr, length, pBuf))
        {
            Len -= length;
            Addr += length;
            pBuf += length;
            continue;
        }

        if(!fee_transfer(fee_addr, length))
        {
            return false;                           // Flash Error
        }
    }
    return true;
}

#endif  //  FEE_SIZE
