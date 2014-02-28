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
#include "../library/smart/smart.h"
#include "../library/att_usi/USI.h"
#include "levels.h"

#define DEF_FADE    1

// Constants
// Device  description
const uint8_t PROGMEM  device_id[] = {"DIMMER_2CH_V0.1"};
// Variables description
const s_SMART_CONFIG_t PROGMEM cfg_data[] =
{
  // Variables description
  { 7,                                    // Record length
    (accWrite | objUInt8),                // variable type + access
    0,                                    // Offset
    {"Chn 1"}},                           // Description
  { 7, (accWrite | objUInt8), 1, {"Chn 2"}},
  { 6, (accWrite | objUInt8), 2, {"Fade"}},
  { 9, (accWrite | accRead | objUInt8), 0x10, {"Address"}},
};
// Please don't change 
const PROGMEM uint8_t smart_config[] = {sizeof(device_id) - 1,
                                        sizeof(cfg_data)/sizeof(s_SMART_CONFIG_t)};

// Local Variables and subroutines
static uint8_t FadeTicks;
static uint8_t fadeChn1, fadeChn2;
static uint16_t actVal1, actVal2;   // Actual timeout values for channels

__attribute__((OS_main)) int main(void)
{
#ifdef DHT_USE_WATCHDOG
  wdt_enable(WDTO_8S);
#endif  //  DHT_USE_WATCHDOG

  PRR = (1<<PRADC);   // Disable ADC
  ACSR = (1<<ACD);    // Disable Comparator
  
  uint8_t tmp;

  DDRB = (1<<OUT1) | (1<<OUT2);
  PORTB = 0;

  tmp = eeprom_read_byte((uint8_t *)EE_ADDR_I2CADDR);
  if(tmp == 0xFF)
  {
    tmp = DEF_I2C_ADDR;
    eeprom_write_byte((uint8_t *)EE_ADDR_I2CADDR, DEF_I2C_ADDR);
    FadeTicks = DEF_FADE;
    eeprom_write_byte((uint8_t *)EE_ADDR_FADE, DEF_FADE);
  }
  else
    FadeTicks = eeprom_read_byte((uint8_t *)EE_ADDR_FADE);

  InitUSI(tmp);

// Zero cross detector interrupt on.
  PCMSK = (1<<ZCD);   // Interrupt on Zero Cross Voltage 
  GIMSK = (1<<PCIE);  // Pin Change Interrupt Enable

  fadeChn1 = 0;
  fadeChn2 = 0;

  sei();

  while(1)
  {
  }
}

uint8_t GetUserDataLen(uint8_t reg)
{
  if(reg == 0x10)
    return 1;

  return 0;
}

uint8_t GetUserData(uint8_t reg, uint8_t offset)
{
  if(reg == 0x10)
    return TWI_addr();
  return 0xFF;
}

uint8_t WriteUserData(uint8_t reg, uint8_t offset, uint8_t data)
{
  reg += offset;

  switch(reg)
  {
    case 0:   // Channel 1
      fadeChn1 = data;
      break;
    case 1:   // Channel 2
      fadeChn2 = data;
      break;
    case 2:   // Fade
      FadeTicks = data;
      eeprom_write_byte((uint8_t *)EE_ADDR_FADE, data);
      break;
    case 0x10:   // I2C Address, write to EEPROM
      eeprom_write_byte((uint8_t *)EE_ADDR_I2CADDR, data);
      break;
  }

  return 1; // No data to write
}

// ZCD Interrupt
ISR(PCINT0_vect)
{
  static uint8_t actFade = 0xFF;
  static uint8_t currChn1 = 0, currChn2 = 0;
  
  if((PINB & (1<<ZCD)) == 0)  // Falling edge
  {
    // Disable outputs
    PORTB &= ~((1<<OUT1) | (1<<OUT2));
    // Stop timer
    TCCR1 = 0;
    GTCCR = 0;
    TIMSK &= ~((1<<OCIE1A) | (1<<OCIE1B) | (1<<TOIE1));

    if(actFade > 0)
      actFade--;
    else if(FadeTicks == 0)   // Fade is disabled
    {
      currChn1 = fadeChn1;
      currChn2 = fadeChn2;
    }
    else
    {
      actFade = FadeTicks;

      if(currChn1 != fadeChn1)
      {
        if(currChn1 < fadeChn1)
          currChn1++;
        else
          currChn1--;
      }

      if(currChn2 != fadeChn2)
      {
        if(currChn2 < fadeChn2)
          currChn2++;
        else
          currChn2--;
      }
    }

    // Load Values
    if(currChn1 > 62)
      PORTB |= (1<<OUT1);
    else
      actVal1 = pgm_read_word((const void *)&levels[currChn1]);

    if(currChn2 > 62)
      PORTB |= (1<<OUT2);
    else
      actVal2 = pgm_read_word((const void *)&levels[currChn2]);
  }
  else  // Rising Edge 
  {
    // Start Timer
    TCNT1 = 0;
    TIFR = (1<<TOV1);   // Clear Timer1 overflow interrupt flag
    TIMSK = (1<<TOIE1); // Enable interrupt on Timer1 overflow
    TCCR1 = (1<<CS10);
  }
}

// Interrupt on TIM1 overflow
ISR(TIM1_OVF_vect)
{
  if(actVal1 >= 256)
    actVal1 -= 256;
  else if(actVal1 > 0)
  {
    OCR1B = actVal2;
    GTCCR |= (3<<COM1B0);
    actVal1 = 0;
  }

  if(actVal2 >= 256)
    actVal2 -= 256;
  else if(actVal2 > 0)
  {
    OCR1A = actVal2;
    TCCR1 |= (3<<COM1A0);
    actVal2 = 0;
  }
}

