/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

#include "config.h"

// RX Section
static uint8_t  *           uartv_pRxBufTmp = NULL;   // Temporary RX Buffer
static uint8_t              uartv_RxAddrTmp;          // Temporary RX Addr
static uint8_t  *           uattv_pRxBuf;             // Ready RX Data
static uint8_t              uartv_RxAddr;             // Ready Rx Addr
// Tx Section
static uint8_t  *           uartv_pTxBufTmp = NULL;   // Temporary TX Buffer
static uint8_t              uartv_TxAddrTmp;          // Temporary TX Addr
static uint8_t  *           uartv_pTxBuf;             // Work TX Buffer
// Common Data
volatile static uint8_t     uartv_Flags;

#define UART_FL_RX_DB       1
#define UART_FL_TX_DB       2
#define UART_FL_RX_RDY      4

#define UART_FL_RX_ADDR_RDY 0x10

// Interrupt handlers
ISR(USART_RX_vect)
{
  static uint8_t uartv_RxPos = 0;
  static uint8_t uartv_RxLen = 1;

  uint8_t data = USART_DATA;

  if(data == 0xC0)    // Packet is finished
  {
    if(uartv_RxPos == uartv_RxLen)
    {
      uartv_RxAddr = uartv_RxAddrTmp;
      uattv_pRxBuf = uartv_pRxBufTmp;
      uartv_Flags |= UART_FL_RX_RDY;
    }
    else if(uartv_pRxBufTmp)                    // Bad packet
      mqRelease((MQ_t *)uartv_pRxBufTmp);

    uartv_pRxBufTmp = NULL;
    uartv_Flags &= ~UART_FL_RX_ADDR_RDY;
    uartv_RxPos = 0;
    uartv_RxLen = 1;
    return;
  }
  
  if(uartv_RxPos < uartv_RxLen)
  {
    if(data == 0xDB)
    {
      uartv_Flags |= UART_FL_RX_DB;
      return;
    }

    if(uartv_Flags & UART_FL_RX_DB)
    {
      uartv_Flags &= ~UART_FL_RX_DB;
      data ^= 0x20;
    }

    if(uartv_RxPos == 0)
    {
      if(!(uartv_Flags & UART_FL_RX_ADDR_RDY))        // Get address
      {
        uartv_RxAddrTmp = data;
        uartv_Flags |= UART_FL_RX_ADDR_RDY;
      }    
      else if((data > 1) && (data < (MQTTS_MSG_SIZE + 3)))
      {
        MQ_t * pBuf;
        if((pBuf = mqAssert()) == NULL)          // No memory
            uartv_RxPos = 0xFF;
        else
        {
          pBuf->Length = data;
          uartv_pRxBufTmp = (uint8_t *)pBuf;
          uartv_RxLen = data;
          uartv_RxPos = 1;
        }
      }
      else
        uartv_RxPos = 0xFF;
    }
    else
      uartv_pRxBufTmp[uartv_RxPos++] = data;
  }
}

ISR(USART_UDRE_vect)
{
  static uint8_t uartv_TxPos = 0xFF;
  static uint8_t uartv_TxLen = 0;

  if(uartv_TxPos < uartv_TxLen)
  {
    uint8_t data = uartv_pTxBuf[uartv_TxPos];

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
    mqRelease((MQ_t *)uartv_pTxBuf);
  }
  else if(uartv_pTxBufTmp)
  {
    uartv_pTxBuf = uartv_pTxBufTmp;
    uartv_pTxBufTmp = NULL;
    uartv_TxPos = 0;
    uartv_TxLen = uartv_pTxBuf[0];
    USART_DATA = uartv_TxAddrTmp;
  }
  else  // all data is transmitted
  {
    uartv_TxLen = 0;
    USART_DISABLE_DREINT();         // Disable UDRE interrupt
  }
}

void InitUART(uint16_t baudrate)
{
  USART_CONFIG_PORT();        // Enable USART & Configure Port
  USART_SET_BAUD(baudrate);   // Set Baud
  USART_CONFIGURE();          // Configure

  uartv_Flags = 0;
}

uint8_t * uGetBuf(uint8_t * pAddr)
{
  if(uartv_Flags & UART_FL_RX_RDY)
  {
    uartv_Flags &= ~UART_FL_RX_RDY;
    *pAddr = uartv_RxAddr;
    return uattv_pRxBuf;
  }
  return NULL;
}

void uPutBuf(uint8_t *pBuf, uint8_t * pAddr)
{
  if(uartv_pTxBufTmp) // Overflow
  {
    mqRelease((MQ_t *)pBuf);
    return;
  }

  uartv_pTxBufTmp = pBuf;
  uartv_TxAddrTmp = *pAddr;
  USART_ENABLE_DREINT();                  // Enable UDRE interrupt
}
