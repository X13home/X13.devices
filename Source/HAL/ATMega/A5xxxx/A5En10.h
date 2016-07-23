/*
Copyright (c) 2011-2016 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _A5EN10_H
#define _A5EN10_H

// Board:   MegaD-2561
// uc:      ATMega2561
// PHY1:    ENC28J60
// PHY2:    --

// Dio  Prt Pin CN          FN1     FN2
//  52  PG4 0   XP1.5
//  31  PD7 1   XP1.7
//  30  PD6 2   XP1.9
//  32  PE0 3   XP1.11      RXD0
//  15  PB7 4   XP1.13      OC1C
//  12  PB4 5   XP1.15     (OC2A)
//  14  PB6 6   XP1.17      OC1B
//  13  PB5 7   XP1.19      OC1A
//  53  PG5 8   XP1.21      OC0B
//  40  PF0 9   XP1.23      Ain0
//  41  PF1 10  XP1.25      Ain1
//  42  PF2 11  XP1.27      Ain2
//  43  PF3 12  XP1.29      Ain3
//  44  PF4 13  XP1.31      Ain4    JTAG-TCK
//  45  PF5 14  XP1.33      Ain5    JTAG-TMS
//  53  PG3 15  XP2.5
//  39  PE7 16  XP2.7
//  38  PE6 17  XP2.9
//  34  PE2 18  XP2.11
//  37  PE5 19  XP2.13      OC3C
//  33  PE1 20  XP2.15      TXD0
//  36  PE4 21  XP2.17      OC3B
//  35  PE3 22  XP2.19      OC3A
//  0   PA0 23  XP2.21
//  1   PA1 24  XP2.23
//  2   PA2 25  XP2.25
//  3   PA3 26  XP2.27
//  4   PA4 27  XP2.29
//  5   PA5 28  XP2.31
//  6   PA6 29  XP2.33
//  24  PD0 30  XT2.4       SCL
//  25  PD1 31  XT2.5       SDA
//  26  PD2 32  XT2.7       RXD1
//  27  PD3 33  XT2.8       TXD1
//  28  PD4 34  XT2.10
//  29  PD5 35  XT2.11
//  46  PF6 36  XP5.1       Ain6    JTAG-TDO
//  47  PF7 37  XP6.1       Ain7    JTAG-TDI
//  18  PC2 38  XP4.5
//  17  PC1 39  XP4.6
//  49  PG1 40  XP4.7
//  16  PC0 41  XP4.8
//  21  PC5 42  XP4.9
//  22  PC6 43  XP4.10
//  50  PG2 44  XP4.11
//  19  PC3 45  XP4.12
//  23  PC7 46  XP4.13
//  20  PC4 47  XP4.14
//  7   PA7 48  XP4.15

//  11  PB3 XP3.1       MISO
//  10  PB2 XP3.4       MOSI
//  9   PB1 XP3.3       SCK
//  8   PB0 --          NSS
//  48  PG0 --

#ifdef __cplusplus
extern "C" {
#endif

#define F_CPU                       12500000UL



////////////////////////////////////////////////////////
// Extensions configuration

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_MAXPORT_NR           11          // Number of used physical digital Ports
#define HAL_DIO_MAPPING     {52, 31, 30, 32, 15, 12, 14, 13, 53, 40, 41, 42, 43, 44, 45,    \
                             53, 39, 38, 34, 37, 33, 36, 35, 0, 1, 2, 3, 4, 5, 6,           \
                             24, 25, 26, 27, 28, 29, 46, 47,                                \
                             18, 17, 49, 16, 21, 22, 50, 19, 23, 20, 7}
// End DIO Section

// End Extensions configuration
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// Start PHY Section

// ENC28J60 Section
#define HAL_USE_SPI                 1
#define ENC_USE_SPI                 1
#define ENC_NSS_PIN                 8  // PB0
#define ENC_SELECT()                (PORTB &= ~(1<<PB0))
#define ENC_RELEASE()               (PORTB |= (1<<PB0))
#define ENC28J60_PHY                1
#include "PHY/ENC28J60/enc28j60_phy.h"
// End ENC28J60 Section

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           50      // Size of identificators list
#define OD_DEV_UC_TYPE              'A'
#define OD_DEV_UC_SUBTYPE           '5'
#define OD_DEV_PHY1                 'E'
#define OD_DEV_PHY2                 'n'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '0'

#define OD_DEV_MAC                  {0x00,0x04,0xA3,0x00,0x01,0x05}   // MAC MSB->LSB
//#define OD_DEF_IP_ADDR              inet_addr(192,168,10,105)
//#define OD_DEF_IP_MASK              inet_addr(255,255,255,0)
//#define OD_DEF_IP_ROUTER            inet_addr(192,168,10,1)
//#define OD_DEF_IP_BROKER            inet_addr(192,168,20,8)

#ifdef __cplusplus
}
#endif

#endif // _A4SN12_H