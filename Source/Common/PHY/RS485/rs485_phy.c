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

#define RS485_SOF           0xC3        // Start of Frame

#ifndef LED_On
#define LED_On()
#endif  //  LED_On

typedef enum
{
    RS_STATE_INIT   = 0,
    RS_STATE_RX_IDLE,
    RS_STATE_RX_HDR,
    RS_STATE_RX_DATA,
    RS_STATE_RX_BUSY,

    RS_STATE_TX
}RS485_STATES_t;

static Queue_t              rs_tx_queue = {NULL, NULL, 4, 0};
static uint8_t              rs_addr = 0xFF;
static RS485_STATES_t       rs_state = RS_STATE_INIT;
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
        hal_uart_init_hw(RS485_PHY_PORT, UART_BAUD_128K, 7);   // init uart 128000, DE + Rx + Tx
    }
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
    static uint8_t  rx_CRC, rx_Pos, rx_Len, rx_Src = 0;
    static MQ_t   * pRx_buf;
    static uint16_t rs_rx_wd = 0;       // Rx Timeout
    // Tx Variables
    static uint8_t *pTx_buf;
    // Tx Timeouts
    static uint16_t rs_tx_wd = 128;     // 32 ms
    static uint16_t rs_tx_to = 0;       // Timeouts, in 0.25ms

    while(hal_uart_datardy(RS485_PHY_PORT))
    {
        uint8_t ch = hal_uart_get(RS485_PHY_PORT);

        if(rs_state == RS_STATE_RX_IDLE)
        {
            if(ch == RS485_SOF)
            {
                rs_rx_wd = HAL_get_250us_tick();
                rs_state = RS_STATE_RX_HDR;
                rx_Pos = 0;
            }
            // else - Arbitration Lost
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
                    rs_state = RS_STATE_RX_IDLE;
                }
            }
            else if(rx_Pos == 1)                            // source address
            {
                rx_Src = rs_addr - ch;

                if(rx_Len == 1)                             // Synchro Frame
                {
                    rs_tx_wd = HAL_get_250us_tick();
                    rs_tx_to = rx_Src;
                    rs_state = RS_STATE_RX_IDLE;
                }
                else
                {
                    rx_Pos = 2;
                    rx_CRC += ch;
                }
            }
            else                                            // destination address
            {
                rx_CRC += ch;

                if((ch == rs_addr) || (ch == 0))
                {
                    LED_On();

                    pRx_buf = mqAlloc(sizeof(MQ_t));
                    pRx_buf->a.phy1addr[0] = ch;

                    rs_state = RS_STATE_RX_DATA;
                    rx_Pos = 0;
                    rx_Len -= 2;
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
                if(ch == rx_CRC)
                {
                    rs_tx_wd = HAL_get_250us_tick();
                    rs_tx_to = rx_Src;
                    pRx_buf->Length = rx_Len;
                    return pRx_buf;
                }

                mqFree(pRx_buf);
            }
        }
        else if(rs_state == RS_STATE_RX_BUSY)
        {
            if(rx_Pos < rx_Len)
            {
                rx_Pos++;
                rx_CRC += ch;
            }
            else
            {
                if(ch == rx_CRC)
                {
                    rs_tx_wd = HAL_get_250us_tick();
                    rs_tx_to = rx_Src;
                }
                rs_state = RS_STATE_RX_IDLE;
            }
        }
    }

    if(rs_state == RS_STATE_RX_IDLE)
    {
        uint16_t act_ticks = HAL_get_250us_tick();
        uint16_t diff = act_ticks - rs_tx_wd;

        // Tx Task
        if(diff > rs_tx_to)
        {
            if(hal_uart_free(RS485_PHY_PORT))
            {
                rs_tx_wd = act_ticks;
                rs_tx_to = 255;                         // Free Run Sync Timeout 64mS

                if(rs_tx_queue.Size != 0)
                {
                    MQ_t * pBuf = mqDequeue(&rs_tx_queue);
                    if(pBuf != NULL)
                    {
                        LED_On();

                        pTx_buf = (uint8_t *)pBuf;
                        uint8_t  tx_Len = pBuf->Length;
                        uint8_t  tx_Dst = pBuf->a.phy1addr[0];
                        uint8_t  tx_CRC, in_Pos, out_Pos = 4;

                        pTx_buf[0] = RS485_SOF;                     // Start of Frame
                        pTx_buf[1] = tx_Len + 2;                    // Lenght
                        tx_CRC     = tx_Len + 2;
                        pTx_buf[2] = rs_addr;                       // Source address
                        tx_CRC    += rs_addr;
                        pTx_buf[3] = tx_Dst;                        // Destination address
                        tx_CRC    += tx_Dst;

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
                else        // Send Synchro Frame
                {
                    hal_uart_send(RS485_PHY_PORT, sizeof(rs_token), rs_token);
                }
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
    else    // Rx Watchdog
    {
        uint16_t diff = HAL_get_250us_tick() - rs_rx_wd;

        if(diff > 20)               // 64 bytes, 128kps, timeout = 5ms // 20 ticks
        {
            if(rs_state == RS_STATE_RX_DATA)
            {
                mqFree(pRx_buf);
            }
            rs_state = RS_STATE_RX_IDLE;
        }
    }

    return NULL;
}

void * RS485_GetAddr(void)
{
    return &rs_addr;
}

#endif  //  UART_PHY

