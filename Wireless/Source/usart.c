/*
Copyright (c) 2011-2012 <comparator@gmx.de>

This file is part of the X13.Home project.
https://github.com/X13home

BSD License
See LICENSE.txt file for license details.
*/

#include "config.h"

#ifdef GATEWAY

#include "usart.h"

volatile static uint8_t usartv_RxTail;
volatile static uint8_t usartv_RxHead;
volatile static uint8_t usartv_RxBuf[USART_RX_BUFFER_SIZE];
volatile static uint8_t usartv_TxTail;
volatile static uint8_t usartv_TxHead;
volatile static uint8_t usartv_TxBuf[USART_TX_BUFFER_SIZE];

// Interrupt handlers
ISR(USART_RX_vect)
{
    uint8_t data = USART_DATA;
    uint8_t tmphead = usartv_RxHead + 1;
    if(tmphead >= USART_RX_BUFFER_SIZE)
        tmphead -= USART_RX_BUFFER_SIZE;
    if (tmphead == usartv_RxTail)         // Buffer Overflow
        return;

    usartv_RxBuf[usartv_RxHead] = data;   // Store received data in buffer
    usartv_RxHead = tmphead;             // Store new index
}

ISR(USART_UDRE_vect)
{
    // Check if all data is transmitted
    if(usartv_TxTail != usartv_TxHead)
    {
        USART_DATA = usartv_TxBuf[usartv_TxTail];      // Start transmition
        uint8_t tmptail = usartv_TxTail + 1;
        if(tmptail >= USART_TX_BUFFER_SIZE)
            tmptail -= USART_TX_BUFFER_SIZE;
        usartv_TxTail = tmptail;
    }
    else
        USART_DISABLE_DREINT();         // Disable UDRE interrupt
}

void InitUSART(uint16_t baudrate)
{
    USART_CONFIG_PORT();        // Enable USART & Configure Port
    USART_SET_BAUD(baudrate);   // Set Baud
    USART_CONFIGURE();          // Configure
     // Flush buffers
    usartv_RxTail = 0;
    usartv_RxHead = 0;
    usartv_TxTail = 0;
    usartv_TxHead = 0;   
}

void uPutChar(uint8_t data)
{
    // Calculate buffer index
    uint8_t tmphead = usartv_TxHead + 1;
    if(tmphead >= USART_TX_BUFFER_SIZE)
        tmphead -= USART_TX_BUFFER_SIZE;
    while (tmphead == usartv_TxTail);    // Wait for fr_ee space in buffer
    usartv_TxBuf[usartv_TxHead] = data;   // Store data in buffer
    usartv_TxHead = tmphead;             // Store new index
    USART_ENABLE_DREINT();              // Enable UDRE interrupt
}

uint8_t uDataReady(void)
{
    return (usartv_RxTail != usartv_RxHead);
}

uint8_t uGetChar(void)
{
    while(usartv_RxTail == usartv_RxHead);
    uint8_t data = usartv_RxBuf[usartv_RxTail];
    uint8_t tmptail = usartv_RxTail + 1;
    if(tmptail >= USART_RX_BUFFER_SIZE)
        tmptail -= USART_RX_BUFFER_SIZE;
    usartv_RxTail = tmptail;
    return data;
}

#endif  //  GATEWAY
