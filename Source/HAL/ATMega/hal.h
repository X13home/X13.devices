#ifndef __HAL_H
#define __HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <avr/io.h>
#include <avr/eeprom.h>

#ifndef PRR
#define PRR PRR0
#endif  //  PRR

#define ENTER_CRITICAL_SECTION()    asm volatile ( "in      __tmp_reg__, __SREG__" :: );    \
                                    asm volatile ( "cli" :: );                              \
                                    asm volatile ( "push    __tmp_reg__" :: )

#define LEAVE_CRITICAL_SECTION()    asm volatile ( "pop     __tmp_reg__" :: );              \
                                    asm volatile ( "out     __SREG__, __tmp_reg__" :: )

#define eeprom_init_hw()
#define eeprom_read(pBuf, Addr, Len)  eeprom_read_block((void *)pBuf, (const void *)Addr, (size_t)Len)
#define eeprom_write(pBuf, Addr, Len) eeprom_write_block((const void *)pBuf, (void *)Addr, (size_t)Len)

// AVR Architecture specifics.
#define portBYTE_ALIGNMENT          1
#define portPOINTER_SIZE_TYPE       uint16_t
#define configTOTAL_HEAP_SIZE       1024

//////////////////////////////////////////////////////////////
// DIO Section
#define DIO_PORT_POS                3
#define DIO_PORT_MASK               0x07
#define DIO_PORT_TYPE               uint8_t

// DIO Types
typedef enum
{
    DIO_MODE_IN_FLOAT   = 0,
    DIO_MODE_IN_PD      = 0,
    DIO_MODE_IN_PU      = 0x01,

    DIO_MODE_OUT_PP_HS  = 0x08,
    DIO_MODE_OUT_PP     = 0x08,
    
    DIO_MODE_AIN        = 0x18
}DIOmode_e;

uint8_t     hal_dio_base2pin(uint16_t base);
void        hal_dio_configure(uint8_t Pin, uint8_t Mode);
uint8_t     hal_dio_read(uint8_t PortNr);
void        hal_dio_set(uint8_t PortNr, uint8_t Mask);
void        hal_dio_reset(uint8_t PortNr, uint8_t Mask);
// DIO Section
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// AIN Section
uint8_t     hal_ain_apin2dio(uint8_t apin);
void        hal_ain_configure(uint8_t apin, uint8_t aref);
void        hal_ain_select(uint8_t apin, uint8_t aref);
int16_t     hal_ain_get(void);
// AIN Section
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// TWI Section
void        hal_twi_get_pins(uint8_t * pSCL, uint8_t * pSDA);
bool        hal_twi_configure(uint8_t enable);
void        hal_twi_stop(void);
void        hal_twi_start(void);
// TWI Section
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// UART Section
void        hal_uart_get_pins(uint8_t port, uint8_t * pRx, uint8_t * pTx);
void        hal_uart_deinit(uint8_t port);
void        hal_uart_init_hw(uint8_t port, uint8_t nBaud, uint8_t enable);
bool        hal_uart_free(uint8_t port);
void        hal_uart_send(uint8_t port, uint8_t len, uint8_t * pBuf);
bool        hal_uart_datardy(uint8_t port);
uint8_t     hal_uart_get(uint8_t port);
// UART Section
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// SPI Section
#define HAL_SPI_MODE_0              0
#define HAL_SPI_MODE_1              1
#define HAL_SPI_MODE_2              2
#define HAL_SPI_MODE_3              3
#define HAL_SPI_MSB                 0
#define HAL_SPI_LSB                 4
#define HAL_SPI_8B                  0
#define HAL_SPI_16B                 8

void        hal_spi_cfg(uint8_t port, uint8_t mode, uint32_t speed);
uint8_t     hal_spi_exch8(uint8_t port, uint8_t data);
//uint16_t    hal_spi_exch16(uint8_t port, uint16_t data);
// SPI Section
//////////////////////////////////////////////////////////////

#include "HW_ATMega.h"

#ifdef __cplusplus
}
#endif

#endif  //  __HAL_H
