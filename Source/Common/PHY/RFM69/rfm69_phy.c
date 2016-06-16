/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// RFM69/SX1231 RF interface

#include "../../config.h"

#ifdef RFM69_PHY

#include "rfm69_reg.h"

// RF States
typedef enum
{
    RFM69_TRV_SLEEP = 0,    // Sleep mode, all blocks disabled
    RFM69_TRV_RX,
    RFM69_TRV_TX
}RFM69_TRVSTATE_t;

static const uint8_t rfm69config[][2] = {
    // 0x02 - Data operation mode and Modulation settings
    // RegDataModul 0x02
    {RFM69_REG_DATAMODUL, RFM69_DATAMODUL_DATAMODE_PACKET |     // Packet mode
                          RFM69_DATAMODUL_MODULATIONTYPE_FSK |  // Modulation - FSK
                          RFM69_DATAMODUL_MODULATIONSHAPING_2}, // FSK: Gaussian filter, BT = 0.5
    // 0x03/0x04 - Bit Rate setting
    {RFM69_REG_BITRATEMSB, RFM69_REG_BITRATEMSB_DEF},           // RegBitrateMsb 0x03
    {RFM69_REG_BITRATELSB, RFM69_REG_BITRATELSB_DEF},           // RegBitrateLsb 0x41
    // 0x05/0x06 - Frequency Deviation setting
    {RFM69_REG_FDEVMSB, RFM69_REG_FDEVMSB_DEF},                 // RegFdevMsb 0x01
    {RFM69_REG_FDEVLSB, RFM69_REG_FDEVLSB_DEF},                 // RegFdevLsb 0x48
    // 0x0B - AFC control in low modulation index situations
    // RegAfcCtrl - 0x00
    {RFM69_REG_AFCCTRL, RFM69_AFCCTRL_LOWBETA_OFF},
    // Low power, PA0 +13 dBm, OCP on
    // 0x11 - PA selection and Output Power control
    // RegPaLevel 0x9F
    {RFM69_REG_PALEVEL, RFM69_PALEVEL_PA0_ON | 31},
    // 0x12 - // Control of the PA ramp time in FSK mode
    // Ramp Time = 40 us
    // RegPaRamp 0x09
    {RFM69_REG_PARAMP, RFM69_PARAMP_40},
    // 0x13 - Over Current Protection control
    // Overcurrent protection enabled, 95 mA
    // RegOcp - 0x1A
    {RFM69_REG_OCP, RFM69_OCP_ON | RFM69_OCP_TRIM_95},
    // 0x18 - LNA settings
    // RegLna 0x88
    {RFM69_REG_LNA, RFM69_LNA_ZIN_200 |                         // Zin = 200 ohms
                    RFM69_LNA_GAINSELECT_AUTO},                 // gain set by the internal AGC loop
    // 0x19 - Channel Filter BW Control
    {RFM69_REG_RXBW, RFM69_REG_RXBW_DEF},                       // RegRxBw 0x4A
    {0x1A, 0x8B},   //  RegAfcBw
    {0x1E, 0x10},   //  RegAfcFei
    // 0x25 - Mapping of pins DIO0 to DIO3, 
    // RegDioMapping1 0x01
    {RFM69_REG_DIOMAPPING1, RFM69_DIOMAPPING1_DIO0_1},          // DIO0 - Rx PayloadReady
    // 0x26 - Mapping of pins DIO4 and DIO5, ClkOut frequency
    // RegDioMapping2 - 0x07
    {RFM69_REG_DIOMAPPING2, RFM69_DIOMAPPING2_CLKOUT_OFF},      // ClkOut Disable
    // 0x29 - RSSI Threshold control, Val = -THdbm * 2
    // RegRssiThresh - 0xE4(default)
    {RFM69_REG_RSSITHRESH, 200},                                // -100 dBm
    {0x2A, 0x00},   //  RegRxTimeout1
    {0x2B, 0x00},   //  RegRxTimeout2
    // 0x2C/0x2D  Preamble length
    // RegPreambleMsb, RegPreambleLsb 0x0003
    {RFM69_REG_PREAMBLEMSB, 0x00},
    {RFM69_REG_PREAMBLELSB, 0x03},
    // 0x2E - Sync Word Recognition control
    // RegSyncConfig - 0x88
    {RFM69_REG_SYNCCONFIG,  RFM69_SYNC |                        // Enable Sync
                            RFM69_SYNC_SIZE_2 |                 // Sync Size - 2 bytes
                            RFM69_SYNC_TOL_0},                  // Tolerance 0 words
    // 0x37 - Packet mode settings
    // RegPacketConfig1 - 0x94
    {RFM69_REG_PACKETCONFIG1, RFM69_PACKET1_FORMAT_VARIABLE |   // Packet variable length
                              RFM69_PACKET1_DCFREE_OFF |        // without encoding
                              RFM69_PACKET1_CRC_ON |            // Enable CRC
                              RFM69_PACKET1_CRCAUTOCLEAR_ON |   // On CRC error, clear FIFO and 
                                                                // restart new packet reception
                  RFM69_PACKET1_ADRSFILTERING_NODEBROADCAST},   // Address field must
                                                                // match NodeAddress
                                                                // or BroadcastAddress
    // 0x38 - Payload length setting
    // RegPayloadLength - 0x40
    {RFM69_REG_PAYLOADLENGTH, 0x40},
    // 0x3A - Broadcast address
    // RegBroadcastAdrs - 0x00
    {RFM69_REG_BROADCASTADRS, RFM69_BROADCAST_ADDR},
    {0x3B, 0x00},   //  RegAutoModes
    // 0x3C - Fifo threshold, Tx start condition
    // Start Transmission if FIFO not empty
    // RegFifoThresh - 0x8F
    {RFM69_REG_FIFOTHRESH,  RFM69_FIFOTHRESH_TXSTART_FIFONOTEMPTY | RFM69_FIFOTHRESH_VALUE},
    // 0x3D - Packet mode settings
    // Rx automatically restarted after InterPacketRxDelay = 1, AES - Off
    // RegPacketConfig2 0x02
    {RFM69_REG_PACKETCONFIG2, RFM69_REG_PACKETCONFIG2_DEF},
    {0x58, 0x1B},   //  RegTestLna
    // 0x6F - Fading Margin Improvement
    // RegTestDagc - 0x30
    // run DAGC continuously in RX mode for Fading Margin Improvement, 
    // recommended default for AfcLowBetaOn=0
    {RFM69_REG_TESTDAGC, RFM69_DAGC_IMPROVED_LOWBETA0},
    {0x71, 0x00}    //  RegTestAfc
};

