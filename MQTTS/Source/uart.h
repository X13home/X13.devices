/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _UART_H
#define _UART_H

void InitUART(uint16_t baudrate);
uint8_t * uGetBuf(void);
void uPutBuf(uint8_t *pBuf);

#endif
