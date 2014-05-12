/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _HWCONFIG_ENC_H
#define _HWCONFIG_ENC_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>

#include <util/crc16.h>
#include <util/delay.h>

#include <stdlib.h>
#include <string.h>

// Hardware definitions LAN Node + ENC28J60

// 0 - 7    PORTA - not exist
// PORTB
// --   PB0     --      LED
// --   PB1     -- 
// --   PB2     --      PHY_CSN
// --   PB3     ISP-4   PHY_MOSI
// --   PB4     ISP-1   PHY_MISO
// --   PB5     ISP-3   PHY_SCK
// --   PB6     --      CLK_IN
// --   PB7     --
// PORT C
// 16   PC0     SV1-3   Ain0
// 17   PC1     SV1-4   Ain1
// 18   PC2     SV1-5   Ain2
// 19   PC3     SV1-6   Ain3
// 20   PC4     SV1-7   SDA/Ain4
// 21   PC5     SV1-8 - SCL/Ain5
// --   PC6     ISP-5   RESET
// --   --      Ain6
// --   --      SV1-1   Ain7
// PORT D
// 24   PD0     SV1-11  RXD - On gateway busy
// 25   PD1     SV1-12  TXD - On gateway busy
// 26   PD2     SV1-13  IRQ 0
// 27   PD3     SV1-14  IRQ 1
// 28   PD4     SV1-15
// 29   PD5     SV1-16  PWM0
// 30   PD6     SV1-17  PWM1
// 31   PD7     SV1-18

// Object's Dictionary Section
#define OD_DEV_TYP_0        'L'
#define OD_DEV_TYP_1        'N'
#define OD_DEV_TYP_2        '0'
#define OD_DEV_TYP_3        '0'

#define OD_DEV_MAC          {0x06, 0x00,0x04,0xA3,0x00,0x00,0x01}   // LEN - 6 bytes, MAC MSB->LSB
// End OD Section

// Digital IO's
#define EXTDIO_MAXPORT_NR       2           // Number of digital Ports
#define EXTDIO_BASE_OFFSET      2           // Numeration started from Port: 0 - A, 1 - B, 2 - C ...

#define PORTNUM_2_PORT          {(uint16_t)&PORTC, (uint16_t)&PORTD}
#define PORTNUM_2_MASK          {0xC0, 0x00}
// End Digital IO's

// Analogue Inputs
#define EXTAIN_MAXPORT_NR       8          // ADC0-ADC5, ADC7, Vbg
#define EXTAIN_BASE_2_APIN      {0, 1, 2, 3, 4, 5, 0xFF, 6, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 7, 0xFF}
// End Analogue Inputs

#define LED_ON()                PORTB &= ~(1<<PORTB0)
#define LED_OFF()               PORTB |= (1<<PORTB0);
#define LED_TGL()               PORTB ^= (1<<PORTB0);

// ENC28J60 Section
#define ENC_DDR                 DDRB
#define ENC_PORT                PORTB
#define ENC_PIN_SS              PORTB2
#define ENC_PIN_MOSI            PORTB3
#define ENC_PIN_MISO            PORTB4
#define ENC_PIN_SCK             PORTB5

// PIN IRQ is not used, but for compatibility with another board
#define ENC_IRQ_DDR             DDRD
#define ENC_IRQ_PORT            PORTD
#define ENC_IRQ_PORTIN          PIND
#define ENC_IRQ_PIN             PORTD2

#define ENC_PORT_INIT()     {ENC_PORT |= (1<<ENC_PIN_SS) | (1<<PORTB0); \
                             ENC_DDR  &= ~(1<<ENC_PIN_MISO);         \
                             ENC_DDR  |= (1<<ENC_PIN_SCK) | (1<<ENC_PIN_MOSI) | (1<<ENC_PIN_SS) | \
                             (1<<PORTB0);  \
                             ENC_IRQ_DDR &= ~(1<<ENC_IRQ_PIN); ENC_IRQ_PORT |= (1<<ENC_IRQ_PIN); }
#define ENC_SELECT()        ENC_PORT &= ~(1<<ENC_PIN_SS)
#define ENC_RELEASE()       ENC_PORT |= (1<<ENC_PIN_SS)

#define ENC_SPI_INIT()          {SPCR = (1<<SPE)|(1<<MSTR); SPSR |= (1<<SPI2X);}
#define ENC_SPI_DATA            SPDR
#define ENC_SPI_BISY            (!(SPSR &(1<<SPIF)))
// End ENC28J60 Section

#define LAN_NODE              1
#define ENC28J60_EN           1

typedef struct
{
  uint8_t mac[6];
  uint8_t ip[4];
}S_ADDR;

#define ADDR_BROADCAST {{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}, {0xFF,0xFF,0xFF,0xFF}}

#include "HWConfigDefaultATM.h"

#endif
