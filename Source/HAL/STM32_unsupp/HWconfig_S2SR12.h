/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef HWCONFIG_S2SR12_H
#define HWCONFIG_S2SR12_H

// Board: STM32F0DISCOVERY, MB1034B
// uC: STM32F051
// PHY1: UART
// PHY2: RFM12

// GPIOA
// Pin  Port    Func
//   0  PA0     
//   1  PA1
//   2  PA2     USART2_TX
//   3  PA3     USART2_RX
//   4  PA4     SPI1_NSS
//   5  PA5     SPI1_SCK
//   6  PA6     SPI1_MISO
//   7  PA7     SPI1_MOSI
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
//  26  PB10
//  27  PB11    RFM12_IRQ
//  28  PB12    SPI2_NSS
//  29  PB13    SPI2_SCK
//  30  PB14    SPI2_MISO
//  31  PB15    SPI2_MOSI
// GPIOC
//  32  PC0
//  33  PC1
//  34  PC2
//  35  PC3
//  36  PC4
//  37  PC5
//  38  PC6
//  39  PC7
//  40  PC8     LED_Blue, Activity UART
//  41  PC9     LED_Green, Activity RF
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
#define EXTDIO_PORTNUM2MASK         {(uint16_t)0x600C, (uint16_t)0xF800, (uint16_t)0x0300}
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
#define HAL_USE_USART1              1   // Mapping to logical port
#define HAL_USE_USART2              0

#define EXTSER_USED                 1

#define UART_PHY_PORT               1   // Logical Port 0/1/2...
// End UART Section

// TWI Section
#define EXTTWI_USED                 1
// End TWI Section

// RFM12 Section
#define HAL_USE_EXTI                1
#define HAL_USE_SPI2                1

#define RFM12_USE_SPI               2   // 1 - SPI1 PA4-PA7, 2 - SPI2 PB12-PB15, PB11 - IRQ

#define RFM12_NSS_PORT              GPIOB
#define RFM12_NSS_PIN               GPIO_Pin_12

#define RFM12_IRQ_PORT              GPIOB
#define RFM12_IRQ_PIN               GPIO_Pin_11
// End RFM12 Section

// LEDs
#define LED1_On()                   GPIOC->BSRR = GPIO_BSRR_BS_8
#define LED1_Off()                  GPIOC->BSRR = GPIO_BSRR_BR_8
#define LED2_On()                   GPIOC->BSRR = GPIO_BSRR_BS_9
#define LED2_Off()                  GPIOC->BSRR = GPIO_BSRR_BR_9

#define LEDsInit()                  hal_dio_gpio_cfg(GPIOC, GPIO_Pin_8 | GPIO_Pin_8, DIO_MODE_OUT_PP)
// End LEDs

#define UART_PHY                    1
#define RFM12_PHY                   2

// Object's Dictionary Section
#define OD_DEV_UC_TYPE              'S'
#define OD_DEV_UC_SUBTYPE           '2'
#define OD_DEV_PHY1                 'S'
#define OD_DEV_PHY2                 'R'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '2'

#include "../PHY/UART/uart_phy.h"
#include "../PHY/RFM12/rfm12_phy.h"

#ifdef __cplusplus
}
#endif

#endif // HWCONFIG_S2SR12_H
