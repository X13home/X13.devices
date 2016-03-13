/*
Copyright (c) 2011-2016 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _S2SC14_H
#define _S2SC14_H

// Board: Gesture Switch
// uC: STM32F051K8T6
// PHY1: UART
// PHY2: CC1101/26M

// GPIOA
// Pin  CN  Port    Func
//   0  3   PA0                     TIM2_CH1_AF2
//   1  4   PA1                     TIM2_CH2_AF2
//   2  5   PA2     USART2_TX       TIM2_CH3_AF2
//   3  6   PA3     USART2_RX       TIM2_CH4_AF2
//   4  15  PA4                     TIM14_CH2_AF4
//   5  16  PA5                     !TIM2_CH1_AF2
//   6  17  PA6                     TIM3_CH1_AF1
//   7  18  PA7                     TIM3_CH2_AF1
//   8      PA8     Sensor_Int
//   9  9   PA9   * USART1_TX       TIM1_CH2_AF2
//  10  10  PA10  * USART1_RX       TIM1_CH3_AF2
//  11  7   PA11                    TIM1_CH4_AF2
//  12  8   PA12    LED
//  13      PA13    SWDIO
//  14      PA14    SWCLK
//  15      PA15    CC11_Select
// GPIOB
//  16  19  PB0                     TIM3_CH3_AF1
//  17  20  PB1                     TIM3_CH4_AF1
//  19      PB3     CC11_SCK
//  20      PB4     CC11_MISO
//  21      PB5     CC11_MOSI
//  22  13  PB6   * SCL
//  23  14  PB7   * SDA

#ifdef __cplusplus
extern "C" {
#endif

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_MAXPORT_NR           2
#define HAL_DIO_MAPPING             {17, 16, 7, 6, 5, 4, 255, 255, 255, 255, 12, 11, 3, 2, 1, 0} /* PB1, PB0, PA7, PA6, PA5, PA4, PB7, PB6, PA10, PA9, PA12, PA11, PA3, PA2, PA1, PA0 */
// End DIO Section

// PWM Section
#define EXTPWM_USED                 1
#define HAL_PWM_PORT2CFG            {((1<<8) |  (3<<3) | 3),    /* PB1:  AF1, TIM3_CH4  */ \
                                     ((1<<8) |  (3<<3) | 2),    /* PB0:  AF1, TIM3_CH3  */ \
                                     ((1<<8) |  (3<<3) | 1),    /* PA7:  AF1, TIM3_CH2  */ \
                                     ((1<<8) |  (3<<3) | 0),    /* PA6:  AF1, TIM3_CH1  */ \
                                     ((2<<8) |  (2<<3) | 0),    /* PA5:  AF2, TIM2_CH1  */ \
                                     ((4<<8) | (14<<3) | 0),    /* PA4:  AF4, TIM14_CH1 */ \
                                     255, 255,                  /* PB6, PB7: No Config  */ \
                                     255, 255,                  /* PA10, PA9: Busy      */ \
                                     255,                       /* PA12: No Config      */ \
                                     ((2<<8) |  (1<<3) | 3),    /* PA11: AF2, TIM1_CH4  */ \
                                     ((2<<8) |  (2<<3) | 3),    /* PA3:  AF2, TIM2_CH4  */ \
                                     ((2<<8) |  (2<<3) | 2),    /* PA2:  AF2, TIM2_CH3  */ \
                                     ((2<<8) |  (2<<3) | 1),    /* PA1:  AF2, TIM2_CH2  */ \
                                     ((2<<8) |  (2<<3) | 0)}    /* PA0:  AF2, TIM2_CH1  */
// End PWM Section

// PA0-PA7: 0 - 7
// PB0-PB1: 8 - 9
// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           10
#define EXTAIN_BASE_2_APIN          {9, 8, 7, 6, 5, 4, 255, 255, 255, 255, 255, 255, 3, 2, 1, 0}    /* PB0, PB1, PA7 - PA4, GAP, PA3 - PA0 */
#define EXTAIN_REF                  0x02        // Bit0 - Ext, Bit1 - Vcc, Bit2 - Int1, Bit3 - Int2
// End Analogue Inputs

// UART Section
#define HAL_UART_NUM_PORTS          2
#define HAL_USE_USART1              1       // PA9, PA10 - PHY
#define HAL_USE_USART2              0       // PA2, PA3 - User Port

#define EXTSER_USED                 1

// UART PHY Section
#define UART_PHY_PORT               1       // Logical port
#define UART_PHY                    1
#include "PHY/UART/uart_phy.h"
// End UART PHY Section
// End UART Section

// User Extensions
// Si1143
#define HAL_TWI_BUS                 1       // I2C1 On PB6, PB7
#define HAL_USE_EXTI                1
#define EXT_OPS_USED                1
#define EXT_OPS_IRQ_PIN             8
// End User Extensions

// CC11 Section
#define HAL_USE_SPI1                2       // SPI1, Config 2: PB3-PB5
#define CC11_USE_SPI                1
#define CC11_NSS_PIN                15                                  // PA15
#define CC11_WAIT_LOW_MISO()        while(GPIOB->IDR & GPIO_Pin_4)      // PB4
#define CC11_SELECT()               GPIOA->BRR = GPIO_Pin_15
#define CC11_RELEASE()              GPIOA->BSRR = GPIO_Pin_15
#define CC11_PHY                    2
#include "PHY/CC1101/cc11_phy.h"
// End CC11 Section

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           20
#define OD_DEV_UC_TYPE              'S'
#define OD_DEV_UC_SUBTYPE           '2'
#define OD_DEV_PHY1                 'S'
#define OD_DEV_PHY2                 'C'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '4'

#ifdef __cplusplus
}
#endif

#endif // _S2SN12_H
