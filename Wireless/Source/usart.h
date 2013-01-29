/*
Copyright (c) 2011-2012 <comparator@gmx.de>

This file is part of the X13.Home project.
https://github.com/X13home

BSD License
See LICENSE.txt file for license details.
*/

#ifndef _USART_H
#define _USART_H

#define USART_RX_BUFFER_SIZE    32
#define USART_TX_BUFFER_SIZE    32

void InitUSART(uint16_t baudrate);
void uPutChar(uint8_t data);
uint8_t uDataReady(void);
uint8_t uGetChar(void);

#endif
