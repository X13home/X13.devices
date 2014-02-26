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
// Please define SMART_SIZEOF_DEVICEID
// Please define SMART_CFGDATA_NUM

const PROGMEM uint8_t  device_id[] = {"TEST_ATM168PA_V0.1"};   // Device ID
const PROGMEM s_SMART_CONFIG_t cfg_data[] =
{
  // Variables description
  { 6,                                    // Record length
    (accWrite | objBool),                 // variable type + access
    0,                                    // Offset
    {"LED1"}},                            // Description
  { 6, (accWrite | objBool), 1, {"LED2"}},
  { 5, (accRead | objBool),  2, {"SW1"}},
  { 5, (accRead | objBool),  3, {"SW2"}}
};

extern uint8_t sm_stat_reg;

uint8_t stat_sw1, stat_sw2;
volatile uint8_t ostat_sw1, ostat_sw2;

__attribute__((OS_main)) int main(void)
{
  cli();
  wdt_reset();
  wdt_disable();

  InitTWI(12);

  DDRC = (1<<LED1) | (1<<LED2) | (1<<SW1) | (1<<SW2);
  PORTC = (1<<SW1) | (1<<SW2);

  sei();

  stat_sw1 = 0;
  stat_sw2 = 0;

  ostat_sw1 = 0;
  ostat_sw2 = 0;

  while(1)
  {
    stat_sw1 = ((PINC & (1<<SW1)) == 0) ? 1 : 0;
    stat_sw2 = ((PINC & (1<<SW2)) == 0) ? 1 : 0;

    if(sm_stat_reg == 0xFF)
    {
      if(stat_sw1 != ostat_sw1)
        sm_stat_reg = 2;
      else if(stat_sw2 != ostat_sw2)
        sm_stat_reg = 3;
    }
  }
}

uint8_t GetUserDataLen(uint8_t reg)
{
  if(reg < 4)
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
    return 0;
  }
  else if(reg == 1)
  {
    if(data != 0)
      PORTLED |= (1<<LED2);
    else
      PORTLED &= ~(1<<LED2);
    return 0;
  }

  return 1;
}
