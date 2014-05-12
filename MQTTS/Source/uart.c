/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// UART Gateway interface

#include "config.h"

#ifdef GATEWAY
#include <stdbool.h>

// Local Variables
// Rx Section
static uint8_t              uartv_RxTail;
volatile static uint8_t     uartv_RxHead;
MQ_t                      * uartv_pRxBuf[UART_RX_QUEUE_SIZE];

// Tx Section
volatile static uint8_t     uartv_TxTail;
static uint8_t              uartv_TxHead;
static uint8_t            * uartv_pTxBuf[UART_TX_QUEUE_SIZE];

///////////////
// USART API //
///////////////

void InitUART(uint16_t baudrate)
{
  USART_CONFIG_PORT();        // Enable USART & Configure Port
  USART_SET_BAUD(baudrate);   // Set Baud
  USART_CONFIGURE();          // Configure

  // Initialize Variables
  uartv_RxTail = 0;
  uartv_RxHead = 0;
  uartv_TxTail = 0;
  uartv_TxHead = 0;
}

MQ_t * uGetBuf(void)
{
  uint8_t tmp_tail;
  if(uartv_RxTail != uartv_RxHead)
  {
    tmp_tail = uartv_RxTail;
    uartv_RxTail++;
    if(uartv_RxTail >= UART_RX_QUEUE_SIZE)
      uartv_RxTail -= UART_RX_QUEUE_SIZE;
    return uartv_pRxBuf[tmp_tail];
  }
  return NULL;
}

void uPutBuf(MQ_t * pBuf)
{
  // Calculate buffer index
  uint8_t tmphead = uartv_TxHead + 1;
  if(tmphead >= UART_TX_QUEUE_SIZE)
    tmphead = 0;
  if(tmphead == uartv_TxTail)                   // Overflow
  {
    mqRelease(pBuf);
    return;
  }
  uartv_pTxBuf[uartv_TxHead] = (uint8_t *)pBuf; // Store pointer in buffer
  uartv_TxHead = tmphead;                       // Store new index
// Enable the USARTx Transmit interrupt
  USART_ENABLE_DREINT();                        // Enable UDRE interrupt
}

ISR(USART_RX_vect)
{
  static MQ_t *   uartv_pRxBufTmp = NULL;
  static uint8_t  uartv_RxPos = 0;
  static uint8_t  uartv_RxLen = 2;
  static bool     uartv_rx_db = false;
  
  // Read one byte from the receive data register
  uint8_t data = USART_DATA;

  if(data == 0xC0)                            // Packet is finished
  {
    if(uartv_RxPos == uartv_RxLen)
    {
      uint8_t tmp_head;
      tmp_head = uartv_RxHead + 1;
      if(tmp_head >= UART_RX_QUEUE_SIZE)
        tmp_head -= UART_RX_QUEUE_SIZE;

      if(tmp_head == uartv_RxTail)            // Overflow
      {
        mqRelease(uartv_pRxBufTmp);
      }
      else
      {
        uartv_pRxBuf[uartv_RxHead] = uartv_pRxBufTmp;
        uartv_RxHead = tmp_head;
      }
    }
    else if(uartv_pRxBufTmp != NULL)          // Bad packet
      mqRelease(uartv_pRxBufTmp);

    uartv_pRxBufTmp = NULL;
    uartv_RxPos = 0;
    uartv_RxLen = 2;
    uartv_rx_db = false;
    return;
  }
  
  if(uartv_RxPos >= uartv_RxLen)
  {
    uartv_RxPos = 0xFF;
    return;
  }

  if(data == 0xDB)
  {
    if(uartv_rx_db)
    {
      uartv_RxPos = 0xFF;
      return;
    }  
    uartv_rx_db = true;
    return;
  }
  
  if(uartv_rx_db)
  {
    uartv_rx_db = false;
    data ^= 0x20;
  }

  switch(uartv_RxPos)
  {
    case 0:     // Get address
      uartv_pRxBufTmp = mqAssert();
      if(uartv_pRxBufTmp == NULL)              // No memory
      {
        uartv_RxPos = 0xFF;
        return;
      }
      uartv_pRxBufTmp->addr = data;
      break;
    case 1:     // Get Length
      if((data > 1) && (data < (MQTTS_MSG_SIZE + 3)))
      {
        uartv_RxLen = data + 1;
        uartv_pRxBufTmp->mq.Length = data;
        break;
      }
      uartv_RxPos = 0xFF;
      return;
    case 2:     // Get Message Type
      uartv_pRxBufTmp->mq.MsgType = data;
      break;
    default:
      uartv_pRxBufTmp->mq.m.raw[uartv_RxPos - 3] = data;
  }
  uartv_RxPos++;
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