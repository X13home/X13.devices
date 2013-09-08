/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

#include "config.h"

#ifdef GATEWAY

static uint8_t *            uartv_pRxBufTmp;
static uint8_t *            uattv_pRxBuf;

volatile static uint8_t     uartv_TxTail;
static uint8_t              uartv_TxHead;

volatile static uint8_t     uartv_TxLen;
static uint8_t *            uartv_pTxBuf[UART_TX_QUEUE_SIZE];

volatile static uint8_t     uartv_Flags;

#define UART_FL_RX_DB       1
#define UART_FL_TX_DB       2
#define UART_FL_RX_RDY      4

// Interrupt handlers
ISR(USART_RX_vect)
{
    uint8_t data = USART_DATA;
    
    static uint8_t uartv_RxPos = 0;
    static uint8_t uartv_RxLen = 2;
    static uint8_t uartv_RxAddr = 0;

    if(data == 0xC0)    // Packet is finished
    {
        if(uartv_RxPos == uartv_RxLen)
        {
            uattv_pRxBuf = uartv_pRxBufTmp;
            uartv_Flags |= UART_FL_RX_RDY;
        }
        else if(uartv_RxPos > 2)                    // Bad packet
            mqRelease((MQ_t *)uartv_pRxBufTmp);

        uartv_RxPos = 0;
        uartv_RxLen = 2;
    }
    else if(uartv_RxPos < uartv_RxLen)
    {
        if(data == 0xDB)
        {
            uartv_Flags |= UART_FL_RX_DB;
        }
        else
        {
            if(uartv_Flags & UART_FL_RX_DB)
            {
                uartv_Flags &= ~UART_FL_RX_DB;
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
                        uartv_pRxBufTmp = pBuf;
                        uartv_pRxBufTmp[0] = uartv_RxAddr;
                        uartv_pRxBufTmp[1] = data;
                        uartv_RxPos = 2;
                    }
                }
                else
                    uartv_RxPos = 0xFF;
            }
            else
                uartv_pRxBufTmp[uartv_RxPos++] = data;
        }
    }
}

ISR(USART_UDRE_vect)
{
    static uint8_t uartv_TxPos = 0xFF;
    static uint8_t uartv_TxLen = 0;

    if(uartv_TxPos < uartv_TxLen)
    {
        uint8_t data = uartv_pTxBuf[uartv_TxTail][uartv_TxPos];
        
        if(uartv_Flags & UART_FL_TX_DB)
        {
            uartv_Flags &= ~UART_FL_TX_DB;
            USART_DATA = data ^ 0x20;
            uartv_TxPos++;
        }
        else if((data == 0xC0) || (data == 0xDB))
        { 
            USART_DATA = 0xDB;
            uartv_Flags |= UART_FL_TX_DB;
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

    uartv_TxTail = 0;
    uartv_TxHead = 0;

    uartv_Flags = 0;
}

uint8_t * uGetBuf(void)
{
    if(uartv_Flags & UART_FL_RX_RDY)
    {
        uartv_Flags &= ~UART_FL_RX_RDY;
        return uattv_pRxBuf;
    }
    return NULL;
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

#endif  //  GATEWAY