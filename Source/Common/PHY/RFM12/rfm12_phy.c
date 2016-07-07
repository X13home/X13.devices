/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// RFM12 RF interface

#include "../../config.h"

#ifdef RFM12_PHY

#include "rfm12_reg.h"

// Chip configuration
#define RFM12_BAUD          RFM12_BAUD_38K4         // Follow FSK shift & bandwidth
// Quartz 50 ppm
#define RFM12_BANDWIDTH     RFM12_RXCTRL_BW_270
#define RFM12_FSKWIDTH      RFM12_TXCONF_FS_135
// Quartz 20 ppm
//#define RFM12_BANDWIDTH     RFM12_RXCTRL_BW_134
//#define RFM12_FSKWIDTH      RFM12_TXCONF_FS_90

#define RFM12_GAIN          RFM12_RXCTRL_LNA_6
#define RFM12_DRSSI         RFM12_RXCTRL_RSSI_91
#define RFM12_POWER         RFM12_TXCONF_POWER_0

#define RFM12_TX_RETRYS     5   // delay 15 - 50 ms

// RF States
typedef enum
{
    RFM12_TRVPOR = 0,
    RFM12_TRVIDLE,
    RFM12_TRVRXIDLE,
    RFM12_TRVRXHDR_DST,
    RFM12_TRVRXHDR_SRC,
    RFM12_TRVRXDATA,
    RFM12_TRVRXDONE,
    RFM12_TRVTXPREAMB,
    RFM12_TRVTXHDR,
    RFM12_TRVTXDATA,
    RFM12_TRVTXCRC,
    RFM12_TRVTXDONE,
    RFM12_TRVASLEEP,
    RFM12_TRVWKUP
}RFM12_TRVSTATE_t;

typedef union
{
    uint8_t raw[5];
    struct
    {
        uint8_t Group[2];
        uint8_t Len;
        uint8_t Dst;
        uint8_t Src;
    };
}RFM12_TX_HDR_t;

static RFM12_TRVSTATE_t    rfm12_state;
static RFM12_TX_HDR_t   rfm12_tx_hdr;
static Queue_t          rfm12_tx_queue = {NULL, NULL, 4, 0};
static MQ_t           * rfm12_pRxBuf = NULL;
static MQ_t           * rfm12_pTxBuf = NULL;

// Local subroutines
static void rfm12_CalcCRC(uint8_t data, uint16_t *pCRC)     // CRC Calculation compatible with cc1101
{
    uint8_t i;
    uint16_t crcReg = *pCRC;
    for (i = 0; i < 8; i++)
    {
        if(((crcReg & 0x8000) >> 8) ^ (data & 0x80))
            crcReg = (crcReg<<1) ^ 0x8005;
        else
            crcReg = (crcReg<<1);
        data <<= 1;
    }
    *pCRC = crcReg;
}

static void rfm12_tx_task(void)
{
    static uint8_t rfm12_tx_delay = 0;
    static uint8_t rfm12_tx_retry = RFM12_TX_RETRYS;

    if(rfm12_tx_queue.Size == 0)
        return;

    // CDMA
    if(rfm12_tx_delay > 0)
    {
        static uint8_t rfm12_ms = 0;
        uint8_t act_ms = HAL_get_ms() & 0xFF;

        if(rfm12_ms == act_ms)
            return;
        rfm12_ms = act_ms;
        rfm12_tx_delay--;
        return;
    }

    // Carrier ?
    if((hal_rfm12_spiExch(0) & RFM12_STATUS_RSSI) != 0)   // Channel Busy
    {
        if(rfm12_tx_retry > 0)
        {
            rfm12_tx_retry--;
            rfm12_tx_delay = (HAL_RNG() & 0x7) + 3;
            return;
        }
    }

    // Send
    rfm12_pTxBuf = mqDequeue(&rfm12_tx_queue);
    if(rfm12_pTxBuf == NULL)                    // Queue Busy
        return;

    rfm12_tx_hdr.Len = rfm12_pTxBuf->Length + 2;
    rfm12_tx_hdr.Dst = rfm12_pTxBuf->a.phy1addr[0];

    rfm12_tx_delay = 0;
    rfm12_tx_retry = RFM12_TX_RETRYS;
    rfm12_state = RFM12_TRVTXPREAMB;

    hal_rfm12_spiExch(RFM12_IDLE_MODE);         // Switch to Idle state
    hal_rfm12_spiExch(RFM12_TXFIFO_ENA);        // Enable TX FIFO

    hal_rfm12_spiExch(RFM12_TRANSMIT_MODE);
}

