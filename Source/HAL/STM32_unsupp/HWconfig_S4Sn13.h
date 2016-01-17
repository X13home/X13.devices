/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef HWCONFIG_S4SN13_H
#define HWCONFIG_S4SN13_H

// uC: STM32F405RG
// PHY1: UART

// GPIOA
// Pin  Port    Func
//   0  PA0     _USART4_TX      AIn0
//   1  PA1     _USART4_RX      AIn1
//   2  PA2     USART2_TX       Ain2
//   3  PA3     USART2_RX       AIn3
//   4  PA4                     AIn4
//   5  PA5     _SPI1_SCK       AIn5    LED
//   6  PA6     _SPI1_MISO      AIn6
//   7  PA7     _SPI1_MOSI      AIn7
//   8  PA8
//   9  PA9     _USART1_TX
//  10  PA10    _USART1_RX
//  11  PA11
//  12  PA12
//  13  PA13    SWDIO
//  14  PA14    SWCLK
//  15  PA15
// GPIOB
//  16  PB0                     AIn8
//  17  PB1                     AIn9
//  18  PB2
//  19  PB3
//  20  PB4
//  21  PB5
//  22  PB6
//  23  PB7
//  24  PB8     I2C1-SCL
//  25  PB9     I2C1-SDA
//  26  PB10    _USART3_TX
//  27  PB11    _USART3_RX
//  28  PB12
//  29  PB13
//  30  PB14
//  31  PB15
// GPIOC
//  32  PC0                     AIn10
//  33  PC1                     AIn11
//  34  PC2                     AIn12
//  35  PC3                     AIn13
//  36  PC4                     AIn14
//  37  PC5                     AIn15
//  38  PC6
//  39  PC7
//  40  PC8
//  41  PC9
//  42  PC10    SPI3_SCK        FRAM_CLK
//  43  PC11    SPI3_MISO       FRAM_SO
//  44  PC12    SPI3_MOSI       FRAM_SI
//  45  PC13                    USER_BTN
//  46  PC14
//  47  PC15
// GPIOD
//      PD2                     FRAM_CS


#ifdef __cplusplus
extern "C" {
#endif

#ifdef OD_MAX_INDEX_LIST
#undef OD_MAX_INDEX_LIST
#endif  // OD_MAX_INDEX_LIST

#define OD_MAX_INDEX_LIST           40

#include "STM32/hal.h"

// FRAM Section
#define HAL_USE_SPI3

#define FRAM_NSS_PORT               GPIOD
#define FRAM_NSS_PIN                GPIO_Pin_2
#define FRAM_SPI_PORT               13
// End FRAM Section

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_MAXPORT_NR           3
#define EXTDIO_PORTNUM2MASK         {(uint16_t)0x600C, (uint16_t)0x0000, (uint16_t)0x1C00}
// End DIO Section

// PA0-PA7: 0 - 7
// PB0-PB1: 8 - 9
// PC0-PC5: 10-15
// Analogue Inputs
#define EXTAIN_USED                 0
#define EXTAIN_MAXPORT_NR           16
#define EXTAIN_BASE_2_APIN          {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}  // MUST BE ALIGNED TO 4
#define EXTAIN_REF                  0x02        // Bit0 - Ext, Bit1 - Vcc, Bit2 - Int1, Bit3 - Int2
// End Analogue Inputs

// UART Section
#define HAL_UART_NUM_PORTS          4
#define HAL_USE_USART1              0   // Mapping to logical port
#define HAL_USE_USART2              3
#define HAL_USE_USART3              1
#define HAL_USE_UART4               2


#define EXTSER_USED                 3

#define UART_PHY_PORT               3   // Logical Port 0/1/2...
// End UART Section

// TWI Section
//#define EXTTWI_USED                 0       // I2C_Bus 1 - I2C1, 2 - I2C2
// End TWI Section

#define UART_PHY                    1

#define PHY1_ADDR_t                 uint8_t
#define ADDR_BROADCAST_PHY1         (PHY1_ADDR_t)0x00
#define ADDR_UNDEF_PHY1             (PHY1_ADDR_t)0xFF

#define RF_ADDR_t                   uint8_t
#define ADDR_UNDEF_RF               (RF_ADDR_t)0xFF
//#define ADDR_DEFAULT_RF             (RF_ADDR_t)0x04

// Object's Dictionary Section
#define OD_DEV_UC_TYPE              'S'
#define OD_DEV_UC_SUBTYPE           '4'
#define OD_DEV_PHY1                 'S'
#define OD_DEV_PHY2                 'n'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '3'

#define OD_ADDR_TYPE                objUInt8

#include "../PHY/UART/uart_phy.h"

#define PHY1_Init                   UART_Init
#define PHY1_Send                   UART_Send
#define PHY1_Get                    UART_Get
#define PHY1_GetAddr                UART_GetAddr
#define PHY1_NodeId                 objRFNodeId
#define PHY1_GateId                 objGateID

#ifdef __cplusplus
}
#endif

#endif // HWCONFIG_S4SN13_H
