/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef HWCONFIG_S2SC12_H
#define HWCONFIG_S2SC12_H

// Board: STM32F0DISCOVERY, MB1034B
// uC: STM32F051
// PHY1: UART
// PHY2: CC11/Anaren

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
//  26  PB10    SCL2
//  27  PB11    SDA2
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
#define EXTDIO_PORTNUM2MASK         {(uint16_t)0x6600, (uint16_t)0xF000, (uint16_t)0x0000}
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

// CC11 Section
#define CC11_ANAREN                 1
#define CC11_USE_SPI                2   // 1 - SPI1 PA4-PA7, 2 - SPI2 PB12-PB15

#if (CC11_USE_SPI == 1)
#define HAL_USE_SPI1                1

#define CC11_NSS_PORT               GPIOA
#define CC11_NSS_PIN                GPIO_Pin_4

#define SPIc_PORT                   GPIOA
#define SPIc_SCK_PIN                GPIO_Pin_5
#define SPIc_MISO_PIN               GPIO_Pin_6
#define SPIc_MOSI_PIN               GPIO_Pin_7

#define SPIc                        SPI1

#elif (CC11_USE_SPI == 2)
#define HAL_USE_SPI2                1

#define CC11_NSS_PORT               GPIOB
#define CC11_NSS_PIN                GPIO_Pin_12

#define SPIc_PORT                   GPIOB
#define SPIc_SCK_PIN                GPIO_Pin_13
#define SPIc_MISO_PIN               GPIO_Pin_14
#define SPIc_MOSI_PIN               GPIO_Pin_15

#define SPIc                        SPI2

#else
#error unknown CC11 configuration
#endif  //  CC11_USE_SPI

#define CC11_WAIT_LOW_MISO()        while(SPIc_PORT->IDR & SPIc_MISO_PIN)

#define CC11_SELECT()               CC11_NSS_PORT->BRR = CC11_NSS_PIN
#define CC11_RELEASE()              CC11_NSS_PORT->BSRR = CC11_NSS_PIN

// End CC11 Section

#define UART_PHY                    1
#define CC11_PHY                    2

// Object's Dictionary Section
#define OD_DEV_UC_TYPE              'S'
#define OD_DEV_UC_SUBTYPE           '2'
#define OD_DEV_PHY1                 'S'
#define OD_DEV_PHY2                 'C'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '2'

#include "../PHY/UART/uart_phy.h"
#include "../PHY/CC1101/cc11_phy.h"

#ifdef __cplusplus
}
#endif

#endif // HWCONFIG_S2SN12_H
