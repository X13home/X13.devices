/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// Hardware definitions uNode

#ifndef _HWCONFIG_UN_H
#define _HWCONFIG_UN_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>

#include <util/crc16.h>

#include <stdlib.h>
#include <string.h>

// uNode Vers 1.0; Vers 1.1
// uc ATMega328p
// PHY: RF - MRF49XA/RFM12

// 0 - 7    PORTA - not exist
// PORTB
// --   PB0     --      RF_IRQ
// --   PB1     --      LED
// --   PB2     --      RF_CSN
// --   PB3     ISP-4   RF_MOSI
// --   PB4     ISP-1   RF_MISO
// --   PB5     ISP-3   RF_SCK
// --   PB6     --      OSC
// --   PB7     --      OSC
// PORT C
// 16   PC0     SV1-3   Ain0
// 17   PC1     SV1-4   Ain1
// 18   PC2     SV1-5   Ain2
// 19   PC3     SV1-6   Ain3
// 20   PC4     SV1-7   SDA
// 21   PC5     SV1-8 - SCL
// --   PC6     ISP-5   RESET
// --   --      RSSI    Ain6    *Optional
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
#define OD_DEV_TYP_0            'U'
#define OD_DEV_TYP_1            'N'
#define OD_DEV_TYP_2            '1'
#define OD_DEV_TYP_3            '0'
#ifdef GATEWAY
#define OD_DEFAULT_ADDR         0x07
#endif  //  GATEWAY
// End OD Section

#define SYSTEM_RESET()          {cli();RxLEDon();asm("jmp 0x0000");}

// Digital IO's
#define EXTDIO_MAXPORT_NR       2           // Number of digital Ports
#define EXTDIO_BASE_OFFSET      2           // Numeration started from Port: 0 - A, 1 - B, 2 - C ...

#define PORTNUM_2_PORT          {(uint16_t)&PORTC, (uint16_t)&PORTD}
#ifdef GATEWAY
#define PORTNUM_2_MASK          {0xC0, 0x03}
#else   // GATEWAY
#define PORTNUM_2_MASK          {0xC0, 0x00}
#endif  // GATEWAY
// End Digital IO's

// Analogue Inputs
#define EXTAIN_MAXPORT_NR       8          // ADC0-ADC5, ADC7, Vbg
#define EXTAIN_BASE_2_APIN      {0, 1, 2, 3, 4, 5, 0xFF, 6, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 7, 0xFF}
// End Analogue Inputs

// RF Section
#define RF_PIN_IRQ              PORTB0
#define RF_LEDS                 PORTB1

#define TxLEDon()               RF_PORT &= ~(1<<RF_LEDS);
#define RxLEDon()               RF_PORT &= ~(1<<RF_LEDS);
#define LEDsOff()               RF_PORT |=  (1<<RF_LEDS);

#define RF_PORT_INIT()          {RF_PORT = (1<<RF_PIN_SS) | (1<<RF_PIN_IRQ) | (1<<RF_LEDS); \
                                 RF_DDR = (1<<RF_PIN_SCK) | (1<<RF_PIN_MOSI) |              \
                                          (1<<RF_PIN_SS) | (1<<RF_LEDS);}

#if (F_CPU > 10000000UL)

#define RF_SPI_INIT()           {SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR0); SPSR = (1<<SPI2X);}

#define RF_SPI_SLOW()           SPCR |= (1<<SPR0)
#define RF_SPI_FAST()           SPCR &= ~(1<<SPR0)

#else   //  (F_CPU <= 10000000UL)

#define RF_SPI_INIT()           {SPCR = (1<<SPE) | (1<<MSTR); SPSR = 0;}

#define RF_SPI_SLOW()           SPSR = 0
#define RF_SPI_FAST()           SPSR = (1<<SPI2X)

#endif  //  (F_CPU > 10000000UL)
// End RF Section

#define RF_NODE                 1
#define RFM12_EN                1

#include "RFM12/rfm12.h"
//  End RF Section

#include "HWConfigDefaultATM.h"

#endif  //  _HWCONFIG_UN_H
