/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
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

// uNode Vers 1.0
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

// Power Reduction
#ifdef USE_RTC_OSC
#define CONFIG_PRR()            {ACSR = (1<<ACD); \
                                 PRR = (1<<PRTWI) | (1<<PRTIM0) | (1<<PRUSART0) | (1<<PRADC);}
#else   //  USE_RTC_OSC
#define CONFIG_PRR()            {ACSR = (1<<ACD); \
                                 PRR = (1<<PRTWI) | (1<<PRTIM0) | (1<<PRTIM1) | \
                                 (1<<PRUSART0) | (1<<PRADC);}
#endif  //  USE_RTC_OSC

// USART Section
// ATMEGA 168PA
#ifndef USART_RX_vect
#define USART_RX_vect  USART__RX_vect
#endif  //  USART_RX_vect

#ifndef USART_UDRE_vect
#define USART_UDRE_vect USART__UDRE_vect
#endif  //  USART_UDRE_vect

#define USART_DATA              UDR0
#define USART_SET_BAUD(baud)    {if(baud & 0xF000){baud &= 0x0FFF; UCSR0A |= (1<<U2X0);}    \
                                 UBRR0H = (baud>>8); UBRR0L = (baud & 0xFF);}
#define USART_DISABLE_DREINT()  UCSR0B &= ~(1<<UDRIE0)
#define USART_ENABLE_DREINT()   UCSR0B |= (1<<UDRIE0)

#ifdef GATEWAY

#define RXD                     PORTD0
#define TXD                     PORTD1

#define USART_CONFIG_PORT()     {PRR &= ~(1<<PRUSART0); \
                                 PORTD |= (1<<RXD) | (1<<TXD); DDRD |= (1<<TXD); DDRD &= ~(1<<RXD);}
#define USART_BAUD              ((F_CPU/16/38400) - 1) // Baud = 38400, val = Fosc/(16 * baud) - 1
#define USART_CONFIGURE()       {UCSR0B = ((1<<RXCIE0) | (1<<RXEN0) | (1<<TXEN0));  \
                                 UCSR0C = (3<<UCSZ00);}
#include "uart.h"
// End USART Section

#else   //  !GATEWAY

// Serial Output
#define SER_PIN_TX              25
#define SER_PIN_RX              24

#define SER_ENABLE()            PRR &= ~(1<<PRUSART0)

#define SER_ENABLE_RX()         {UCSR0B |= (1<<RXCIE0) | (1<<RXEN0); UCSR0C = (3<<UCSZ00);}
#define SER_DISABLE_RX()        {UCSR0B &= ~((1<<RXCIE0) | (1<<RXEN0));             \
                                 if((UCSR0B & ((1<<TXEN0) | (1<<RXEN0))) == 0)      \
                                    PRR &= ~(1<<PRUSART0);}
#define SER_ENABLE_TX()         {UCSR0B |= (1<<TXEN0); UCSR0C = (3<<UCSZ00);}
#define SER_DISABLE_TX()        {UCSR0B &= ~(1<<TXEN0);                             \
                                 if((UCSR0B & ((1<<TXEN0) | (1<<RXEN0))) == 0)      \
                                    PRR |= (1<<PRUSART0);}
#endif  //  GATEWAY

// Timer Section
#define POOL_TMR_FREQ           64     // Pool Frequency (Hz)
#define TIMER_ISR               TIMER2_COMPA_vect

#ifdef USE_RTC_OSC
#define InitTimer()             {ASSR = (1<<AS2);                                   \
                                 TCCR2A = (1<<WGM21); TCNT2 = 0;                    \
                                 OCR2A = ((32768/8/POOL_TMR_FREQ)-1);               \
                                 TIFR2 = (1<<OCF2A); TIMSK2 = (1<<OCIE2A);          \
                                 TCCR2B = (1<<WGM22) | (2 << CS20);                 \
                                 while(ASSR & 0x1F);}
#define config_sleep_rtc()      {TCCR2A = 0; TCCR2B = (5 << CS20); while(ASSR & 0x1F);}
#else   //  USE_RTC_OSC
#define InitTimer()             {TCCR2A = (1<<WGM21); TCNT2 = 0;            \
                                 OCR2A = ((F_CPU/1024/POOL_TMR_FREQ)-1);    \
                                 TIFR2 = (1<<OCF2A); TIMSK2 = (1<<OCIE2A);  \
                                 TCCR2B =(1<<WGM22) | (7<<CS20);}
#define config_sleep_wdt()      {wdt_reset(); MCUSR &= ~(1<<WDRF);                      \
                                 WDTCSR |= (1<<WDCE) | (1<<WDE); WDTCSR = (6<<WDP0);    \
                                 WDTCSR |= (1<<WDIF); WDTCSR |= (1<<WDIE);}
#endif  //  USE_RTC_OSC
// End Timer Section

// Digital IO's
#define EXTDIO_MAXPORT_NR       2           // Number of digital Ports

#define PORTNUM2                0
#define PORTDDR2                DDRC
#define PORTOUT2                PORTC
#define PORTIN2                 PINC
#define PORT2MASK               0xC0        // PC0-PC5

