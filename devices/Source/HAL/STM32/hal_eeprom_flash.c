// EEPROM emulation for STM32F0/STM32F1

// Так как у STM'а по непонятной мне причине, нельзя записать в ячейку если состояние ячеики не равно 0xFFFF
// Даже если ну очень хочется, даже если только нолики по местам единичек, все равно нельзя.
//
// То вынужден использовать только четные адреса, 
// из за этого реальный размер EEPROM'а равен: FEE_EEPROM_SIZE / 2
// Выделенное пространство под EEPROM равно: FEE_EEPROM_SIZE * FEE_MIRROR_NUMBER
//
// Не забываем в самом кокосе, уменьшить размер доступной флэшки.
// Чтобы случайно не пересеклись собственно код и псевдоеепром.
//
// Секция IROM1, Size = oldSize - FEE_EEPROM_SIZE * FEE_MIRROR_NUMBER


#include "../../config.h"

#if ((defined STM32F051R8) || \
     (defined STM32F051C8) || \
     (defined STM32F051K8) || \
     (defined STM32F103R8))

#define FEE_BASE_ADDRESS            (uint32_t)0x0800F000    // Base for 'EEPROM'
#define FEE_SECTOR_SIZE             (uint32_t)0x00000400    // Size of FLASH Sectors

#define FEE_EEPROM_SIZE             (uint32_t)0x00000800    // Size of 'EEPROM'
#define FEE_MIRROR_NUMBER           (uint8_t)2

#elif ((defined STM32F103RB) || (defined STM32F103CB) || (defined STM32F100RB))

#define FEE_BASE_ADDRESS            (uint32_t)0x0801F000    // Base for 'EEPROM'
#define FEE_SECTOR_SIZE             (uint32_t)0x00000400    // Size of FLASH Sectors

#define FEE_EEPROM_SIZE             (uint32_t)0x00000800    // Size of 'EEPROM'
#define FEE_MIRROR_NUMBER           (uint8_t)2

#define FLASH_FLAG_WRPERR FLASH_FLAG_WRPRTERR

#elif (defined STM32F401RE)

#define FEE_BASE_ADDRESS            (uint32_t)0x08040000    // Base for 'EEPROM'
#define FEE_SECTOR_SIZE             (uint32_t)0x00020000    // Size of FLASH Sectors

#define FEE_EEPROM_SIZE             (uint32_t)0x00020000    // Size of 'EEPROM'
#define FEE_MIRROR_NUMBER           (uint8_t)2

#define FLASH_FLAG_PGERR            (FLASH_FLAG_OPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR)

#define FLASH_ErasePage(page)       FLASH_EraseSector(page, VoltageRange_3)

#endif

#define FEE_SECTORS               (uint16_t)(FEE_EEPROM_SIZE / FEE_SECTOR_SIZE)

typedef enum
{
  eFEE_PAGE_INVALID     = (uint16_t)0x0000,
  eFEE_PAGE_VALID       = (uint16_t)0xF0F0,
  eFEE_PAGE_ERASED      = (uint16_t)0xFFFF
}eFEE_STATUS;

//const uint8_t eeprom_arr[FEE_EEPROM_SIZE * FEE_MIRROR_NUMBER] __attribute__((section("IROM2"))) = {0xFF,};

static uint8_t fee_pages_map[FEE_SECTORS];

// Convert virtual EEPROM address to physical FLASH address
static uint32_t fee_get_fee_addr(uint32_t virt_addr)
{
  virt_addr *= 2;
  uint16_t page = virt_addr / (FEE_SECTOR_SIZE - 2);
  virt_addr += (page + 1)*2;

  if((virt_addr >= FEE_EEPROM_SIZE) || (fee_pages_map[page] == 0xFF))
    return 0;

  return FEE_BASE_ADDRESS + FEE_EEPROM_SIZE * fee_pages_map[page] + virt_addr;
}

// write data to FLASH, only within 1 page.
static bool fee_write_page(uint32_t fee_addr, uint32_t len, uint8_t *pData)
{
  uint16_t din, dout;

  for(; len > 0; len--)
  {
    din = (uint16_t)(*(pData++)) | 0xFF00;
    dout = *(uint16_t *)(fee_addr);

    if(din != dout)
    {
      if(dout != 0xFFFF)
        return false;

      if(FLASH_ProgramHalfWord(fee_addr, din) != FLASH_COMPLETE)
      {
        FLASH_ClearFlag(FLASH_FLAG_PGERR |FLASH_FLAG_WRPERR|FLASH_FLAG_EOP);
        return false;
      }
    }
    fee_addr += 2;
  }

  return true;
}

