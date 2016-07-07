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

// FXO = 32000000UL
// FSTEP = FXO/524288UL (2^19)

// Packet Mode
// Modulation (G)FSK
// FSK: Gaussian filter, BT = 1.0
#define RFM69_REG_DATAMODUL_DEF     (RFM69_DATAMODUL_DATAMODE_PACKET |      \
                                     RFM69_DATAMODUL_MODULATIONTYPE_FSK |   \
                                     RFM69_DATAMODUL_MODULATIONSHAPING_1)
// Baud: 38400
// BITRATE = RFM69_FXO/Baud
#define RFM69_REG_BITRATEMSB_DEF    0x03
#define RFM69_REG_BITRATELSB_DEF    0x41
// Deviation: 40000
// FDEV = Deviation / FSTEP
// Modulation index: 2 = 2 * Deviation / Baud
#define RFM69_REG_FDEVMSB_DEF       0x02
#define RFM69_REG_FDEVLSB_DEF       0x8F
// RXBW >= 2 * Deviation + Baud + FXOerr
// FSK Mode: RxBw = FXOSC / (RxBwMant * 2^(RxBwExp + 2))
// RX filter Bandwidth 167 KHz = 32M /(24 * 2^(2 + 1))
// CuttOff 4%
#define RFM69_REG_RXBW_DEF          (RFM69_RXBW_DCCFREQ_2 | RFM69_RXBW_MANT_24 | RFM69_RXBW_EXP_1)
// AFC RX filter Bandwidth 50 KHz = 32M /(20 * 2^(2 + 3))
// CuttOff Freq: 500 Hz
#define RFM69_REG_AFCBW_DEF         (RFM69_AFCBW_DCCFREQAFC_4 | RFM69_AFCBW_MANTAFC_20 | \
                                     RFM69_AFCBW_EXPAFC_3)

// Rx Mapping DIO0 - RSSI, DIO3 - Sync Address
#define RFM69_REG_DIOMAPPING1_RX    RFM69_DIOMAPPING1_DIO0_3 | RFM69_DIOMAPPING1_DIO3_2
// Tx Mapping DIO0 - Packet Sent, DIO3 - TxReady
#define RFM69_REG_DIOMAPPING1_TX    RFM69_DIOMAPPING1_DIO0_0 | RFM69_DIOMAPPING1_DIO3_2

// RSSI Threshold -90 dBm
#define RFM69_REG_RSSITHRESH_DEF    180
// Sync word - Device Network ID
#define RFM69_REG_SYNCVALUE1_DEF    0xB1
#define RFM69_REG_SYNCVALUE2_DEF    0x82
// Packet Mode Configuration
// Packet variable length
// DC_FREE - Whitening
// Enable CRC
// On CRC error, clear FIFO and restart new packet reception
// Address field must match NodeAddress or BroadcastAddress
#define RFM69_REG_PACKETCONFIG1_DEF (RFM69_PACKET1_FORMAT_VARIABLE |            \
                                     RFM69_PACKET1_DCFREE_WHITENING |           \
                                     RFM69_PACKET1_CRC_ON |                     \
                                     RFM69_PACKET1_ADRSFILTERING_NODEBROADCAST)
// RF States
#define RFM69_TRV_SLEEP             0
#define RFM69_TRV_RX                1
#define RFM69_TRV_TX                2

#ifndef LED_On
#define LED_On()
#endif  //  LED_On

