/*
Copyright (c) 2011-2016 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _S4SC11_H
#define _S4SC11_H

// Board: uNode V3.0
// uC: STM32F303K8/STM32F334K8
// PHY1: UART
// PHY2: CC1101

// GPIOA
// Pin  Port    CN      Func
//   0  PA0     P1.3    AIN0
//   1  PA1     P1.4    AIN1
//   2  PA2     P1.5    AIN2
//   3  PA3     P1.6    AIN3
//   4  PA4     P1.7    AIN4
//   5  PA5     P1.8    AIN5
//   6  PA6     P1.9    AIN6
//   7  PA7     P1.10   AIN7
//   8  PA8     --
//   9  PA9     P2.9  * TX1
//  10  PA10    P2.8  * RX1
//  11  PA11    P2.7    DE
//  12  PA12    --
//  13  PA13    P2.6    SWDIO
//  14  PA14    P2.5    SWCLK
//  15  PA15    --      CC11_SEL
// GPIOB
//  16  PB0     --      LED1
//  17  PB1     --      LED2
//  19  PB3     --      CC11_SCK
//  20  PB4     --      CC11_MISO
//  21  PB5     --      CC11_MOSI
//  22  PB6     P2.3    SCL1
//  23  PB7     P2.4    SDA1
// System
//      3.3V    P1.1
//      3.3V    P2.1
//      GND     P1.2
//      GND     P2.2
//      NRST    P2.10

#ifdef __cplusplus
extern "C" {
#endif

// Sytem settings
#define HAL_USE_RTC                 1
#define HAL_RTC_USE_HSE             1

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_MAXPORT_NR           1
#define HAL_DIO_MAPPING         { /* PA0 - PA7 */                               \
                                       0,  1,     2,    3,    4,    5,   6, 7,  \
                                  /* PB7, PB6, PA14, PA13, PA11, PA10, PA9 */   \
                                      23,  22, 14,   13,   11                 \
                                }
// End DIO Section

// PWM Section
#define EXTPWM_USED                 1
#define HAL_PWM_PORT2CFG            {((1<<8)  |  (2<<3) | 0),   /* PA0:  AF1, TIM2_CH1  */ \
                                     ((1<<8)  |  (2<<3) | 1),   /* PA1:  AF1, TIM2_CH2  */ \
                                     ((1<<8)  |  (2<<3) | 2),   /* PA2:  AF1, TIM2_CH3  */ \
                                     ((1<<8)  |  (2<<3) | 3),   /* PA3:  AF1, TIM2_CH4  */ \
                                     ((2<<8)  |  (3<<3) | 1),   /* PA4:  AF2, TIM3_CH2  */ \
                                     ((1<<8)  |  (2<<3) | 0),   /* PA5:  AF1, TIM2_CH1  */ \
                                     ((2<<8)  |  (3<<3) | 0),   /* PA6:  AF2, TIM3_CH1  */ \
                                     ((1<<8)  | (17<<3) | 0),   /* PA7:  AF1, TIM17_CH1 */ \
                                     255,                       /* PB6                  */ \
                                     ((10<<8) |  (3<<3) | 3),   /* PB7:  AF10, TIM3_CH4 */ \
                                     255, 255,                  /* PA14, PA13,          */ \
                                     ((11<<8) |  (1<<3) | 3)    /* PA11: AF11, TIM1_CH4 */ \
                                    }
// End PWM Section

// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           8
#define HAL_AIN_BASE2APIN           {0, 1, 2, 3, 4, 5, 6, 7}                   /* PA0 - PA7 */
// End Analogue Inputs

// UART Section
#define HAL_USE_USART2              0
#define EXTSER_USED                 1
// End UART Section

// TWI Section
#define HAL_TWI_BUS                 1       // I2C_Bus 1 - I2C1, 2 - I2C2
#define EXTTWI_USED                 1
// End TWI Section

// LEDs
#define LED_On()                    GPIOB->BSRR = GPIO_BSRR_BS_0
#define LED_Off()                   GPIOB->BSRR = GPIO_BSRR_BR_0
#define LED_Init()                  hal_gpio_cfg(GPIOB, GPIO_Pin_0, DIO_MODE_OUT_PP)

// UART PHY Section
#define HAL_USE_USART1              1
#define UART_PHY_PORT               1   // Logical port
#define UART_PHY                    1
#include "PHY/UART/uart_phy.h"
// End UART PHY Section

// CC11 PHY Section
#define HAL_USE_SPI1                2                               // SPI1 on PB3 - PB5
#define CC11_USE_SPI                1
#define CC11_NSS_PIN                15                              // PA15
#define CC11_WAIT_LOW_MISO()        while(GPIOB->IDR & GPIO_Pin_4)
#define CC11_SELECT()               GPIOA->BRR = GPIO_Pin_15
#define CC11_RELEASE()              GPIOA->BSRR = GPIO_Pin_15
#define CC11_PHY                    2
#include "PHY/CC1101/cc11_phy.h"
// End CC11 PHY Section

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           32
#define OD_DEV_UC_TYPE              'S'
#define OD_DEV_UC_SUBTYPE           '4'
#define OD_DEV_PHY1                 'S'
#define OD_DEV_PHY2                 'C'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '1'
// End OD Section

#ifdef __cplusplus
}
#endif

#endif // _S4SC11_H
