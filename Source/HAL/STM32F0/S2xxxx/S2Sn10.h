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
// Pin  Port    CN  Func
//   0  PA0     
//   1  PA1     
//   2  PA2        _USART2_TX
//   3  PA3        _USART2_RX
//   4  PA4     
//   5  PA5     
//   6  PA6     
//   7  PA7     
//   8  PA8     
//   9  PA9        _USART1_TX
//  10  PA10       _USART1_RX
//  11  PA11    
//  12  PA12    
//  13  PA13        SWDIO
//  14  PA14        SWCLK
//  15  PA15
// GPIOB
//  16  PB0     
//  17  PB1     
//  18  PB2     
//  19  PB3
//  20  PB4
//  21  PB5
//  22  PB6
//  23  PB7
//  24  PB8
//  25  PB9
//  26  PB10      SCL2
//  27  PB11      SDA2
//  28  PB12    
//  29  PB13    
//  30  PB14    
//  31  PB15    


#ifdef __cplusplus
extern "C" {
#endif

// UART PHY Section
#define HAL_UART_NUM_PORTS          1
#define HAL_USE_USART1              0

#define UART_PHY_PORT               0   // Logical port
#define UART_PHY                    1
#include "PHY/UART/uart_phy.h"
// End UART PHY Section

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           40
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
