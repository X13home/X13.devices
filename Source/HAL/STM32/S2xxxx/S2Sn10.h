/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef __S2Sn10_H
#define __S2Sn10_H

// Board: MB1034B - STM32F0 Discovery
// uC: STM32F051R8T6
// PHY1: UART
// PHY2: --

// GPIOA
// Pin  Port    CN      Busy    AIn     Func            PWM
//   0  PA0     P1.15           In0     USER_BTN        TIM2_CH1
//   1  PA1     P1.16           In1                     TIM2_CH2
//   2  PA2     P1.17           In2     USART2_TX       TIM2_CH3
//   3  PA3     P1.18           In3     USART2_RX       TIM2_CH4
//   4  PA4     P1.21           In4
//   5  PA5     P1.22           In5                     
//   6  PA6     P1.23           In6
//   7  PA7     P1.24           In7
//   8  PA8     P2.25                                   TIM1_CH1
//   9  PA9     P2.24                   USART1_TX       TIM1_CH2
//  10  PA10    P2.23                   USART1_RX       TIM1_CH3
//  11  PA11    P2.22                                   TIM1_CH4
//  12  PA12    P2.21
//  13  PA13    P2.20   *               SWDIO
//  14  PA14    P2.17   *               SWCLK
//  15  PA15    P2.16                                   
// GPIOB
//  16  PB0     P1.27           In8
//  17  PB1     P1.28           In9
//  18  PB2     P1.29
//  19  PB3     P2.11   ?               SWO
//  20  PB4     P2.10
//  21  PB5     P2.9
//  22  PB6     P2.8    *               /USART1_TX
//  23  PB7     P2.7    *               /USART1_RX
//  24  PB8     P2.4                                    TIM16_CH1
//  25  PB9     P2.3                                    TIM17_CH1
//  26  PB10    P1.30                   SCL2
//  27  PB11    P1.31                   SDA2
//  28  PB12    P1.32
//  29  PB13    P2.32
//  30  PB14    P2.31                                   TIM15_CH1
//  31  PB15    P2.30                                   TIM15_CH2
// GPIOC
//  32  PC0     P1.11           In10
//  33  PC1     P1.12           In11
//  34  PC2     P1.13           In12
//  35  PC3     P1.14           In13
//  36  PC4     P1.25           In14
//  37  PC5     P1.26           In15
//  38  PC6     P2.29                                   TIM3_CH1
//  39  PC7     P2.28                                   TIM3_CH2
//  40  PC8     P2.27                   LED_BLUE        TIM3_CH3
//  41  PC9     P2.26                   LED_GREEN       TIM3_CH4
//  42  PC10    P2.15
//  43  PC11    P2.14
//  44  PC12    P2.13
//  45  PC13    P1.4
//  46  PC14    P1.5
//  47  PC15    P1.6
// PD2, PF0-PF7 - Not Used

#ifdef __cplusplus
extern "C" {
#endif

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_MAXPORT_NR           3
#define HAL_DIO_MAPPING             {0,  1,  2,  3,  4,  5,    6,   7,  8,  9, 10, 11, 12, 255, 255,  15, \
                                     16, 17, 18, 19, 20, 21, 255, 255, 24, 25, 26, 27, 28,  29,  30,  31, \
                                     32, 33, 34, 35, 36, 37,   38, 39, 40, 41, 42, 43, 44,  45,  46,  47}
// End DIO Section

// UART PHY Section
#define HAL_UART_NUM_PORTS          1
#define HAL_USE_ALT_USART1          0   // USART1 on PB6/PB7

#define UART_PHY_PORT               0   // Logical port
#define UART_PHY                    1
#include "PHY/UART/uart_phy.h"
// End UART PHY Section

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           40
#define OD_DEV_UC_TYPE              'S'
#define OD_DEV_UC_SUBTYPE           '2'
#define OD_DEV_PHY1                 'S'
#define OD_DEV_PHY2                 'n'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '0'

#ifdef __cplusplus
}
#endif

#endif // __S2Sn10_H
