/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef __S2Sn11_H
#define __S2Sn11_H

// Board: MB11136 - STM32F0 Nucleo
// uC: STM32F091RC
// PHY1: UART
// PHY2: --

// Pin  Port    DIO     CN      Arduino     Func
//   0  PA0     0       7.28    8.1(A0)     USART4_TX_AF4   TIM2_CH1_AF2
//   1  PA1     1       7.30    8.2(A1)     USART4_RX_AF4   TIM2_CH2_AF2
//   4  PA4     2       7.32    8.3(A2)     SPI1_NSS_AF0    TIM14_CH1_AF4   USART6_TX_AF5
//  16  PB0     3       7.34    8.4(A3)                     TIM3_CH3_AF1
//  33  PC1     4       7.36    8.5(A4)
//  32  PC0     5       7.38    8.6(A5)
//  34  PC2     6       7.35        A6
//  35  PC3     7       7.37        A7
//   3  PA3     8       10.37   9.1(D0)  *  USART2_RX_AF1   TIM15_CH2_AF0   TIM2_CH4_AF2
//   2  PA2     9       10.35   9.2(D1)  *  USART2_TX_AF1   TIM15_CH1_AF0   TIM2_CH3_AF2
//  10  PA10    10      10.33   9.3(D2)     USART1_RX_AF1   TIM1_CH3_AF2    I2C1_SDA_AF4
//  19  PB3     11      10.31   9.4(D3)     SPI1_SCK_AF0    TIM2_CH2_AF2    USART5_TX_AF4
//  21  PB5     12      10.29   9.5(D4)     SPI1_MOSI_AF0   TIM3_CH2_AF1
//  20  PB4     13      10.27   9.6(D5)     SPI1_MISO_AF0   TIM3_CH1_AF1    USART5_RX_AF4
//  26  PB10    14      10.25   9.7(D6)     USART3_TX_AF4   I2C2_SCL_AF1    TIM2_CH3_AF2
//   8  PA8     15      10.23   9.8(D7)                     TIM1_CH1_AF2
//   9  PA9     16      10.21   5.1(D8)     USART1_TX_AF1   TIM1_CH2_AF2    I2C1_SCL_AF4
//  39  PC7     17      10.19   5.2(D9)                     TIM3_CH2_AF0
//  22  PB6     18      10.17   5.3(D10)    USART1_TX_AF0   I2C1_SCL_AF1
//   7  PA7     19      10.15   5.4(D11)    SPI1_MOSI_AF0   TIM3_CH2_AF1    TIM14_CH1_AF4   TIM17_CH1_AF5
//   6  PA6     20      10.13   5.5(D12)    SPI1_MISO_AF0   TIM3_CH1_AF1    TIM16_CH1_AF5
//   5  PA5     21      10.11   5.6(D13)    SPI1_SCK_AF0    TIM2_CH1_AF2    USART6_RX_AF5
//  25  PB9     22      10.5    5.9(D14)    IR_OUT_AF0      I2C1_SDA_AF1    TIM17_CH1_AF2
//  24  PB8     23      10.3    5.10(D15)                   I2C1_SCL_AF1
//  42  PC10    24      7.1
//  43  PC11    25      7.2
//  44  PC12    26      7.3
//  13  PA13    27      7.13             *  SWDIO
//  14  PA14    28      7.15             *  SWCLK
//  15  PA15    29      7.17                                TIM2_CH1_AF2
//  23  PB7     30      7.21                USART1_RX_AF0   I2C1_SDA_AF1
//  45  PC13    31      7.23    /USER_BTN
//  46  PC14    32      7.25    OSC32_IN
//  47  PC15    33      7.27    OSC32_OUT
//  36  PC4     34      10.34               USART3_TX_AF1
//  29  PB13    35      10.30               SPI2_SCK_AF0                    I2C2_SCL_AF5
//  30  PB14    36      10.28               SPI2_MISO_AF0   TIM15_CH1_AF1   I2C2_SDA_AF5
//  31  PB15    37      10.26               SPI2_MOSI_AF0   TIM15_CH2_AF1
//  17  PB1     38      10.24                               TIM3_CH4_AF1    TIM14_CH1_AF0
//  18  PB2     39      10.22
//  27  PB11    40      10.18               USART3_RX_AF4   I2C2_SDA_AF1    TIM2_CH4_AF2
//  28  PB12    41      10.16               SPI2_NSS_AF0
//  11  PA11    42      10.14                                               I2C2_SCL_AF5
//  12  PA12    43      10.12                               TIM1_CH4_AF2    I2C2_SDA_AF5
//  37  PC5     44      10.6                USART3_RX_AF1
//  38  PC6     45      10.4                                TIM3_CH1_AF0
//  40  PC8     46      10.2                                TIM3_CH3_AF0
//  41  PC9     47      10.1                                TIM3_CH4_AF0

