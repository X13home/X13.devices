/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _A1SR11_H
#define _A1SR11_H

// uNode Version 1.1
// uc ATMega328p
// Phy1: RFM12, IRQ at PB0

// 0 - 7    PORTA - not exist
// PORTB
// --   PB0     --          RFM12_IRQ
// --   PB1     --          LED
// --   PB2     --          RFM12_CSN
// --   PB3     ISP-4       RFM12_MOSI
// --   PB4     ISP-1       RFM12_MISO
// --   PB5     ISP-3       RFM12_SCK
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
// 26   PD2     SV1-8       IRQ 0
// 27   PD3     SV1-7       IRQ 1
// 28   PD4     SV1-6
// 29   PD5     SV1-5       PWM0
// 30   PD6     SV1-4       PWM1
// 31   PD7     SV1-3

#ifdef __cplusplus
extern "C" {
#endif

#define F_CPU                       14745600UL

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_PORT_OFFSET          2
#define EXTDIO_MAXPORT_NR           2           // Number of used physical digital Ports
#define HAL_DIO_MAPPING             {0xFF, 0xFF, 16, 17, 18, 19, 20, 21,    \
                                     0xFF, 0xFF, 26, 27, 28, 29, 30, 31}
// End DIO Section

// PWM Section
#define EXTPWM_USED                 1
#define HAL_PWM_BASE_OFFSET         13
#define HAL_PWM_PORT2CFG            {1, 0}      // Mapping PWM channel to configuration
                                                // bits 7-3 Timer, bits 2-0 Channel
// End PWM Section

// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           15                                  // ADC0-ADC7, Vbg
#define HAL_AIN_BASE2APIN           {6, 7, 0, 1, 2, 3, 4, 5, 14}
// End Analogue Inputs

// TWI Section
#define EXTTWI_USED                 1
// End TWI Section

// LED
#define LED_On()                    PORTB &= ~(1<<PB1)
#define LED_Off()                   PORTB |= (1<<PB1)
#define LED_Init()                  {DDRB |= (1<<PB1); PORTB |= (1<<PB1);}

// UART PHY Section
#define HAL_USE_USART0              0
#define HAL_UART_NUM_PORTS          1
#define UART_PHY_PORT               0
#define UART_PHY                    1
#include "PHY/UART/uart_phy.h"
// End UART PHY Section

// RF Section
#define RFM12_PORT                  PORTB
#define RFM12_DDR                   DDRB
#define RFM12_PIN_SS                PB2
#define RFM12_PIN_MOSI              PB3
#define RFM12_PIN_MISO              PB4
#define RFM12_PIN_SCK               PB5
#define RFM12_IRQ_PORT              PORTB
#define RFM12_IRQ_PORT_PIN          PINB
#define RFM12_IRQ_PIN               PB0
#define RFM12_PHY                   2
#include "PHY/RFM12/rfm12_phy.h"
//  End RF Section

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           15      // Size of identificators list
#define OD_DEV_UC_TYPE              'A'
#define OD_DEV_UC_SUBTYPE           '1'
#define OD_DEV_PHY1                 'S'
#define OD_DEV_PHY2                 'R'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '1'

#ifdef __cplusplus
}
#endif

#endif // _A1SC12_H
