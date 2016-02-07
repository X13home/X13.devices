/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef HWCONFIG_A1SN12_H
#define HWCONFIG_A1SN12_H

// uNode Version 2.0
// uc ATMega328p
// Phy1: UART

// 0 - 7    PORTA - not exist
// PORTB
//  8   PB0     --      LED
//  9   PB1     --      OC1A
// 10   PB2     --      OC1B
// 11   PB3     ISP-4
// 12   PB4     ISP-1
// 13   PB5     ISP-3
// 14   PB6     --      OSC
// 15   PB7     --      OSC
// PORT C
// 16   PC0     SV1-18  Ain0
// 17   PC1     SV1-17  Ain1
// 18   PC2     SV1-16  Ain2
// 19   PC3     SV1-15  Ain3
// 20   PC4     SV1-14  SDA
// 21   PC5     SV1-13  SCL
// --   PC6     ISP-5   RESET
// --   --      SV1-20  Ain6
// --   --      SV1-19  Ain7
// PORT D
// 24   PD0     SV1-10  RXD
// 25   PD1     SV1-9   TXD
// 26   PD2     SV1-8
// 27   PD3     SV1-7
// 28   PD4     SV1-6
// 29   PD5     SV1-5   OC0B
// 30   PD6     SV1-4   OC0A
// 31   PD7     SV1-3

#ifdef __cplusplus
extern "C" {
#endif

#define F_CPU                       8000000UL

//#define ASLEEP                      1       // Enable ASleep mode
//#define OD_DEFAULT_TASLEEP          0       // Default ASleep disabled

////////////////////////////////////////////////////////
// Extensions configuration

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_PORT_OFFSET          1
#define EXTDIO_MAXPORT_NR           3                                   // Number of used physical digital Ports
#define HAL_DIO_MAPPING             {0xFF, 0xFF, 16, 17, 18, 19, 20, 21, 0xFF, 0xFF, 26, 27, 28, 29, 30, 31, 9, 10, 11, 12, 13}
// End DIO Section

// PWM Section
#define EXTPWM_USED                 1
#define EXTPWM_BASE_OFFSET          13
#define EXTPWM_PORT2CFG             {1, 0, 0xFF, 8, 9}                  // Mapping PWM channel to configuration
                                                                        // bits 7-3 Timer, bits 2-0 Channel
#define EXTPWM_PORT2DIO             {29, 30, 0xFF, 9, 10}               // Mapping PWM channel to DIO
// End PWM Section

// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           9                                   // ADC0-ADC7, Vbg
#define EXTAIN_BASE_2_APIN          {6, 7, 0, 1, 2, 3, 4, 5, 14}
#define EXTAIN_REF                  0x06                                // Bit0 - Ext, Bit1 - Vcc, Bit2 - Int1, Bit3 - Int2
// End Analogue Inputs

// TWI Section
#define EXTTWI_USED                 1
// End TWI Section

// End Extensions configuration
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// Start PHY Section
// LED
#define LED_On()                    PORTB &= ~(1<<PB0)
#define LED_Off()                   PORTB |= (1<<PB0)
#define LED_Init()                  {DDRB |= (1<<PB0); PORTB |= (1<<PB0);}

// UART Section
#define HAL_USE_USART0              0       // Logical Port Number 0,1,2,...
#define HAL_UART_NUM_PORTS          1

#define UART_PHY_PORT               0       // Logical Port Number 0,1,2,...
// End UART Section

#define UART_PHY                    1

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           20      // Size of identificators list
#define OD_DEV_UC_TYPE              'A'
#define OD_DEV_UC_SUBTYPE           '1'
#define OD_DEV_PHY1                 'S'
#define OD_DEV_PHY2                 'n'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '2'

#include "PHY/UART/uart_phy.h"

#ifdef __cplusplus
}
#endif

#endif // HWCONFIG_A1SN12_H
