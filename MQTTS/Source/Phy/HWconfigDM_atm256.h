/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

// Hardware definitions Dummy ATMega 256

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

// Timer Section
#define POOL_TMR_FREQ           64     // Pool Frequency (Hz)
#define TIMER_ISR               TIMER2_COMPA_vect

#define InitTimer()             {TCCR2A = (1<<WGM21); TCNT2 = 0;            \
                                 OCR2A = ((F_CPU/1024/POOL_TMR_FREQ)-1);    \
                                 TIFR2 = (1<<OCF2A); TIMSK2 = (1<<OCIE2A);  \
                                 TCCR2B =(1<<WGM22) | (7<<CS20);}
#define config_sleep_wdt()      {wdt_reset(); MCUSR &= ~(1<<WDRF);                      \
                                 WDTCSR |= (1<<WDCE) | (1<<WDE); WDTCSR = (6<<WDP0);    \
                                 WDTCSR |= (1<<WDIF); WDTCSR |= (1<<WDIE);}
// End Timer Section

// Digital IO's
#define EXTDIO_MAXPORT_NR       3           // Number of digital Ports

#define PORTNUM0                0
#define PORTDDR0                DDRA
#define PORTOUT0                PORTA
#define PORTIN0                 PINA
#define PORT1MASK               0x00

#define PORTNUM1                1
#define PORTDDR1                DDRB
#define PORTOUT1                PORTB
#define PORTIN1                 PINB
#define PORT1MASK               0x00

#define PORTNUM2                2
#define PORTDDR2                DDRC
#define PORTOUT2                PORTC
#define PORTIN2                 PINC
#define PORT2MASK               0x00

#define RF_NODE                 1
#define OD_DEFAULT_GROUP        0x2DD4
#define OD_DEFAULT_CHANNEL      0x12

#define DUMMY

#define s_Addr                  uint8_t
#define AddrBroadcast           0

#endif  //  _HWCONFIG_DM_ATM256_H
