/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef HWCONFIG_A1CN12_H
#define HWCONFIG_A1CN12_H

// uNode Version 2.0
// uc ATMega328p
// Phy1: CC1101

// 0 - 7    PORTA - not exist
// PORTB
// --   PB0     --          LED
// --   PB1     --          CC11_GDO
// --   PB2     --          CC11_CSN
// --   PB3     ISP-4       CC11_MOSI
// --   PB4     ISP-1       CC11_MISO
// --   PB5     ISP-3       CC11_SCK
// --   PB6     --          OSC
// --   PB7     --          OSC
// PORT C
// 16   PC0     SV1-18      Ain0
// 17   PC1     SV1-17      Ain1
// 18   PC2     SV1-16      Ain2
// 19   PC3     SV1-15      Ain3
// 20   PC4     SV1-14      SDA
// 21   PC5     SV1-13      SCL
// --   PC6     ISP-5       RESET
// --   --      SV1-20      Ain6
// --   --      SV1-19      Ain7
// PORT D
// 24   PD0     SV1-10      RXD
// 25   PD1     SV1-9       TXD
// 26   PD2     SV1-8       IRQ 0 //** RF-GDO
// 27   PD3     SV1-7       IRQ 1
// 28   PD4     SV1-6
// 29   PD5     SV1-5       PWM0
// 30   PD6     SV1-4       PWM1
// 31   PD7     SV1-3

#ifdef __cplusplus
extern "C" {
#endif

#define F_CPU                       8000000UL

#define ASLEEP                      1       // Enable ASleep mode
#define OD_DEFAULT_TASLEEP          0       // Default ASleep disabled

#include "AVR/hal.h"
#include <util/delay.h>

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_BASE_OFFSET          2
#define EXTDIO_MAXPORT_NR           2                                   // Number of digital Ports
#define EXTDIO_MAPPING              {0xFF, 0xFF, 16, 17, 18, 19, 20, 21, 24, 25, 26, 27, 28, 29, 30, 31}
// End DIO Section

// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           9                                   // ADC0-ADC7, Vbg
#define EXTAIN_BASE_2_APIN          {6, 7, 0, 1, 2, 3, 4, 5, 14}
#define EXTAIN_REF                  0x06                                // Bit0 - Ext, Bit1 - Vcc, Bit2 - Int1, Bit3 - Int2
// End Analogue Inputs

// TWI Section
#define EXTTWI_USED                 1
// End TWI Section

/*
// PWM Section
//#define EXTPWM_USED                 1
#define EXTPWM_MAXPORT_NR           2
#define EXTPWM_PORT2CFG             {1, 0}          // bits 7-3 Timer, bits 2-0 Channel
#define EXTPWM_PORT2DIO             {29,30}         // Mapping PWM channel to DIO
// End PWM Section

// UART Section
#define HAL_USE_USART0              0
#define HAL_UART_NUM_PORTS          1

#define EXTSER_USED                 1
// End UART Section
*/

// LEDs
#define LED1_On()                   PORTB &= ~(1<<PB0)
#define LED1_Off()                  PORTB |= (1<<PB0)
#define LEDsInit()                  {DDRB |= (1<<PB0); PORTB |= (1<<PB0);}

// RF Section
#define CC11_PORT                   PORTB
#define CC11_DDR                    DDRB
#define CC11_PIN                    PINB
#define CC11_PIN_SS                 PB2
#define CC11_PIN_MOSI               PB3
#define CC11_PIN_MISO               PB4
#define CC11_PIN_SCK                PB5

#define CC11_WAIT_LOW_MISO()        while(CC11_PIN & (1<<CC11_PIN_MISO))
#define CC11_SELECT()               CC11_PORT &= ~(1<<CC11_PIN_SS)
#define CC11_RELEASE()              CC11_PORT |= (1<<CC11_PIN_SS)
//  End RF Section

#define CC11_PHY                    1

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           17      // Size of identificators list
#define OD_DEV_UC_TYPE              'A'
#define OD_DEV_UC_SUBTYPE           '1'
#define OD_DEV_PHY1                 'C'
#define OD_DEV_PHY2                 'n'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '2'

#include "../PHY/CC1101/cc11_phy.h"

#ifdef __cplusplus
}
#endif

#endif // HWCONFIG_A1CN12_H