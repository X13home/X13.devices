/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// Default Hardware definitions

#ifndef _HWCONFIG_DEFAULT_ATM_H
#define _HWCONFIG_DEFAULT_ATM_H

#ifndef SYSTEM_RESET
#define SYSTEM_RESET()          {cli();asm("jmp 0x0000");}
#endif  //  SystemReset

// Power Reduction
#ifndef PRR // ATMega2560
#define PRR                     PRR0
#endif  //  PRR

#ifndef CONFIG_PRR
#define CONFIG_PRR()            {ACSR = (1<<ACD);                   \
                                 PRR = (1<<PRTWI) | (1<<PRTIM0) | (1<<PRTIM1) | \
                                       (1<<PRUSART0) | (1<<PRADC);}
#endif  //  CONFIG_PRR()

// Analogue Inputs
#ifndef EXTAIN_CHN_MASK
#ifndef MUX4
#define EXTAIN_CHN_MASK         0x0F
#elif (defined MUX5)
#define EXTAIN_CHN_MASK         0x3F
#else
#define EXTAIN_CHN_MASK         0x1F
#endif  //  MUX4/MUX5
#endif  //  EXTAIN_CHN_MASK

#ifndef EXTAIN_SELECT
#ifdef MUX5
#define EXTAIN_SELECT(val)      {ADMUX = (val & 0xDF); ADCSRB = ((val & 0x20)>>2);}
#else //  !MUX5
#define EXTAIN_SELECT(val)      ADMUX = (uint8_t)val
#endif  //  MUX5
#endif  //  EXTAIN_SELECT

#ifndef EXTAIN_MUX_GND
#define EXTAIN_MUX_GND          0x0F
#endif  //  EXTAIN_MUX_GND

#ifndef ENABLE_ADC
#define ENABLE_ADC()            PRR &= ~(1<<PRADC)
#endif  //  ENABLE_ADC

#ifndef DISABLE_ADC
#define DISABLE_ADC()           {ADCSRA = (1<<ADIF); EXTAIN_SELECT(EXTAIN_MUX_GND); PRR |= (1<<PRADC);}
#endif  //  DISABLE_ADC
// End Analogue Inputs

// TWI(I2C) Section
#ifndef TWI_PIN_SDA
#define TWI_PIN_SDA             20      // Pin base PC4, for ATMega xx8P[A]
#endif  //  TWI_PIN_SDA

#ifndef TWI_PIN_SCL
#define TWI_PIN_SCL             21      // Pin base PC5, for ATMega xx8P[A]
#endif  //  TWI_PIN_SCL

#ifndef TWI_TWBR
#define TWI_TWBR                (((F_CPU/100000UL)-16)/2)   // 100kHz
#endif  //  TWI_TWBR

#ifndef TWI_ENABLE
#define TWI_ENABLE()            {PRR &= ~(1<<PRTWI); \
                                 TWBR = TWI_TWBR; TWDR = 0xFF; TWCR = (1<<TWEN);}
#endif  //  TWI_ENABLE

#ifndef TWI_DISABLE
#define TWI_DISABLE()           {TWCR = 0; PRR |= (1<<PRTWI);}
#endif  //  TWI_DISABLE
// End TWI Section

// USART Section
#ifndef USART_DATA
#define USART_DATA              UDR0
#endif  //  USART_DATA

#ifndef USART_ENABLE_DREINT
#define USART_ENABLE_DREINT()   UCSR0B |= (1<<UDRIE0)
#endif  //  USART_ENABLE_DREINT

#ifndef USART_DISABLE_DREINT
#define USART_DISABLE_DREINT()  UCSR0B &= ~(1<<UDRIE0)
#endif  //  USART_DISABLE_DREINT

#ifndef USART_SET_BAUD
#define USART_SET_BAUD(baud)    {if(baud & 0xF000){baud &= 0x0FFF; UCSR0A |= (1<<U2X0);}    \
                                 UBRR0H = (baud>>8); UBRR0L = (baud & 0xFF);}
