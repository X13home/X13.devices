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

/*
Board: MB11136 - STM32F0 Nucleo
uC: STM32F091RC
PHY1: UART
PHY2: --

    Pin  Port    DIO     CN      Arduino     Comm            PWM
     0  PA0     0       7.28    8.1(A0)                     TIM2_CH1_AF2
     1  PA1     1       7.30    8.2(A1)                     TIM2_CH2_AF2
     4  PA4     2       7.32    8.3(A2)                     TIM14_CH1_AF4
    16  PB0     3       7.34    8.4(A3)                     TIM3_CH3_AF1
    33  PC1     4       7.36    8.5(A4)
    32  PC0     5       7.38    8.6(A5)
    34  PC2     6       7.35        A6
    35  PC3     7       7.37        A7
     3  PA3     8       10.37   9.1(D0)  *  USART2_RX_AF1
     2  PA2     9       10.35   9.2(D1)  *  USART2_TX_AF1
    10  PA10    10      10.33   9.3(D2)                     TIM1_CH3_AF2
    19  PB3     11      10.31   9.4(D3)                     TIM2_CH2_AF2
    21  PB5     12      10.29   9.5(D4)                     TIM3_CH2_AF1
    20  PB4     13      10.27   9.6(D5)                     TIM3_CH1_AF1
    26  PB10    14      10.25   9.7(D6)                     TIM2_CH3_AF2
     8  PA8     15      10.23   9.8(D7)                     TIM1_CH1_AF2
     9  PA9     16      10.21   5.1(D8)                     TIM1_CH2_AF2
    39  PC7     17      10.19   5.2(D9)                     TIM3_CH2_AF0
    22  PB6     18      10.17   5.3(D10)
     7  PA7     19      10.15   5.4(D11)                    TIM14_CH1_AF4
     6  PA6     20      10.13   5.5(D12)                    TIM16_CH1_AF5
     5  PA5     21      10.11   5.6(D13)                    TIM2_CH1_AF2
    25  PB9     22      10.5    5.9(D14)    I2C1_SDA_AF1    TIM17_CH1_AF2
    24  PB8     23      10.3    5.10(D15)   I2C1_SCL_AF1
    42  PC10    24      7.1                 USART3_TX_AF1
    43  PC11    25      7.2                 USART3_RX_AF1
    44  PC12    26      7.3
    50  PD2             7.4                 USART3_RTS_AF1
    13  PA13    27      7.13             *  SWDIO
    14  PA14    28      7.15             *  SWCLK
    15  PA15    29      7.17                                TIM2_CH1_AF2
    23  PB7     30      7.21
    45  PC13    31      7.23    /USER_BTN
    46  PC14    32      7.25    OSC32_IN
    47  PC15    33      7.27    OSC32_OUT
    36  PC4     34      10.34
    29  PB13    35      10.30
    30  PB14    36      10.28                               TIM15_CH1_AF1
    31  PB15    37      10.26                               TIM15_CH2_AF1
    17  PB1     38      10.24                               TIM3_CH4_AF1
    18  PB2     39      10.22
    27  PB11    40      10.18                               TIM2_CH4_AF2
    28  PB12    41      10.16
    11  PA11    42      10.14                               TIM1_CH4_AF2
    12  PA12    43      10.12
    37  PC5     44      10.6
    38  PC6     45      10.4                                TIM3_CH1_AF0
    40  PC8     46      10.2                                TIM3_CH3_AF0
    41  PC9     47      10.1                                TIM3_CH4_AF0
        PF0             7.29    OSC_IN
        PF1             7.31    OSC_OUT
        VDD             7.5
        E5V             7.6
        BOOT0           7.7
        GND             7.8
        --              7.9 - 7.11, 7.26, 10.10, 10.36, 10.38
        IOREF           7.12
        RESET           7.14
        +3V3            7.16
        +5V             7.18
        GND             7.19 - 7.20, 7.22, 10.9, 10.20
        Vin             7.24
        VBAT            7.33
        AVDD            10.7
        U5V             10.8
        AGND            10.32
*/

#ifdef __cplusplus
extern "C" {
#endif

// System Settings
#define HSE_CRYSTAL_BYPASS          1   //  HSE crystal oscillator bypassed with external clock
#define HAL_USE_RTC                 1
//#define HAL_RTC_CHECK_LSE           1
#define HAL_RTC_USE_HSE             1

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_MAXPORT_NR           3
#define HAL_DIO_MAPPING  {  \
                /*  PA0,  PA1,  PA4,  PB0,  PC1,  PC0,  PC2,  PC3,  PA3,  PA2, PA10,  PB3 */    \
                      0,    1,    4,   16,   33,   32,   34,   35,  255,  255,   10,   19,      \
                /*  PB5,  PB4, PB10,  PA8,  PA9,  PC7,  PB6,  PA7,  PA6,  PA5,  PB9,  PB8 */    \
                     21,   20,   26,    8,    9,   39,   22,    7,    6,    5,   25,   24,      \
                /* PC10, PC11, PC12, PA13, PA14, PA15,  PB7, PC13, PC14, PC15,  PC4, PB13 */    \
                     42,   43,   44,  255,  255,   15,   23,   45,   46,   47,   36,   29,      \
                /* PB14, PB15,  PB1,  PB2, PB11, PB12, PA11, PA12,  PC5,  PC6,  PC8,  PC9 */    \
                     30,   31,   17,   18,   27,   28,   11,   12,   37,   38,   40,   41} 
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
#define EXTAIN_MAXPORT_NR           14
#define HAL_AIN_BASE2APIN           {   /* PA0, PA1, PA4, PB0, PC1, PC0, PC2, PC3, PA3, PA2 */  \
                                             0,   1,   4,   8,  11,  10,  12,  13,   3,   2,    \
                                      255, 255, 255, 255, 255, 255, 255, 255, 255,   /* GAP */  \
                                        /* PA7, PA6, PA5 */                                     \
                                             7,   6,   5}
// End Analogue Inputs

// UART Section
#define HAL_USE_USART3              0
#define HAL_USART3_REMAP            1   // USART3 on PC10/PC11
#define EXTSER_USED                 1
// End UART Section

// TWI Section
#define HAL_TWI_BUS                 1   // I2C1 on PB8/PB9
#define HAL_TWI_REMAP               1
#define EXTTWI_USED                 1
// End TWI Section

// UART PHY Section
#define HAL_USE_USART2              2   // USART2 on PA2/PA3
#define UART_PHY_PORT               2   // Logical port
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