static const uint8_t rfm69config[][2] = {
    // 0x02 - Data operation mode and Modulation settings
    // RegDataModul - 0x02
    {RFM69_REG_DATAMODUL, RFM69_REG_DATAMODUL_DEF},
    // 0x03/0x04 - Bit Rate setting
    // RegBitrateMsb / RegBitrateLsb - 0x0341
    {RFM69_REG_BITRATEMSB, RFM69_REG_BITRATEMSB_DEF},
    {RFM69_REG_BITRATELSB, RFM69_REG_BITRATELSB_DEF},
    // 0x05/0x06 - Frequency Deviation setting
    // RegFdevMsb / RegFdevLsb - 0x0275
    {RFM69_REG_FDEVMSB, RFM69_REG_FDEVMSB_DEF},
    {RFM69_REG_FDEVLSB, RFM69_REG_FDEVLSB_DEF},
    // 0x0B - AFC control in low modulation index situations
    // RegAfcCtrl - 0x00
    {RFM69_REG_AFCCTRL, RFM69_AFCCTRL_LOWBETA_OFF},
    // 0x12 - // Control of the PA ramp time in FSK mode
    // Ramp Time = 40 us
    // RegPaRamp - 0x09
    {RFM69_REG_PARAMP, RFM69_PARAMP_40},
    // 0x13 - Over Current Protection control
    // Overcurrent protection enabled, 95 mA
    // RegOcp - 0x1A
    {RFM69_REG_OCP, RFM69_OCP_ON | RFM69_OCP_TRIM_95},
    // 0x18 - LNA settings
    // Zin = 200 ohms, gain set by the internal AGC loop
    // RegLna - 0x88
    {RFM69_REG_LNA, RFM69_LNA_ZIN_200 | RFM69_LNA_GAINSELECT_AUTO},
    // 0x19 - Channel Filter BW Control
    // RegRxBw - 0x51
    {RFM69_REG_RXBW, RFM69_REG_RXBW_DEF},
    // 0x1A - Channel Filter BW control during the AFC routine
    // RegAfcBw - 0x8B
    {RFM69_REG_AFCBW, RFM69_REG_AFCBW_DEF},
    // 0x25 - Mapping of pins DIO0 to DIO3,
    // DIO0 - Rx PayloadReady
    // RegDioMapping1 0x01
    {RFM69_REG_DIOMAPPING1, RFM69_REG_DIOMAPPING1_RX},
    // 0x26 - Mapping of pins DIO4 and DIO5, ClkOut frequency
    // ClkOut Disable
    // RegDioMapping2 - 0x07
    {RFM69_REG_DIOMAPPING2, RFM69_DIOMAPPING2_CLKOUT_OFF},
    // 0x29 - RSSI Threshold control, Val = -THdbm * 2
    // RegRssiThresh - 0xE4(default)
    {RFM69_REG_RSSITHRESH, RFM69_REG_RSSITHRESH_DEF},
    // 0x2A - Timeout duration between Rx request and RSSI detection
    // RegRxTimeout1- 0x00 - disabled
    {RFM69_REG_RXTIMEOUT1, 0x00},
    // 0x2B - Timeout duration between RSSI detection and PayloadReady
    // Timeout = Val * 16 / Baud
    // RegRxTimeout2 - 0x1C - 11,66mS
    {RFM69_REG_RXTIMEOUT2, 0x1C},
    // 0x2C/0x2D  Preamble length
    // RegPreambleMsb, RegPreambleLsb 0x0004
    {RFM69_REG_PREAMBLEMSB, 0x00},
    {RFM69_REG_PREAMBLELSB, 0x04},
    // 0x2E - Sync Word Recognition control
    // RegSyncConfig - 0x88
    {RFM69_REG_SYNCCONFIG,  RFM69_SYNC |                        // Enable Sync
                            RFM69_SYNC_SIZE_4 |                 // Sync Size - 4 bytes
                            RFM69_SYNC_TOL_0},                  // Tolerance 0 bit
    // 0x2F - 0x36 Sync Word
    // RegSyncValue1/8 - 0xB5A2
    {RFM69_REG_SYNCVALUE1, RFM69_REG_SYNCVALUE1_DEF},
    {RFM69_REG_SYNCVALUE2, RFM69_REG_SYNCVALUE2_DEF},
    // 0x37 - Packet mode settings
    // RegPacketConfig1 - 0xD4
    {RFM69_REG_PACKETCONFIG1, RFM69_REG_PACKETCONFIG1_DEF},
    // 0x38 - Payload length setting
    // RegPayloadLength - 0x40
    {RFM69_REG_PAYLOADLENGTH, MQTTSN_MSG_SIZE},
    // 0x3A - Broadcast address
    // RegBroadcastAdrs - 0x00
    {RFM69_REG_BROADCASTADRS, 0x00},
    // 0x3B - Auto modes settings
    // RegAutoModes - 0x00 - disabled
    {RFM69_REG_AUTOMODES, 0x00},
    // 0x3C - Fifo threshold, Tx start condition
    // Start Transmission if FIFO not empty
    // RegFifoThresh - 0x8F
    {RFM69_REG_FIFOTHRESH,  RFM69_FIFOTHRESH_TXSTART_FIFONOTEMPTY | RFM69_FIFOTHRESH_VALUE},
    // 0x3D - Packet mode settings
    // Rx automatically restarted after InterPacketRxDelay = 1, AES - Off
    // RegPacketConfig2 0x00
    {RFM69_REG_PACKETCONFIG2, RFM69_PACKET2_AUTORXRESTART_OFF},
    // 0x58 - Sensitivity boost
    // Normal Mode
    // RegTestLna - 0x1B
    {RFM69_REG_TESTLNA, RFM69_REG_TESTLNA_NORM},
    // 0x5A/0x5C - High Power PA settings
    // RegTestPa1 - 0x5A, RegTestPa1 - 0x5C
    // default low power - 0x55/0x70
    {RFM69_REG_TESTPA1, RFM69_REG_TESTPA1_LOW},
    {RFM69_REG_TESTPA2, RFM69_REG_TESTPA2_LOW},
    // 0x6F - Fading Margin Improvement
    // RegTestDagc - 0x30
    // run DAGC continuously in RX mode for Fading Margin Improvement, 
    // recommended default for AfcLowBetaOn=0
    {RFM69_REG_TESTDAGC, RFM69_DAGC_IMPROVED_LOWBETA0}
};