static uint8_t          rfm69_NodeID;
static Queue_t          rfm69_tx_queue = {NULL, NULL, 4, 0};
static uint8_t          rfm69_state = RFM69_TRV_SLEEP;

// Local Subroutines

// Write to register
static void rfm69_writeReg(uint8_t addr, uint8_t value)
{
    RFM69_SELECT();
    hal_spi_exch8(RFM69_USE_SPI, addr | 0x80);
    hal_spi_exch8(RFM69_USE_SPI, value);
    RFM69_RELEASE();
}

// Read from register
static uint8_t rfm69_readReg(uint8_t addr)
{
    uint8_t retval;
    
    RFM69_SELECT();
    hal_spi_exch8(RFM69_USE_SPI, addr & 0x7F);
    retval = hal_spi_exch8(RFM69_USE_SPI, 0);
    RFM69_RELEASE();
    return retval;
}

static void rmm69_setMode(uint8_t mode)
{
    rfm69_writeReg(RFM69_REG_OPMODE, mode);
    while((rfm69_readReg(RFM69_REG_IRQFLAGS1) & RFM69_IRQFLAGS1_MODEREADY) == 0);
}

// Channel space - 25KHz
static void rfm69_setFreq(uint8_t chan)
{
    // More accurate calculation
    int64_t frf;

    frf = RFM69_BASE_FREQ;
    frf += chan * 25000U;

    frf *= 524288UL;        // 2^19
    frf /= 32000000UL;      // FXO

    RFM69_SELECT();
    hal_spi_exch8(RFM69_USE_SPI, 0x80 | RFM69_REG_FRFMSB);
    hal_spi_exch8(RFM69_USE_SPI, (frf >> 16) & 0xFF);
    hal_spi_exch8(RFM69_USE_SPI, (frf >> 8) & 0xFF);
    hal_spi_exch8(RFM69_USE_SPI, frf & 0xFF);
    RFM69_RELEASE();
}

