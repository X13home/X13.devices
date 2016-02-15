// EEPROM emulation for STM32F0/STM32F1

#include "config.h"

#ifdef FEE_SECTORS

// If defined FEE_BASE, data allocated in the FLASH from address: FEE_BASE
// User MUST check, that:
//  data aligned to page
//  enough FLASH
//  data not overlapped with program memory

#ifndef FEE_BASE
// Data allocated in program memory
//static const uint8_t fee_data[] __attribute__ ((aligned(FEE_SECTOR_SIZE))) = { [0 ... (FEE_SECTORS * FEE_SECTOR_SIZE - 1)] = 0xFF};

// Data allocated in program memory, but counted in bss Size, section ".feedata" must be defined in linker script
/*// EEPROM Emulation  section
  .feedata ALIGN(0x0400) (NOLOAD) :
  {
    *(.feedata)
    *(.feedata*)
  } >FLASH
*/
static const uint8_t fee_data[FEE_SECTORS * FEE_SECTOR_SIZE] __attribute__ ((section (".feedata")));

#define FEE_BASE    (uint32_t)fee_data

#endif  //  FEE_BASE

static uint8_t fee_pages_map[FEE_SECTORS];

// HAL Flash
static void flash_clear_flags(void)
{
#if (defined STM32F0)
    FLASH->SR = (FLASH_SR_PGERR | FLASH_SR_WRPERR | FLASH_SR_EOP);
#elif (defined STM32F1)
    FLASH->SR = (FLASH_SR_PGERR | FLASH_SR_WRPRTERR | FLASH_SR_EOP);
#else
#error flash_clear_flags Unknown uC
#endif
}

static void flash_unlock(void)
{
    if((FLASH->CR & FLASH_CR_LOCK) != RESET)
    {
        // Unlocking the program memory access
        FLASH->KEYR = FLASH_KEY1;
        FLASH->KEYR = FLASH_KEY2;
    }
}

static bool flash_wait(uint32_t Timeout)
{
    // Wait for a FLASH operation to complete or a TIMEOUT to occur
    while(FLASH->SR & FLASH_SR_BSY)
    {
        if(Timeout > 0)
            Timeout--;
        else
            return false;
    }

    return ((FLASH->SR & (FLASH_SR_WRPRTERR | FLASH_SR_PGERR)) == 0);
}

static bool flash_erase_page(uint32_t Page_Address)
{
    bool status = flash_wait(FEE_EraseTimeout);
    if(status)
    {
        // If the previous operation is completed, proceed to erase the page
        FLASH->CR |= FLASH_CR_PER;
        FLASH->AR  = Page_Address & ~(FEE_SECTOR_SIZE - 1);
        FLASH->CR |= FLASH_CR_STRT;
    
        // Wait for last operation to be completed
        status = flash_wait(FEE_EraseTimeout);
    
        // Disable the PER Bit
        FLASH->CR &= ~FLASH_CR_PER;
    }

    return status;
}

// Programm Half Word
static bool flash_programm(uint32_t Address, uint16_t Data)
{
    bool status = flash_wait(FEE_ProgramTimeout);
    if(status)
    {
        // If the previous operation is completed, proceed to program the new data
        FLASH->CR |= FLASH_CR_PG;

        *(__IO uint16_t*)Address = Data;

        // Wait for last operation to be completed
        status = flash_wait(FEE_ProgramTimeout);
    
        // Disable the PG Bit */
        FLASH->CR &= ~FLASH_CR_PG;
        
        if(*(__IO uint16_t*)Address != Data)
            status = false;
    }

    return status;
}
// End HAL Flash

// Convert virtual EEPROM address to physical FLASH address
static uint32_t fee_virt2real(uint32_t addr)
{
    if(addr >= ((FEE_SECTORS - FEE_TRANSIT_SECTORS)*(FEE_SECTOR_SIZE - 2)))
        while(1);                                       // Error

    uint16_t page = 0;
    addr += 2;
    while(addr >= FEE_SECTOR_SIZE)
    {
        addr -= (FEE_SECTOR_SIZE - 2);
        page++;
    }
    addr += (uint32_t)FEE_SECTOR_SIZE * fee_pages_map[page];
    addr += FEE_BASE;
    return addr;
}

