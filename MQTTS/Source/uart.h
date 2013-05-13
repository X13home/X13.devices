/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

#ifndef _UART_H
#define _UART_H

void InitUART(uint16_t baudrate);
uint8_t * uGetBuf(uint8_t * pAddr);
void uPutBuf(uint8_t *pBuf, uint8_t * pAddr);

#endif