// IRQ subroutine
void rfm12_irq(void)
{
    uint16_t intstat = hal_rfm12_spiExch(0);

    if(intstat & (uint16_t)RFM12_STATUS_POR)        // Power-on reset
    {
        hal_rfm12_spiExch(RFM12_SLEEP_MODE);
        rfm12_state = RFM12_TRVPOR;
        return;
    }
    
    if(intstat & (uint16_t)RFM12_STATUS_RGIT)       // Rx/Tx FIFO events
    {
        static uint16_t rfm12v_RfCRC;               // Actual CRC
        static uint8_t  rfm12v_RfLen = 0;           // Packet Length
        static uint8_t  rfm12v_Pos = 0;             // Position
        uint8_t ch;

        switch(rfm12_state)
        {
            // Start Rx Section
            case RFM12_TRVRXIDLE:              // Get Packet Length
                ch = hal_rfm12_spiExch(RFM12_CMD_READ) & 0xFF;
                if((ch < 4) || (ch > (MQTTSN_MSG_SIZE + 3)))
                    break;
                rfm12v_RfCRC = 0xFFFF;
                rfm12_CalcCRC(ch, (uint16_t *)&rfm12v_RfCRC);
                rfm12_state = RFM12_TRVRXHDR_DST;
                rfm12v_RfLen = ch - 2;      // Packet length(data), w/o CRC
                rfm12_pRxBuf->Length = rfm12v_RfLen;
                return;
            case RFM12_TRVRXHDR_DST:           // Destination Address
                ch = hal_rfm12_spiExch(RFM12_CMD_READ) & 0xFF;
                if((ch == 0) || (ch == rfm12_tx_hdr.Src))
                {
#ifdef LED_On
                    LED_On();
#endif  //  LED_On
                    rfm12_CalcCRC(ch, (uint16_t *)&rfm12v_RfCRC);
                    rfm12_state = RFM12_TRVRXHDR_SRC;
                    return;
                }
                break;
            case RFM12_TRVRXHDR_SRC:           // Source Address
                ch = hal_rfm12_spiExch(RFM12_CMD_READ) & 0xFF;
                rfm12_CalcCRC(ch, (uint16_t *)&rfm12v_RfCRC);

                rfm12_pRxBuf->a.phy1addr[0] = ch;
                rfm12v_Pos = 0;
                rfm12_state = RFM12_TRVRXDATA;
                return;
            case RFM12_TRVRXDATA:
                ch = hal_rfm12_spiExch(RFM12_CMD_READ) & 0xFF;
                if(rfm12v_Pos < rfm12v_RfLen)
                {
                    rfm12_CalcCRC(ch, (uint16_t *)&rfm12v_RfCRC);
                    rfm12_pRxBuf->m.raw[rfm12v_Pos++] = ch;
                    return;
                }
                else if(rfm12v_Pos == rfm12v_RfLen)     // 1st CRC byte;
                {
                    uint8_t bTmp = rfm12v_RfCRC>>8;
                    if(ch == bTmp)
                    {
                        rfm12v_Pos++;
                        return;
                    }
                }
                else if(ch == (rfm12v_RfCRC & 0xFF))    // 2nd CRC byte
                {
                    hal_rfm12_spiExch(RFM12_IDLE_MODE);
                    hal_rfm12_spiExch(RFM12_RXFIFO_DIS);

                    rfm12_state = RFM12_TRVRXDONE;
                    //rfm12v_Foffs = (uint8_t)(intstat & 0x1F)<<3;    // int8_t frequency offset
                    return;
                }
                break;

            // Start Tx Section
            // Send Preamble, initialise variables
            case RFM12_TRVTXPREAMB:
                hal_rfm12_spiExch(RFM12_CMD_TX | 0xAA);
                rfm12v_Pos = 0;
                rfm12_state = RFM12_TRVTXHDR;
#ifdef LED_On
                LED_On();
#endif  //  LED_On
                return;
            // Send header
            case RFM12_TRVTXHDR:
                ch = rfm12_tx_hdr.raw[rfm12v_Pos];

                if(rfm12v_Pos > 1)
                {
                    rfm12_CalcCRC(ch, (uint16_t *)&rfm12v_RfCRC);
                }
                else
                {
                    rfm12v_RfCRC = 0xFFFF;
                    
                }

                rfm12v_Pos++;
                hal_rfm12_spiExch(RFM12_CMD_TX | ch);
                
                if(rfm12v_Pos == sizeof(RFM12_TX_HDR_t))
                {
                    rfm12v_Pos = 0;
                    rfm12_state = RFM12_TRVTXDATA;
                }
                return;
            // Send Data
            case RFM12_TRVTXDATA:
                ch = rfm12_pTxBuf->m.raw[rfm12v_Pos++];
                hal_rfm12_spiExch(RFM12_CMD_TX | ch);
                rfm12_CalcCRC(ch, (uint16_t *)&rfm12v_RfCRC);
                
                if(rfm12v_Pos == rfm12_pTxBuf->Length)
                {
                    rfm12_state = RFM12_TRVTXCRC;
                    rfm12v_Pos = 0;
                }
                return;
            // Send CRC and dummy byte
            case RFM12_TRVTXCRC:
                if(rfm12v_Pos == 0)         // Send MSB CRC
                    ch = rfm12v_RfCRC>>8;
                else if(rfm12v_Pos == 1)    // Send LSB CRC
                    ch = rfm12v_RfCRC & 0xFF;
                else if(rfm12v_Pos == 2)    // Send Dummy
                    ch = 0xFF;
                else
                {
                    hal_rfm12_spiExch(RFM12_IDLE_MODE);
                    hal_rfm12_spiExch(RFM12_TXFIFO_DIS);
                    rfm12_state = RFM12_TRVTXDONE;
                    rfm12v_Pos = 0;
                    return;
                }

                hal_rfm12_spiExch(RFM12_CMD_TX | ch);
                rfm12v_Pos++;
                return;
            default:            // Unknown State
                break;
        }
    }

    // Switch to Receive mode
    hal_rfm12_spiExch(RFM12_IDLE_MODE);
    hal_rfm12_spiExch(RFM12_RXFIFO_DIS);
    hal_rfm12_spiExch(RFM12_TXFIFO_DIS);
    
    hal_rfm12_spiExch(RFM12_RECEIVE_MODE);
    hal_rfm12_spiExch(RFM12_RXFIFO_ENA);
    rfm12_state = RFM12_TRVRXIDLE;
}