// transfer data from old page to new mirror
static bool fee_transfer_page(uint32_t fee_addr, uint32_t len)
{
  uint16_t page = ((fee_addr - FEE_BASE_ADDRESS) & (FEE_EEPROM_SIZE - 1)) / FEE_SECTOR_SIZE;
  uint8_t old_mirror = fee_pages_map[page];
  uint8_t new_mirror = old_mirror + 1;
  if(new_mirror == FEE_MIRROR_NUMBER)
    new_mirror = 0;
    
  bool fee_retry = false;
  uint32_t dst_addr, src_addr;
  
  uint16_t din, dout;
    
fee_retransfer:
  dst_addr = FEE_BASE_ADDRESS + FEE_EEPROM_SIZE * new_mirror + FEE_SECTOR_SIZE * page;
  src_addr = FEE_BASE_ADDRESS + FEE_EEPROM_SIZE * old_mirror + FEE_SECTOR_SIZE * page + 2;

  if((*(uint16_t *)(dst_addr)) != eFEE_PAGE_ERASED)
    FLASH_ErasePage(dst_addr);

  dst_addr += 2;

  // write head
  for(;src_addr < fee_addr; src_addr += 2)
  {
    din = (*(uint8_t *)src_addr) | 0xFF00;
    dout = *(uint16_t *)(dst_addr);
    
    if(din != dout)
    {
      if((dout != 0xFFFF) ||
         (FLASH_ProgramHalfWord(dst_addr, din) != FLASH_COMPLETE))
      {
        FLASH_ClearFlag(FLASH_FLAG_PGERR |FLASH_FLAG_WRPERR|FLASH_FLAG_EOP);
        if(fee_retry)
          return false;

        dst_addr &= ~(FEE_SECTOR_SIZE - 1);
        FLASH_ErasePage(dst_addr);
        fee_retry = true;    
        goto fee_retransfer;      
      }
    }

    dst_addr += 2;
  }
  
  // write tail
  src_addr += len* 2;
  dst_addr += len* 2;
  fee_addr |= (FEE_SECTOR_SIZE - 1);

  for(;src_addr < fee_addr; src_addr += 2)
  {
    din = (*(uint8_t *)src_addr) | 0xFF00;
    dout = *(uint16_t *)(dst_addr);
    
    if(din != dout)
    {
      if((dout != 0xFFFF) ||
         (FLASH_ProgramHalfWord(dst_addr, din) != FLASH_COMPLETE))
      {
        FLASH_ClearFlag(FLASH_FLAG_PGERR |FLASH_FLAG_WRPERR|FLASH_FLAG_EOP);
        if(fee_retry)
          return false;

        dst_addr &= ~(FEE_SECTOR_SIZE - 1);
        FLASH_ErasePage(dst_addr);
        fee_retry = true;    
        goto fee_retransfer;      
      }
    }

    dst_addr += 2;
  }

  dst_addr -= FEE_SECTOR_SIZE;
  src_addr -= FEE_SECTOR_SIZE;

  FLASH_ProgramHalfWord(dst_addr, eFEE_PAGE_VALID);
  FLASH_ErasePage(src_addr);
  fee_pages_map[page] = new_mirror;
  return true;
}

/////////////////////////////
// API

// Initialise EEPROM hardware
void eeprom_init_hw(void)
{
  // unlock flash
  FLASH_Unlock();
  // Clear Flags
  FLASH_ClearFlag(FLASH_FLAG_PGERR |FLASH_FLAG_WRPERR|FLASH_FLAG_EOP);
  
  uint8_t mirror;
  uint16_t page;
  uint32_t addr;
  
  // Read pages status
  eFEE_STATUS pages_status[FEE_MIRROR_NUMBER];
  eFEE_STATUS status;
  
  for(page = 0; page < FEE_SECTORS; page++)
  {
    fee_pages_map[page] = 0xFF;
    for(mirror = 0; mirror < FEE_MIRROR_NUMBER; mirror++)
    {
      addr = FEE_BASE_ADDRESS + FEE_EEPROM_SIZE * mirror + FEE_SECTOR_SIZE * page;
      status = *(uint16_t *)(addr);
      pages_status[mirror] = status;
      
      if((status == eFEE_PAGE_VALID) && (fee_pages_map[page] == 0xFF))
      {
        fee_pages_map[page] = mirror;
      }
      else if(status != eFEE_PAGE_ERASED)
      {
        if(FLASH_ErasePage(addr) == FLASH_COMPLETE)
          pages_status[mirror] = eFEE_PAGE_ERASED;
        else
        {
          FLASH_ClearFlag(FLASH_FLAG_PGERR |FLASH_FLAG_WRPERR|FLASH_FLAG_EOP);
          pages_status[mirror] = eFEE_PAGE_INVALID;
        }
      }
    }

    if(fee_pages_map[page] == 0xFF) //  No Valid mirror
      for(mirror = 0; mirror < FEE_MIRROR_NUMBER; mirror++)
      {
        if(pages_status[mirror] == eFEE_PAGE_ERASED)
        {
          addr = FEE_BASE_ADDRESS + FEE_EEPROM_SIZE * mirror + FEE_SECTOR_SIZE * page;
          if(FLASH_ProgramHalfWord(addr, eFEE_PAGE_VALID) == FLASH_COMPLETE)
          {
            fee_pages_map[page] = mirror;
            break;
          }
          else
            FLASH_ClearFlag(FLASH_FLAG_PGERR |FLASH_FLAG_WRPERR|FLASH_FLAG_EOP);
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
    if((fee_addr & (FEE_SECTOR_SIZE - 1)) == 0)
    {
      fee_addr = fee_get_fee_addr(Addr);
      if(fee_addr == 0)
        return;
    }

    *(pBuf++) = *(uint8_t *)(fee_addr);
    fee_addr += 2;
    Addr++;
    Len--;
  }
}

// Write data to EEPROM
void eeprom_write(uint8_t *pBuf, uint32_t Addr, uint32_t Len)
{
  while(Len > 0)
  {
    uint32_t fee_addr = fee_get_fee_addr(Addr);
    if(fee_addr == 0)
      return;
      
    uint32_t length = (~(fee_addr - 1) & (FEE_SECTOR_SIZE - 1))/2;
    
    if(Len < length)
      length = Len;

    if(fee_write_page(fee_addr, length, pBuf))
    {
      Len -= length;
      Addr += length;
      continue;
    }
    
    if(!fee_transfer_page(fee_addr, length))
      return;
  }
}
