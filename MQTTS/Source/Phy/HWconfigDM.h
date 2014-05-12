/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// Hardware definitions Dummy

// Dummy Vers 1.0
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
// 16   PC0     SV1-18   Ain0
// 17   PC1     SV1-17   Ain1
// 18   PC2     SV1-16   Ain2
// 19   PC3     SV1-15   Ain3
// 20   PC4     SV1-14   SDA
// 21   PC5     SV1-13   SCL
// --   PC6     ISP-5    RESET
// --   --      SV1-20   Ain6
// --   --      SV1-19   Ain7
// PORT D
// 24   PD0     SV1-10  RXD - On gateway busy
// 25   PD1     SV1-9  TXD - On gateway busy
// 26   PD2     SV1-8  IRQ 0
// 27   PD3     SV1-7  IRQ 1
// 28   PD4     SV1-6
// 29   PD5     SV1-5  PWM0
// 30   PD6     SV1-4  PWM1
// 31   PD7     SV1-3

#ifndef _HWCONFIG_DM_H
#define _HWCONFIG_DM_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>

#include <util/crc16.h>

#include <stdlib.h>
#include <string.h>

// Object's Dictionary Section
#define OD_DEV_TYP_0            'D'
#define OD_DEV_TYP_1            'M'
#define OD_DEV_TYP_2            '0'
#define OD_DEV_TYP_3            '1'
#define OD_DEFAULT_ADDR         0x04
// End OD Section

// Digital IO's
#define EXTDIO_MAXPORT_NR       3           // Number of digital Ports
#define EXTDIO_BASE_OFFSET      1           // Numeration started from Port: 0 - A, 1 - B, 2 - C ...

#define PORTNUM_2_PORT          {(uint16_t)&PORTB, (uint16_t)&PORTC, (uint16_t)&PORTD}
#define PORTNUM_2_MASK          {0xC0, 0xC0, 0x00}
// End Digital IO's

// Analogue Inputs
#define EXTAIN_MAXPORT_NR       10          // ADC0-ADC8, Vbg
#define EXTAIN_BASE_2_APIN      {0, 1, 2, 3, 4, 5, 6, 7, 8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 9, 0xFF}
// End Analogue Inputs

#define RF_NODE                 1

#include "HWConfigDefaultATM.h"

#endif