static const uint16_t rfm12_config[] =
{
    RFM12_CMD_CFG |             // Configuration Setting
    RFM12_CFG_EL |              // Enable TX FIFO
    RFM12_CFG_EF |              // Enable RX FIFO
    RFM12_BAND |                // Select Band
    RFM12_XTAL_12PF,            // Set XTAL capacitor
    
    RFM12_SLEEP_MODE,

    RFM12_CMD_DATARATE |        // Data Rate Command
    RFM12_BAUD,
    
    RFM12_CMD_RXCTRL |          // Receiver Control Command
    RFM12_RXCTRL_P16_VDI |      // Pin16 - VDI output
    RFM12_RXCTRL_VDI_MEDIUM |   // VDI response - Medium
    RFM12_GAIN |                // gain select
    RFM12_BANDWIDTH |           // Receiver baseband bandwidth
    RFM12_DRSSI,                // RSSI detector threshold
    
    RFM12_CMD_DATAFILTER |      // Data Filter Command
    RFM12_DATAFILTER_AL |       // Clock recovery (CR) auto lock control, Slow mode,
                                //  Digital Filter
    RFM12_DQD_THRESH_4,         // DQD threshold = 4( good)
    
    RFM12_RXFIFO_DIS,           // FIFO and Reset Mode Command
    
    RFM12_CMD_AFC |             // AFC Command
#if (RFM12_PHY != 1)            // Gateway
    RFM12_AFC_AUTO_VDI |        // Keep the foffset only during receiving (VDI=high)
#else   // Node
    RFM12_AFC_AUTO_KEEP |       // Keep the foffset value independently from the state of the VDI signal
#endif  // (RFM12_PHY == 2)
    RFM12_AFC_LIMIT_16 |        // Limit  +75 -80 kHz
    RFM12_AFC_FI |              // accuracy (fine) mode
    RFM12_AFC_OE |              // Enables the frequency offset register
    RFM12_AFC_EN,               // Enables the calculation of the offset frequency by the AFC circuit.
    
    RFM12_CMD_TXCONF |          // TX Configuration Control Command
    RFM12_FSKWIDTH |            // FSK =  fo + offset
    RFM12_POWER,                // Relative Output Power
    
    RFM12_CMD_PLL |             // PLL Setting Command
    RFM12_CLK_FRQ_LOW |         // uC CLK Frequency <= 2,5MHz
    RFM12_PLL_DDIT,             // disable the dithering in the PLL loop.
    
    RFM12_CMD_WAKEUP,           // Wake-Up Timer Command, not used
    
    RFM12_CMD_DUTYCYCLE,        // Low Duty-Cycle Command, not used
    
    RFM12_CMD_LBDMCD |          // Low Battery Detector and 
    RFM12_MCD_DIV5              // Microcontroller Clock Divider Command, CLK Out = 2 MHz
};