#endif  //  USART_SET_BAUD

#ifndef USART_PIN_RX
#define USART_PIN_RX            PORTD0
#endif  //  USART_PIN_RX

#ifndef USART_PIN_TX
#define USART_PIN_TX            PORTD1
#endif  //  USART_PIN_TX

#ifndef USART_PORT
#define USART_PORT              PORTD
#endif  //  USART_PORT

#ifndef USART_DDR
#define USART_DDR               DDRD
#endif  //  USART_PORT

#ifndef USART_CONFIG_PORT
#define USART_CONFIG_PORT()     {PRR &= ~(1<<PRUSART0);               \
                                 USART_PORT |= (1<<USART_PIN_RX) | (1<<USART_PIN_TX);   \
                                 USART_DDR |= (1<<USART_PIN_TX); USART_DDR &= ~(1<<USART_PIN_RX);}
#endif  // USART_CONFIG_PORT

#ifndef USART_BAUD
#define USART_BAUD              ((F_CPU/16/38400) - 1) // Baud = 38400, val = Fosc/(16 * baud) - 1
#endif  //  USART_BAUD

#ifndef USART_CONFIGURE
#define USART_CONFIGURE()       {UCSR0B = ((1<<RXCIE0) | (1<<RXEN0) | (1<<TXEN0));  \
                                 UCSR0C = (3<<UCSZ00);}
#endif  //  USART_CONFIGURE

// Serial Output
#ifndef SER_PIN_RX
#define SER_PIN_RX              24
#endif

#ifndef SER_PIN_TX
#define SER_PIN_TX              25
#endif

#ifndef SER_ENABLE
#define SER_ENABLE()            PRR &= ~(1<<PRUSART0)
#endif

#ifndef SER_DISABLE
#define SER_DISABLE()           PRR |= (1<<PRUSART0)
#endif  //  SER_DISABLE

#ifndef SER_DATA
#define SER_DATA                UDR0
#endif  //  SER_DATA

#ifndef SER_UCSRXB
#define SER_UCSRXB              UCSR0B
#endif  //  SER_UCSRXB

#ifndef SER_UCSRXC
#define SER_UCSRXC              UCSR0C
#endif  //  SER_UCSRXC

#ifndef SER_RX_VECT
#define SER_RX_VECT             USART_RX_vect
#endif  //  SER_RX_VECT

#ifndef SER_UDRE_VECT
#define SER_UDRE_VECT           USART_UDRE_vect
#endif  //  SER_UDRE_VECT

#ifndef SER_SET_BAUD
#define SER_SET_BAUD(baud)    {if(baud & 0xF000){baud &= 0x0FFF; UCSR0A |= (1<<U2X0);}    \
                                 UBRR0H = (baud>>8); UBRR0L = (baud & 0xFF);}
#endif  //  SER_SET_BAUD

#ifndef SER_IS_ENABLED
#define SER_IS_ENABLED()        (SER_UCSRXB & ((1<<TXEN0) | (1<<RXEN0)))
#endif  //  SER_IS_ENABLED

#ifndef SER_ENABLE_RX
#define SER_ENABLE_RX()         {SER_UCSRXB |= (1<<RXCIE0) | (1<<RXEN0); SER_UCSRXC = (3<<UCSZ00);}
#endif  //  SER_ENABLE_RX

#ifndef SER_DISABLE_RX
#define SER_DISABLE_RX()        SER_UCSRXB &= ~((1<<RXCIE0) | (1<<RXEN0))
#endif  //  SER_DISABLE_RX

#ifndef SER_ENABLE_TX
#define SER_ENABLE_TX()         {SER_UCSRXB |= (1<<TXEN0); UCSR0C = (3<<UCSZ00);}
#endif  //  SER_ENABLE_TX

#ifndef SER_DISABLE_TX
#define SER_DISABLE_TX()        SER_UCSRXB &= ~(1<<TXEN0)
#endif  //  SER_DISABLE_TX

