/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef HWCONFIG_A4ES12_H
#define HWCONFIG_A4ES12_H

// Arduino MEGA
// uc ATMega2560
// PHY1: ENC28J60
// PHY2: UART

// Dio  Prt CN          FN1     FN2

//  0   PA0 22
//  1   PA1 23
//  2   PA2 24
//  3   PA3 25
//  4   PA4 26
//  5   PA5 27
//  6   PA6 28
//  7   PA7 29
//  8   PB0 53          SS
//  9   PB1 52/ICSP-3   SCK
//  10  PB2 51/ICSP-4   MOSI
//  11  PB3 50/ICSP-1   MISO
//  12  PB4 10
//  13  PB5 11          PWM7
//  14  PB6 12          PWM8
//  15  PB7 13          PWM9    LED_L
//  16  PC0 37
//  17  PC1 36
//  18  PC2 35
//  19  PC3 34
//  20  PC4 33
//  21  PC5 32
//  22  PC6 31
//  23  PC7 30
//  24  PD0 21          SCL
//  25  PD1 20          SDA
//  26  PD2 19          RXD1
//  27  PD3 18          TXD1
//  28  PD4 --
//  29  PD5 --
//  30  PD6 --
//  31  PD7 38
//  32  PE0 0           RXD0
//  33  PE1 1           TXD0
//  34  PE2 --
//  35  PE3 5           PWM3
//  36  PE4 2           PWM0
//  37  PE5 3           PWM1
//  38  PE6 --
//  39  PE7 --
//  40  PF0 A0          Ain0
//  41  PF1 A1          Ain1
//  42  PF2 A2          Ain2
//  43  PF3 A3          Ain3
//  44  PF4 A4          JTAG-TCK
//  45  PF5 A5          JTAG-TMS
//  46  PF6 A6          JTAG-TDO
//  47  PF7 A7          JTAG-TDO
//  48  PG0 41
//  49  PG1 40
//  50  PG2 39
//  51  PG3 --
//  52  PG4 --
//  53  PG5 4           PWM2
//  56  PH0 17          RXD2
//  57  PH1 16          TXD2
//  58  PH2 --
//  59  PH3 6           PWM4
//  60  PH4 7           PWM5
//  61  PH5 8           PWM6
//  62  PH6 9
//  63  PH7 --
//  64  PJ0 15          RXD3
//  65  PJ1 14          TXD3
//  66  PJ2 --
//  67  PJ3 --
//  68  PJ4 --
//  69  PJ5 --
//  70  PJ6 --
//  71  PJ7 --
//  72  PK0 A8          Ain8
//  73  PK1 A9          Ain9
//  74  PK2 A10         Ain10
//  75  PK3 A11         Ain11
//  76  PK4 A12         Ain12
//  77  PK5 A13         Ain13
//  78  PK6 A14         Ain14
//  79  PK7 A15         Ain15
//  80  PL0 49
//  81  PL1 48
//  82  PL2 47
//  83  PL3 46          PWM10
//  84  PL4 45          PWM11
//  85  PL5 44          PWM12
//  86  PL6 43
//  87  PL7 42

#ifdef __cplusplus
extern "C" {
#endif

#define F_CPU                       16000000UL

#ifdef OD_MAX_INDEX_LIST
#undef OD_MAX_INDEX_LIST
#endif	// OD_MAX_INDEX_LIST

#define OD_MAX_INDEX_LIST           40

#include "AVR/hal.h"
#include <util/delay.h>

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_MAXPORT_NR           11                                     // Number of digital Ports
                                     // PA, PB, PC, PD
                                     // PE, PF, PG, PH,
                                     // PJ, PK, PL
#define EXTDIO_PORTNUM2MASK         {(uint8_t)0x00, (uint8_t)0x0F, (uint8_t)0x00, (uint8_t)0x73,  \
                                     (uint8_t)0xC7, (uint8_t)0xF0, (uint8_t)0xD8, (uint8_t)0x84,  \
                                     (uint8_t)0xFC, (uint8_t)0x00, (uint8_t)0x00}