// API Section
void RFM12_Init(void)
{
    uint8_t     Channel;
    uint16_t    FR;

    // Free Tx Queue
    MQ_t * pBuf;
    while((pBuf = mqDequeue(&rfm12_tx_queue)) != NULL)
        mqFree(pBuf);
    
    // Free Tx Buffer;
    if(rfm12_pTxBuf != NULL)
    {
        mqFree(rfm12_pTxBuf);
        rfm12_pTxBuf = NULL;
    }

    // Load Device ID
    uint8_t Len = sizeof(uint8_t);
    ReadOD(objRFNodeId, MQTTSN_FL_TOPICID_PREDEF,  &Len, &rfm12_tx_hdr.Src);
    // Load Frequency channel
    ReadOD(objRFChannel, MQTTSN_FL_TOPICID_PREDEF, &Len, &Channel);
    // Load Group ID(Synchro)
    Len = sizeof(uint16_t);
    uint16_t Group;
    ReadOD(objRFGroup, MQTTSN_FL_TOPICID_PREDEF,  &Len, (uint8_t *)&Group);
    rfm12_tx_hdr.Group[0] = Group >> 8;
    rfm12_tx_hdr.Group[1] = Group & 0xFF;
    
// 433 MHz
#if (RF_BASE_FREQ > 433050000UL) && (RF_BASE_FREQ < 434790000UL)
    FR = 1200 + ((uint16_t)Channel * 10);
// 868 MHz
#elif (RF_BASE_FREQ > 868000000UL) && (RF_BASE_FREQ < 870000000UL)
    FR = 1600 + ((uint16_t)Channel * 5);
// 915 MHz
#else
    FR = (800 + ((uint16_t)Channel * 10))/3;
#endif

    hal_rfm12_init_hw();
    
    hal_rfm12_spiExch(0);           // initial SPI transfer added to avoid power-up problem
    hal_rfm12_spiExch(RFM12_SLEEP_MODE);
    
    // wait until RFM12B is out of power-up reset, this takes several *seconds*
    hal_rfm12_spiExch(RFM12_CMD_TX);
    do
    {
        hal_rfm12_spiExch(0);
    }
    while(hal_rfm12_irq_stat());
    
    uint8_t pos;
    for(pos = 0; pos < (sizeof(rfm12_config)/sizeof(rfm12_config[0])); pos++)
        hal_rfm12_spiExch(rfm12_config[pos]);

    hal_rfm12_spiExch(RFM12_CMD_FREQUENCY |     // Frequency Setting Command
                      FR);
    hal_rfm12_spiExch(RFM12_CMD_SYNCPATTERN |   // Synchro Pattern = 0x2D[grp]
                      rfm12_tx_hdr.Group[1]);

    rfm12_state = RFM12_TRVIDLE;
    hal_rfm12_spiExch(RFM12_IDLE_MODE);
    hal_rfm12_enable_irq();
}

