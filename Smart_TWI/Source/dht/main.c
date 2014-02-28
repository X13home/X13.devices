/*
 * I2C Bridge to DHT Sensors, 3-ch

 Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org

BSD New License
See LICENSE file for license details.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>

#include "config.h"
#include "../library/att_dht/dht.h"
#include "../library/smart/smart.h"
#include "../library/att_usi/USI.h"

// Constants
// Device  description
const PROGMEM uint8_t device_id[] = {"DHT_BR_3CH_V0.1"};
// Variables description
const PROGMEM s_SMART_CONFIG_t  cfg_data[] =
{
  // Variables description
  { 9,                                    // Record length
    (accRead | objInt16),                 // variable type + access
    0,                                    // Offset
    {"Temp S1"}},                         // Description
  { 9, (accRead | objUInt16), 1, {"Humi S1"}},
  { 9, (accRead | objInt16),  2, {"Temp S2"}},
  { 9, (accRead | objUInt16), 3, {"Humi S2"}},
  { 9, (accRead | objInt16),  4, {"Temp S3"}},
  { 9, (accRead | objUInt16), 5, {"Humi S3"}},
  { 9, (accWrite | accRead | objUInt8), 0x10, {"Address"}},
};
// Please don't change 
const PROGMEM uint8_t smart_config[] = {sizeof(device_id) - 1,
                                        sizeof(cfg_data)/sizeof(s_SMART_CONFIG_t)};

// Local Variables and Subroutines
static const uint8_t Sensors[] = {(1<<SENS1), (1<<SENS2), (1<<SENS3)};

uint16_t sData[6];          // Measured Data 0 - Temp, 1 - Humi Sensor 1, 2/3 - Sensor 2 ....
uint16_t soData[6];         // Output data

extern uint8_t sm_stat_reg; // Status register address
extern uint8_t sm_stat_onl; // Status online

__attribute__((OS_main)) int main(void)
{
#ifdef DHT_USE_WATCHDOG
  wdt_enable(WDTO_8S);
#endif  //  DHT_USE_WATCHDOG

  PRR = (1<<PRADC);   // Disable ADC
  ACSR = (1<<ACD);    // Disable Comparator

  // Initialise TIM1 as timeout counter
  TCNT1 = 0;
  OCR1C = (F_CPU/8192/4 - 1);       // 250 mS
  OCR1A = (F_CPU/8192/20 - 1);      // 50 mS
  TCCR1 = (1<<CTC1) | (0x0E<<CS10); //  Prescaler 8192, count to OCR1C

  uint8_t mainstate = 0;
  uint8_t timeoutS[3] = {10,20,30};

  uint8_t act_pnt = 0;

  uint8_t tmp;

  for(tmp = 0; tmp < 6; tmp++)
    soData[tmp] = 0;

  tmp = eeprom_read_byte((uint8_t *)EE_ADDR_I2CADDR);
  if(tmp == 0xFF)
    tmp = DEF_I2C_ADDR;

  InitUSI(tmp);

  sei();

  while(1)
  {
    if(!(TIFR & (1<<OCF1A)))
      continue;

    TIFR = (1<<OCF1A);

    switch(mainstate)
    {
      case 0:     // Wait a timeout
        for(tmp = 0; tmp < 3; tmp ++)
        {
          if(timeoutS[tmp] > 0)
            timeoutS[tmp]--;
          else
            mainstate = tmp + 1;
        }
        break;
      case 1:
      case 2:
      case 3:
        dht_start_conversion(Sensors[mainstate-1]);
        mainstate += 0x10;
        break;
      case 0x11:
      case 0x12:
      case 0x13:
        tmp = dht_get_data(&sData[(mainstate-0x11)*2]);
        if(tmp > 0x40)      // error
          timeoutS[mainstate-0x11] = 255;
        else
          timeoutS[mainstate-0x11] = DHT_REQ_PER;
      default:
        mainstate = 0;
        break;
    }

    tmp = smart_status();
    if(tmp == SM_STAT_OFFLINE)
    {
      for(act_pnt = 0; act_pnt < 6; act_pnt++)
        soData[act_pnt] = 0;
    }
    else if(tmp == SM_STATUS_FREE)
    {
      if(act_pnt > 5)
        act_pnt = 0;

      if(sData[act_pnt] != soData[act_pnt])
        smart_set_reg(act_pnt);

      act_pnt++;
    }
  }
}

uint8_t GetUserDataLen(uint8_t reg)
{
  if(reg < 6)
    return 2;
  else if(reg == 0x10)
    return 1;

  return 0;
}

uint8_t GetUserData(uint8_t reg, uint8_t offset)
{
  while(offset >= 2)
  {
    offset -=2;
    reg++;
  }

  if(reg < 6)
  {
    if(offset == 0)
    {
      soData[reg] = sData[reg];
      return soData[reg] & 0xFF;
    }
    else
      return soData[reg]>>8;
  }
  else   if(reg == 0x10)
    return TWI_addr();

  return 0xFF;
}

uint8_t WriteUserData(uint8_t reg, uint8_t offset, uint8_t data)
{
  reg += offset;
  if(reg == 0x10)
    eeprom_write_byte((uint8_t *)EE_ADDR_I2CADDR, data);

  return 1; // No data to write
}
