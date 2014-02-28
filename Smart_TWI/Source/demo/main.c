/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org

BSD New License
See LICENSE file for license details.
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "config.h"
#include "../library/atm_twis/twis.h"
#include "../library/smart/smart.h"

// Constants
// Device  description
const PROGMEM uint8_t  device_id[] = {"TEST_ATM168PA_V0.1"};   // Device ID
// Variables description
const PROGMEM s_SMART_CONFIG_t cfg_data[] =
{
  // Variables description
  { 6,                                    // Record length
    (accWrite | objBool),                 // variable type + access
    0,                                    // Offset
    {"LED1"}},                            // Description
  { 6, (accWrite | objBool), 1, {"LED2"}},
  { 5, (accRead | objBool),  2, {"SW1"}},
  { 5, (accRead | objBool),  3, {"SW2"}},
  { 9, (accWrite | accRead | objUInt8), 0x10, {"Address"}},
};
// Please don't change 
const PROGMEM uint8_t smart_config[] = {sizeof(device_id) - 1,
                                        sizeof(cfg_data)/sizeof(s_SMART_CONFIG_t)};

static uint8_t stat_sw1, stat_sw2;
static uint8_t ostat_sw1, ostat_sw2;

__attribute__((OS_main)) int main(void)
{
  cli();
  wdt_reset();
  wdt_disable();

  uint8_t tmp;

  tmp = eeprom_read_byte((uint8_t *)EE_ADDR_I2CADDR);
  if(tmp == 0xFF)
    tmp = DEF_I2C_ADDR;

  InitTWI(12);

  PORTC  = (1<<LED1) | (1<<LED2) | (1<<SW1) | (1<<SW2);
  DDRC = (1<<LED1) | (1<<LED2);

  sei();

  stat_sw1 = 0;
  stat_sw2 = 0;

  ostat_sw1 = 0;
  ostat_sw2 = 0;

  while(1)
  {
    stat_sw1 = ((PINC & (1<<SW1)) == 0) ? 1 : 0;
    stat_sw2 = ((PINC & (1<<SW2)) == 0) ? 1 : 0;

    if(smart_status() == SM_STATUS_FREE)
    {
      if(stat_sw1 != ostat_sw1)
        smart_set_reg(2);
      else if(stat_sw2 != ostat_sw2)
        smart_set_reg(3);
    }
  }
}

uint8_t GetUserDataLen(uint8_t reg)
{
  if((reg == 2) || (reg == 3) || (reg == 0x10))
    return 1;

  return 0;
}

uint8_t GetUserData(uint8_t reg, uint8_t offset)
{
  reg += offset;

  if(reg == 2)
  {
    ostat_sw1 = stat_sw1;
    return ostat_sw1;
  }
  else if(reg == 3)
  {
    ostat_sw2 = stat_sw2;
    return ostat_sw2;
  }
  else   if(reg == 0x10)
    return TWI_addr();

  return 0xFF;
}

uint8_t WriteUserData(uint8_t reg, uint8_t offset, uint8_t data)
{
  reg += offset;

  if(reg == 0)
  {
    if(data != 0)
      PORTLED |= (1<<LED1);
    else
      PORTLED &= ~(1<<LED1);
  }
  else if(reg == 1)
  {
    if(data != 0)
      PORTLED |= (1<<LED2);
    else
      PORTLED &= ~(1<<LED2);
  }
  if(reg == 0x10)
    eeprom_write_byte((uint8_t *)EE_ADDR_I2CADDR, data);

  return 1;
}
