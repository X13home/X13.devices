/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// Hardware definitions Dummy ATMega2560

#ifndef _HWCONFIG_DM_ATM256_H
#define _HWCONFIG_DM_ATM256_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>

#include <util/crc16.h>

#include <stdlib.h>
#include <string.h>

// Dummy Vers 0.2 - ATMega2560
// Test Only, NOT SUPPORTED !!

// Object's Dictionary Section
#define OD_DEV_TYP_0            'D'
#define OD_DEV_TYP_1            'M'
#define OD_DEV_TYP_2            '0'
#define OD_DEV_TYP_3            '2'
#ifdef GATEWAY
#define OD_DEFAULT_ADDR         0x04
#endif  //  GATEWAY
// End OD Section

#define SystemReset()           {cli();asm("jmp 0x0000");}

// Power Reduction
#define CONFIG_PRR()

// USART Section
#define USART_RX_vect           USART0_RX_vect
#define USART_UDRE_vect         USART0_UDRE_vect

#define USART_DATA              UDR0
#define USART_SET_BAUD(baud)    {if(baud & 0xF000){baud &= 0x0FFF; UCSR0A |= (1<<U2X0);}    \
                                 UBRR0H = (baud>>8); UBRR0L = (baud & 0xFF);}
#define USART_DISABLE_DREINT()  UCSR0B &= ~(1<<UDRIE0)
#define USART_ENABLE_DREINT()   UCSR0B |= (1<<UDRIE0)

#define RXD                     PORTE0
#define TXD                     PORTE1

#define USART_CONFIG_PORT()     {PORTE |= (1<<RXD) | (1<<TXD); DDRE |= (1<<TXD); DDRE &= ~(1<<RXD);}
#define USART_BAUD              ((F_CPU/16/38400) - 1) // Baud = 38400, val = Fosc/(16 * baud) - 1
#define USART_CONFIGURE()       {UCSR0B = ((1<<RXCIE0) | (1<<RXEN0) | (1<<TXEN0));  \
                                 UCSR0C = (3<<UCSZ00);}
// End USART Section

// Digital IO's
#define EXTDIO_MAXPORT_NR       4           // Number of digital Ports
#define EXTDIO_BASE_OFFSET      0           // Numeration started from Port: 0 - A, 1 - B, 2 - C ...

#define PORTNUM_2_PORT          {(uint16_t)&PORTA, (uint16_t)&PORTB, (uint16_t)&PORTC, (uint16_t)&PORTD}
#define PORTNUM_2_MASK          {0x00, 0x00, 0x00, 0x00}
// End Digital IO's

// Analogue Inputs
#define EXTAIN_MAXPORT_NR       8          // ADC0-ADC7
#define EXTAIN_BASE_2_APIN      {0, 1, 2, 3, 4, 5, 6, 7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
// End Analogue Inputs

#define RF_NODE                 1
#define DUMMY                   1

#include "HWConfigDefaultATM.h"

#endif  //  _HWCONFIG_DM_ATM256_H
