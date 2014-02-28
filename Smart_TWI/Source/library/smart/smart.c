/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org

BSD New License
See LICENSE file for license details.
 */ 

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "smart.h"

// Device  and variables descriptions section
extern const PROGMEM uint8_t            device_id[];
extern const PROGMEM s_SMART_CONFIG_t   cfg_data[];
extern const PROGMEM uint8_t            smart_config[];

// External subroutines
extern uint8_t GetUserDataLen(uint8_t reg);
extern uint8_t GetUserData(uint8_t reg, uint8_t offset);
extern uint8_t WriteUserData(uint8_t reg, uint8_t offset, uint8_t data);

// Local Variables
static uint8_t sm_reg = REG_UNDEF;                // Requested Register address
static uint8_t sm_offs = 0;                       // Requested Register offset
static uint8_t sm_stat_reg = REG_DEVICE_DESCR;    // Status, register address
static uint8_t sm_stat_len = 0;                   // Status, register length
static uint8_t sm_stat_onl = 0;                   // Status online

// Local subroutines
static uint8_t sm_get_len(uint8_t reg)
{
  // User Data
  if(reg < REG_MAX_USER)
    return GetUserDataLen(reg);
  else if(reg == REG_DEVICE_DESCR)    // Device Id
  {
    return pgm_read_byte(&smart_config[0]);
  }
  else                    // Configuration
  {
    reg -= REG_VARIABLE_DESCR;
    if(reg < pgm_read_byte(&smart_config[1]))
      return pgm_read_byte(&cfg_data[reg].lenC);
  }

  return 0;
}

void smart_reset_reg(void)
{
  sm_reg = REG_UNDEF;
  sm_offs = 0xFF;
}

void smart_reset_offs(void)
{
  sm_offs = 0x00;
}

uint8_t smart_read_data(void)
{
  uint16_t pnt;

  if(sm_reg == REG_STATUS)                    // Read Status
  {
    if(sm_offs == 0)                          // Read status register, REG
    {
#ifdef DHT_USE_WATCHDOG
      wdt_reset();
#endif  //  DHT_USE_WATCHDOG
      sm_offs = 1;
      return sm_stat_reg;
    }
    else if(sm_offs == 1)                     // Read status register, LEN
    {
      sm_offs = 0xFE;
      uint8_t tmp = sm_stat_len;
      if(tmp == 0)
        tmp = sm_get_len(sm_stat_reg);
      sm_stat_reg = REG_UNDEF;
      sm_stat_len = 0;
      return tmp;
    }
  }
  else if(sm_reg <= REG_MAX_USER)             // User Section (PDO)
    return GetUserData(sm_reg, sm_offs++);
  else if(sm_reg == REG_DEVICE_DESCR)         // Read device id and configuration data
  {
    pnt = (uint16_t)&device_id;
    pnt += sm_offs++;
    return pgm_read_byte(pnt);
  }
  // Read Variable Description
  else if((sm_reg >= REG_VARIABLE_DESCR) && 
          (sm_reg < (REG_VARIABLE_DESCR + pgm_read_byte(&smart_config[1]))))
  {
    sm_offs++;
    pnt = (uint16_t)&cfg_data[sm_reg - REG_VARIABLE_DESCR];
    pnt += sm_offs;
    return pgm_read_byte(pnt);
  }
  return 0xFF;
}

uint8_t smart_write_data(uint8_t data)
{
  if((sm_reg == REG_UNDEF) && (sm_offs == 0xFF))
  {
    sm_reg = data;
    sm_offs = 0;
    return 0;
  }
  else if(sm_reg <= REG_MAX_USER)              // User data
    return WriteUserData(sm_reg, sm_offs++, data);
  else if(sm_reg == REG_CONTROL)              // Control register
  {
    if(sm_offs == 0)
    {
      sm_offs = 0xFE;
      switch(data)
      {
        case 'R':                             // Reset
          sm_stat_onl = 0;
          sm_stat_reg = REG_DEVICE_DESCR;
          break;
        case 'I':                             // Init
          sm_stat_onl = 1;
          break;
        case 'r':                             // Read data
          sm_offs = 1;
          return 0;
      }
    }
    else if(sm_offs == 1)
    {
      sm_offs = 2;
      sm_stat_reg = data;
      return 0;
    }
    else if(sm_offs == 2)
    {
      sm_offs = 0xFE;
      sm_stat_len = data;
    }
  }

  return 1;
}

uint8_t smart_status(void)
{
  if(sm_stat_onl == 0)
    return SM_STAT_OFFLINE;
  else if(sm_stat_reg != REG_UNDEF)
    return SM_STATUS_BUSY;

  return SM_STATUS_FREE;
}

void smart_set_reg(uint8_t reg)
{
  if((sm_stat_onl == 1) && (sm_stat_reg == REG_UNDEF))
    sm_stat_reg = reg;
}