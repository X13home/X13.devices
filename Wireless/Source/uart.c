/*
Copyright (c) 2011-2012 <comparator@gmx.de>

This file is part of the X13.Home project.
https://github.com/X13home

BSD License
See LICENSE.txt file for license details.
*/

#include "config.h"

static uint8_t     uartv_RxTail;
volatile static uint8_t     uartv_RxHead;
volatile static uint8_t     uartv_RxPos;
volatile static uint8_t     uartv_RxLen;
volatile static uint8_t     uartv_RxAddr;
volatile static uint8_t *   uartv_pRxBuf[UART_RX_QUEUE_SIZE];

volatile static uint8_t     uartv_TxTail;
volatile static uint8_t     uartv_TxHead;
volatile static uint8_t     uartv_TxPos;
volatile static uint8_t     uartv_TxLen;
volatile static uint8_t *   uartv_pTxBuf[UART_TX_QUEUE_SIZE];

volatile static uint8_t     uartv_Flags;

// Interrupt handlers
ISR(USART_RX_vect)
{
    uint8_t data = USART_DATA;
    
    if(data == 0xC0)    // Packet is finished
    {
        if(uartv_RxPos == uartv_RxLen)
        {
            uint8_t tmphead = uartv_RxHead + 1;
            if(tmphead >= UART_RX_QUEUE_SIZE)
                tmphead -= UART_RX_QUEUE_SIZE;
            if(tmphead != uartv_RxTail)
                uartv_RxHead = tmphead;
            else                                    // Overflow
                mqRelease((MQ_t *)uartv_pRxBuf[uartv_RxHead]);
        }
        else if(uartv_RxPos > 2)                    // Bad packet
            mqRelease((MQ_t *)uartv_pRxBuf[uartv_RxHead]);

        uartv_RxPos = 0;
        uartv_RxLen = 2;
    }
    else if(uartv_RxPos < uartv_RxLen)
    {
        if(data == 0xDB)
        {
            uartv_Flags |= 1;
        }
        else
        {
            if(uartv_Flags & 1)
            {
                uartv_Flags &= ~1;
                data ^= 0x20;
            }
            
            if(uartv_RxPos == 0)    // Get address
            {
                uartv_RxAddr = data;
                uartv_RxPos = 1;
            }
            else if(uartv_RxPos == 1)
            {
                if((data > 1) && (data < (MQTTS_MSG_SIZE + 3)))
                {
                    uartv_RxLen = data + 1;
                    uint8_t * pBuf;
                    if((pBuf = (uint8_t *)mqAssert()) == NULL)          // No memory
                        uartv_RxPos = 0xFF;
                    else
                    {
                        uartv_pRxBuf[uartv_RxHead] = pBuf;
                        uartv_pRxBuf[uartv_RxHead][0] = uartv_RxAddr;
                        uartv_pRxBuf[uartv_RxHead][1] = data;
                        uartv_RxPos = 2;
                    }
                }
                else
                    uartv_RxPos = 0xFF;
            }
            else
                uartv_pRxBuf[uartv_RxHead][uartv_RxPos++] = data;
        }
    }
}

ISR(USART_UDRE_vect)
{
    if(uartv_TxPos < uartv_TxLen)
    {
        uint8_t data = uartv_pTxBuf[uartv_TxTail][uartv_TxPos];
        
        if(uartv_Flags & 2)
        {
            uartv_Flags &= ~2;
            USART_DATA = data ^ 0x20;
            uartv_TxPos++;
        }
        else if((data == 0xC0) || (data == 0xDB))
        { 
            USART_DATA = 0xDB;
            uartv_Flags |= 2;
        }
        else
        {
            USART_DATA = data;
            uartv_TxPos++;
        }
    }
    else if(uartv_TxPos == uartv_TxLen)
    {
        USART_DATA = 0xC0;
        uartv_TxPos = 0xFF;
        uartv_TxLen = 0;
        mqRelease((MQ_t *)uartv_pTxBuf[uartv_TxTail]);
        if(++uartv_TxTail >= UART_TX_QUEUE_SIZE)
            uartv_TxTail -= UART_TX_QUEUE_SIZE;
    }
    else
    {
        if(uartv_TxTail == uartv_TxHead)    // Check if all data is transmitted
        {
            USART_DISABLE_DREINT();         // Disable UDRE interrupt
            uartv_TxLen = 0;
        }
        else
        {
            uartv_TxPos = 1;
            uartv_TxLen = uartv_pTxBuf[uartv_TxTail][1] + 1;
            USART_DATA = uartv_pTxBuf[uartv_TxTail][0];
        }
    }
}

void InitUART(uint16_t baudrate)
{
    USART_CONFIG_PORT();        // Enable USART & Configure Port
    USART_SET_BAUD(baudrate);   // Set Baud
    USART_CONFIGURE();          // Configure

    uartv_RxTail = 0;
    uartv_RxHead = 0;
    uartv_RxPos = 0;
    uartv_RxLen = 2;
    
    uartv_TxTail = 0;
    uartv_TxHead = 0;
    uartv_TxPos = 0xFF;
    uartv_TxLen = 0;

    uartv_Flags = 0;
}

uint8_t * uGetBuf(void)
{
    uint8_t * pRetVal = NULL;
    if(uartv_RxTail != uartv_RxHead)
    {
        pRetVal = (uint8_t *)uartv_pRxBuf[uartv_RxTail];

        uartv_RxTail++;
        if(uartv_RxTail >= UART_RX_QUEUE_SIZE)
            uartv_RxTail -= UART_RX_QUEUE_SIZE;
    }
    return pRetVal;
}

void uPutBuf(uint8_t *pBuf)
{
    // Calculate buffer index
    uint8_t tmphead = uartv_TxHead + 1;
    if(tmphead >= UART_TX_QUEUE_SIZE)
        tmphead -= UART_TX_QUEUE_SIZE;
    if(tmphead == uartv_TxTail)             // Overflow
    {
        mqRelease((MQ_t *)pBuf);
        return;
    }
    uartv_pTxBuf[uartv_TxHead] = pBuf;      // Store pointer in buffer
    uartv_TxHead = tmphead;                 // Store new index
    USART_ENABLE_DREINT();                  // Enable UDRE interrupt
}