#ifndef SER_ENABLE_DREINT
#define SER_ENABLE_DREINT()     SER_UCSRXB |= (1<<UDRIE0)
#endif  //  SER_ENABLE_DREINT

#ifndef SER_DISABLE_DREINT
#define SER_DISABLE_DREINT()    SER_UCSRXB &= ~(1<<UDRIE0)
#endif  //  SER_DISABLE_DREINT
// End USART Section

// PWM Section
#ifndef PWM_PIN0
#define PWM_PIN0                29
#endif  //  PWM_PIN0

#ifndef PWM_PIN1
#define PWM_PIN1                30
#endif  //  PWM_PIN1

#ifndef PWM_OCR0
#define PWM_OCR0                OCR0B
#endif  //  PWM_OCR0

#ifndef PWM_OCR1
#define PWM_OCR1                OCR0A
#endif  //  PWM_OCR1

#ifndef ENABLE_PWM0
#define ENABLE_PWM0()           TCCR0A |= (1<<COM0B1)
#endif  //  ENABLE_PWM0

#ifndef ENABLE_PWM1
#define ENABLE_PWM1()           TCCR0A |= (1<<COM0A1)
#endif  //  ENABLE_PWM1

#ifndef DISABLE_PWM0
#define DISABLE_PWM0()          TCCR0A &= ~(1<<COM0B1)
#endif  //  DISABLE_PWM1

#ifndef DISABLE_PWM1
#define DISABLE_PWM1()          TCCR0A &= ~(1<<COM0A1)
#endif  //  DISABLE_PWM1

#ifndef PWM_ENABLE
#define PWM_ENABLE()        {if((TCCR0A & ((1<<COM0A1) | (1<<COM0B1))) == 0)    \
                             {PRR &= ~(1<<PRTIM0); TCCR0A = (3<<WGM00); TCCR0B = (4<<CS00);}}
#endif  //  PWM_ENABLE
                      
#ifndef PWM_DISABLE
#define PWM_DISABLE()       {if((TCCR0A & ((1<<COM0A1) | (1<<COM0B1))) == 0)    \
                             {TCCR0A = 0; TCCR0B = 0; PRR |= (1<<PRTIM0);}}
#endif  //  PWM_DISABLE
// End PWM Section

// Timer Section
#ifndef POLL_TMR_FREQ
#define POLL_TMR_FREQ           64     // Polling Frequency (Hz)
#endif  //  POLL_TMR_FREQ

#ifndef TIMER_ISR
#define TIMER_ISR               TIMER2_COMPA_vect
#endif  //  TIMER_ISR

#ifndef INIT_TIMER
#ifdef USE_RTC_OSC
#define INIT_TIMER()            {PRR &= ~(1<<PRTIM1);                               \
                                 ASSR = (1<<AS2);                                   \
                                 TCCR2A = (1<<WGM21); TCNT2 = 0;                    \
                                 OCR2A = ((32768/8/POLL_TMR_FREQ)-1);               \
                                 TIFR2 = (1<<OCF2A); TIMSK2 = (1<<OCIE2A);          \
                                 TCCR2B = (2<<CS20);                                \
                                 while(ASSR & 0x1F);}
#define CONFIG_SLEEP_RTC()      {TCCR2A = 0; TCCR2B = (5 << CS20); while(ASSR & 0x1F);}
#else   //  !USE_RTC_OSC
#define INIT_TIMER()            {TCCR2A = (1<<WGM21); TCNT2 = 0;            \
                                 OCR2A = ((F_CPU/1024/POLL_TMR_FREQ)-1);    \
                                 TIFR2 = (1<<OCF2A); TIMSK2 = (1<<OCIE2A);  \
                                 TCCR2B = (7<<CS20);}
#define CONFIG_SLEEP_WDT()      {wdt_reset(); MCUSR &= ~(1<<WDRF);                      \
                                 WDTCSR = (1<<WDCE) | (1<<WDE);                         \
                                 WDTCSR = (1<<WDIF) | (1<<WDIE) | (1<<WDE) | (6<<WDP0);}