void RFM12_Send(void *pBuf)
{
    if(!mqEnqueue(&rfm12_tx_queue, pBuf))
        mqFree(pBuf);
}

void * RFM12_Get(void)
{
    static uint16_t wd_cnt = 0;
    
    switch(rfm12_state)
    {
        case RFM12_TRVPOR:     // Power On Reset
            wd_cnt = 0;
            RFM12_Init();
            break;
        case RFM12_TRVIDLE:
            if(rfm12_pRxBuf == NULL)
                rfm12_pRxBuf = mqAlloc(sizeof(MQ_t));

            wd_cnt = 0;
            rfm12_state = RFM12_TRVRXIDLE;
            hal_rfm12_spiExch(RFM12_RECEIVE_MODE);
            hal_rfm12_spiExch(RFM12_RXFIFO_ENA);
            break;
        case RFM12_TRVRXDONE:  // Rx Data Ready
            {
            wd_cnt = 0;
            MQ_t * pRetVal = rfm12_pRxBuf;
            rfm12_pRxBuf = NULL;
            rfm12_state = RFM12_TRVIDLE;
            return pRetVal;
            }
        case RFM12_TRVRXIDLE:
            wd_cnt = 0;
            rfm12_tx_task();
            break;
        case RFM12_TRVTXDONE:              // Data Sent
            mqFree(rfm12_pTxBuf);
            rfm12_pTxBuf = NULL;
            rfm12_state = RFM12_TRVIDLE;
            break;
        default:
            wd_cnt--;
            if(wd_cnt == 0)
            {
                hal_rfm12_spiExch(RFM12_IDLE_MODE);
                hal_rfm12_spiExch(RFM12_RXFIFO_DIS);
                hal_rfm12_spiExch(RFM12_TXFIFO_DIS);
                rfm12_state = RFM12_TRVIDLE;
            }
    }

    return NULL;
}

void * RFM12_GetAddr(void)
{
    return &rfm12_tx_hdr.Src;
}

#ifdef ASLEEP
void RFM12_ASleep(void)
{
    if((rfm12_state > RFM12_TRVRXIDLE) && (rfm12_state < RFM12_TRVRXDONE))          // Receive data
    {
        while(rfm12_state != RFM12_TRVRXDONE);
    }
    else if((rfm12_state >= RFM12_TRVTXPREAMB) && (rfm12_state < RFM12_TRVTXDONE))  // Send Data
    {
        while(rfm12_state != RFM12_TRVTXDONE);
        mqFree(rfm12_pTxBuf);
        rfm12_pTxBuf = NULL;
    }

    hal_rfm12_spiExch(RFM12_SLEEP_MODE);
    rfm12_state = RFM12_TRVASLEEP;
}

void RFM12_AWake(void)
{
    hal_rfm12_spiExch(RFM12_IDLE_MODE);
    rfm12_state = RFM12_TRVIDLE;
}
#endif  //  ASLEEP

#endif  //  RFM12_PHY
