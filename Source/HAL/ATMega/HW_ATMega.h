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

// ATMega328P
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
// ATMega2560
#elif   (defined CFG_A4Sn10)
#include "A4xxxx/A4Sn10.h"
// Unknown
#else
#error Unknown configuration
#endif  //  Configuration

#if (defined __AVR_ATmega328P__)
// DIO
#define HAL_DIO_PORTNUM2PORT        {0xFFFF, (uint16_t)&PORTB, (uint16_t)&PORTC, (uint16_t)&PORTD}
// AIn
#define HAL_AIN_BASE2DIO            {16,   17,   18,   19,   20,   21,   0xFE, 0xFE,    /* PC0 - PC5, Ain6, Ain7 */ \
                                     0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFE}    // TempSens, Vbg, GND}
// SPI
#define SPI_DDR                     DDRB
#define SPI_PIN_SS                  PB2
#define SPI_PIN_MOSI                PB3
#define SPI_PIN_MISO                PB4
#define SPI_PIN_SCK                 PB5

#elif defined (__AVR_ATmega2560__)
#define HAL_DIO_PORTNUM2PORT        {(uint16_t)&PORTA, (uint16_t)&PORTB, (uint16_t)&PORTC, (uint16_t)&PORTD,    \
                                     (uint16_t)&PORTE, (uint16_t)&PORTF, (uint16_t)&PORTG, (uint16_t)&PORTH,    \
                                     (uint16_t)&PORTJ, (uint16_t)&PORTK, (uint16_t)&PORTL}
#define HAL_AIN_BASE2DIO            {40, 41, 42, 43, 44, 45, 46, 47,                    /* PF0 - PF7 */ \
                                     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,                    \
                                     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,                    \
                                     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF,    /* Vbg */       \
                                     72, 73, 74, 75, 76, 77, 78, 79}                    /* PK0 - PK7 */
#else
#error HW_ATMega.h unknown uC
#endif  //  uC

#endif  //  HW_ATMEGA_H