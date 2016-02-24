/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// UART interface

#include "../../config.h"

#ifdef UART_PHY

#if (UART_PHY == 1)
#define UART_ADDR_t             PHY1_ADDR_t
#define UART_NODE_ID            PHY1_NodeId

#elif (UART_PHY == 2)
#define UART_ADDR_t             PHY2_ADDR_t
#define UART_NODE_ID            PHY2_NodeId

#endif  //  UART_PHY

static Queue_t      uart_tx_queue = {NULL, NULL, 4, 0};
static UART_ADDR_t  uart_addr;

static void uart_tx_task(void)
{
    static MQ_t   * pTx_buf = NULL;

    if(hal_uart_free(UART_PHY_PORT))
    {
        if(pTx_buf != NULL)
        {
            mqFree(pTx_buf);
            pTx_buf = NULL;
        }

        if(uart_tx_queue.Size != 0)
        {
            pTx_buf = mqDequeue(&uart_tx_queue);
            // Paranoid check
            if(pTx_buf != NULL)
            {
                hal_uart_send(UART_PHY_PORT, (pTx_buf->Length + 1), &pTx_buf->Length);
#ifdef LED_On
                LED_On();
#endif  //  LED_On

            }
        }
    }
}

void UART_Init(void)
{
    MQ_t * pBuf;
    while((pBuf = mqDequeue(&uart_tx_queue)) != NULL)
        mqFree(pBuf);

    uint8_t Len = sizeof(UART_ADDR_t);
    ReadOD(UART_NODE_ID, MQTTSN_FL_TOPICID_PREDEF, &Len, (uint8_t *)&uart_addr);

    hal_uart_init_hw(UART_PHY_PORT, 4, 3);      // init uart 38400, Rx + Tx
}

void UART_Send(void *pBuf)
{
    if(!mqEnqueue(&uart_tx_queue, pBuf))
        mqFree(pBuf);
    else
        uart_tx_task();
}

void * UART_Get(void)
{
    uart_tx_task();
    
    // Rx Task
    static uint8_t  rx_pos = 0;
    static uint8_t  rx_len = 0;
    static MQ_t   * pRx_buf;
    static uint16_t rx_wd = 0;

    while(hal_uart_datardy(UART_PHY_PORT))
    {
        uint8_t data = hal_uart_get(UART_PHY_PORT);

        rx_wd = (HAL_get_ms() & 0xFFFF);

        if(rx_len == 0)
        {
            if((data >= 2) && (data < sizeof(MQTTSN_MESSAGE_t)))
            {
                pRx_buf = mqAlloc(sizeof(MQ_t));
                rx_len = data;
                rx_pos = 0;
            }
        }
        else
        {
            if(rx_pos < sizeof(MQTTSN_MESSAGE_t))
                pRx_buf->m.raw[rx_pos++] = data;

            if(rx_pos == rx_len)
            {
                memcpy(pRx_buf->a.phy1addr, (const void *)&uart_addr, sizeof(UART_ADDR_t));
                pRx_buf->Length = rx_len;
                rx_len = 0;
#ifdef LED_On
                LED_On();
#endif  //  LED_On
                return pRx_buf;
            }
        }
    }

    if((rx_len != 0) && (((HAL_get_ms() & 0xFFFF) - rx_wd) > 100))
    {
        rx_len = 0;
        mqFree(pRx_buf);
    }

    return NULL;
}

void * UART_GetAddr(void)
{
    return &uart_addr;
}

#endif  //  UART_PHY
