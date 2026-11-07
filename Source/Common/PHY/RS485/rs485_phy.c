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
// [SOF1] [SOF2] [LEN] [DST] [SRC] [DATA ... ] [CRC]
// Len = sizeof(Data) + 2
// CRC = Summ(Len /-/ Data)

#define RS485_SOF1          0xF0        // Start of Frame
#define RS485_SOF2          0x1C

// Timeouts and Time Constant
// Tick Periode = 50 uS
#define RX_FRAME_WD         4           // timeout = 100uS

#ifndef LED_On
#define LED_On()
#endif  //  LED_On

typedef enum
{
    RS_STATE_INIT   = 0,
    RS_STATE_RX_IDLE,
    RS_STATE_RX_SOF2,
    RS_STATE_RX_HDR_LEN,
    RS_STATE_RX_HDR_DST,
    RS_STATE_RX_HDR_SRC,
    RS_STATE_RX_DATA,

    RS_STATE_BUS_BUSY,

    RS_STATE_TX
}RS485_STATES_t;

static Queue_t              rs_tx_queue = {NULL, NULL, 4, 0};
static uint8_t              rs_addr = 0xFF;
static RS485_STATES_t       rs_state = RS_STATE_INIT;

void RS485_Init(void)
{
    MQ_t * pBuf;
    while((pBuf = mqDequeue(&rs_tx_queue)) != NULL)
    {
        mqFree(pBuf);
    }

    uint8_t Len = 1;
    ReadOD(objRFNodeId, MQTTSN_FL_TOPICID_PREDEF, &Len, &rs_addr);

    if(rs_state == RS_STATE_INIT)                               // Init Hardware
    {
        hal_uart_init_hw(RS485_PHY_PORT, UART_BAUD_200K, 7);    // init uart 200000, DE + Rx + Tx
    }
    rs_state = RS_STATE_RX_IDLE;
}

void * RS485_GetAddr(void)
{
    return &rs_addr;
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
    static uint8_t rx_Pos, rx_Len, rx_CRC;
    static MQ_t * pRx_buf;
    static uint16_t rs_rx_wd;

    // Tx Variables
    static uint8_t *pTx_buf;

    while(hal_uart_datardy(RS485_PHY_PORT))
    {
        uint8_t ch = hal_uart_get(RS485_PHY_PORT);

        switch(rs_state)
        {
            case RS_STATE_RX_IDLE:
                rs_rx_wd = HAL_get_submstick();
                if(ch == RS485_SOF1)
                {
                    rs_state = RS_STATE_RX_SOF2;
                }
                else
                {
                    rs_state = RS_STATE_BUS_BUSY;
                }
                break;

            case RS_STATE_RX_SOF2:
                if(ch == RS485_SOF2)
                {
                    rs_state = RS_STATE_RX_HDR_LEN;
                }
                else
                {
                    rs_state = RS_STATE_BUS_BUSY;
                }
                break;

            case RS_STATE_RX_HDR_LEN:
                if((ch >= 4) && (ch < (sizeof(MQ_t) + 2)))
                {
                    rx_Len = ch - 2;
                    rx_CRC = ch;
                    rs_rx_wd += ch;
                    rs_state = RS_STATE_RX_HDR_DST;
                }
                else                                                // Bad Length, Arbitration Lost
                {
                    rs_state = RS_STATE_BUS_BUSY;
                }
                break;

            case RS_STATE_RX_HDR_DST:
                if((ch == rs_addr) || (ch == 0))                    // Own or Broadcast
                {
                    LED_On();

                    rx_CRC += ch;
                    rs_state = RS_STATE_RX_HDR_SRC;
                }
                else
                {
                    rs_state = RS_STATE_BUS_BUSY;
                }
                break;

            case RS_STATE_RX_HDR_SRC:
                pRx_buf = mqAlloc(sizeof(MQ_t));
                pRx_buf->a.phy1addr[0] = ch;
                rx_CRC += ch;
                rx_Pos = 0;
                rs_state = RS_STATE_RX_DATA;
                break;

            case RS_STATE_RX_DATA:
                if(rx_Pos < rx_Len)
                {
                    pRx_buf->m.raw[rx_Pos++] = ch;
                    rx_CRC += ch;
                }
                else                                                    // CRC
                {
                    rs_state = RS_STATE_RX_IDLE;
                    if(ch == rx_CRC)
                    {
                        pRx_buf->Length = rx_Len;
                        return pRx_buf;
                    }

                    mqFree(pRx_buf);
                }
                break;

            case RS_STATE_BUS_BUSY:
                rs_rx_wd = HAL_get_submstick();
                break;

            default:
                break;
        }
    }

    if(rs_state == RS_STATE_RX_IDLE)
    {
        if((rs_tx_queue.Size != 0) && hal_uart_free(RS485_PHY_PORT))
        {
            MQ_t * pBuf = mqDequeue(&rs_tx_queue);
            if(pBuf != NULL)
            {
                LED_On();

                pTx_buf = (uint8_t *)pBuf;
                uint8_t  tx_Len = pBuf->Length;
                uint8_t  tx_Dst = pBuf->a.phy1addr[0];
                uint8_t  tx_CRC, in_Pos, out_Pos = 5;

                pTx_buf[0] = RS485_SOF1;                    // Start of Frame
                pTx_buf[1] = RS485_SOF2;

                tx_CRC     = tx_Len + 2;
                pTx_buf[2] = tx_CRC;                        // Lenght
                pTx_buf[3] = tx_Dst;                        // Destination address
                tx_CRC    += tx_Dst;
                pTx_buf[4] = rs_addr;                       // Source address
                tx_CRC    += rs_addr;

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
        }
    }
    else if(rs_state == RS_STATE_TX)
    {
        if(hal_uart_free(RS485_PHY_PORT))
        {
            mqFree(pTx_buf);
            rs_state = RS_STATE_RX_IDLE;
        }
    }
    else if(rs_state == RS_STATE_BUS_BUSY)
    {
        uint16_t diff = HAL_get_submstick() - rs_rx_wd;
        if(diff > RX_FRAME_WD)
        {
            //if(rs_state == RS_STATE_RX_DATA)
            //{
            //    mqFree(pRx_buf);
            //}
            rs_state = RS_STATE_RX_IDLE;
        }
    }

    return NULL;
}

#endif  //  UART_PHY

