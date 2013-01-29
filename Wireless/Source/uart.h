/*
Copyright (c) 2011-2012 <comparator@gmx.de>

This file is part of the X13.Home project.
https://github.com/X13home

BSD License
See LICENSE.txt file for license details.
*/

#ifndef _UART_H
#define _UART_H

void InitUART(uint16_t baudrate);
uint8_t * uGetBuf(void);
void uPutBuf(uint8_t *pBuf);

#endif