#define PORTNUM3                1
#define PORTDDR3                DDRD
#define PORTOUT3                PORTD
#define PORTIN3                 PIND

#ifdef GATEWAY
#define PORT3MASK               0x03
#else   // GATEWAY
#define PORT3MASK               0x00
#endif  // GATEWAY

// PWM
#define PWM_PIN0                29
#define PWM_PIN1                30
#define PWM_OCR0                OCR0B
#define PWM_OCR1                OCR0A
#define DISABLE_PWM0()          TCCR0A &= ~(1<<COM0B1)
#define DISABLE_PWM1()          TCCR0A &= ~(1<<COM0A1)
#define ENABLE_PWM0()           TCCR0A |= (1<<COM0B1)
#define ENABLE_PWM1()           TCCR0A |= (1<<COM0A1)
#define PWM_ENABLE()        {if((TCCR0A & ((1<<COM0A1) | (1<<COM0B1))) == 0)    \
                             {PRR &= ~(1<<PRTIM0); TCCR0A = (3<<WGM00); TCCR0B = (4<<CS00);}}
#define PWM_DISABLE()       {if((TCCR0A & ((1<<COM0A1) | (1<<COM0B1))) == 0)    \
                             {TCCR0A = 0; TCCR0B = 0; PRR |= (1<<PRTIM0);}}
// End PWM
// End Digital IO's

// Analog Inputs
#define EXTAI_PORT_NUM          PORTNUM2    // PORTC Analog Inputs
#define EXTAI_CHN_MASK          0x0F
#define EXTAI_MAXPORT_NR        10          // ADC0-ADC7, ADC8, Vbg

//#define EXTAI_RSSI_IN           6
//#define EXTAI_RSSI_BASE         ((1<<6) | (1<<REFS0))

#define ENABLE_ADC()            {PRR &= ~(1<<PRADC); ADMUX = 0x0F; ADCSRA = (1<<ADEN) | \
                                        (1<<ADSC) | (1<<ADIF) | (1<<ADIE) | (7<<ADPS0);}
#define DISABLE_ADC()           {ADCSRA = (1<<ADIF); ADMUX = 0x0F; PRR |= (1<<PRADC);}
// End Analog Inputs

// TWI(I2C)
#define TWI_PIN_SDA             20      // Pin base
#define TWI_PIN_SCL             21      // Pin base
#define TWI_TWBR                (((F_CPU/100000UL)-16)/2)   // 100kHz
#define TWI_ENABLE()            {PRR &= ~(1<<PRTWI); \
                                 TWBR = TWI_TWBR; TWDR = 0xFF; TWCR = (1<<TWEN);}
#define TWI_DISABLE()           {TWCR = 0; PRR |= (1<<PRTWI);}
// End TWI

// RF Section
#define RF_DDR                  DDRB
#define RF_PORT                 PORTB
#define RF_PIN                  PINB
#define RF_PIN_IRQ              PORTB0
#define RF_LEDS                 PORTB1
#define RF_PIN_SS               PORTB2
#define RF_PIN_MOSI             PORTB3
#define RF_PIN_MISO             PORTB4
#define RF_PIN_SCK              PORTB5

#define TxLEDon()               RF_PORT &= ~(1<<RF_LEDS);
#define RxLEDon()               RF_PORT &= ~(1<<RF_LEDS);
#define LEDsOff()               RF_PORT |= (1<<RF_LEDS);

#define RF_PORT_INIT()          {RF_PORT = (1<<RF_PIN_SS) | (1<<RF_PIN_IRQ) | (1<<RF_LEDS); \
                                 RF_DDR = (1<<RF_PIN_SCK) | (1<<RF_PIN_MOSI) |              \
                                          (1<<RF_PIN_SS) | (1<<RF_LEDS);}

#define RF_SELECT()             RF_PORT &= ~(1<<RF_PIN_SS)
#define RF_RELEASE()            RF_PORT |= (1<<RF_PIN_SS)

#define RF_SPI_DATA             SPDR
#define RF_SPI_BISY             (!(SPSR &(1<<SPIF)))

#if (F_CPU > 10000000UL)

#define RF_SPI_INIT()           {SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR0); SPSR = (1<<SPI2X);}

#define RF_SPI_SLOW()           SPCR |= (1<<SPR0)
#define RF_SPI_FAST()           SPCR &= ~(1<<SPR0)

#else   //  (F_CPU <= 10000000UL)

#define RF_SPI_INIT()           {SPCR = (1<<SPE) | (1<<MSTR); SPSR = 0;}

#define RF_SPI_SLOW()           SPSR = 0
#define RF_SPI_FAST()           SPSR = (1<<SPI2X)

#endif  //  (F_CPU > 10000000UL)

#define RF_STAT_IRQ             (RF_PIN & (1<<RF_PIN_IRQ))
#define RF_IRQ_CFG()            PCICR = (1<<PCIE0)

#define RF_INT_vect             PCINT0_vect

#define RF_ENABLE_IRQ()         PCMSK0 = (1<<RF_PIN_IRQ)
#define RF_DISABLE_IRQ()        PCMSK0 = 0

#include "RF/rfm12/rfm12.h"

#endif