// write data to FLASH, only within 1 page.
static bool fee_write_page(uint32_t fee_addr, uint32_t len, uint8_t *pData)
{
    uint16_t din, dout;

    while(len > 0)
    {
        if(fee_addr & 1)
        {
            len++;
            fee_addr--;
            din = ((*(pData++))<<8);
            dout = *(uint16_t *)(fee_addr);
            din |= dout & 0x00FF;
        }
        else if(len > 1)
        {
            din  = *(pData++);
            din |= ((*(pData++))<<8);
            dout = *(uint16_t *)(fee_addr);
        }
        else  // len == 1
        {
            din = *pData;
            dout = *(uint16_t *)(fee_addr);
            din |= dout & 0xFF00;
            len = 2;
        }

        if(din != dout)
        {
            if(dout != 0xFFFF)
                return false;

            if(flash_programm(fee_addr, din) != true)
            {
                flash_clear_flags();
                return false;
            }
        }
        fee_addr += 2;
        len -= 2;
    }

    return true;
}

// Transfer data from old to new sector
static bool fee_transfer(uint32_t fee_addr, uint32_t len, uint8_t *pData)
{
    uint32_t src_sddr = fee_addr & ~(FEE_SECTOR_SIZE - 1);
    uint32_t dst_addr = (fee_pages_map[FEE_SECTORS - FEE_TRANSIT_SECTORS] * FEE_SECTOR_SIZE) + FEE_BASE;

    // Read and update version
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
        return false;
    }

    src_sddr += 2;
    dst_addr += 2;
    uint32_t dst_len = fee_addr - src_sddr;

    uint16_t data;

    // Transfer Head
    while(dst_len > 1)
    {
        data = *(uint16_t *)(src_sddr);
        if(data != 0xFFFF)
        {
            if(flash_programm(dst_addr, data) != true)
            {
                flash_clear_flags();
                return false;
            }
        }

        src_sddr += 2;
        dst_addr += 2;
        dst_len  -= 2;
    }

    // Write Body
    if(dst_len)
    {
        data = *(uint8_t *)(src_sddr);
        data |= (*(pData++))<<8;

        if(data != 0xFFFF)
        {
            if(flash_programm(dst_addr, data) != true)
            {
                flash_clear_flags();
                return false;
            }
        }

        src_sddr += 2;
        dst_addr += 2;
        len--;
    }

    while(len > 1)
    {
        data = *(pData++);
        data |= (*(pData++))<<8;
        
        if(data != 0xFFFF)
        {
            if(flash_programm(dst_addr, data) != true)
            {
                flash_clear_flags();
                return false;
            }
        }

        src_sddr += 2;
        dst_addr += 2;
        len -= 2;
    }

    // Transfer last odd byte
    if(len)
    {
        data = *(uint16_t *)(src_sddr);
        data |= *pData;

        if(data != 0xFFFF)
        {
            if(flash_programm(dst_addr, data) != true)
            {
                flash_clear_flags();
                return false;
            }
        }
        src_sddr += 2;
        dst_addr += 2;
    }

    // Transfer Tail
    dst_len = FEE_SECTOR_SIZE - (dst_addr & (FEE_SECTOR_SIZE - 1));
    while(dst_len > 1)
    {
        data = *(uint16_t *)(src_sddr);
        if(data != 0xFFFF)
        {
            if(flash_programm(dst_addr, data) != true)
            {
                flash_clear_flags();
                return false;
            }
        }

        src_sddr += 2;
        dst_addr += 2;
        dst_len  -= 2;
    }

    // Erase Old Page
    if(!flash_erase_page(fee_addr))
        return false;

    // Rotate free sectors    
    uint8_t sector;
    for(sector = (FEE_SECTORS - FEE_TRANSIT_SECTORS); sector < (FEE_SECTORS - 1); sector++)
        fee_pages_map[sector] = fee_pages_map[sector + 1];
    fee_pages_map[FEE_SECTORS - 1] = (fee_addr & (FEE_SECTORS * FEE_SECTOR_SIZE - 1))/FEE_SECTOR_SIZE;

    return true;
}

static bool fee_check_blank(uint32_t addr)
{
    addr &= ~(uint32_t)(FEE_SECTOR_SIZE - 1);
    uint32_t pos;
    for(pos = 0; pos < FEE_SECTOR_SIZE; pos += 2)
    {
        if(*(uint16_t *)(addr) != 0xFFFF)
            return false;
        
        addr += 2;
    }
    return true;
}

/////////////////////////////
// API

