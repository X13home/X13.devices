/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef HWCONFIG_S2SN12_H
#define HWCONFIG_S2SN12_H

// Board: STM32F0DISCOVERY, MB1034B
// uC: STM32F051
// PHY1: UART

// GPIOA
// Pin  Port    Func
//   0  PA0     PNP - User Btn
//   1  PA1
//   2  PA2     USART2_TX
//   3  PA3     USART2_RX
//   4  PA4
//   5  PA5
//   6  PA6
//   7  PA7
//   8  PA8
//   9  PA9     USART1_TX
//  10  PA10    USART1_RX
//  11  PA11
//  12  PA12
//  13  PA13    SWDIO
//  14  PA14    SWCLK
//  15  PA15
// GPIOB
//  16  PB0
//  17  PB1
//  18  PB2
//  19  PB3
//  20  PB4
//  21  PB5
//  22  PB6     SCL1
//  23  PB7     SDA1
//  24  PB8
//  25  PB9
//  26  PB10    SCL2
//  27  PB11    SDA2
//  28  PB12
//  29  PB13
//  30  PB14
//  31  PB15
// GPIOC
//  32  PC0
//  33  PC1
//  34  PC2
//  35  PC3
//  36  PC4
//  37  PC5
//  38  PC6
//  39  PC7
//  40  PC8     LED_Blue
//  41  PC9     LED_Green
//  42  PC10
//  43  PC11
//  44  PC12
//  45  PC13
//  46  PC14
//  47  PC15
// GPIOD, GPIOF - Not used


#ifdef __cplusplus
extern "C" {
#endif

#ifdef OD_MAX_INDEX_LIST
#undef OD_MAX_INDEX_LIST
#endif  // OD_MAX_INDEX_LIST

#define OD_MAX_INDEX_LIST           40

#include "STM32/hal.h"

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_MAXPORT_NR           3
#define EXTDIO_PORTNUM2MASK         {(uint16_t)0x600C, (uint16_t)0x0000, (uint16_t)0x0000}
// End DIO Section

// PA0-PA7: 0 - 7
// PB0-PB1: 8 - 9
// PC0-PC5: 10-15
// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           16
#define EXTAIN_BASE_2_APIN          {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}  // MUST BE ALIGNED TO 4
#define EXTAIN_REF                  0x02        // Bit0 - Ext, Bit1 - Vcc, Bit2 - Int1, Bit3 - Int2
// End Analogue Inputs

// UART Section
#define HAL_UART_NUM_PORTS          2
#define HAL_USE_USART1              0   // Mapping to logical port
#define HAL_USE_USART2              1

#define EXTSER_USED                 1

#define UART_PHY_PORT               1   // Logical Port 0/1/2...
// End UART Section

// TWI Section
#define EXTTWI_USED                 1       // I2C_Bus 1 - I2C1, 2 - I2C2
// End TWI Section

#define UART_PHY                    1

// Object's Dictionary Section
#define OD_DEV_UC_TYPE              'S'
#define OD_DEV_UC_SUBTYPE           '2'
#define OD_DEV_PHY1                 'S'
#define OD_DEV_PHY2                 'n'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '2'

#include "../PHY/UART/uart_phy.h"

#ifdef __cplusplus
}
#endif

#endif // HWCONFIG_S2SN12_H
