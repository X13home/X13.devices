/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _S3Sn10_H
#define _S3Sn10_H

// Board: Maple Mini
// uC: STM32F103CBT6
// PHY1: UART
// PHY2: --

// GPIOA
// Pin  Port    Maple   Func
//   0  PA0     11      AIN0                    TIM2_CH1
//   1  PA1     10      AIN1                    TIM2_CH2
//   2  PA2     9       AIN2    USART2_TX       TIM2_CH3
//   3  PA3     8       AIN3    USART2_RX       TIM2_CH4
//   4  PA4     7       AIN4    NSS1
//   5  PA5     6       AIN5    SCK1
//   6  PA6     5       AIN6    MISO1           TIM3_CH1
//   7  PA7     4       AIN7    MOSI1           TIM3_CH2
//   8  PA8     27                              TIM1_CH1
//   9  PA9     26      USART1_TX               TIM1_CH2
//  10  PA10    25      USART1_RX               TIM1_CH3
//  11  PA11    24      USBDM                   TIM1_CH4
//  12  PA12    23      USBDP
//  13  PA13    22    * JTMS/SWDIO
//  14  PA14    21    * JTCK/SWCLK
//  15  PA15    20      JTDI
// GPIOB
//  16  PB0     3       AIN8                    TIM3_CH3
//  17  PB1           * LED                     TIM3_CH4
//  18  PB2     2       BOOT1
//  19  PB3     19      JTDO/SWO
//  20  PB4     18      JTRST
//  21  PB5     17
//  22  PB6     16      SCL1                    TIM4_CH1
//  23  PB7     15      SDA1                    TIM4_CH2
//  24  PB8           * BUT_SW                  TIM4_CH3
//  25  PB9           * DISC                    TIM4_CH4
//  26  PB10    1       SCL2  * USART3_TX
//  27  PB11    0       SDA2  * USART3_RX
//  28  PB12    31      NSS2
//  29  PB13    30      SCLK2
//  30  PB14    29      MISO2
//  31  PB15    28      MOSI2
// GPIOC
//  45  PC13    14
//  46  PC14    13      OSC32 In
//  47  PC15    12      OSC32 Out
// GPIOD - Not Used
//  48  PD0             OSC In 8M
//  49  PD1             OSC Out 8M

#ifdef __cplusplus
extern "C" {
#endif

// System Settings
#define HAL_USE_RTC                 1
#define HAL_RTC_USE_HSE             1

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_MAXPORT_NR           3
#define HAL_DIO_MAPPING     {   /* PB11, PB10, PB2, PB0, PA7 - PA0 */               \
                                   0xFF, 0xFF,  18,  16,   7, 6, 5, 4, 3, 2, 1, 0,  \
                                /* PC13 - PC15, PB7 - PB3 */                        \
                                     47,   46,  45,  23,  22, 21, 20, 19,           \
                                /* PA15 - PA8 */                                    \
                                     15,  255, 255, 12, 11, 10, 9, 8,               \
                                /* PB15 - PB12 */                                   \
                                     31,   30,  29, 28                              \
                            }
// End DIO Section

// PWM Section
#define EXTPWM_USED                 1
#define HAL_PWM_PORT2CFG            {255, 255, 255,             /* PB11, PB10, PB2  */ \
                                     ((3<<3) | 2),              /* PB0:  TIM3_CH3   */ \
                                     ((3<<3) | 1),              /* PA7:  TIM3_CH2   */ \
                                     ((3<<3) | 0),              /* PA6:  TIM3_CH1   */ \
                                     255, 255,                  /* PA5, PA4         */ \
                                     ((2<<3) | 3),              /* PA3:  TIM2_CH4   */ \
                                     ((2<<3) | 2),              /* PA2:  TIM2_CH3   */ \
                                     ((2<<3) | 1),              /* PA1:  TIM2_CH2   */ \
                                     ((2<<3) | 0),              /* PA0:  TIM2_CH1   */ \
                                     255, 255, 255,             /* PC13 - PC15      */ \
                                     ((4<<3) | 1),              /* PB7:  TIM4_CH2   */ \
                                     ((4<<3) | 0),              /* PB6:  TIM4_CH1   */ \
                                     255, 255, 255,             /* PB5 - PB3        */ \
                                     255, 255, 255, 255,        /* PA15 - PA12      */ \
                                     ((1<<3) | 3),              /* PA11: TIM1_CH4   */ \
                                     ((1<<3) | 2),              /* PA10: TIM1_CH3   */ \
                                     ((1<<3) | 1),              /* PA9:  TIM1_CH2   */ \
                                     ((1<<3) | 0)}              /* PA8:  TIM1_CH1   */
// End PWM Section

// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           9
                                  /* PB11, PB10,  PB2, PB0, PA7 - PA0 */
#define HAL_AIN_BASE2APIN           {0xFF, 0xFF, 0xFF,   8,   7, 6, 5, 4, 3, 2, 1, 0}
// End Analogue Inputs

// UART Section
#define HAL_USE_USART1              0           // Mapping to logical port
#define HAL_USE_USART2              1
#define EXTSER_USED                 2
// End UART Section

// TWI Section
#define HAL_TWI_BUS                 1
#define EXTTWI_USED                 1
// End TWI Section

// LEDs
#define LED_On()                    GPIOB->BSRR = GPIO_BSRR_BS1
#define LED_Off()                   GPIOB->BSRR = GPIO_BSRR_BR1
#define LED_Init()                  hal_gpio_cfg(GPIOB, GPIO_Pin_1, DIO_MODE_OUT_PP)

// UART PHY Section
#define HAL_USE_USART3              2
#define UART_PHY_PORT               2   // Logical port
#define UART_PHY                    1
#include "PHY/UART/uart_phy.h"
// End UART PHY Section

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           64
#define OD_DEV_UC_TYPE              'S'
#define OD_DEV_UC_SUBTYPE           '3'
#define OD_DEV_PHY1                 'S'
#define OD_DEV_PHY2                 'n'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '0'
// End OD Section

#ifdef __cplusplus
}
#endif

#endif // _S3Sn10_H