static MQ_t * rfm69_rx_task(void)
{
    MQ_t * pRxBuf;
    uint8_t i;
    
#ifdef LED_On
    LED_On();
#endif  //  LED_On
    uint8_t frameLen;

    RFM69_SELECT();
    hal_spi_exch8(RFM69_USE_SPI, RFM69_REG_FIFO);
    frameLen = hal_spi_exch8(RFM69_USE_SPI, 0);
    if((frameLen < 5) || (frameLen > MQTTSN_MSG_SIZE))      // Packet is too big/small
    {
        RFM69_RELEASE();
        rfm69_writeReg(RFM69_REG_PACKETCONFIG2, rfm69_readReg(RFM69_REG_PACKETCONFIG2) |
                                                              RFM69_PACKET2_RXRESTART);
        rfm69_state = RFM69_TRV_SLEEP;
        return NULL;
    }

    pRxBuf = mqAlloc(sizeof(MQ_t));
    frameLen -= 2;
    pRxBuf->Length = frameLen;

    i = hal_spi_exch8(RFM69_USE_SPI, 0);                        // Read destination address
    pRxBuf->a.phy1addr[0] = hal_spi_exch8(RFM69_USE_SPI, 0);    // Read Source address

    for(i = 0; i < frameLen; i++)                               // Read Payload
    {
        pRxBuf->m.raw[i] = hal_spi_exch8(RFM69_USE_SPI, 0);
    }

    RFM69_RELEASE();

    rfm69_writeReg(RFM69_REG_OPMODE, RFM69_OPMODE_RX);          // Enter to RX State

    return pRxBuf;
}

#define RFM69_TX_RETRYS  5      // delay 15 - 50 ms

static void rfm69_tx_task(void)
{
    static uint8_t rfm69_tx_delay = 0;
    static uint8_t rfm69_tx_retry = RFM69_TX_RETRYS;

#ifdef LED_On
    LED_On();
#endif  //  LED_On
    
    // CDMA
    if(rfm69_tx_delay > 0)
    {
        static uint8_t rfm69_ms = 0;
        uint8_t act_ms = HAL_get_ms() & 0xFF;

        if(rfm69_ms == act_ms)
        {
            return;
        }
        rfm69_ms = act_ms;
        rfm69_tx_delay--;
        return;
    }

    // Channel Busy ?
    if(rfm69_readReg(RFM69_REG_RSSIVALUE) < 200)    // -100dBm
    {
        if(rfm69_tx_retry > 0)
        {
            rfm69_tx_retry--;
            rfm69_tx_delay = (HAL_RNG() & 0x7) + 3;
            return;
        }
    }

    rfm69_tx_delay = 0;
    rfm69_tx_retry = RFM69_TX_RETRYS;

    MQ_t * pTxBuf = mqDequeue(&rfm69_tx_queue);
    if(pTxBuf == NULL)      // Queue Busy
    {
        return;
    }
    
    rmm69_setMode(RFM69_OPMODE_STDBY);
    rfm69_writeReg(RFM69_REG_DIOMAPPING1, RFM69_DIOMAPPING1_DIO0_0);    // DIO0 - Packet Sent

    // Fill Buffer
    uint8_t i, len;

    len = pTxBuf->Length;
    // Send burst
    RFM69_SELECT();
    hal_spi_exch8(RFM69_USE_SPI, RFM69_REG_FIFO | 0x80);
    hal_spi_exch8(RFM69_USE_SPI, len + 2);                      // Set data length at the
                                                                //  first position of the TX FIFO 
    hal_spi_exch8(RFM69_USE_SPI, pTxBuf->a.phy1addr[0]);        // Send destination address
    hal_spi_exch8(RFM69_USE_SPI, rfm69_NodeID);                 // Send Source address
    for(i = 0; i < len; i++)                                    // Send Payload
    {
        hal_spi_exch8(RFM69_USE_SPI, pTxBuf->m.raw[i]);
    }
    RFM69_RELEASE();

    mqFree(pTxBuf);
    
    rfm69_writeReg(RFM69_REG_OPMODE, RFM69_OPMODE_TX);          // Switch to TX state
    rfm69_state = RFM69_TRV_TX;
}

