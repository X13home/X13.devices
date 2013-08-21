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

#define SystemReset()           {cli();asm("jmp 0x0000");}

// Power Reduction
#define CONFIG_PRR()            {ACSR = (1<<ACD); \
                                PRR = (1<<PRTWI) | (1<<PRTIM0) | (1<<PRTIM1) | \
                                (1<<PRUSART0) | (1<<PRADC);}
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
// Timer Section
#define POOL_TMR_FREQ           64     // Pool Frequency (Hz)
#define TIMER_ISR               TIMER2_COMPA_vect
#define InitTimer()             {TCCR2A = (1<<WGM21); TCNT2 = 0;            \
                                OCR2A = ((F_CPU/1024/POOL_TMR_FREQ)-1);    \
                                TIFR2 = (1<<OCF2A); TIMSK2 = (1<<OCIE2A);  \
                                TCCR2B =(1<<WGM22) | (7<<CS20);}
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
#define PORT3MASK               0x04

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
#define EXTAI_BASE_2_APIN       {0, 1, 2, 3, 4, 5, 0xFF, 6, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 7, 0xFF}
#define EXTAI_MAXPORT_NR        8          // ADC0-ADC5, ADC7, Vbg

#define ENABLE_ADC()            {PRR &= ~(1<<PRADC); ADMUX = 0x0F; \
                                 ADCSRA = (1<<ADEN) | (1<<ADIF) | (1<<ADIE) | (7<<ADPS0);  \
                                 ADCSRA |= (1<<ADSC); }
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
}s_Addr;

#define AddrBroadcast {{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}, {0xFF,0xFF,0xFF,0xFF}}

#endif
