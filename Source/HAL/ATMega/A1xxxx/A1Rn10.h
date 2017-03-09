/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _A1SR10_H
#define _A1SR10_H

// Board:   JeeNode
// uc:      ATMega328p
// Phy1:    RFM12

// 08   PB0     ISP-7
// 09   PB1     ISP-8
// 10   PB2                 RFM12_CSN
// 11   PB3     ISP-4       RFM12_MOSI
// 12   PB4     ISP-1       RFM12_MISO
// 13   PB5     ISP-3       RFM12_SCK
// 14   PB6                 OSC
// 15   PB7                 OSC
// PORT C
// 16   PC0     P1.5        Ain0
// 17   PC1     P2.5        Ain1
// 18   PC2     P3.5        Ain2
// 19   PC3     P4.5        Ain3
// 20   PC4     S.5         SDA
// 21   PC5     S.6         SCL
// 22   PC6     ISP-5       RESET

// PORT D
// 24   PD0
// 25   PD1
// 26   PD2                 RFM12_IRQ
// 27   PD3     Px.6        IRQ
// 28   PD4     P1.2
// 29   PD5     P2.2
// 30   PD6     P2.3
// 31   PD7     P2.3

#ifdef __cplusplus
extern "C" {
#endif

#define F_CPU                       16000000UL

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_PORT_OFFSET          1
#define EXTDIO_MAXPORT_NR           2         // Number of used physical digital Ports
#define HAL_DIO_MAPPING             {16, 17, 18, 19, 28, 29, 30, 31}
// End DIO Section

// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           4                                   // ADC0-ADC3
#define HAL_AIN_BASE2APIN           {0, 1, 2, 3}
// End Analogue Inputs

// RF Section
#define RFM12_PORT                  PORTB
#define RFM12_DDR                   DDRB
#define RFM12_PIN_SS                PB2
#define RFM12_PIN_MOSI              PB3
#define RFM12_PIN_MISO              PB4
#define RFM12_PIN_SCK               PB5
#define RFM12_IRQ_PORT              PORTD
#define RFM12_IRQ_PORT_PIN          PIND
#define RFM12_IRQ_PIN               PD2
#define RFM12_INT0                  1
#define RFM12_PHY                   1
#include "PHY/RFM12/rfm12_phy.h"
//  End RF Section

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           21      // Size of identificators list,
                                            // PC0-PC5 + ADC6/7 + Vbg + PD2-PD7 + PB0-PB4
#define OD_DEV_UC_TYPE              'A'
#define OD_DEV_UC_SUBTYPE           '1'
#define OD_DEV_PHY1                 'R'
#define OD_DEV_PHY2                 'n'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '0'

#ifdef __cplusplus
}
#endif

#endif // _A1SR10_H