void RFM69_Init(void)
{
    uint8_t     Channel;
    uint16_t    GroupID;

    MQ_t * pBuf;
    while((pBuf = mqDequeue(&rfm69_tx_queue)) != NULL)
    {
        mqFree(pBuf);
    }

    // Load Device ID
    uint8_t Len = sizeof(uint8_t);
    ReadOD(objRFNodeId, MQTTSN_FL_TOPICID_PREDEF,  &Len, &rfm69_NodeID);
    // Load Frequency channel
    ReadOD(objRFChannel, MQTTSN_FL_TOPICID_PREDEF, &Len, (uint8_t *)&Channel);
    Len = sizeof(uint16_t);
    // Load Group ID(Synchro)
    ReadOD(objRFGroup, MQTTSN_FL_TOPICID_PREDEF,  &Len, (uint8_t *)&GroupID);

    // Init Hardware

    // Configure NSS Pin
    hal_dio_configure(RFM69_NSS_PIN, DIO_MODE_OUT_PP_HS);
    RFM69_RELEASE();

    // Configure IRQ Pin
    hal_dio_configure(RFM69_IRQ_PIN, DIO_MODE_IN_PD);

    // Configure SPI
    hal_spi_cfg(RFM69_USE_SPI, (HAL_SPI_MODE_0 | HAL_SPI_MSB | HAL_SPI_8B), 10000000UL);

    // Chip Mode - Standby
    rmm69_setMode(RFM69_OPMODE_STDBY);

/*
    // Start RC Calibration
    rfm69_writeReg(RFM69_REG_OSC1, (rfm69_readReg(RFM69_REG_OSC1) | RFM69_OSC1_RCCAL_START));
    while((rfm69_readReg(RFM69_REG_OSC1) & RFM69_OSC1_RCCAL_DONE) == 0);
*/
    // Load Config
    uint8_t i;
    for(i = 0; i<(sizeof(rfm69config)/2); i++)
    {
        rfm69_writeReg(rfm69config[i][0], rfm69config[i][1]);
    }

    // Load Settings 
    rfm69_writeReg(RFM69_REG_NODEADRS, rfm69_NodeID);
    rfm69_writeReg(RFM69_REG_SYNCVALUE1, GroupID >> 8);
    rfm69_writeReg(RFM69_REG_SYNCVALUE2, GroupID & 0xFF);

    rfm69_setFreq(Channel);
    
    //rmm69_setMode(RFM69_OPMODE_SLEEP);
    rfm69_state = RFM69_TRV_SLEEP;
}

void RFM69_Send(void *pBuf)
{
    if(!mqEnqueue(&rfm69_tx_queue, pBuf))
    {
        mqFree(pBuf);
    }
}

void  * RFM69_Get(void)
{
    static uint16_t rfm_wd = 0;

    if(rfm69_state == RFM69_TRV_RX)
    {
        if(RFM69_IRQ_STATE() || (rfm_wd == 0))
        {
            rfm_wd = 0xFFFF;
            if(rfm69_readReg(RFM69_REG_IRQFLAGS2) & RFM69_IRQFLAGS2_PAYLOADREADY)
            {
                return rfm69_rx_task();
            }
//            else
            // dummy read check config
        }
        else if(rfm69_tx_queue.Size != 0)
        {
            rfm69_tx_task();
            rfm_wd = 0;
        }
        rfm_wd--;
    }
    else if(rfm69_state == RFM69_TRV_TX)
    {
        if(RFM69_IRQ_STATE() || (rfm_wd == 0))
        {
            if(rfm69_readReg(RFM69_REG_IRQFLAGS2) & RFM69_IRQFLAGS2_PACKETSENT)
            {
                // DIO0 - PayloadReady
                rfm69_writeReg(RFM69_REG_DIOMAPPING1, RFM69_DIOMAPPING1_DIO0_1);
                rmm69_setMode(RFM69_OPMODE_RX);     // Enter to RX State
                rfm69_state = RFM69_TRV_RX;
            }
        }
        rfm_wd--;
    }
    else
    {
        rmm69_setMode(RFM69_OPMODE_SLEEP);                                  // Reset Chip
        rfm69_writeReg(RFM69_REG_DIOMAPPING1, RFM69_DIOMAPPING1_DIO0_1);    // DIO0 - PayloadReady
        rmm69_setMode(RFM69_OPMODE_RX);                                     // Enter to RX State
        rfm69_state = RFM69_TRV_RX;
    }

    return NULL;
}

void  * RFM69_GetAddr(void)
{
    return &rfm69_NodeID;
}

#endif  //  RFM69_PHY
