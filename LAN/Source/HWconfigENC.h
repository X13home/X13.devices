/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
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

// Object's Dictionary Section
#define OD_DEV_TYP_0        'L'
#define OD_DEV_TYP_1        'N'
#define OD_DEV_TYP_2        '0'
#define OD_DEV_TYP_3        '0'
// End OD Section

#define SystemReset()       {cli();asm("jmp 0x0000");}

// Power Reduction
#define CONFIG_PRR()        {ACSR = (1<<ACD); \
                             PRR = (1<<PRTWI) | (1<<PRTIM0) | (1<<PRTIM1) | \
                             (1<<PRUSART0) | (1<<PRADC);}
// Timer Section
#define POOL_TMR_FREQ       64     // Pool Frequency (Hz)
#define TIMER_ISR           TIMER2_COMPA_vect
#define InitTimer()         {TCCR2A = (1<<WGM21); TCNT2 = 0;            \
                            OCR2A = ((F_CPU/1024/POOL_TMR_FREQ)-1);    \
                            TIFR2 = (1<<OCF2A); TIMSK2 = (1<<OCIE2A);  \
                            TCCR2B =(1<<WGM22) | (7<<CS20);}
// End Timer Section

// ENC28J60 Section
#define ENC_DDR             DDRB
#define ENC_PORT            PORTB
#define ENC_PIN_SS          PORTB2
#define ENC_PIN_MOSI        PORTB3
#define ENC_PIN_MISO        PORTB4
#define ENC_PIN_SCK         PORTB5

#define ENC_IRQ_DDR         DDRD
#define ENC_IRQ_PORT        PORTD
#define ENC_IRQ_PORTIN      PIND
#define ENC_IRQ_PIN         PORTD2

#define ENC_PORT_INIT()     {ENC_PORT |= (1<<ENC_PIN_SS); ENC_DDR  &= ~(1<<ENC_PIN_MISO);         \
                             ENC_DDR  |= (1<<ENC_PIN_SCK) | (1<<ENC_PIN_MOSI) | (1<<ENC_PIN_SS);  \
                             ENC_IRQ_DDR &= ~(1<<ENC_IRQ_PIN); ENC_IRQ_PORT |= (1<<ENC_IRQ_PIN); }
#define ENC_SELECT()        ENC_PORT &= ~(1<<ENC_PIN_SS)
#define ENC_RELEASE()       ENC_PORT |= (1<<ENC_PIN_SS)

#define ENC_SPI_INIT()      {SPCR = (1<<SPE)|(1<<MSTR); SPSR |= (1<<SPI2X);}
#define ENC_SPI_DATA        SPDR
#define ENC_SPI_BISY        (!(SPSR &(1<<SPIF)))

#define ENC_DISABLE_IRQ()   {}
#define ENC_IRQ_CFG()       {}
// End ENC28J60 Section

#endif
