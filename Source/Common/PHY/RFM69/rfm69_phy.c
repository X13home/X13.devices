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

#define RFM69_FXO       32000000UL
#define RFM69_BAUD      38400U          // (BitRate < 2 * RxBw)
#define RFM68_FSTEP     (RFM69_FXO/524288UL)
#define RFM69_FDEV      20000U          // (FDEV + BitRate / 2 <= 500KHz)
// FSK Mode: RxBw = FXOSC / (RxBwMant * 2^(RxBwExp + 2))
// RX filter BW 100 KHz = 32M /(20 * 2^(2 + 2))
#define RFM69_RXBW      (RFM69_RXBW_MANT_20 | RFM69_RXBW_EXP_2)

// RF States
typedef enum
{
    RFM69_TRV_SLEEP = 0,    // Sleep mode, all blocks disabled
    RFM69_TRV_IDLE,
    RFM69_TRV_RXIDLE,
    RFM69_TRV_RXDONE,
    RFM69_TRV_TXIDLE,
    RFM69_TRV_TXDONE
}RFM69_TRVSTATE_t;

static const uint8_t rfm69config[][2] = {
    // Data operation mode and Modulation settings
    {RFM69_REG_DATAMODUL,   RFM69_DATAMODUL_DATAMODE_PACKET |               // Packet mode
                            RFM69_DATAMODUL_MODULATIONTYPE_FSK |            // Modulation - FSK
                            RFM69_DATAMODUL_MODULATIONSHAPING_0},           // No shaping
    // Bit Rate setting
    {RFM69_REG_BITRATEMSB,  ((RFM69_FXO / RFM69_BAUD) >> 8)},
    {RFM69_REG_BITRATELSB,  ((RFM69_FXO / RFM69_BAUD) & 0xFF)},
    // Frequency Deviation setting
    {RFM69_REG_FDEVMSB,     ((RFM69_FDEV / RFM68_FSTEP) >> 8)},
    {RFM69_REG_FDEVLSB,     ((RFM69_FDEV / RFM68_FSTEP) & 0xFF)},
    // Channel Filter BW Control
    {RFM69_REG_RXBW,        RFM69_RXBW_DCCFREQ_2 |                  // Recommended Cut-off frequency
                            RFM69_RXBW},
    // Mapping of pins DIO0 to DIO3
    {RFM69_REG_DIOMAPPING1, RFM69_DIOMAPPING1_DIO0_1},              // DIO0 - PayloadReady / TxReady
    // Mapping of pins DIO4 and DIO5, ClkOut frequency
    {RFM69_REG_DIOMAPPING2, RFM69_DIOMAPPING2_CLKOUT_OFF},          // ClkOut Disable
    // Status register: FIFO handling flags...
    {RFM69_REG_IRQFLAGS2,   RFM69_IRQFLAGS2_FIFOOVERRUN},           // Clear FIFO overrun flag
    // RSSI Threshold control, Val = -THdbm * 2
    {RFM69_REG_RSSITHRESH, 220},                                    // -110 dBm
    // Sync Word Recognition control
    {RFM69_REG_SYNCCONFIG,  RFM69_SYNC |                            // Enable Sync
                            RFM69_SYNC_SIZE_2},                     // 2 bytes, tolerance 0
    // Packet mode settings
    {RFM69_REG_PACKETCONFIG1, RFM69_PACKET1_FORMAT_VARIABLE |       // Packet variable length
                              RFM69_PACKET1_DCFREE_OFF |            // without encoding
                              RFM69_PACKET1_CRC_ON |                // Enable CRC
                              // On CRC error, clear FIFO and restart new packet reception
                              RFM69_PACKET1_CRCAUTOCLEAR_ON |
                              // Address field must match NodeAddress or BroadcastAddress
                              RFM69_PACKET1_ADRSFILTERING_NODEBROADCAST},
    // Payload length setting, max length in Rx
    {RFM69_REG_PAYLOADLENGTH, sizeof(MQ_t)},
    // Broadcast address
    {RFM69_REG_BROADCASTADRS, 0},
    // Fifo threshold, Tx start condition
    // Start Transmission if FIFO not empty
    {RFM69_REG_FIFOTHRESH,  RFM69_FIFOTHRESH_TXSTART_FIFONOTEMPTY | RFM69_FIFOTHRESH_VALUE},
    // Packet mode settings
    //  Rx automatically restarted after InterPacketRxDelay.
    {RFM69_REG_PACKETCONFIG2, RFM69_PACKET2_RXRESTARTDELAY_2BITS |
                              RFM69_PACKET2_AUTORXRESTART_ON |      // ?
                              RFM69_PACKET2_AES_OFF},
    // run DAGC continuously in RX mode for Fading Margin Improvement, 
    // recommended default for AfcLowBetaOn=0
    {RFM69_REG_TESTDAGC, RFM69_DAGC_IMPROVED_LOWBETA0}
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

// Channel space - 5KHz
static void rfm69_setFreq(uint16_t chan)
{
    // More accurate calculation
    int64_t frf;

    int32_t offset = chan & 0x7FFF;
    offset *= 5000U;

    frf = RFM69_FRQ;

    if(chan & 0x8000U)
    {
        frf -= offset;
    }
    else
    {
        frf += offset;
    }

    if((frf < RFM69_FRQ_MIN) || (frf >= RFM69_FRQ_MAX))
    {
        frf = RFM69_FRQ;
    }

    frf *= 524288UL;        // 2^19
    frf /= RFM69_FXO;

    RFM69_SELECT();
    hal_spi_exch8(RFM69_USE_SPI, 0x80 | RFM69_REG_FRFMSB);
    hal_spi_exch8(RFM69_USE_SPI, (frf >> 16) & 0xFF);
    hal_spi_exch8(RFM69_USE_SPI, (frf >> 8) & 0xFF);
    hal_spi_exch8(RFM69_USE_SPI, frf & 0xFF);
    RFM69_RELEASE();
}

void RFM69_Init(void)
{
    uint16_t     Channel;
    uint16_t    GroupID;

    MQ_t * pBuf;
    while((pBuf = mqDequeue(&rfm69_tx_queue)) != NULL)
    {
        mqFree(pBuf);
    }

    // Load Device ID
    uint8_t Len = sizeof(uint8_t);
    ReadOD(objRFNodeId, MQTTSN_FL_TOPICID_PREDEF,  &Len, &rfm69_NodeID);
    Len = sizeof(uint16_t);
    // Load Frequency channel
    ReadOD(objRFChannel, MQTTSN_FL_TOPICID_PREDEF, &Len, (uint8_t *)&Channel);
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

    // Start RC Calibration
    rfm69_writeReg(RFM69_REG_OSC1, (rfm69_readReg(RFM69_REG_OSC1) | RFM69_OSC1_RCCAL_START));
    while((rfm69_readReg(RFM69_REG_OSC1) & RFM69_OSC1_RCCAL_DONE) == 0);

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

    rmm69_setMode(RFM69_OPMODE_SLEEP);
    rfm69_state = RFM69_TRV_SLEEP;
}

void RFM69_Send(void *pBuf)
{
    // ToDo Dummy
    mqFree(pBuf);
}

void  * RFM69_Get(void)
{
    // ToDo Dummy
    return NULL;
}

void  * RFM69_GetAddr(void)
{
    return &rfm69_NodeID;
}

#endif  //  RFM69_PHY
