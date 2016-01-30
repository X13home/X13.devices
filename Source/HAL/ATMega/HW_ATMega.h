/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef HW_ATMEGA_H
#define HW_ATMEGA_H

#if     (defined CFG_A1Cn12)        // CC1101
#include "A1xxxx/A1Cn12.h"
#elif   (defined CFG_A1SC12)        // UART + CC1101
#include "A1xxxx/A1SC12.h"
#elif   (defined CFG_A1En12)        // ENC28J60
#include "A1xxxx/A1En12.h"
#elif   (defined CFG_A1Sn12)        // UART
#include "A1xxxx/A1Sn12.h"
#elif   (defined CFG_A1Sn10)        // Arduino Nano + UART
#include "A1xxxx/A1Sn10.h"
#elif   (defined CFG_A1SR11)        // RFM12 vers. 1.1
#include "A1xxxx/A1SR11.h"
#elif   (defined CFG_A1Rn11)        // RFM12 vers. 1.1, node
#include "A1xxxx/A1Rn11.h"
#else
#error Unknown configuration
#endif  //  Configuration

#if (defined __AVR_ATmega328P__)
#define HAL_DIO_PORTNUM2PORT        {0xFFFF, (uint16_t)&PORTB, (uint16_t)&PORTC, (uint16_t)&PORTD}
#define HAL_AIN_BASE2DIO            {16,   17,   18,   19,   20,   21,   0xFE, 0xFE,    /* PC0 - PC5, Ain6, Ain7 */ \
                                     0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFE}    // TempSens, Vbg, GND}
#endif  //  __AVR_ATmega328P__

#endif  //  HW_ATMEGA_H