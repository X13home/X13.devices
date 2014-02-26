/*
 * Interrupt driven Library for DHT11/DHT22

 Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org

BSD New License
See LICENSE file for license details.
 */

#include <avr/io.h>
#include <avr/interrupt.h>

// Please define F_CPU =8000000UL
// Please define DHT_PORT PORTB
// Please define DHT_DDR  DDRB

// Interrupts variables
static uint8_t iBuf[4];
volatile static uint8_t isr_state;

static uint8_t sens_pin;

uint8_t dht_get_data(uint16_t * pBuf)
{
  if(isr_state == 0x40)
  {
    if((iBuf[1] == 0) && (iBuf[3] == 0))    // DHT11
    {
      pBuf[0] = (int16_t)iBuf[2]*10;
      pBuf[1] = (uint16_t)iBuf[0]*10;
    }
    else
    {
      int16_t tmp = ((uint16_t)(iBuf[2] & 0x7F)<<8) | iBuf[3];
      if(iBuf[2] & 0x80)
        tmp = -tmp;

      pBuf[0] = tmp;
      pBuf[1] = ((uint16_t)iBuf[0]<<8) | iBuf[1];
    }
  }
  else if(isr_state > 0x40)   // error
  {
    pBuf[0] = 0;
    pBuf[1] = 0;
  }

  return isr_state;
}

void dht_start_conversion(uint8_t pin)
{
  sens_pin = pin;
  // Generate Start Impulse, 18-20 mS
  DHT_DDR |= pin;                       // Pin as output
  DHT_PORT &= ~pin;

  isr_state = 0;

  TCNT0 = 0;
  OCR0A = ((F_CPU/1024/50) - 1);        // 20ms
  TCCR0B = (5<<CS00);                   // CLK = F_OSC/1024
  TIFR = (1<<OCF0A);
  TIMSK |= (1<<OCIE0A);
}

ISR(TIM0_COMPA_vect)
{
  if(isr_state == 0)                    //  Start Impulse completed
  {
    DHT_PORT |= sens_pin;
    DHT_DDR &= ~sens_pin;               // Pin as output
    isr_state = 1;
    TCNT0 = 0;
    OCR0A = ((F_CPU/8/6666) - 1);       //  Timeout - 150 uS
    TCCR0B = (2<<CS00);                 // CLK = F_OSC/8
    PCMSK = sens_pin;
    GIFR = (1<<PCIF);                   // Clear pin change flag
    GIMSK = (1<<PCIE);                  // Pin change interrupt enable
  }
  else        // Timeout
  {
    GIMSK = 0;
    TCCR0B = 0;
    TIMSK &= ~(1<<OCIE0A);

    isr_state = 0xFF;                   // Error
  }
}

ISR(PCINT0_vect)
{
  static uint8_t bits, pos, tmpdata, crc;
  uint8_t timer;

  timer = TCNT0;
  TCNT0 = 0;

  switch(isr_state)
  {
    case 1:                             // Start pause 5-70 uS
      if((timer >= 5) && (timer <= 70))
      {
        isr_state = 2;
        bits = 0;
        pos = 0;
        crc = 0;
        return;
      }
      break;
    case 2:                             // Presence impulse 0, 50-100 uS
    case 3:                             // Presence impulse 1, 50 - 100 uS
      if((timer > 50) && (timer < 100))
      {
        isr_state++;
        return;
      }
      break;
    case 4:                             // Synchro pause,  30-70 uS
      if((timer > 30) && (timer < 70))
      {
        isr_state = 5;
        return;
      }
      break;
    case 5:                             // Data Impulse
      if((timer > 15) && (timer < 90))
      {
        tmpdata <<= 1;
        if(timer > 45)                  // Data 1
          tmpdata |= 1;

        bits++;
        if(bits == 8)
        {
          bits = 0;
          if(pos == 4)
          {
            GIMSK = 0;
            TCCR0B = 0;
            TIMSK &= ~(1<<OCIE0A);

            if(crc != tmpdata)          // Bad CRC
              isr_state = 0xF0;
            else
              isr_state = 0x40;         // Data Ready
            return;
          }
          
          iBuf[pos] = tmpdata;
          crc += tmpdata;
          pos++;
        }
        isr_state = 4;
        return;
      }
      break;
  }

  // Error
  GIMSK = 0;
  TCCR0B = 0;
  TIMSK &= ~(1<<OCIE0A);

  isr_state += 0xF0;                    // Error, number 0xF0 + state
}
