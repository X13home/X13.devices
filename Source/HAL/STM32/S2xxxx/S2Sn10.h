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
//   4  PA4     P1.21           In4                     TIM14_CH1
//   5  PA5     P1.22           In5                     TIM2_CH1
//   6  PA6     P1.23           In6                     TIM16_CH1
//   7  PA7     P1.24           In7                     TIM17_CH1
//   8  PA8     P2.25                                   TIM1_CH1
//   9  PA9     P2.24                                   TIM1_CH2
//  10  PA10    P2.23                                   TIM1_CH3
//  11  PA11    P2.22                                   TIM1_CH4
//  12  PA12    P2.21
//  13  PA13    P2.20   *               SWDIO
//  14  PA14    P2.17   *               SWCLK
//  15  PA15    P2.16                                   TIM2_CH1
// GPIOB
//  16  PB0     P1.27           In8                     TIM3_CH3
//  17  PB1     P1.28           In9                     TIM3_CH4
//  18  PB2     P1.29
//  19  PB3     P2.11   ?               SWO             TIM2_CH2
//  20  PB4     P2.10                                   TIM3_CH1
//  21  PB5     P2.9                                    TIM3_CH2
//  22  PB6     P2.8    *               USART1_TX
//  23  PB7     P2.7    *               USART1_RX
//  24  PB8     P2.4                    SCL1            TIM16_CH1
//  25  PB9     P2.3                    SDA1            TIM17_CH1
//  26  PB10    P1.30                   SCL2            TIM2_CH3
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
// Mux
//  50  PD2     P2.12
//  80  PF0     NC-P1.7 *               OSC_In
//  81  PF1     P1.8                    OSC_Out
//  84  PF4     P1.19
//  85  PF5     P1.20
//  86  PF6     P2.19
//  87  PF7     P2.18

#ifdef __cplusplus
extern "C" {
#endif


// Sytem settings
#define HSE_CRYSTAL_BYPASS          1   //  HSE crystal oscillator bypassed with external clock
#define HAL_USE_RTC                 1
#define HAL_RTC_USE_HSE             1
// End System Settings

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_MAXPORT_NR           6
#define HAL_DIO_MAPPING         {  /* PA0 - PA15   */ \
                                   0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12,255,255, 15, \
                                   /* PB0 - PB15   */ \
                                  16, 17, 18, 19, 20, 21,255,255, 24, 25, 26, 27, 28, 29, 30 ,31, \
                                   /* PC0 - PC15   */ \
                                  32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, \
                                   /* PD2          */ \
                                 255,255, 50,255,255,255,255,255,255,255,255,255,255,255,255,255, \
                                   /* PE not exist */ \
                                 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255, \
                                   /* PF0 - PF7    */ \
                                 255, 81,255,255, 84, 85, 86, 87 \
                                }
// End DIO Section

// PWM Section
#define EXTPWM_USED                 1
#define HAL_PWM_PORT2CFG            {((2<<8) |  (2<<3) | 0),    /* PA0:  AF2, TIM2_CH1      */ \
                                     ((2<<8) |  (2<<3) | 1),    /* PA1:  AF2, TIM2_CH2      */ \
                                     ((2<<8) |  (2<<3) | 2),    /* PA2:  AF2, TIM2_CH3      */ \
                                     ((2<<8) |  (2<<3) | 3),    /* PA3:  AF2, TIM2_CH4      */ \
                                     ((4<<8) | (14<<3) | 0),    /* PA4:  AF4, TIM14_CH1     */ \
                                     ((2<<8) |  (2<<3) | 0),    /* PA5:  AF2, TIM2_CH1      */ \
                                     ((5<<8) | (16<<3) | 0),    /* PA6:  AF5, TIM16_CH1     */ \
                                     ((5<<8) | (17<<3) | 0),    /* PA7:  AF5, TIM17_CH1     */ \
                                     ((2<<8) |  (1<<3) | 0),    /* PA8:  AF2, TIM1_CH1      */ \
                                     ((2<<8) |  (1<<3) | 1),    /* PA9:  AF2, TIM1_CH2      */ \
                                     ((2<<8) |  (1<<3) | 2),    /* PA10: AF2, TIM1_CH3      */ \
                                     ((2<<8) |  (1<<3) | 3),    /* PA11: AF2, TIM1_CH4      */ \
                                     255,255,255,               /* PA12 - PA14, no config   */ \
                                     ((2<<8) |  (2<<3) | 0),    /* PA15: AF2, TIM2_CH1      */ \
                                     ((1<<8) |  (3<<3) | 2),    /* PB0:  AF1, TIM3_CH3      */ \
                                     ((1<<8) |  (3<<3) | 3),    /* PB1:  AF1, TIM3_CH4      */ \
                                     255,                       /* PB2, no config           */ \
                                     ((2<<8) |  (2<<3) | 1),    /* PB3:  AF2, TIM2_CH2      */ \
                                     ((1<<8) |  (3<<3) | 0),    /* PB4:  AF1, TIM3_CH1      */ \
                                     ((1<<8) |  (3<<3) | 1),    /* PB5:  AF1, TIM3_CH2      */ \
                                     255,255,                   /* PB6 - PB7,   no config   */ \
                                     ((2<<8) | (16<<3) | 0),    /* PB8:  AF2, TIM16_CH1     */ \
                                     ((2<<8) | (17<<3) | 0),    /* PB9:  AF2, TIM17_CH1     */ \
                                     ((2<<8) |  (2<<3) | 2),    /* PB10: AF2, TIM2_CH3      */ \
                                     255,255,255,               /* PB11 - PB13, no config   */ \
                                     ((1<<8) | (15<<3) | 0),    /* PB14:  AF1, TIM15_CH1    */ \
                                     ((1<<8) | (15<<3) | 1),    /* PB15:  AF1, TIM15_CH2    */ \
                                     255,255,255,               /* PC0 - PC2,   no config   */ \
                                     255,255,255,               /* PC3 - PC5,   no config   */ \
                                     ((0<<8) |  (3<<3) | 0),    /* PC6:  AF0, TIM3_CH1      */ \
                                     ((0<<8) |  (3<<3) | 1),    /* PC7:  AF0, TIM3_CH2      */ \
                                     ((0<<8) |  (3<<3) | 2),    /* PC8:  AF0, TIM3_CH3      */ \
                                     ((0<<8) |  (3<<3) | 3)     /* PC9:  AF0, TIM3_CH4      */ \
                                    }
// End PWM Section

// Analog Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           16
#define HAL_AIN_BASE2APIN           {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}
// End Analog Inputs

// UART Section
#define HAL_USE_USART2              0
#define EXTSER_USED                 1
// End UART Section

// TWI Section
#define HAL_TWI_BUS                 1   // I2C On PB8/PB9
#define HAL_TWI_REMAP               1
#define EXTTWI_USED                 1
// End TWI Section

// UART PHY Section
#define HAL_USE_USART1              1
#define HAL_USART1_REMAP            1   // USART1 on PB6/PB7
#define UART_PHY_PORT               1   // Logical port
#define UART_PHY                    1
#include "PHY/UART/uart_phy.h"
// End UART PHY Section

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           64
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
