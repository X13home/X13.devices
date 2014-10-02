/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// Hardware definitions, panstamp - Arduino + CC1101

#ifndef _HWCONFIG_PS_H
#define _HWCONFIG_PS_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>

#include <util/crc16.h>

#include <stdlib.h>
#include <string.h>

// panSTamp V2.0
// http://www.panstamp.com/
// 0 - 7    PORTA - not exist
// PORTB
// --   PB0     P1-2    LED_TX(Red)
// --   PB1     P1-3    LED_RX(Green)
// --   PB2     --      RF_CSN
// --   PB3     P3-5    RF_MOSI
// --   PB4     P3-4    RF_MISO
// --   PB5     P3-2    RF_SCK
// --   PB6     --      OSC
// --   PB7     --      OSC
// PORT C
// 16   PC0     P1-4(A0)   Ain0
// 17   PC1     P1-5(A1)    Ain1
// 18   PC2     P1-6(A2)    Ain2
// 19   PC3     P1-8(A3)    Ain3
// 20   PC4     P1-9(A4)    SDA
// 21   PC5     P1-10(A5)   SCL
// --   PC6     P2-12   RESET
// --   --      P1-11(A6)   Ain6
// --   --      P1-12(A7)   Ain7
// PORT D
// 24   PD0     P2-9(D0)    RXD - On gateway busy
// 25   PD1     P2-8(D1)    TXD - On gateway busy
// --   PD2     --      RF_IRQ
// 27   PD3     P2-7(D3)    IRQ 1
// 28   PD4     P2-6(D4)
// 29   PD5     P2-5(D5)    PWM0
// 30   PD6     P2-4(D6)    PWM1
// 31   PD7     P2-3(D7)

// Object's Dictionary Section
#define OD_DEV_TYP_0            'P'
#define OD_DEV_TYP_1            'S'
#define OD_DEV_TYP_2            '2'
#define OD_DEV_TYP_3            '0'
#ifdef GATEWAY
#define OD_DEFAULT_ADDR         0x07
#endif  //  GATEWAY
// End OD Section

#define SYSTEM_RESET()          {cli();RxLEDon(); WDTCSR = (1<<WDCE) | (1<<WDE); WDTCSR = (1<<WDE);}

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
#define EXTAIN_MAXPORT_NR       9          // ADC0-ADC7, Vbg
#define EXTAIN_BASE_2_APIN      {0, 1, 2, 3, 4, 5, 6, 7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 8, 0xFF}
// End Analogue Inputs

// RF Section
#define RF_LEDR                 PORTB0
#define RF_LEDG                 PORTB1

#define TxLEDon()               RF_PORT |= (1<<RF_LEDR);
#define RxLEDon()               RF_PORT |= (1<<RF_LEDG);
#define LEDsOff()               RF_PORT &= ~((1<<RF_LEDG) | (1<<RF_LEDR));

#define RF_PORT_INIT()          {RF_PORT = (1<<RF_PIN_SS) | (1<<RF_PIN_MISO);                   \
                                 RF_DDR = (1<<RF_PIN_SCK) | (1<<RF_PIN_MOSI) | (1<<RF_PIN_SS) | \
                                          (1<<RF_LEDR) | (1<<RF_LEDG);}

#if (F_CPU > 13000000UL)
#define RF_SPI_INIT()           {SPCR = (1<<SPE) | (1<<MSTR); SPSR = 0;}            // F_CPU/4
#else   //  (F_CPU <= 13000000UL)
#define RF_SPI_INIT()           {SPCR = (1<<SPE) | (1<<MSTR); SPSR = (1<<SPI2X);}   // F_CPU/2
#endif  //  (F_CPU > 13000000UL)

#define RF_IRQ_CFG()            {DDRD &= ~(1<<PORTD2); PORTD |= (1<<PORTD2);    \
                                 EICRA = (1<<ISC00);}        // Interrupt on CHANGE state.
#define RF_STAT_IRQ             (PIND & (1<<PORTD2))
#define RF_INT_vect             INT0_vect
#define RF_ENABLE_IRQ()         EIMSK = (1<<INT0);      // INT0 int enable
#define RF_DISABLE_IRQ()        EIMSK = 0;              // INT0 disable

#define RF_NODE                 1
#define CC110_EN                1

#include "cc11/cc11.h"
//  End RF Section

#include "HWConfigDefaultATM.h"

#endif    //  _HWCONFIG_PS_H
