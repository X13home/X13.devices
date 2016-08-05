/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _A1SC10_H
#define _A1SC10_H

// Board:   Arduino Nano/Uno
// uC:      ATMega328p
// PHY1:    UART
// PHY2:    CC1101

// 0 - 7    PORTA - not exist
// PORTB
//  8   PB0     D8
//  9   PB1     D9      OC1A
// 10   PB2     D10   * CC11_CSN
// 11   PB3     D11   * CC11_MOSI
// 12   PB4     D12   * CC11_MISO
// 13   PB5     D13   * CC11_SCK/LED
// 14   PB6     --      OSC
// 15   PB7     --      OSC
// PORT C
// 16   PC0     A0      Ain0
// 17   PC1     A1      Ain1
// 18   PC2     A2      Ain2
// 19   PC3     A3      Ain3
// 20   PC4     A4      Ain4/SDA
// 21   PC5     A5      Ain5/SCL
// --   PC6     --      RESET
// --   --      A6      Ain6
// --   --      A7      Ain7
// PORT D
// 24   PD0     D0    * RXD
// 25   PD1     D1    * TXD
// 26   PD2     D2
// 27   PD3     D3
// 28   PD4     D4
// 29   PD5     D5      OC0B
// 30   PD6     D6      OC0A
// 31   PD7     D7

#ifdef __cplusplus
extern "C" {
#endif

#define F_CPU                       16000000UL

// PC0-PC5, Gap:Ain6-7, Gap:PD0-PD1, PD2-PD7, PB0-PB1
// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_PORT_OFFSET          1
#define EXTDIO_MAXPORT_NR           3         // Number of used physical digital Ports
#define HAL_DIO_MAPPING             {16, 17, 18, 19, 20, 21, 0xFF, 0xFF,    \
                                   0xFF, 0xFF, 26, 27, 28, 29, 30, 31, 8, 9}
// End DIO Section

// PWM Section
#define EXTPWM_USED                 1
#define HAL_PWM_BASE_OFFSET         13
#define HAL_PWM_PORT2CFG            {1, 0, 0xFF, 0xFF, 8}   // Mapping PWM channel to configuration
                                                            // bits 7-3 Timer, bits 2-0 Channel
// End PWM Section

// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           8                                   // ADC0-ADC7
#define HAL_AIN_BASE2APIN           {0, 1, 2, 3, 4, 5, 6, 7}
// End Analogue Inputs

// TWI Section
#define EXTTWI_USED                 1
// End TWI Section

// UART Section
#define HAL_USE_USART0              0
#define HAL_UART_NUM_PORTS          1
#define UART_PHY_PORT               0
#define UART_PHY                    1
#include "PHY/UART/uart_phy.h"
// End UART Section

// CC11 Section
#define HAL_USE_SPI1                1
#define CC11_USE_SPI                1
#define CC11_NSS_PIN                10  // PB2
#define CC11_WAIT_LOW_MISO()        while(PINB & (1<<PB4))
#define CC11_SELECT()               PORTB &= ~(1<<PB2)
#define CC11_RELEASE()              PORTB |= (1<<PB2)
#define CC11_PHY                    2
#include "PHY/CC1101/cc11_phy.h"
//  End CC11 Section

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           21      // Size of identificators list, 
                                            //  PC0-PC5 + ADC6/7 + Vbg + PD2-PD7 + PB0-PB4
#define OD_DEV_UC_TYPE              'A'
#define OD_DEV_UC_SUBTYPE           '1'
#define OD_DEV_PHY1                 'S'
#define OD_DEV_PHY2                 'C'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '0'

#ifdef __cplusplus
}
#endif

#endif // _A1SC10_H
