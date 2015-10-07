/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef HWCONFIG_S4SN12_H
#define HWCONFIG_S4SN12_H

// Board: Nucleo-F401RE
// uC: STM32F401RE
// PHY1: UART

// GPIOA
// Pin  Port    CN      Func
//   0  PA0     7-28
//   1  PA1     7-30
//   2  PA2     10-35   USART2_TX
//   3  PA3     10-37   USART2_RX
//   4  PA4     7-32
//   5  PA5     10-11   SPI1_SCK    // Enc28J60 CLK, compatible with Arduino
//   6  PA6     10-13   SPI1_MISO   // Enc28J60 SO,  compatible with Arduino
//   7  PA7     10-15   SPI1_MOSI   // Enc28J60 SI,  compatible with Arduino
//   8  PA8     10-23
//   9  PA9     10-21   USART1_TX
//  10  PA10    10-33   USART1_RX
//  11  PA11    10-14
//  12  PA12    10-12
//  13  PA13    7-13    SWDIO
//  14  PA14    7-15    SWCLK
//  15  PA15    7-17
// GPIOB
//  16  PB0     7-34
//  17  PB1     10-24
//  18  PB2     10-22
//  19  PB3     10-31
//  20  PB4     10-27
//  21  PB5     10-29
//  22  PB6     10-17   ENC_NSS     // Enc28J60 Chip Select, compatible with Arduino
//  23  PB7     7-21
//  24  PB8     10-3    I2C1-SCL
//  25  PB9     10-5    I2C1-SDA
//  26  PB10    10-25
//  27  PB11    10-18   !! Not Used, internal Vreg
//  28  PB12    10-16
//  29  PB13    10-30
//  30  PB14    10-28
//  31  PB15    10-26
// GPIOC
//  32  PC0     7-38
//  33  PC1     7-36
//  34  PC2     7-35
//  35  PC3     7-37
//  36  PC4     10-34
//  37  PC5     10-6
//  38  PC6     10-4
//  39  PC7     10-19
//  40  PC8     10-2
//  41  PC9     10-1
//  42  PC10    7-1     SPI3_SCK    // Enc28J60, Alternative CLK
//  43  PC11    7-2     SPI3_MISO   // Enc28J60, Alternative SO
//  44  PC12    7-3     SPI3_MOSI   // Enc28J60, Alternative SI
//  45  PC13    7-23                // NPN User Button
//  46  PC14    7-25
//  47  PC15    7-27
// GPIOD
//      PD2     7-4                 // Enc28J60, Alternative NCS


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
//#define EXTAIN_USED                 0
#define EXTAIN_MAXPORT_NR           16
#define EXTAIN_BASE_2_APIN          {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}  // MUST BE ALIGNED TO 4
#define EXTAIN_REF                  0x02        // Bit0 - Ext, Bit1 - Vcc, Bit2 - Int1, Bit3 - Int2
// End Analogue Inputs

// UART Section
#define UART_PHY_PORT               0       // 0 - USART1 PA9,PA10 GPIOA MASK 0x0600, 1 - USART2 PA2,PA3 GPIOC MASK 0x000C
#define EXTSER_USED                 1
#define EXTSER_PORT2UART            {1}
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
#define OD_DEV_HW_TYP_L             '2'

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

#endif // HWCONFIG_S4SN12_H
