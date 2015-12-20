/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef HW_COMMON_H
#define HW_COMMON_H

#ifdef __cplusplus
{
#endif  //  __cplusplus

//////////////////////////////////////////////////////////////
// general HAL procedures
void StartSheduler(void);
void SystemTick(void);

//uint16_t hal_get_ms(void);
uint32_t hal_get_sec(void);
uint16_t halRNG(void);
//void hal_ASleep(uint16_t duration);
void hal_reboot(void);
// general HAL procedures
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// DIO HAL
void hal_dio_base2hw(uint16_t base, uint8_t *pPort, DIO_PORT_TYPE *pMask);
void hal_dio_configure(uint8_t PortNr, DIO_PORT_TYPE Mask, uint16_t Mode);
DIO_PORT_TYPE hal_dio_read(uint8_t PortNr);
void hal_dio_set(uint8_t PortNr, DIO_PORT_TYPE Mask);
void hal_dio_reset(uint8_t PortNr, DIO_PORT_TYPE Mask);
// DIO HAL
//////////////////////////////////////////////////////////////


/*
//////////////////////////////////////////////////////////////
// PWM HAL
uint8_t hal_pwm_base2dio(uint16_t base);
void hal_pwm_configure(uint16_t base);
void hal_pwm_delete(uint16_t base);
void hal_pwm_write(uint16_t base, uint16_t value);
// PWM HAL
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// AIn HAL
uint8_t hal_ain_apin2dio(uint8_t apin);
void hal_ain_configure(uint8_t apin, uint8_t aref);
void hal_ain_select(uint8_t apin, uint8_t aref);
int16_t hal_ain_get(void);
// AIn HAL
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// UART HAL Section
void hal_uart_get_pins(uint8_t port, uint8_t * pRx, uint8_t * pTx);
void hal_uart_deinit(uint8_t port);
void hal_uart_init_hw(uint8_t port, uint8_t nBaud, uint8_t enable);
bool hal_uart_free(uint8_t port);
void hal_uart_send(uint8_t port, uint8_t len, uint8_t * pBuf);
bool hal_uart_datardy(uint8_t port);
uint8_t hal_uart_get(uint8_t port);
// UART HAL Section
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// TWI/I2C Section
void hal_twi_get_pins(uint8_t * pSCL, uint8_t * pSDA);
bool hal_twi_configure(uint8_t enable);
void hal_twi_stop(void);
void hal_twi_start(void);
// TWI/I2C Section
//////////////////////////////////////////////////////////////
*/

#ifdef __cplusplus
}
#endif  //  __cplusplus

#endif  //  HW_COMMON_H