/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

// UART Gateway interface

#include "config.h"

#ifdef GATEWAY
#include <stdbool.h>

// Local Variables

static uint8_t *            uartv_pRxBuf = NULL;

volatile static uint8_t     uartv_TxTail;
static uint8_t              uartv_TxHead;

volatile static uint8_t     uartv_TxLen;
static uint8_t *            uartv_pTxBuf[UART_TX_QUEUE_SIZE];

///////////////
// USART API //
///////////////

void InitUART(uint16_t baudrate)
{
    USART_CONFIG_PORT();        // Enable USART & Configure Port
    USART_SET_BAUD(baudrate);   // Set Baud
    USART_CONFIGURE();          // Configure

    // Initialize Variables
    uartv_TxTail = 0;
    uartv_TxHead = 0;
}

uint8_t * uGetBuf(void)
{
  if(uartv_pRxBuf != NULL)
  {
    uint8_t * rBuf;
    rBuf = uartv_pRxBuf;
    uartv_pRxBuf = NULL;
    return rBuf;
  }
  return NULL;
}

void uPutBuf(uint8_t *pBuf)
{
  // Calculate buffer index
  uint8_t tmphead = uartv_TxHead + 1;
  if(tmphead >= UART_TX_QUEUE_SIZE)
    tmphead = 0;
  if(tmphead == uartv_TxTail)             // Overflow
  {
    mqRelease((MQ_t *)pBuf);
    return;
  }
  uartv_pTxBuf[uartv_TxHead] = pBuf;      // Store pointer in buffer
  uartv_TxHead = tmphead;                 // Store new index
// Enable the USARTx Transmit interrupt
  USART_ENABLE_DREINT();                  // Enable UDRE interrupt
}

ISR(USART_RX_vect)
{
  static uint8_t * uartv_pRxBufTmp;
  static uint8_t uartv_RxPos = 0;
  static uint8_t uartv_RxLen = 2;
  static uint8_t uartv_RxAddr = 0;
  
  static bool uartv_rx_db = false;
  
  // Read one byte from the receive data register
  uint8_t data = USART_DATA;

  if(data == 0xC0)                            // Packet is finished
  {
    if(uartv_RxPos == uartv_RxLen)
    {
      if(uartv_pRxBuf != NULL)
         mqRelease((MQ_t *)uartv_pRxBuf);
    
      uartv_pRxBuf = uartv_pRxBufTmp;
    }
    else if(uartv_RxPos > 2)                  // Bad packet
      mqRelease((MQ_t *)uartv_pRxBufTmp);

    uartv_RxPos = 0;
    uartv_RxLen = 2;
  }
  else if(uartv_RxPos < uartv_RxLen)
  {
    if(data == 0xDB)
    {
      uartv_rx_db = true;
    }
    else
    {
      if(uartv_rx_db)
      {
        uartv_rx_db = false;
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
          pBuf = (uint8_t *)mqAssert();
          if(pBuf == NULL)              // No memory
          {
            uartv_RxPos = 0xFF;
          }
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
  else
    uartv_RxPos = 0xFF;
}

ISR(USART_UDRE_vect)
{
  static uint8_t uartv_TxPos = 0xFF;
  static uint8_t uartv_TxLen = 0;
  static bool uartv_tx_db = false;

start_tx_handler:

  if(uartv_TxPos < uartv_TxLen)
  {
    uint8_t data = uartv_pTxBuf[uartv_TxTail][uartv_TxPos];
    
    if(uartv_tx_db)
    {
      uartv_tx_db = false;
      USART_DATA = data ^ 0x20;
      uartv_TxPos++;
    }
    else if((data == 0xC0) || (data == 0xDB))
    { 
      USART_DATA =  0xDB;
      uartv_tx_db = true;
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
      uartv_TxTail = 0;
  }
  else
  {
    if(uartv_TxTail == uartv_TxHead)    // Check if all data is transmitted
    {
      // Disable the USARTx Transmit interrupt
      USART_DISABLE_DREINT();         // Disable UDRE interrupt
      uartv_TxLen = 0;
    }
    else
    {
      uartv_TxPos = 0;
      uartv_TxLen = uartv_pTxBuf[uartv_TxTail][1] + 1;
      goto start_tx_handler;
    }
  }
}

#endif  //  GATEWAY