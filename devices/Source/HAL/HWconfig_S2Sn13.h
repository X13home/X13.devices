/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef HWCONFIG_S2SN13_H
#define HWCONFIG_S2SN13_H

// Board: S2EC13
// uC: STM32F051C8T6
// PHY1: UART on PB6/PB7

// GPIOA
// Pin  Port    CN  Func
//   0  PA0     3
//   1  PA1     4
//   2  PA2     5   _USART2_TX
//   3  PA3     6   _USART2_RX
//   4  PA4     15
//   5  PA5     16
//   6  PA6     17
//   7  PA7     18
//   8  PA8
//   9  PA9     9
//  10  PA10    10
//  11  PA11    7
//  12  PA12    8
//  13  PA13        SWDIO
//  14  PA14        SWCLK
//  15  PA15
// GPIOB
//  16  PB0     19
//  17  PB1     20
//  18  PB2         LED
//  19  PB3
//  20  PB4
//  21  PB5
//  22  PB6         USART1_TX
//  23  PB7         USART1_RX
//  24  PB8
//  25  PB9
//  26  PB10    13  SCL2
//  27  PB11    14  SDA2
//  28  PB12
//  29  PB13
//  30  PB14
//  31  PB15


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
#define EXTDIO_MAXPORT_NR           2
#define EXTDIO_PORTNUM2MASK         {(uint16_t)0xE100, (uint16_t)0xF3FC}

#define EXTDIO_MAPPING              {0,1,2,3,11,12,9,10,26,27,4,5,6,7,16,17}    // Mapping logical ports to physical, only for ext_dio
// End DIO Section

// PA0-PA7: 0 - 7
// PB0-PB1: 8 - 9
// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           10
#define EXTAIN_BASE_2_APIN          {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xFF, 0xFF}  // MUST BE ALIGNED TO 4
#define EXTAIN_REF                  0x02        // Bit0 - Ext, Bit1 - Vcc, Bit2 - Int1, Bit3 - Int2
// End Analogue Inputs

// UART Section
#define HAL_UART_NUM_PORTS          2
#define HAL_USE_USART2              0           // Mapping to logical port
#define HAL_USE_ALT_USART1          1           // Use alternative pinout for USART1

#define EXTSER_USED                 1
// End UART Section

// TWI Section
#define EXTTWI_USED                 2           // I2C_Bus 1 - I2C1, 2 - I2C2
// End TWI Section

// LEDs
#define LED1_On()                   GPIOB->BSRR = GPIO_BSRR_BS_2
#define LED1_Off()                  GPIOB->BSRR = GPIO_BSRR_BR_2
#define LEDsInit()                  hal_dio_gpio_cfg(GPIOB, GPIO_Pin_2, DIO_MODE_OUT_PP)

// Object's Dictionary Section
#define OD_DEV_UC_TYPE              'S'
#define OD_DEV_UC_SUBTYPE           '2'
#define OD_DEV_PHY1                 'S'
#define OD_DEV_PHY2                 'n'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '3'

// PHY Section
#define UART_PHY                    1
#define UART_PHY_PORT               1   // Logical Port 0/1/2...

#include "../PHY/UART/uart_phy.h"

#ifdef __cplusplus
}
#endif

#endif // HWCONFIG_S2SN13_H
