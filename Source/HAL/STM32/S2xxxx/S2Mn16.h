/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _S2Mn16_H
#define _S2Mn16_H

// Board: S2Mn10
// uC: STM32F051K8T
// PHY1: EXT_RS485
// PHY2:

// GPIOA
// Pin  Port    CN      Func        PWM
//   0  PA0     X1                  TIM2_CH1
//   1  PA1     X2                  TIM2_CH2
//   2  PA2     X3      USART2_TX   TIM2_CH3
//   3  PA3     X4      USART2_RX   TIM2_CH4
//   4  PA4             LED
//   5  PA5
//   6  PA6     X5                  TIM3_CH1
//   7  PA7     X6                  TIM3_CH2
//   8  PA8
//   9  PA9           * USART1_TX
//  10  PA10          * USART1_RX
//  11  PA11
//  12  PA12          * USART1_DE
//  13  PA13    X23   * SWDIO
//  14  PA14    X21   * SWCLK
//  15  PA15
// GPIOB
//  16  PB0     X7                  TIM3_CH3
//  17  PB1     X8                  TIM3_CH4
//  18  PB2
//  19  PB3
//  20  PB4
//  21  PB5
//  22  PB6     X15     SCL1
//  23  PB7     X16     SDA1

#ifdef __cplusplus
extern "C" {
#endif

// System Settings
#define HAL_USE_RTC                 1
#define HAL_RTC_USE_HSE             1

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_MAXPORT_NR           2
#define HAL_DIO_MAPPING             {0, 1, 2, 3, 6, 7, 16, 17}
// End DIO Section

// PWM Section
#define EXTPWM_USED                 1
#define HAL_PWM_PORT2CFG            {((2<<8) |  (2<<3) | 0),    /* PA0:  AF2, TIM2_CH1  */ \
                                     ((2<<8) |  (2<<3) | 1),    /* PA1:  AF2, TIM2_CH2  */ \
                                     ((2<<8) |  (2<<3) | 2),    /* PA2:  AF2, TIM2_CH3  */ \
                                     ((2<<8) |  (2<<3) | 3),    /* PA3:  AF2, TIM2_CH4  */ \
                                     ((1<<8) |  (3<<3) | 0),    /* PA6:  AF1, TIM3_CH1  */ \
                                     ((1<<8) |  (3<<3) | 1),    /* PA7:  AF1, TIM3_CH2  */ \
                                     ((1<<8) |  (3<<3) | 2),    /* PB0:  AF1, TIM3_CH3  */ \
                                     ((1<<8) |  (3<<3) | 3)}    /* PB1:  AF1, TIM3_CH4  */
// End PWM Section

// PA0-PA7: 0 - 7
// PB0-PB1: 8 - 9
// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           10
#define HAL_AIN_BASE2APIN           {0, 1, 2, 3, 6, 7, 8, 9}
// End Analogue Inputs

// UART Section
#define HAL_USE_USART2              0           // Mapping to logical port
#define EXTSER_USED                 1
// End UART Section

// TWI Section
#define HAL_TWI_BUS                 1           // I2C1, PB6/PB7
#define EXTTWI_USED                 1
// End TWI Section

// LED
#define LED_On()                    GPIOA->BSRR = GPIO_BSRR_BS_4
#define LED_Off()                   GPIOA->BSRR = GPIO_BSRR_BR_4
#define LED_Init()                  hal_gpio_cfg(GPIOA, GPIO_Pin_4, DIO_MODE_OUT_PP)

// RS485 PHY Section
#define HAL_USE_SUBMSTICK           1
#define HAL_USE_USART1              1
#define RS485_PHY_PORT              1
#define RS485_PHY                   1
#include "PHY/RS485/rs485_phy.h"
// End RS485 PHY Section

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           16
#define OD_DEV_UC_TYPE              'S'
#define OD_DEV_UC_SUBTYPE           '2'
#define OD_DEV_PHY1                 'M'
#define OD_DEV_PHY2                 'n'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '6'

#ifdef __cplusplus
}
#endif

#endif // _S2Mn16_H
