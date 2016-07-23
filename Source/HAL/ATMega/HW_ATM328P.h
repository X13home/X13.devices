/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef HW_ATM328P_H
#define HW_ATM328P_H

// ATMega328P
#if     (defined CFG_A1Cn12)        // CC1101
#include "A1xxxx/A1Cn12.h"
#elif   (defined CFG_A1SC12)        // UART + CC1101
#include "A1xxxx/A1SC12.h"
#elif   (defined CFG_A1En12)        // ENC28J60
#include "A1xxxx/A1En12.h"
#elif   (defined CFG_A1Sn12)        // UART
#include "A1xxxx/A1Sn12.h"
#elif   (defined CFG_A1SR11)        // RFM12 vers. 1.1
#include "A1xxxx/A1SR11.h"
#elif   (defined CFG_A1Rn11)        // RFM12 vers. 1.1, node
#include "A1xxxx/A1Rn11.h"
// Arduino Nano/Uno ATMega328P + OSC 16MHz
#elif   (defined CFG_A1Sn10)        // Arduino Nano/Uno + UART
#include "A1xxxx/A1Sn10.h"
#elif   (defined CFG_A1SC10)        // Arduino Nano/Uno + UART + CC1101
#include "A1xxxx/A1SC10.h"
#elif   (defined CFG_A1Cn10)        // Arduino Nano/Uno + CC1101
#include "A1xxxx/A1Cn10.h"
// Unknown
#else
#error Unknown configuration
#endif  //  Configuration

#define HAL_SIZEOF_UART_RX_FIFO         32      // Should be 2^n

// DIO
#define HAL_DIO_PORTNUM2PORT        {0xFFFF, (uint16_t)&PORTB, (uint16_t)&PORTC, (uint16_t)&PORTD}
// AIn
#define EXTAIN_REF                  0x06    // Bit0 - Ext, Bit1 - Vcc, Bit2 - Int1, Bit3 - Int2
#define HAL_AIN_APIN2DIO        {   /* PC0 - PC5, Ain6, Ain7 */                         \
                                        16,   17,   18,   19,   20,   21,   0xFE, 0xFE, \
                                    /* TempSens, Vbg, GND   */                          \
                                     0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFE     \
                                }
// TWI
#define TWIM_SCL_STAT()             (PINC & (1<<PC5))
#define I2C_DIO_SDA                 20
#define I2C_DIO_SCL                 21
// UART
#define USART0_RX_vect              USART_RX_vect
#define USART0_UDRE_vect            USART_UDRE_vect
#define UART0_PORT                  PORTD
#define UART0_DDR                   DDRD
#define UART0_RX_PIN                PD0
#define UART0_TX_PIN                PD1
// SPI
#define SPI_DDR                     DDRB
#define SPI_PIN_SS                  PB2
#define SPI_PIN_MOSI                PB3
#define SPI_PIN_MISO                PB4
#define SPI_PIN_SCK                 PB5

#endif  //  HW_ATM328P_H