/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef HWCONFIG_A3SN12_H
#define HWCONFIG_A3SN12_H

#error old format

// Busware CSM V2.0
// uc ATMega1284p
// Phy1: UART

// PORTA
//  0   PA0     --      LEDG
//  1   PA1     --      LEDR
//  2   PA2     --      Loopback to PA3
//  3   PA3     --      Loopback to PA2
//  4   PA4     CN9
//  5   PA5     CN8
//  6   PA6     --
//  7   PA7     --
// PORTB
//  8   PB0     CN30
//  9   PB1     CN22    RF_IRQ
//  10  PB2     CN24
//  11  PB3     CN25    OC0A
//  12  PB4     --      RF_NCS/OC0B
//  13  PB5     CN28    RF_MOSI
//  14  PB6     CN27    RF_MISO/OC3A
//  15  PB7     CN29    RF_SCK/OC3B
// PORTC
//  16  PC0     CN15    SCL
//  17  PC1     CN14    SDA
//  18  PC2     CN13    JTAG_TCK
//  19  PC3     CN11    JTAG_TMS
//  20  PC4     CN10    JTAG_TDO
//  21  PC5     CN12    JTAG_TDI
//  22  PC6     --
//  23  PC7     --
// PORTD
//  24  PD0     CN3     RXD0
//  25  PD1     CN4     TXD0
//  26  PD2     CN23    RF_GDO2/Not Used
//  27  PD3     CN6
//  28  PD4     CN7     OC1B
//  29  PD5     --      LEDB/OC1A
//  30  PD6     --      LEDY
//  31  PD7     --

#ifdef __cplusplus
extern "C" {
#endif

#define F_CPU                       8000000UL

#include "AVR/hal.h"

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_MAXPORT_NR           4                                     // Number of digital Ports
#define EXTDIO_PORTNUM2MASK         {(uint8_t)0xC0, (uint8_t)0x10, (uint8_t)0x83}   // porta,b,d
// End DIO Section

// PWM Section
//#define EXTPWM_USED                 1
#define EXTPWM_MAXPORT_NR           5
#define EXTPWM_PORT2CFG             {0, 9, 8, 24, 25}           // bits 7-3 Timer, bits 2-0 Channel
#define EXTPWM_PORT2DIO             {11, 28, 29, 14, 15}        // Mapping PWM channel to DIO
// End PWM Section

// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           3           // ADC4-ADC5, Vbg
#define EXTAIN_BASE_2_APIN          {4, 5, 30}
#define EXTAIN_REF                  0x0E        // Bit0 - Ext, Bit1 - Vcc, Bit2 - Int1, Bit3 - Int2
// End Analogue Inputs

// TWI Section
#define EXTTWI_USED                 1
// End TWI Section

// LEDs
#define LED1_On()                   PORTA |= (1<<PA0)
#define LED1_Off()                  PORTA &= ~(1<<PA0)
#define LEDsInit()                  {DDRA |= ((1<<PA0)); PORTA |= (1<<PA0);}

// UART Section
#define UART_PHY_PORT               0
// End UART Section

#define UART_PHY                    1

#define PHY1_ADDR_t                 uint8_t
#define ADDR_BROADCAST_PHY1         (PHY1_ADDR_t)0x00
#define ADDR_UNDEF_PHY1             (PHY1_ADDR_t)0xFF

#define RF_ADDR_t                   uint8_t
#define ADDR_UNDEF_RF               (RF_ADDR_t)0xFF
//#define ADDR_DEFAULT_RF             (RF_ADDR_t)0x04

// Object's Dictionary Section
#define OD_DEV_UC_TYPE              'A'
#define OD_DEV_UC_SUBTYPE           '3'
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

#endif // HWCONFIG_A3SN12_H