// Initialise EEPROM hardware and allocate sectors
void eeprom_init_hw(void)
{
    // unlock flash
    flash_unlock();

    // Clear Flags
    flash_clear_flags();

    uint8_t  sector, page;
    uint16_t info;
    uint16_t pginfo[FEE_SECTORS];
    uint32_t addr = FEE_BASE;

    // Read sectors info and clear mapping
    for(sector = 0; sector < FEE_SECTORS; sector++)
    {
        fee_pages_map[sector] = 0xFF;                       // Clear mapping info
        
        info = *(uint16_t *)(addr);                         // Read page info, LSB Sector, MSB Version

        if(((info & 0xFF00) == 0) ||                        // Bad Page Info
           ((info & 0x00FF) >= FEE_SECTORS) ||
           ((info == 0xFFFF) && !fee_check_blank(addr)))    // Page marked as blank, but it's not blank
        {
            if(!flash_erase_page(addr))
                while(1);                                   // FLASH Error, HALT !
            info = 0xFFFF;
        }

        pginfo[sector] = info;
        addr += FEE_SECTOR_SIZE;
    }

    // Allocate Used Sectors
    for(sector = 0; sector < (FEE_SECTORS - FEE_TRANSIT_SECTORS); sector++)
    {
        info = 0xFFFF;

        for(page = 0; page < FEE_SECTORS; page++)
        {
            uint16_t actInf = pginfo[page];

            if((actInf & 0xFF) == sector)
            {
                if(info == 0xFFFF)
                {
                    info = (actInf & 0xFF00) | page;
                }
                else                                        // Transfer Error, use old Page, erase new Page
                {
                    uint16_t vers1 = info >> 8;
                    uint16_t vers2 = actInf >> 8;
                    
                    if(vers1 == 0xFF)
                        vers1 = 0;
                    if(vers2 == 0xFF)
                        vers2 = 0;

                    addr = FEE_BASE;
                    
                    if(vers2 > vers1)
                    {
                        addr += page * FEE_SECTOR_SIZE;
                        pginfo[page] = 0xFFFF;
                    }
                    else
                    {
                        uint8_t pg = info & 0xFF;
                        addr += pg * FEE_SECTOR_SIZE;
                        pginfo[pg] = 0xFFFF;
                    }
                    
                    if(!flash_erase_page(addr))
                        while(1);                   // FLASH Error, HALT !
                }
            }
        }

        if(info == 0xFFFF)                          // Page not allocated
        {
            for(page = 0; page < FEE_SECTORS; page++)
            {
                if(pginfo[page] == 0xFFFF)          // Allocate sector
                {
                    info = 0x0100 | sector;         // 1st Version
                    pginfo[page] = info;

                    addr = FEE_BASE;
                    addr += page * FEE_SECTOR_SIZE;
                    if(!flash_programm(addr, info))
                        while(1);                   // FLASH Error, HALT !

                    info = page;
                    break;
                }
            }
        }

        fee_pages_map[sector] = info & 0xFF;
    }

    // Allocate Free Sectors
    for(;sector < FEE_SECTORS; sector++)
    {
        for(page = 0; page < FEE_SECTORS; page++)
        {
            if(pginfo[page] == 0xFFFF)
            {
                pginfo[page] = sector;
                fee_pages_map[sector] = page;
                break;
            }
        }
    }
}

// Read data from EEPROM
void eeprom_read(uint8_t *pBuf, uint32_t Addr, uint32_t Len)
{
    uint32_t fee_addr = 0;

    while(Len > 0)
    {
        if((fee_addr & (const uint32_t)(FEE_SECTOR_SIZE - 1)) == 0)
            fee_addr = fee_virt2real(Addr);

        *(pBuf++) = *(uint8_t *)(fee_addr);
        fee_addr++;
        Addr++;
        Len--;
    }
}

// Write data to EEPROM
void eeprom_write(uint8_t *pBuf, uint32_t Addr, uint32_t Len)
{
    while(Len > 0)
    {
        uint32_t fee_addr = fee_virt2real(Addr);
        uint32_t length = FEE_SECTOR_SIZE - (fee_addr & (FEE_SECTOR_SIZE - 1));
        if(Len < length)
            length = Len;

        if(fee_write_page(fee_addr, length, pBuf))
        {
            Len -= length;
            Addr += length;
            continue;
        }
        
        if(!fee_transfer(fee_addr, length, pBuf))
            while(1);                               // FLASH Error, HALT !
        
        Len -= length;
        Addr += length;
    }
}

#endif  //  FEE_SECTORS
