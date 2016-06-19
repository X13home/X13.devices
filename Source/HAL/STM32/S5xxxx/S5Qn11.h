/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _S5QN11_H
#define _S5QN11_H

// Board: CPUe
// uC: STM32L051K8
// PHY1: RFM69
// PHY2: --

// GPIOA
// Pin  Port    CN  Func
//   0  PA0     3
//   1  PA1     4
//   2  PA2     5
//   3  PA3     6
//   4  PA4     1
//   5  PA5     16
//   6  PA6     17
//   7  PA7     18
//   8  PA8         RFM69_IRQ
//   9  PA9     9
//  10  PA10    10
//  11  PA11    7
//  12  PA12    8
//  13  PA13        SWDIO
//  14  PA14        SWCLK
//  15  PA15        RFM69_SEL
// GPIOB
//  16  PB0     19
//  17  PB1     20
//  19  PB3         RFM69_SCK
//  20  PB4         RFM69_MISO
//  21  PB5         RFM69_MOSI
//  22  PB6     13  SCL1
//  23  PB7     14  SDA1

// System
//      3.3V    12
//      Vin     2
//      GND     11
//      GND     1

#ifdef __cplusplus
extern "C" {
#endif

// System Settings
#define HAL_USE_RTC                 1   // Enable Hardware RTC
//#define HAL_RTC_CHECK_LSE           1   // Check if exist Low Speed External Oscillator,
                                        // then use LSE, else LSI
// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_MAXPORT_NR           2
/* PB1, PB0, PA7, PA6, PA5, PA4, PB7, PB6, PA10, PA9, PA12, PA11, PA3 - PA0 */
#define HAL_DIO_MAPPING             {17, 16, 7, 6, 5, 4, 23, 22, 10, 9, 12, 11, 3, 2, 1, 0}
// End DIO Section

// PA0-PA7: 0 - 7
// PB0-PB1: 8 - 9
// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           10
#define HAL_AIN_BASE2APIN           {9, 8, 7, 6, 5, 4, 255, 255, 255, 255, 255, 255, 3, 2, 1, 0}
// End Analogue Inputs

// TWI Section
#define HAL_TWI_BUS                 1       // I2C on PB6, PB7
#define EXTTWI_USED                 1
// End TWI Section

// RFM69 Section
#define HAL_USE_SPI1                2                           // SPI1 on PB3 - PB5
#define RFM69_USE_SPI               1
#define RFM69_NSS_PIN               15                          // PA15
#define RFM69_SELECT()              GPIOA->BRR = GPIO_Pin_15
#define RFM69_RELEASE()             GPIOA->BSRR = GPIO_Pin_15
#define RFM69_IRQ_PIN               8                           // PA8
#define RFM69_IRQ_STATE()           ((GPIOA->IDR & GPIO_Pin_8) != 0)
#define RFM69_PHY                   1
#include "PHY/RFM69/rfm69_phy.h"
// End RFM69 Section

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           20
#define OD_DEV_UC_TYPE              'S'
#define OD_DEV_UC_SUBTYPE           '5'
#define OD_DEV_PHY1                 'Q'
#define OD_DEV_PHY2                 'n'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '1'

#ifdef __cplusplus
}
#endif

#endif // _S5QN11_H
