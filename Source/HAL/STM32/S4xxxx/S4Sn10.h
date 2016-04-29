/*
Copyright (c) 2011-2016 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _S4Sn10_H
#define _S4Sn10_H

// Board: Nucleo Nano
// uC: STM32F303K8
// PHY1: UART
// PHY2: --

// GPIOA
// Pin  Port    Nucleo  Func
//   0  PA0     A0      AIN0
//   1  PA1     A1      AIN1
//   2  PA2     A7      AIN2
//   3  PA3     A2      AIN3
//   4  PA4     A3      AIN4
//   5  PA5     A4      AIN5
//   6  PA6     A5      AIN6
//   7  PA7     A6      AIN7
//   8  PA8     D9
//   9  PA9     D1    * USART1_TX
//  10  PA10    D0    * USART1_RX
//  11  PA11    D10
//  12  PA12    D2
//  13  PA13          * SWDIO
//  14  PA14          * SWCLK
//  15  PA15
// GPIOB
//  16  PB0     D3      AIN8
//  17  PB1     D6      AIN9
//  19  PB3     D13
//  20  PB4     D12
//  21  PB5     D11
//  22  PB6     D5      SCL1
//  23  PB7     D4      SDA1

#ifdef __cplusplus
extern "C" {
#endif

// Sytem settings
#define HSE_CRYSTAL_BYPASS          1   //  HSE crystal oscillator bypassed with external clock
// End System Settings

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_MAXPORT_NR           2
#define HAL_DIO_MAPPING             {  0,   1,  3,  4,  5,  6,  7, 2,   /* PA0, PA1, PA3, PA4 - PA7, PA2 */         \
                                     255, 255, 12, 16, 23, 22, 17,      /* *PA10, *PA9, PA12, PB0, PB7, PB6, PB1 */ \
                                     255, 255,  8, 11, 21, 20, 19}      /* PF0, PF1, PA8, PA11, PB5, PB4, PB3 */
// End DIO Section

// UART PHY Section
#define HAL_UART_NUM_PORTS          1
#define HAL_USE_USART1              0

#define UART_PHY_PORT               0   // Logical port
#define UART_PHY                    1
#include "PHY/UART/uart_phy.h"
// End UART PHY Section

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           32
#define OD_DEV_UC_TYPE              'S'
#define OD_DEV_UC_SUBTYPE           '4'
#define OD_DEV_PHY1                 'S'
#define OD_DEV_PHY2                 'n'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '0'
// End OD Section

#ifdef __cplusplus
}
#endif

#endif // _S3Sn10_H