#ifdef __cplusplus
extern "C" {
#endif

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_MAXPORT_NR           3
#define HAL_DIO_MAPPING             {  0,   1,   4,  16,  33,  32,  34,  35, 255, 255,  10, 19, /*  PA0,  PA1,  PA4,  PB0,  PC1,  PC0,  PC2,  PC3,  PA3,  PA2, PA10,  PB3 */ \
                                      21,  20,  26,   8,   9,  39,  22,   7,   6,   5,  25, 24, /*  PB5,  PB4, PB10,  PA8,  PA9,  PC7,  PB6,  PA7,  PA6,  PA5,  PB9,  PB8 */ \
                                      42,  43,  44, 255, 255,  15,  23,  45,  46,  47,  36, 29, /* PC10, PC11, PC12, PA13, PA14, PA15,  PB7, PC13, PC14, PC15,  PC4, PB13 */ \
                                      30,  31,  17,  18,  27,  28,  11,  12,  37,  38,  40, 41} /* PB14, PB15,  PB1,  PB2, PB11, PB12, PA11, PA12,  PC5,  PC6,  PC8,  PC9 */
// End DIO Section

// PWM Section
#define EXTPWM_USED                 1
#define HAL_PWM_PORT2CFG            {((2<<8) | (2<<3)  | 0),    /* PA0:  AF2, TIM2_CH1      */ \
                                     ((2<<8) | (2<<3)  | 1),    /* PA1:  AF2, TIM2_CH2      */ \
                                     ((4<<8) | (14<<3) | 0),    /* PA4:  AF4, TIM14_CH1     */ \
                                     ((1<<8) | (3<<3)  | 2),    /* PB0:  AF1, TIM3_CH3      */ \
                                     255,255,255,255,           /* PC0 - PC3 no config      */ \
                                     255,255,                   /* PA3, PA2  no config      */ \
                                     ((2<<8) | (1<<3)  | 2),    /* PA10: AF2, TIM1_CH3      */ \
                                     ((2<<8) | (1<<2)  | 1),    /* PB3:  AF2, TIM2_CH2      */ \
                                     ((1<<8) | (3<<3)  | 1),    /* PB5:  AF1, TIM3_CH2      */ \
                                     ((1<<8) | (3<<3)  | 0),    /* PB4:  AF1, TIM3_CH1      */ \
                                     ((2<<8) | (2<<3)  | 2),    /* PB10: AF2, TIM2_CH3      */ \
                                     ((2<<8) | (1<<3)  | 0),    /* PA8:  AF2, TIM1_CH1      */ \
                                     ((2<<8) | (1<<3)  | 1),    /* PA9:  AF2, TIM1_CH2      */ \
                                     ((0<<8) | (3<<3)  | 1),    /* PC7:  AF0, TIM3_CH2      */ \
                                     255,                       /* PB6: No Config           */ \
                                     ((4<<8) | (14<<3) | 0),    /* PA7:  AF4, TIM14_CH1     */ \
                                     ((5<<8) | (16<<3) | 0),    /* PA6:  AF5, TIM16_CH1     */ \
                                     ((2<<8) | (2<<3)  | 0),    /* PA5:  AF2, TIM2_CH1      */ \
                                     ((2<<8) | (17<<3) | 0),    /* PB9:  AF2, TIM17_CH1     */ \
                                     255,                       /* PB8: No Config           */ \
                                     255,255,255,               /* PC10 - PC12 no config    */ \
                                     255,255,                   /* PA13 - PA14 no config    */ \
                                     ((2<<8) | (2<<3)  | 0),    /* PA15: AF2, TIM2_CH1      */ \
                                     255,                       /* PB7: No Config           */ \
                                     255,255,255,               /* PC13 - PC15 no config    */ \
                                     255, 255,                  /* PC4, PB13  no config     */ \
                                     ((1<<8) | (15<<3) | 0),    /* PB14: AF1, TIM15_CH1     */ \
                                     ((1<<8) | (15<<3) | 1),    /* PB15: AF1, TIM15_CH2     */ \
                                     ((1<<8) | (3<<3)  | 3),    /* PB1:  AF1, TIM3_CH4      */ \
                                     255,                       /* PB2: No Config           */ \
                                     ((2<<8) | (2<<3)  | 3),    /* PB11: AF2, TIM2_CH4      */ \
                                     255,255,                   /* PB12, PA11 no config     */ \
                                     ((2<<8) | (1<<3)  | 3),    /* PA12: AF2, TIM1_CH4      */ \
                                     255,                       /* PC5: No Config           */ \
                                     ((0<<8) | (3<<3)  | 0),    /* PC6: AF0, TIM3_CH1       */ \
                                     ((0<<8) | (3<<3)  | 0),    /* PC8: AF0, TIM3_CH3       */ \
                                     ((0<<8) | (3<<3)  | 0),    /* PC9: AF0, TIM3_CH4       */ \
                                    }
// End PWM Section

// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           8
#define EXTAIN_BASE_2_APIN          {0, 1, 4, 8, 11, 10, 12, 13}    /*  PA0,  PA1,  PA4,  PB0,  PC1,  PC0,  PC2,  PC3 */
#define EXTAIN_REF                  0x02        // Bit0 - Ext, Bit1 - Vcc, Bit2 - Int1, Bit3 - Int2
// End Analogue Inputs

// UART PHY Section
#define HAL_UART_NUM_PORTS          1
#define HAL_USE_USART2              0   // USART2 on PA2/PA3

#define UART_PHY_PORT               0   // Logical port
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
#define OD_DEV_HW_TYP_L             '1'

#ifdef __cplusplus
}
#endif

#endif // __S2Sn11_H