// End DIO Section
//#define EXTPWM_USED                 1
#define EXTPWM_MAXPORT_NR           13
#define EXTPWM_PORT2CFG             {0x19, 0x1A, 0x01, 0x18, 0x20, 0x21, 0x22, 0x08, 0x09, 0x0A, 0x28, 0x29, 0x2A}  // bits 7-3 Timer, bits 2-0 Channel
#define EXTPWM_PORT2DIO             {36, 37, 53, 35, 59, 60, 61, 13, 14, 15, 83, 84, 85}    // Mapping PWM channel to DIO
// End PWM Section

// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           16           // ADC0-ADC15
#define EXTAIN_BASE_2_APIN          {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,\
                                     0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27}
#define EXTAIN_REF                  0x0F        // Bit0 - Ext, Bit1 - Vcc, Bit2 - Int1, Bit3 - Int2
// End Analogue Inputs

// TWI Section
#define EXTTWI_USED                 1
// End TWI Section

// UART Section
#define UART_PHY_PORT               0

#define EXTSER_USED                 4
// End UART Section

// LAN Section
#define ENC_PORT                    PORTB
#define ENC_DDR                     DDRB
#define ENC_PIN                     PINB
#define ENC_PIN_SS                  PB0
#define ENC_PIN_SCK                 PB1
#define ENC_PIN_MOSI                PB2
#define ENC_PIN_MISO                PB3

#define ENC_SELECT()                (PORTB &= ~(1<<ENC_PIN_SS))
#define ENC_RELEASE()               (PORTB |= (1<<ENC_PIN_SS))
// End LAN Section

#define ENC28J60_PHY                1
#define UART_PHY                    2

#define PHY1_ADDR_t                 uint32_t
#define ADDR_BROADCAST_PHY1         (PHY1_ADDR_t)inet_addr(255,255,255,255)
#define ADDR_UNDEF_PHY1             (PHY1_ADDR_t)inet_addr(255,255,255,255)

#define PHY2_ADDR_t                 uint8_t
#define ADDR_BROADCAST_PHY2         (PHY2_ADDR_t)0x00
#define ADDR_UNDEF_PHY2             (PHY2_ADDR_t)0xFF

#define RF_ADDR_t                   uint8_t
#define ADDR_UNDEF_RF               (RF_ADDR_t)0xFF
#define ADDR_DEFAULT_RF             (RF_ADDR_t)2

// Object's Dictionary Section
#define OD_DEV_UC_TYPE              'A'
#define OD_DEV_UC_SUBTYPE           '4'
#define OD_DEV_PHY1                 'E'
#define OD_DEV_PHY2                 'S'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '2'
#define OD_ADDR_TYPE                objUInt32
#define OD_DEV_MAC                  {0x00,0x04,0xA3,0x00,0x00,0x11}   // MAC MSB->LSB
//#define OD_DEF_IP_ADDR              inet_addr(192,168,20,216)
//#define OD_DEF_IP_MASK              inet_addr(255,255,255,0)
//#define OD_DEF_IP_ROUTER            inet_addr(192,168,20,1)
//#define OD_DEF_IP_BROKER            inet_addr(192,168,10,1)

#include "../PHY/ENC28J60/enc28j60_phy.h"
#include "../PHY/UART/uart_phy.h"

#define PHY1_Init                   ENC28J60_Init
#define PHY1_Send                   ENC28J60_Send
#define PHY1_Get                    ENC28J60_Get
#define PHY1_GetAddr                ENC28J60_GetAddr
#define PHY1_NodeId                 objIPAddr
#define PHY1_GateId                 objIPBroker

#define PHY2_Init                   UART_Init
#define PHY2_Send                   UART_Send
#define PHY2_Get                    UART_Get
#define PHY2_GetAddr                UART_GetAddr
#define PHY2_NodeId                 objRFNodeId

#ifdef __cplusplus
}
#endif

#endif // HWCONFIG_A4EN12_H