static uint8_t          rfm69_NodeID = 0xFF;
static Queue_t          rfm69_tx_queue = {NULL, NULL, 4, 0};
static uint8_t          rfm69_state = RFM69_TRV_SLEEP;
static int8_t           rfm69_rssi = -(RFM69_REG_RSSITHRESH_DEF/2);

// Local Subroutines

// Write to register
static void rfm69_writeReg(uint8_t addr, uint8_t value)
{
    RFM69_SELECT();
    hal_spi_exch8(RFM69_USE_SPI, RFM69_REG_WRITE | addr);
    hal_spi_exch8(RFM69_USE_SPI, value);
    RFM69_RELEASE();
}

// Read from register
static uint8_t rfm69_readReg(uint8_t addr)
{
    uint8_t retval;
    
    RFM69_SELECT();
    hal_spi_exch8(RFM69_USE_SPI, addr);
    retval = hal_spi_exch8(RFM69_USE_SPI, 0);
    RFM69_RELEASE();
    return retval;
}

// Set TRV to StandBy mode
static void rfm69_standby(void)
{
    rfm69_writeReg(RFM69_REG_OPMODE, RFM69_OPMODE_STDBY);
    // Wait for ModeReady
    while((rfm69_readReg(RFM69_REG_IRQFLAGS1) & RFM69_IRQFLAGS1_MODEREADY) == 0);
}

// Set Frequency
// Channel space - 25KHz
static void rfm69_setFreq(uint8_t chan)
{
    // More accurate calculation
    int64_t frf;

    frf = RFM69_BASE_FREQ;
    frf += chan * 25000U;

    if((frf < RFM69_MIN_FREQ) || (frf > RFM69_MAX_FREQ))
    {
        frf = RF_BASE_FREQ;
    }

    // Reg = FREQ / FSTEP
    frf *= 524288UL;        // 2^19
    frf /= 32000000UL;      // FXO

    RFM69_SELECT();
    hal_spi_exch8(RFM69_USE_SPI, RFM69_REG_WRITE | RFM69_REG_FRFMSB);
    hal_spi_exch8(RFM69_USE_SPI, (frf >> 16) & 0xFF);
    hal_spi_exch8(RFM69_USE_SPI, (frf >> 8) & 0xFF);
    hal_spi_exch8(RFM69_USE_SPI, frf & 0xFF);
    RFM69_RELEASE();
}

// Set Output Power
static void rfm_setPWR(int8_t dBm)
{
    uint8_t pa_val;

#if (defined RFM69_HW)
    // RFM69H[C]W - Module with RF switch, Tx connected to PA1 + PA2
    if(dBm > 13)
    {
        if(dBm > 17)
        {
            // Pout = 20 dBm can be used only with matched load and duty < 1%
            // lab environment
            dBm = 17;
        }
        dBm += 14;
        pa_val = RFM69_PALEVEL_PA1_ON | RFM69_PALEVEL_PA2_ON;
        pa_val |= dBm;
    }
    else
    {
        if(dBm < -2)
        {
            dBm = -2;
        }
        dBm += 18;
        pa_val = RFM69_PALEVEL_PA1_ON;
        pa_val |= dBm;
    }
#else
    // RFM69[C]W - Module without RF switch, Tx connected to PA0
    if(dBm > 13)
    {
        pa_val = RFM69_PALEVEL_PA0_ON | RFM69_PALEVEL_MAX;
    }
    else if(dBm < -18)
    {
        pa_val = RFM69_PALEVEL_PA0_ON;
    }
    else
    {
        pa_val = RFM69_PALEVEL_PA0_ON;
        pa_val |= (dBm + 18);
    }
#endif  //  RFM69_HW

    rfm69_writeReg(RFM69_REG_PALEVEL, pa_val);
}

