/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef HWCONFIG_S3EN12_H
#define HWCONFIG_S3EN12_H

// Board: OLIMEXINO-STM32
// uC: STM32F103
// PHY1: ENC28J60

// GPIOA
// Pin  Port    Mapple  Func
//   0  PA0     D2      AIN0
//   1  PA1     D3      AIN1/ LED Yellow
//   2  PA2     D1      AIN2/ USART2_TX
//   3  PA3     D0      AIN3/ USART2_RX
//   4  PA4     D10     AIN4/ /SS1
//   5  PA5     D13     AIN5/ /SCK1/ LED Green
//   6  PA6     D12     AIN6/ MISO1
//   7  PA7     D11     AIN7/ MOSI1
//   8  PA8     D6
//   9  PA9     D7      USART1_TX
//  10  PA10    D8      USART1_RX
//  11  PA11    USBDM
//  12  PA12    USBDP
//  13  PA13    TMS/SWDIO
//  14  PA14    TCK/SWCLK
//  15  PA15    TDI
// GPIOB
//  16  PB0     D27     AIN8
//  17  PB1     D28     AIN9
//  18  PB2     GND
//  19  PB3     TDO/SWO
//  20  PB4     TRST
//  21  PB5     D4      UEXT-10 /CS
//  22  PB6     D5
//  23  PB7     D9
//  24  PB8     D14     CAN-RX
//  25  PB9     D24     CAN-TX
//  26  PB10    D29     SCL2
//  27  PB11    D30     SDA2
//  28  PB12    D31     SPI2-NSS
//  29  PB13    D32     SPI2-SCLK
//  30  PB14    D33     SPI2-MISO
//  31  PB15    D34     SPI2-MOSI
// GPIOC
//  32  PC0     D15(A0) AIN10
//  33  PC1     D16(A1) AIN11
//  34  PC2     D17(A2) AIN12
//  35  PC3     D18(A3) AIN13
//  36  PC4     D19(A4) AIN14
//  37  PC5     D20(A5) AIN15
//  38  PC6     D35
//  39  PC7     D36
//  40  PC8     D37
//  41  PC9     SW_BUT
//  42  PC10    D26
//  43  PC11    USB_P
//  44  PC12    USB_DISC
//  45  PC13    D21     CAN_CTRL
//  46  PC14    D22     OSC32 In
//  47  PC15    D23     OSC32 Out
// GPIOD - Not Used
//  48  PD0     OSC In 8M
//  49  PD1     OSC Out 8M
//  50  PD2     D25     MMC_CS


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
#define EXTDIO_PORTNUM2MASK         {(uint16_t)0xF8E0, (uint16_t)0x001C, (uint16_t)0x1800}
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
#define HAL_USE_USART1              1
#define HAL_USE_USART2              1

#define EXTSER_USED                 1
#define EXTSER_PORT2UART            {2, 1}
// End UART Section

// ENC Section
#define HAL_USE_SPI1                1

#define ENC_USE_SPI                 1   // SPI1 PA5-PA7
#define ENC_NSS_PORT                GPIOB
#define ENC_NSS_PIN                 GPIO_Pin_5

#define ENC_SELECT()                ENC_NSS_PORT->BRR = ENC_NSS_PIN
#define ENC_RELEASE()               {while(SPI1->SR & SPI_SR_BSY); ENC_NSS_PORT->BSRR = ENC_NSS_PIN;}
// End ENC Section

#define ENC28J60_PHY                1

#define PHY1_ADDR_t                 uint32_t
#define ADDR_BROADCAST_PHY1         (PHY1_ADDR_t)inet_addr(255,255,255,255)
#define ADDR_UNDEF_PHY1             (PHY1_ADDR_t)inet_addr(255,255,255,255)

// Object's Dictionary Section
#define OD_DEV_UC_TYPE              'S'
#define OD_DEV_UC_SUBTYPE           '3'
#define OD_DEV_PHY1                 'E'
#define OD_DEV_PHY2                 'n'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '2'

#define OD_ADDR_TYPE                objUInt32
#define OD_DEV_MAC                  {0x00,0x04,0xA3,0x00,0x00,0x03}   // MAC MSB->LSB
//#define OD_DEF_IP_ADDR              inet_addr(192,168,10,202)
//#define OD_DEF_IP_MASK              inet_addr(255,255,255,0)
//#define OD_DEF_IP_ROUTER            inet_addr(192,168,10,1)
//#define OD_DEF_IP_BROKER            inet_addr(192,168,20,8)

#include "../PHY/ENC28J60/enc28j60_phy.h"

#define PHY1_Init                   ENC28J60_Init
#define PHY1_Send                   ENC28J60_Send
#define PHY1_Get                    ENC28J60_Get
#define PHY1_GetAddr                ENC28J60_GetAddr
#define PHY1_NodeId                 objIPAddr
#define PHY1_GateId                 objIPBroker

#ifdef __cplusplus
}
#endif

#endif // HWCONFIG_S3EN12_H
