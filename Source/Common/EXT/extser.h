/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _EXTSER_H
#define _EXTSER_H

#ifdef __cplusplus
extern "C" {
#endif

// UART HAL Section
void    hal_uart_get_pins(uint8_t port, uint8_t * pRx, uint8_t * pTx);
void    hal_uart_deinit(uint8_t port);
void    hal_uart_init_hw(uint8_t port, uint8_t nBaud, uint8_t enable);
bool    hal_uart_free(uint8_t port);
void    hal_uart_send(uint8_t port, uint8_t len, uint8_t * pBuf);
bool    hal_uart_datardy(uint8_t port);
uint8_t hal_uart_get(uint8_t port);

// UART API
void    serInit(void);
bool    serCheckSubidx(subidx_t * pSubidx);
e_MQTTSN_RETURNS_t serRegisterOD(indextable_t *pIdx);
void    serDeleteOD(subidx_t * pSubidx);
void    serProc(void);

#ifdef __cplusplus
}
#endif

#endif  //  _EXTSER_H