// If key present, enable encription
static void rfm69_aes(uint8_t * aes)
{
    bool enable = false;
    uint8_t i, val = aes[0];

    // Check that key != (0x00,0x00..0x00) or (0xFF,0xFF..0xFF)
    if((val != 0) && (val != 0xFF))
    {
        enable = true;
    }
    else
    {
        for(i = 1; i < 16; i++)
        {
            if(aes[i] != val)
            {
                enable = true;
                break;
            }
        }
    }

    if(enable)
    {
        RFM69_SELECT();
        hal_spi_exch8(RFM69_USE_SPI, RFM69_REG_WRITE | RFM69_REG_AESKEY1);
        for(i = 0; i < 16; i++)
        {
            hal_spi_exch8(RFM69_USE_SPI, aes[i]);
        }
        RFM69_RELEASE();
        
        i = rfm69_readReg(RFM69_REG_PACKETCONFIG2);
        i |= RFM69_PACKET2_AES_ON;
        rfm69_writeReg(RFM69_REG_PACKETCONFIG2, i);
    }
}

// Read data from FIFO
static MQ_t * rfm69_getFIFO(void)
{
    MQ_t * pRxBuf = NULL;
    uint8_t i, frameLen;

    rfm69_standby();

    RFM69_SELECT();
    hal_spi_exch8(RFM69_USE_SPI, RFM69_REG_FIFO);

    frameLen = hal_spi_exch8(RFM69_USE_SPI, 0);                     // Read Frame Length
    if(frameLen >= 4)
    {
        pRxBuf = mqAlloc(sizeof(MQ_t));
        frameLen -= 2;
        pRxBuf->Length = frameLen;

        i = hal_spi_exch8(RFM69_USE_SPI, 0);                        // Read destination address
        pRxBuf->a.phy1addr[0] = hal_spi_exch8(RFM69_USE_SPI, 0);    // Read Source address

        for(i = 0; i < frameLen; i++)                               // Read Payload
        {
            pRxBuf->m.raw[i] = hal_spi_exch8(RFM69_USE_SPI, 0);
        }
    }
    RFM69_RELEASE();

    // Enter to RX State
    rfm69_writeReg(RFM69_REG_OPMODE, RFM69_OPMODE_RX);

    return pRxBuf;
}

#define RFM69_TX_RETRYS  5      // delay 15 - 50 ms

static void rfm69_tx_task(void)
{
    static uint8_t rfm69_tx_delay = 0;
    static uint8_t rfm69_tx_retry = RFM69_TX_RETRYS;

    // CDMA
    if(rfm69_tx_delay > 0)
    {
        static uint32_t rfm69_ms = 0;
        uint32_t act_ms = HAL_get_ms();
        if((act_ms - rfm69_ms) < rfm69_tx_delay)
        {
            return;
        }
        rfm69_ms = act_ms;
    }

    // Channel Busy ?
    if(rfm69_readReg(RFM69_REG_RSSIVALUE) < RFM69_REG_RSSITHRESH_DEF)
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

    // Prevent RX while filling the fifo
    rfm69_standby();

    // Fill Buffer
    uint8_t i, len;

    len = pTxBuf->Length;
    // Send burst
    RFM69_SELECT();
    hal_spi_exch8(RFM69_USE_SPI, RFM69_REG_WRITE | RFM69_REG_FIFO);
    hal_spi_exch8(RFM69_USE_SPI, len + 2);                      // Set data length at the
                                                                //  first position of the TX FIFO 
    hal_spi_exch8(RFM69_USE_SPI, pTxBuf->a.phy1addr[0]);        // Send destination address
    hal_spi_exch8(RFM69_USE_SPI, rfm69_NodeID);                 // Send Source address
    for(i = 0; i < len; i++)                                    // Send Payload
    {
        hal_spi_exch8(RFM69_USE_SPI, pTxBuf->m.raw[i]);
    }
    RFM69_RELEASE();

    rfm69_writeReg(RFM69_REG_DIOMAPPING1, RFM69_REG_DIOMAPPING1_TX);    // DIO0 - Packet Sent
    rfm69_writeReg(RFM69_REG_OPMODE, RFM69_OPMODE_TX);          // Switch to TX state
    rfm69_state = RFM69_TRV_TX;

    mqFree(pTxBuf);

}

// RFM69 API

