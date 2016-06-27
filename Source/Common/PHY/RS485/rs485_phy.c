/*
Copyright (c) 2011-2016 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// RS485 interface

#include "../../config.h"

#ifdef RS485_PHY

// Frame Format
// Variable Length Data
// [SOF] [LEN] [SRC] [DST] [DATA ... ] [CRC]
// Len = sizeof(Data) + 2
// CRC = Summ(Len /-/ Data)
// Synchro Frame
// [SOF] [0x01] [SRC]

#define RS485_SOF           0x68        // Start of Frame

#ifndef LED_On
#define LED_On()
#endif  //  LED_On

typedef enum
{
    RS_STATE_INIT   = 0,
    RS_STATE_ARB_LOST,
    RS_STATE_RX_IDLE,
    RS_STATE_RX_HDR,
    RS_STATE_RX_DATA,
    RS_STATE_RX_BUSY,

    RS_STATE_TX,
    RS_STATE_TX_SYNC
}RS485_STATES_t;

static Queue_t              rs_tx_queue = {NULL, NULL, 4, 0};
static uint8_t              rs_addr = 0xFF;
static uint8_t              rs_state = RS_STATE_INIT;
static uint8_t              rs_token[] = {RS485_SOF, 1, 0xFF};

void RS485_Init(void)
{
    MQ_t * pBuf;
    while((pBuf = mqDequeue(&rs_tx_queue)) != NULL)
    {
        mqFree(pBuf);
    }

    uint8_t Len = 1;
    ReadOD(objRFNodeId, MQTTSN_FL_TOPICID_PREDEF, &Len, &rs_addr);

    rs_token[2] = rs_addr;

    if(rs_state == RS_STATE_INIT)   // Init Hardware
    {
        hal_uart_init_hw(RS485_PHY_PORT, UART_BAUD_93750, 3);   // init uart 93750, Rx + Tx
        hal_dio_configure(RS485_DE_PIN, DIO_MODE_OUT_PP);
    }
    RS485_DE_DISABLE();
    rs_state = RS_STATE_RX_IDLE;
}

void RS485_Send(void *pBuf)
{
    if(!mqEnqueue(&rs_tx_queue, pBuf))
    {
        mqFree(pBuf);
    }
}

void * RS485_Get(void)
{
    // Rx Variables
    static uint8_t  rx_CRC, rx_Pos, rx_Len;
    static MQ_t   * pRx_buf;
    // Tx Variables
    static uint8_t *pTx_buf;
    // Timeouts
    static uint16_t rx_wd = 0;
    static uint16_t rx_to = 0;
    uint16_t act_ms = HAL_get_ms() & 0xFFFF;

    // Tx Task
    if(rs_state == RS_STATE_RX_IDLE)
    {
        if(((act_ms - rx_wd) >= rx_to) && (hal_uart_free(RS485_PHY_PORT)))
        {
            rx_wd = act_ms;
            rx_to = 135;
            MQ_t * pBuf;

            RS485_DE_ENABLE();
            

            if((rs_tx_queue.Size != 0) && ((pBuf = mqDequeue(&rs_tx_queue)) != NULL))
            {
                pTx_buf = mqAlloc(sizeof(MQ_t));
                uint8_t  tx_Len = pBuf->Length;
                uint8_t  tx_CRC, in_Pos, out_Pos = 4;

                pTx_buf[0] = RS485_SOF;                     // Start of Frame
                pTx_buf[1] = tx_Len + 2;                    // Lenght
                tx_CRC     = tx_Len + 2;
                pTx_buf[2] = rs_addr;                       // Source address
                tx_CRC    += rs_addr;
                pTx_buf[3] = pBuf->a.phy1addr[0];           // Destination address
                tx_CRC    += pBuf->a.phy1addr[0];

                for(in_Pos = 0; in_Pos < tx_Len; in_Pos++)  // Copy Data, calculate CRC
                {
                    uint8_t ch = pBuf->m.raw[in_Pos];
                    tx_CRC += ch;
                    pTx_buf[out_Pos++] = ch;
                }

                pTx_buf[out_Pos++] = tx_CRC;
                rs_state = RS_STATE_TX;
                hal_uart_send(RS485_PHY_PORT, out_Pos, pTx_buf);
            }
            else        // Send Synchro Frame
            {
                rs_state = RS_STATE_TX_SYNC;
                hal_uart_send(RS485_PHY_PORT, sizeof(rs_token), rs_token);
            }
            return NULL;
        }
    }
    else if(rs_state >= RS_STATE_TX)
    {
        if(hal_uart_free(RS485_PHY_PORT))
        {
            RS485_DE_DISABLE();
            if(rs_state == RS_STATE_TX)
            {
                mqFree(pTx_buf);
            }

            rs_state = RS_STATE_RX_IDLE;
        }

        // Drop Received data
        while(hal_uart_datardy(RS485_PHY_PORT))
        {
            hal_uart_get(RS485_PHY_PORT);
        }
        return NULL;
    }

    while(hal_uart_datardy(RS485_PHY_PORT))
    {
        uint8_t ch = hal_uart_get(RS485_PHY_PORT);

        if((rs_state == RS_STATE_RX_IDLE) ||
           (rs_state == RS_STATE_ARB_LOST))
        {
            rx_wd = act_ms;
            rx_to = 8 + (rs_addr >> 1);

            if(ch == RS485_SOF)
            {
                rs_state = RS_STATE_RX_HDR;
                rx_Pos = 0;
            }
            else    // Arbitration Lost
            {
                rs_state = RS_STATE_ARB_LOST;
            }
        }
        else if(rs_state == RS_STATE_RX_HDR)
        {
            if(rx_Pos == 0)                                 // length
            {
                if((ch == 1) ||                             // Synchro Frame
                  ((ch >= 4) && (ch < (sizeof(MQ_t) + 2)))) // Standard Frame
                {
                    rx_Pos = 1;
                    rx_Len = ch;
                    rx_CRC = ch;
                }
                else                                        // Bad Length, Arbitration Lost
                {
                    rs_state = RS_STATE_ARB_LOST;
                }
            }
            else if(rx_Pos == 1)                            // source address
            {
                rx_to = ((rs_addr - ch) >> 1);

                if(rx_Len == 1)                             // Synchro Frame
                {
                    rs_state = RS_STATE_RX_IDLE;
                }
                else
                {
                    rx_to += (rx_Len >> 3);
                    rx_Pos = 2;
                    rx_CRC += ch;
                }
            }
            else                                            // destination address
            {
                if((ch == rs_addr) || (ch == 0))
                {
                    pRx_buf = mqAlloc(sizeof(MQ_t));
                    pRx_buf->a.phy1addr[0] = ch;

                    rs_state = RS_STATE_RX_DATA;
                    rx_Pos = 0;
                    rx_Len -= 2;
                    rx_CRC += ch;
                }
                else
                {
                    rs_state = RS_STATE_RX_BUSY;
                }
            }
        }
        else if(rs_state == RS_STATE_RX_DATA)
        {
            if(rx_Pos < rx_Len)
            {
                pRx_buf->m.raw[rx_Pos++] = ch;
                rx_CRC += ch;
            }
            else                                    // CRC
            {
                rs_state = RS_STATE_RX_IDLE;
                if(ch != rx_CRC)
                {
                    mqFree(pRx_buf);
                    return NULL;
                }
                else
                {
                    return pRx_buf;
                }
            }
        }
        else if(rs_state == RS_STATE_RX_BUSY)
        {
            if(rx_Pos < rx_Len)
            {
                rx_Pos++;
            }
            else
            {
                rs_state = RS_STATE_RX_IDLE;
            }
        }
    }

    if((rs_state != RS_STATE_RX_IDLE) && ((act_ms - rx_wd) >= rx_to))
    {
        if(rs_state == RS_STATE_RX_DATA)
        {
            mqFree(pRx_buf);
        }
        rs_state = RS_STATE_RX_IDLE;
    }

    return NULL;
}


void * RS485_GetAddr(void)
{
    return &rs_addr;
}

#endif  //  UART_PHY

