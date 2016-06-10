/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _A1EN12_H
#define _A1EN12_H

// uNode Version 2.0
// uc ATMega328p
// Phy1: ENC28J60

// 0 - 7    PORTA - not exist
// PORTB
// --   PB0     --      LED
// --   PB1     --      ENC_IRQ / NC
// --   PB2     --      ENC_CSN
// --   PB3     ISP-4   ENC_MOSI
// --   PB4     ISP-1   ENC_MISO
// --   PB5     ISP-3   ENC_SCK
// --   PB6     --      OSC
// --   PB7     --      OSC
// PORT C
// 16   PC0     SV1-18  Ain0
// 17   PC1     SV1-17  Ain1
// 18   PC2     SV1-16  Ain2
// 19   PC3     SV1-15  Ain3
// 20   PC4     SV1-14  SDA
// 21   PC5     SV1-13  SCL
// --   PC6      ISP-5  RESET
// --   --      SV1-20  Ain6
// --   --      SV1-19  Ain7
// PORT D
// 24   PD0     SV1-10  RXD
// 25   PD1     SV1-9   TXD
// 26   PD2     SV1-8
// 27   PD3     SV1-7
// 28   PD4     SV1-6
// 29   PD5     SV1-5   PWM0
// 30   PD6     SV1-4   PWM1
// 31   PD7     SV1-3

#ifdef __cplusplus
extern "C" {
#endif

#define F_CPU                       12500000UL

#include <util/delay.h>

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_PORT_OFFSET          2
#define EXTDIO_MAXPORT_NR           2           // Number of used physical digital Ports
#define HAL_DIO_MAPPING             {0xFF, 0xFF, 16, 17, 18, 19, 20, 21,    \
                                       24, 25, 26, 27, 28, 29, 30, 31}
// End DIO Section

// PWM Section
#define EXTPWM_USED                 1
#define HAL_PWM_BASE_OFFSET         13
#define HAL_PWM_PORT2CFG            {1, 0}      // Mapping PWM channel to configuration
                                                // bits 7-3 Timer, bits 2-0 Channel
// End PWM Section

// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           15                                   // ADC0-ADC7, Vbg
#define HAL_AIN_BASE2APIN           {6, 7, 0, 1, 2, 3, 4, 5, 14}
// End Analogue Inputs

// UART Section
#define HAL_USE_USART0              0
#define HAL_UART_NUM_PORTS          1

#define EXTSER_USED                 1
// End UART Section

// TWI Section
#define EXTTWI_USED                 1
// End TWI Section

// LED
#define LED_On()                    PORTB &= ~(1<<PB0)
#define LED_Off()                   PORTB |= (1<<PB0)
#define LED_Init()                  {DDRB |= (1<<PB0); PORTB |= (1<<PB0);}

// ENC28J60 Section
#define HAL_USE_SPI                 1
#define ENC_USE_SPI                 1
#define ENC_NSS_PIN                 10  // PB2
#define ENC_SELECT()                (PORTB &= ~(1<<PB2))
#define ENC_RELEASE()               (PORTB |= (1<<PB2))
#define ENC28J60_PHY                1
#include "PHY/ENC28J60/enc28j60_phy.h"
// End ENC28J60 Section

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           17      // Size of identificators list
#define OD_DEV_UC_TYPE              'A'
#define OD_DEV_UC_SUBTYPE           '1'
#define OD_DEV_PHY1                 'E'
#define OD_DEV_PHY2                 'n'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '2'

#define OD_DEV_MAC                  {0x00,0x04,0xA3,0x00,0x00,0x05}   // MAC MSB->LSB
//#define OD_DEF_IP_ADDR              inet_addr(192,168,10,205)
//#define OD_DEF_IP_MASK              inet_addr(255,255,255,0)
//#define OD_DEF_IP_ROUTER            inet_addr(192,168,10,1)
//#define OD_DEF_IP_BROKER            inet_addr(192,168,20,8)

#ifdef __cplusplus
}
#endif

#endif // _A1EN12_H