// Init and configure Module
void RFM69_Init(void)
{
    uint8_t i;

    MQ_t * pBuf;
    while((pBuf = mqDequeue(&rfm69_tx_queue)) != NULL)
    {
        mqFree(pBuf);
    }

    // Init Hardware
    // Configure NSS Pin
    hal_dio_configure(RFM69_NSS_PIN, DIO_MODE_OUT_PP_HS);
    RFM69_RELEASE();

    // Configure IRQ Pin
    hal_dio_configure(RFM69_IRQ_PIN, DIO_MODE_IN_PD);

    // Configure SPI
    hal_spi_cfg(RFM69_USE_SPI, (HAL_SPI_MODE_0 | HAL_SPI_MSB | HAL_SPI_8B), 10000000UL);
    
    // Check connection
    i = 0;
    while(i != 0x55)
    {
        rfm69_writeReg(RFM69_REG_SYNCVALUE8, 0x55);
        i = rfm69_readReg(RFM69_REG_SYNCVALUE8);
    }
    
    while(i != 0xAA)
    {
        rfm69_writeReg(RFM69_REG_SYNCVALUE8, 0xAA);
        i = rfm69_readReg(RFM69_REG_SYNCVALUE8);
    }

    // Chip Mode - Standby
    rfm69_standby();

    // Start RC Calibration (Once at POR)
    rfm69_writeReg(RFM69_REG_OSC1, (rfm69_readReg(RFM69_REG_OSC1) | RFM69_OSC1_RCCAL_START));
    while((rfm69_readReg(RFM69_REG_OSC1) & RFM69_OSC1_RCCAL_DONE) == 0);

    // Load common settings
    for(i = 0; i<(sizeof(rfm69config)/2); i++)
    {
        rfm69_writeReg(rfm69config[i][0], rfm69config[i][1]);
    }

    // Load Config
    uint8_t     Channel;
    uint16_t    GroupID;
    uint8_t Len = sizeof(uint8_t);
    // Load Frequency channel
    ReadOD(objRFChannel, MQTTSN_FL_TOPICID_PREDEF, &Len, (uint8_t *)&Channel);
    rfm69_setFreq(Channel);
    // Set Power
    ReadOD(objRFpower, MQTTSN_FL_TOPICID_PREDEF, &Len, &i);
    rfm_setPWR(i);
    // Load Device ID
    ReadOD(objRFNodeId, MQTTSN_FL_TOPICID_PREDEF,  &Len, &rfm69_NodeID);
    rfm69_writeReg(RFM69_REG_NODEADRS, rfm69_NodeID);
    // Load Group ID(Synchro)
    Len = sizeof(uint16_t);
    ReadOD(objRFGroup, MQTTSN_FL_TOPICID_PREDEF,  &Len, (uint8_t *)&GroupID);
    rfm69_writeReg(RFM69_REG_SYNCVALUE3, GroupID >> 8);
    rfm69_writeReg(RFM69_REG_SYNCVALUE4, GroupID & 0xFF);
    // Load AES key
    uint8_t aes[16];
    Len = 16;
    ReadOD(objRFkey, MQTTSN_FL_TOPICID_PREDEF, &Len, aes);
    rfm69_aes(aes);

    rfm69_state = RFM69_TRV_SLEEP;
}

void RFM69_Send(void *pBuf)
{
    if(!mqEnqueue(&rfm69_tx_queue, pBuf))
    {
        mqFree(pBuf);
    }
}

void * RFM69_Get(void)
{
    if(RFM69_IRQ_STATE())
    {
        if(rfm69_state == RFM69_TRV_RX)
        {
            if(rfm69_readReg(RFM69_REG_IRQFLAGS2) & RFM69_IRQFLAGS2_PAYLOADREADY)
            {
                LED_On();
                rfm69_rssi = -(rfm69_readReg(RFM69_REG_RSSIVALUE) >> 1);
                return rfm69_getFIFO();
            }

            if((rfm69_readReg(RFM69_REG_IRQFLAGS1) & RFM69_IRQFLAGS1_TIMEOUT) == 0)
            {
                return NULL;
            }
        }
        else    // Tx - Packet Sent Interrupt
        {
            // set DIO0 to "RSSI detected" in receive mode
            rfm69_writeReg(RFM69_REG_DIOMAPPING1, RFM69_REG_DIOMAPPING1_RX);
        }

        rfm69_standby();
        rfm69_state = RFM69_TRV_SLEEP;
    }

    if(rfm69_state == RFM69_TRV_RX)
    {
        if(rfm69_tx_queue.Size != 0)
        {
            LED_On();
            rfm69_tx_task();
        }
    }
    else if(rfm69_state == RFM69_TRV_SLEEP)
    {
        // Enter to RX State
        rfm69_writeReg(RFM69_REG_OPMODE, RFM69_OPMODE_RX);
        rfm69_state = RFM69_TRV_RX;
    }

    return NULL;
}

void  * RFM69_GetAddr(void)
{
    return &rfm69_NodeID;
}

int8_t RFM69_GetRSSI(void)
{
    return rfm69_rssi;
}

#endif  //  RFM69_PHY
