/*
Copyright (c) 2011-2016 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _A4SN10_H
#define _A4SN10_H

// Board:   Arduino MEGA
// uc:      ATMega2560
// PHY1:    UART
// PHY2:    --

// Dio  Prt CN          FN1     FN2
//  40  PF0 A0          Ain0
//  41  PF1 A1          Ain1
//  42  PF2 A2          Ain2
//  43  PF3 A3          Ain3
//  44  PF4 A4          Ain4    JTAG-TCK
//  45  PF5 A5          Ain5    JTAG-TMS
//  46  PF6 A6          Ain6    JTAG-TDO
//  47  PF7 A7          Ain7    JTAG-TDI
//  72  PK0 A8          Ain8
//  73  PK1 A9          Ain9
//  74  PK2 A10         Ain10
//  75  PK3 A11         Ain11
//  76  PK4 A12         Ain12
//  77  PK5 A13         Ain13
//  78  PK6 A14         Ain14
//  79  PK7 A15         Ain15
//  32  PE0 0         * RXD0
//  33  PE1 1         * TXD0
//  36  PE4 2           OC3B
//  37  PE5 3           OC3C
//  53  PG5 4           OC0B
//  35  PE3 5           OC3A
//  59  PH3 6           OC4A
//  60  PH4 7           OC4B
//  61  PH5 8           OC4C
//  62  PH6 9           (OC2B)
//  12  PB4 10          (OC2A)
//  13  PB5 11          OC1A
//  14  PB6 12          OC1B
//  15  PB7 13          OC1C    LED_L
//  65  PJ1 14          TXD3
//  64  PJ0 15          RXD3
//  57  PH1 16          TXD2
//  56  PH0 17          RXD2
//  27  PD3 18          TXD1
//  26  PD2 19          RXD1
//  25  PD1 20          SDA
//  24  PD0 21          SCL
//  0   PA0 22
//  1   PA1 23
//  2   PA2 24
//  3   PA3 25
//  4   PA4 26
//  5   PA5 27
//  6   PA6 28
//  7   PA7 29
//  23  PC7 30
//  22  PC6 31
//  21  PC5 32
//  20  PC4 33
//  19  PC3 34
//  18  PC2 35
//  17  PC1 36
//  16  PC0 37
//  31  PD7 38
//  50  PG2 39
//  49  PG1 40
//  48  PG0 41
//  87  PL7 42
//  86  PL6 43
//  85  PL5 44          OC5C
//  84  PL4 45          OC5B
//  83  PL3 46          OC5A
//  82  PL2 47
//  81  PL1 48
//  80  PL0 49
//  11  PB3 50/ICSP-1   MISO
//  10  PB2 51/ICSP-4   MOSI
//  9   PB1 52/ICSP-3   SCK
//  8   PB0 53          SS

#ifdef __cplusplus
extern "C" {
#endif

#define F_CPU                       16000000UL

////////////////////////////////////////////////////////
// Extensions configuration

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_MAXPORT_NR           11          // Number of used physical digital Ports
#define HAL_DIO_MAPPING     { 40, 41, 42, 43, 44, 45, 46, 47, 72, 73, 74, 75, 76, 77, 78, 79,   \
                             255, 255, 36, 37, 53, 35, 59, 60, 61, 62, 12, 13, 14, 15, 65, 64,  \
                              57, 56, 27, 26, 25, 24, 0, 1, 2, 3, 4, 5, 6, 7, 23, 22,           \
                              21, 20, 19, 18, 17, 16, 31, 50, 49, 48, 87, 86, 85, 84, 83, 82,   \
                              81, 80, 11, 10, 9, 8}
// End DIO Section

// PWM Section
#define EXTPWM_USED                 1
#define HAL_PWM_BASE_OFFSET          18
#define HAL_PWM_PORT2CFG             {((3<<3) | 1),      /* PE4, OC3B */ \
                                      ((3<<3) | 2),      /* PE5, OC3C */ \
                                      ((0<<3) | 1),      /* PG5, OC0B */ \
                                      ((3<<3) | 0),      /* PE3, OC3A */ \
                                      ((4<<3) | 0),      /* PH3, OC4A */ \
                                      ((4<<3) | 1),      /* PH4, OC4B */ \
                                      ((4<<3) | 2),      /* PH5, OC4C */ \
                                      255,255,           /* PH6, PB4, OC2 busy */\
                                      ((1<<3) | 0),      /* PB5, OC1A */ \
                                      ((1<<3) | 1),      /* PB6, OC1B */ \
                                      ((1<<3) | 2)}      /* PB7, OC1C */
 // End PWM Section

// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           0x28                                  // ADC0 - ADC15, Vbg
#define HAL_AIN_BASE2APIN           {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,        \
                                     0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x1E}
// End Analogue Inputs

// UART Section
#define HAL_UART_NUM_PORTS          4
#define HAL_USE_USART0              3
#define HAL_USE_USART1              0
#define HAL_USE_USART2              1
#define HAL_USE_USART3              2
#define EXTSER_USED                 3
// End UART Section

// TWI Section
#define EXTTWI_USED                 1
// End TWI Section

// PLC Section
#define EXTPLC_USED                     1
#define EXTPLC_SIZEOF_PRG               2048
#define EXTPLC_SIZEOF_PRG_CACHE         32      // Must be 2^n, bytes
#define EXTPLC_SIZEOF_RAM               256     // size in uint32_t
#define EXTPLC_SIZEOF_RW                16      // size in uint32_t
// PLC Section

// End Extensions configuration
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// Start PHY Section

// UART PHY Section
#define UART_PHY_PORT               3       // Logical Port Number 0,1,2,...
#define UART_PHY                    1
#include "PHY/UART/uart_phy.h"
// End UART PHY Section

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           80      // Size of identificators list
#define OD_DEV_UC_TYPE              'A'
#define OD_DEV_UC_SUBTYPE           '4'
#define OD_DEV_PHY1                 'S'
#define OD_DEV_PHY2                 'n'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '0'

#ifdef __cplusplus
}
#endif

#endif // _A4SN12_H