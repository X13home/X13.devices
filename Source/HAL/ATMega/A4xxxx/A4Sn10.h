/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _A4SN10_H
#define _A4SN10_H

// Arduino MEGA
// uc ATMega2560
// PHY: UART

// Dio  Prt CN          FN1     FN2
//  40  PF0 A0          Ain0
//  41  PF1 A1          Ain1
//  42  PF2 A2          Ain2
//  43  PF3 A3          Ain3
//  44  PF4 A4          Ain4    JTAG-TCK
//  45  PF5 A5          Ain5    JTAG-TMS
//  46  PF6 A6          Ain6    JTAG-TDO
//  47  PF7 A7          Ain7    JTAG-TDO
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
//  36  PE4 2           PWM0
//  37  PE5 3           PWM1
//  53  PG5 4           PWM2
//  35  PE3 5           PWM3
//  59  PH3 6           PWM4
//  60  PH4 7           PWM5
//  61  PH5 8           PWM6
//  62  PH6 9
//  12  PB4 10
//  13  PB5 11          PWM7
//  14  PB6 12          PWM8
//  15  PB7 13          PWM9    LED_L
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
//  85  PL5 44          PWM12
//  84  PL4 45          PWM11
//  83  PL3 46          PWM10
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
#define EXTDIO_MAXPORT_NR           11                                  // Number of used physical digital Ports
#define HAL_DIO_MAPPING             {40, 41, 42, 43, 44, 45, 46, 47, 72, 73, 74, 75, 76, 77, 78, 79,    \
                                     255, 255, 36, 37, 53, 35, 59, 60, 61, 62, 12, 13, 14, 15, 65, 64,  \
                                     57, 56, 27, 26, 25, 24, 0, 1, 2, 3, 4, 5, 6, 7, 23, 22,            \
                                     21, 20, 19, 18, 17, 16, 31, 50, 49, 48, 87, 86, 85, 84, 83, 82,    \
                                     81, 80, 11, 10, 9, 8}
// End DIO Section

// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           17                                  // ADC0 - ADC15, Vbg
#define EXTAIN_BASE_2_APIN          {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x1E}
#define EXTAIN_REF                  0x0F                                // Bit0 - Ext, Bit1 - Vcc, Bit2 - Int1, Bit3 - Int2
// End Analogue Inputs

/*
//#define EXTPWM_USED                 1
#define EXTPWM_MAXPORT_NR           13
#define EXTPWM_PORT2CFG             {0x19, 0x1A, 0x01, 0x18, 0x20, 0x21, 0x22, 0x08, 0x09, 0x0A, 0x28, 0x29, 0x2A}  // bits 7-3 Timer, bits 2-0 Channel
#define EXTPWM_PORT2DIO             {36, 37, 53, 35, 59, 60, 61, 13, 14, 15, 83, 84, 85}    // Mapping PWM channel to DIO
// End PWM Section

// TWI Section
#define EXTTWI_USED                 1
// End TWI Section

// UART Section
#define UART_PHY_PORT               0

#define EXTSER_USED                 4
// End UART Section
*/

// End Extensions configuration
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// Start PHY Section

// UART Section
#define HAL_USE_USART0              0       // Logical Port Number 0,1,2,...
#define HAL_UART_NUM_PORTS          1

#define UART_PHY_PORT               0       // Logical Port Number 0,1,2,...
// End UART Section

#define UART_PHY                    1

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           80      // Size of identificators list
#define OD_DEV_UC_TYPE              'A'
#define OD_DEV_UC_SUBTYPE           '4'
#define OD_DEV_PHY1                 'S'
#define OD_DEV_PHY2                 'n'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '0'

#include "PHY/UART/uart_phy.h"

#ifdef __cplusplus
}
#endif

#endif // _A4SN12_H