#endif  //  USE_RTC_OSC
#endif  //  InitTimer
// End Timer Section

// RF Section
#ifdef RF_NODE
#ifndef S_ADDR
#define S_ADDR                  uint8_t
#endif  //  S_ADDR

#ifndef ADDR_BROADCAST
#define ADDR_BROADCAST          0
#endif  //  ADDR_BROADCAST

#ifndef OD_DEFAULT_ADDR
#define OD_DEFAULT_ADDR         0xFF
#endif  //  OD_DEFAULT_ADDR

#ifndef OD_DEFAULT_GROUP
#define OD_DEFAULT_GROUP        0x2DD4
#endif  //  OD_DEFAULT_GROUP

#ifndef OD_DEFAULT_CHANNEL
#define OD_DEFAULT_CHANNEL      0x12
#endif  //  OD_DEFAULT_CHANNEL

// Ports
#ifndef RF_DDR
#define RF_DDR                  DDRB
#endif  //  RF_DDR

#ifndef RF_PORT
#define RF_PORT                 PORTB
#endif  //  RF_PORT

#ifndef RF_PIN
#define RF_PIN                  PINB
#endif  //  RF_PIN

// Pins
#ifndef RF_PIN_IRQ
#define RF_PIN_IRQ              PORTB1    // UNode v 2.0
#endif

#ifndef RF_PIN_SS
#define RF_PIN_SS               PORTB2
#endif  //  RF_PIN_SS

#ifndef RF_PIN_MOSI
#define RF_PIN_MOSI             PORTB3
#endif  //  RF_PIN_MOSI

#ifndef RF_PIN_MISO
#define RF_PIN_MISO             PORTB4
#endif  //  RF_PIN_MISO

#ifndef RF_PIN_SCK
#define RF_PIN_SCK              PORTB5
#endif  //  RF_PIN_SCK

#ifndef RF_PORT_INIT
#define RF_PORT_INIT()          {RF_PORT |= (1<<RF_PIN_SS) | (1<<RF_PIN_IRQ);     \
                                 RF_DDR &= ~((1<<RF_PIN_IRQ) | (RF_PIN_MISO));    \
                                 RF_DDR |= (1<<RF_PIN_SCK) | (1<<RF_PIN_MOSI) |   \
                                          (1<<RF_PIN_SS) | (1<<RF_LEDS);}
#endif  //  RF_PORT_INIT

#ifndef RF_SELECT
#define RF_SELECT()             RF_PORT &= ~(1<<RF_PIN_SS)
#endif  //  RF_SELECT

#ifndef RF_RELEASE
#define RF_RELEASE()            RF_PORT |= (1<<RF_PIN_SS)
#endif  //  RF_RELEASE

#ifndef RF_SPI_DATA
#define RF_SPI_DATA             SPDR
#endif  //  RF_SPI_DATA

#ifndef RF_SPI_BISY
#define RF_SPI_BISY             (!(SPSR &(1<<SPIF)))
#endif  //  RF_SPI_BISY

#ifndef RF_STAT_IRQ
#define RF_STAT_IRQ             (RF_PIN & (1<<RF_PIN_IRQ))
#endif  //  RF_STAT_IRQ

#ifndef RF_IRQ_CFG
#define RF_IRQ_CFG()            PCICR = (1<<PCIE0)
#endif  //  RF_IRQ_CFG

#ifndef RF_INT_vect
#define RF_INT_vect             PCINT0_vect
#endif  //  RF_INT_vect

#ifndef RF_ENABLE_IRQ
#define RF_ENABLE_IRQ()         PCMSK0 = (1<<RF_PIN_IRQ)
#endif  //  RF_ENABLE_IRQ

#ifndef RF_DISABLE_IRQ
#define RF_DISABLE_IRQ()        PCMSK0 = 0
#endif  //  RF_DISABLE_IRQ
#endif  //  RF_NODE

#endif