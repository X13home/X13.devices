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
//   0  PA0     A0      AIN0               #TIM2_CH1
//   1  PA1     A1      AIN1                TIM2_CH2
//   2  PA2     A7            * USART2_TX
//   3  PA3     A2      AIN3                TIM2_CH4
//   4  PA4     A3      AIN4               #TIM3_CH2
//   5  PA5     A4      AIN5               #TIM2_CH1
//   6  PA6     A5      AIN6               #TIM3_CH1
//   7  PA7     A6      AIN7               #TIM17_CH1
//   8  PA8     D9                          TIM1_CH1
//   9  PA9     D1              USART1_TX   TIM1_CH2
//  10  PA10    D0              USART1_RX   TIM1_CH3
//  11  PA11    D10                         TIM1_CH4
//  12  PA12    D2                          TIM16_CH1
//  13  PA13          * SWDIO
//  14  PA14          * SWCLK
//  15  PA15                  * USART2_RX
// GPIOB
//  16  PB0     D3      AIN8                TIM3_CH3
//  17  PB1     D6      AIN9               #TIM3_CH4
//  19  PB3     D13                        #TIM3_CH1
//  20  PB4     D12                        #TIM3_CH2
//  21  PB5     D11                        #TIM17_CH1
//  22  PB6     D5      SCL1
//  23  PB7     D4      SDA1               #TIM3_CH4
// PF0 - D7 / PF1 - D8: Not used

// Solder Bridges
// SB6: Off  - PF0 Disconnected from D7
// SB4: On   - MCO From ST-LINK Connect to OSC-IN - PF0
// SB16: Off - PB6 Disconnected from A5 - PA6
// SB18: Off - PB7 Disconnected from A4 - PA5

#ifdef __cplusplus
extern "C" {
#endif

// Sytem settings
#define HAL_USE_RTC                 1   // Enable Hardware RTC
#define HAL_RTC_USE_HSE             1   // As RTC clock source used HSE
#define HSE_CRYSTAL_BYPASS          1   // HSE crystal oscillator bypassed with external clock
// End System Settings

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_MAXPORT_NR           2
#define HAL_DIO_MAPPING         {   /*  PA0, PA1, PA3 -                PA7, *PA2 */ \
                                          0,   1,   3,     4,   5,   6,  7, 255,    \
                                    /* PA10, PA9, PA12,  PB0, PB7, PB6, PB1 */      \
                                         10,   9,   12,   16,  23,  22,  17,        \
                                    /*  PF0, PF1,  PA8, PA11, PB5, PB4, PB3 */      \
                                        255, 255,    8,   11,  21,  20,  19         \
                                }
// End DIO Section

// PWM Section
#define EXTPWM_USED                 1
#define HAL_PWM_PORT2CFG            {((1<<8)  |  (2<<3) | 0),   /* PA0:  AF1, TIM2_CH1  */ \
                                     ((1<<8)  |  (2<<3) | 1),   /* PA1:  AF1, TIM2_CH2  */ \
                                     ((1<<8)  |  (2<<3) | 3),   /* PA3:  AF1, TIM2_CH4  */ \
                                     ((2<<8)  |  (3<<3) | 1),   /* PA4:  AF2, TIM3_CH2  */ \
                                     ((1<<8)  |  (2<<3) | 0),   /* PA5:  AF1, TIM2_CH1  */ \
                                     ((2<<8)  |  (3<<3) | 0),   /* PA6:  AF2, TIM3_CH1  */ \
                                     ((1<<8)  | (17<<3) | 0),   /* PA7:  AF1, TIM17_CH1 */ \
                                     255,                       /* PA2                  */ \
                                     ((6<<8)  |  (1<<3) | 2),   /* PA10: AF6, TIM1_CH3  */ \
                                     ((6<<8)  |  (1<<3) | 1),   /* PA9:  AF6, TIM1_CH2  */ \
                                     ((1<<8)  | (16<<3) | 0),   /* PA12: AF1, TIM16_CH1 */ \
                                     ((2<<8)  |  (3<<3) | 2),   /* PB0:  AF2, TIM3_CH3  */ \
                                     ((10<<8) |  (3<<3) | 3),   /* PB7:  AF10, TIM3_CH4 */ \
                                     255,                       /* PB6                  */ \
                                     ((2<<8)  |  (3<<3) | 3),   /* PB1:  AF2, TIM3_CH4  */ \
                                     255,255,                   /* PF0, PF1             */ \
                                     ((6<<8)  |  (1<<3) | 0),   /* PA8:  AF6, TIM1_CH1  */ \
                                     ((11<<8) |  (1<<3) | 3),   /* PA11: AF11, TIM1_CH4 */ \
                                     ((10<<8) | (17<<3) | 0),   /* PB5:  AF10, TIM17_CH1 */ \
                                     ((2<<8)  |  (3<<3) | 1),   /* PB4:  AF2, TIM3_CH2  */ \
                                     ((2<<8)  |  (3<<3) | 0)}   /* PB3:  AF2, TIM3_CH1  */
// End PWM Section

// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           8
#define HAL_AIN_BASE2APIN           {0, 1, 3, 4, 5, 6, 7}       /* PA0, PA1, PA3 - PA7 */
// End Analogue Inputs

// UART Section
#define HAL_USE_USART1              0
#define EXTSER_USED                 1
// End UART Section

// TWI Section
#define HAL_TWI_BUS                 1       // I2C_Bus 1 - I2C1, 2 - I2C2
#define EXTTWI_USED                 1
// End TWI Section

// UART PHY Section
#define HAL_USE_USART2              1
#define HAL_USART2_REMAP            13  // PA2/PA15
#define UART_PHY_PORT               1   // Logical port
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
