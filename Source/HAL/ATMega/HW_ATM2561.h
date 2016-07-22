/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef HW_ATM2561_H
#define HW_ATM2561_H

// ATMega2561
#if   (defined CFG_A5En10)
#include "A5xxxx/A5En10.h"          // Mega2561 + ENC28J60
// Unknown
#else
#error Unknown configuration
#endif  //  Configuration

#define HAL_SIZEOF_UART_RX_FIFO         32      // Should be 2^n

// DIO
#define HAL_DIO_PORTNUM2PORT    {    (uint16_t)&PORTA, (uint16_t)&PORTB, (uint16_t)&PORTC,  \
                                     (uint16_t)&PORTD, (uint16_t)&PORTE, (uint16_t)&PORTF,  \
                                     (uint16_t)&PORTG                                       \
                                }
// AIN
#define EXTAIN_REF                  0x0F    // Bit0 - Ext, Bit1 - Vcc, Bit2 - Int1, Bit3 - Int2
#define HAL_AIN_APIN2DIO            {40, 41, 42, 43, 44, 45, 46, 47}              /* PF0 - PF7 */

// TWI
#define TWIM_SCL_STAT()             (PIND & (1<<PD0))
#define I2C_DIO_SCL                 24
#define I2C_DIO_SDA                 25
// UART
#define UART0_PORT                  PORTE
#define UART0_DDR                   DDRE
#define UART0_RX_PIN                PE0
#define UART0_TX_PIN                PE1

#define UART1_PORT                  PORTD
#define UART1_DDR                   DDRD
#define UART1_RX_PIN                PD2
#define UART1_TX_PIN                PD3

// SPI
#define SPI_DDR                     DDRB
#define SPI_PIN_SS                  PB0
#define SPI_PIN_MOSI                PB2
#define SPI_PIN_MISO                PB3
#define SPI_PIN_SCK                 PB1



#endif  //  HW_ATM2561_H
