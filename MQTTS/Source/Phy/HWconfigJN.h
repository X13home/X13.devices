/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// Hardware definitions, JeeNode - Arduino + RFM12

#ifndef _HWCONFIG_JN_H
#define _HWCONFIG_JN_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>

#include <util/crc16.h>

#include <stdlib.h>
#include <string.h>

// jeeNode/Arduino
// http://jeelabs.org, http://arduino.cc 
// 0 - 7    PORTA - not exist
// PORTB
// --   PB0     ISP-7   LEDR
// --   PB1     ISP-8   LEDG
// --   PB2     --      RF_CSN
// --   PB3     ISP-4   RF_MOSI
// --   PB4     ISP-1   RF_MISO
// --   PB5     ISP-3   RF_SCK
// --   PB6     --      OSC
// --   PB7     --      OSC
// PORT C
// 16   PC0     P1-AIO  Ain0
// 17   PC1     P2-AIO  Ain1
// 18   PC2     P3-AIO  Ain2
// 19   PC3     P4-AIO  Ain3
// 20   PC4     SDA     SDA
// 21   PC5     SCL -   SCL
// --   PC6     ISP-5   RESET
// PORT D
// 24   PD0     RXD     RXD - On gateway busy
// 25   PD1     TXD     TXD - On gateway busy
// 26   PD2     --      RF_IRQ
// 27   PD3     P1-4 IRQ  IRQ 1
// 28   PD4     P1-DIO
// 29   PD5     P2-DIO  PWM0
// 30   PD6     P3-DIO  PWM1
// 31   PD7     P4-DIO

// Object's Dictionary Section
#define OD_DEV_TYP_0            'J'
#define OD_DEV_TYP_1            'N'
#define OD_DEV_TYP_2            'v'
#define OD_DEV_TYP_3            '6'
#ifdef GATEWAY
#define OD_DEFAULT_ADDR         0x06
#endif  //  GATEWAY
// End OD Section

#define SYSTEM_RESET()          {cli();RxLEDon();asm("jmp 0x0000");}

// Digital IO's
#define EXTDIO_MAXPORT_NR       2           // Number of digital Ports
#define EXTDIO_BASE_OFFSET      2           // Numeration started from Port: 0 - A, 1 - B, 2 - C ...

#define PORTNUM_2_PORT          {(uint16_t)&PORTC, (uint16_t)&PORTD}
#ifdef GATEWAY
#define PORTNUM_2_MASK          {0xC0, 0x07}
#else   // GATEWAY
#define PORTNUM_2_MASK          {0xC0, 0x04}
#endif  // GATEWAY
// End Digital IO's

// Analogue Inputs
#define EXTAIN_MAXPORT_NR       7          // ADC0-ADC5, Vbg
#define EXTAIN_BASE_2_APIN      {0, 1, 2, 3, 4, 5, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 6, 0xFF}
// End Analogue Inputs

// RF Section
#define RF_LEDR                 PORTB0
#define RF_LEDG                 PORTB1

#define TxLEDon()               RF_PORT |= (1<<RF_LEDR);
#define RxLEDon()               RF_PORT |= (1<<RF_LEDG);
#define LEDsOff()               RF_PORT &= ~((1<<RF_LEDR) | (1<<RF_LEDG));

#define RF_PORT_INIT()          {RF_PORT = (1<<RF_PIN_SS) | (1<<RF_LEDR);       \
                                 RF_DDR = (1<<RF_PIN_SCK) | (1<<RF_PIN_MOSI) |     \
                                         (1<<RF_PIN_SS) | (1<<RF_LEDR) | (1<<RF_LEDG);}
                                         
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

#define RF_IRQ_CFG()            {DDRD &= ~(1<<PORTD2); PORTD |= (1<<PORTD2); EICRA = (0<<ISC00);}
#define RF_STAT_IRQ             (PIND & (1<<PORTD2))

#define RF_INT_vect             INT0_vect

#define RF_ENABLE_IRQ()         EIMSK = (1<<INT0);      // INT0 int enable
#define RF_DISABLE_IRQ()        EIMSK = 0;              // INT0 disable

#define RF_NODE                 1
#define RFM12_EN                1

#include "rfm12/rfm12.h"
//  End RF Section

#include "HWConfigDefaultATM.h"

#endif
