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
// RX filter BW 100 kHz = 32M /(20 * 2^(2 + 2))
#define RFM69_RXBW      (RFM69_RXBW_MANT_20 | RFM69_RXBW_EXP_2)


#define RFM69_RFBASE    868000000UL

// Freq:
// 290-340  - 315
// 424-510  - 433
// 862-890  - 868
// 890-1020 - 915

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
    // RF Carrier Frequency
    // {RFM69_REG_FRFMSB,      ((RF_BASE_FREQ / RFM68_FSTEP) >> 16)},
    // {RFM69_REG_FRFMID,     (((RF_BASE_FREQ / RFM68_FSTEP) >> 8) & 0xFF)},
    // {RFM69_REG_FRFLSB,      ((RF_BASE_FREQ / RFM68_FSTEP) & 0xFF)},
    // RC Oscillators Settings
    // {RFM69_REG_OSC1,
    // AFC control in low modulation index situations
    // {RFM69_REG_AFCCTRL
    // Listen Mode settings
    // {RFM69_REG_LISTEN1
    // Listen Mode Idle duration
    // {RFM69_REG_LISTEN2
    // Listen Mode Rx duration
    // {RFM69_REG_LISTEN3
    // PA selection and Output Power control
    // {RFM69_REG_PALEVEL
    // Control of the PA ramp time in FSK mode
    // {RFM69_REG_PARAMP
    // Over Current Protection control
    // {RFM69_REG_OCP
    // LNA settings
    // {RFM69_REG_LNA
    // Channel Filter BW Control
    {RFM69_REG_RXBW,        RFM69_RXBW_DCCFREQ_2 |                  // Recommended Cut-off frequency
                            RFM69_RXBW},
    // Channel Filter BW control during the AFC routine
    // {RFM69_REG_AFCBW,
    // OOK demodulator selection and control in peak mode
    // {RFM69_REG_OOKPEAK
    // Average threshold control of the OOK demodulator
    // {RFM69_REG_OOKAVG
    // Fixed threshold control of the OOK demodulator
    // {RFM69_REG_OOKFIX
    // AFC and FEI control and status
    // {RFM69_REG_AFCFEI
    // frequency correction of the AFC
    // RSSI-related settings
    // {RFM69_REG_RSSICONFIG
    // Mapping of pins DIO0 to DIO3
    {RFM69_REG_DIOMAPPING1, RFM69_DIOMAPPING1_DIO0_1},              // DIO0 - PayloadReady / TxReady
    // Mapping of pins DIO4 and DIO5, ClkOut frequency
    {RFM69_REG_DIOMAPPING2, RFM69_DIOMAPPING2_CLKOUT_OFF},          // ClkOut Disable
    // Status register: FIFO handling flags...
    {RFM69_REG_IRQFLAGS2,   RFM69_IRQFLAGS2_FIFOOVERRUN},           // Clear FIFO overrun flag
    // RSSI Threshold control
    {RFM69_REG_RSSITHRESH, 220},                                    // -110 dBm
    // Timeout duration between Rx request and RSSI detection
    //{RFM69_REG_RXTIMEOUT1
    // Timeout duration between RSSI detection and PayloadReady
    //{RFM69_REG_RXTIMEOUT2
    // Preamble length, default value - 0x03
    //{RFM69_REG_PREAMBLEMSB
    //{RFM69_REG_PREAMBLELSB
    // Sync Word Recognition control
    {RFM69_REG_SYNCCONFIG,  RFM69_SYNC |                            // Enable Sync
                            RFM69_SYNC_SIZE_2},                     // 2 bytes, tolerance 0
    // Sync Word bytes, 1 through 8
    //{RFM69_REG_SYNCVALUE1
    //{RFM69_REG_SYNCVALUE8
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
    // Node address
    //{RFM69_REG_NODEADRS
    // Broadcast address
    {RFM69_REG_BROADCASTADRS, 0},
    // Auto modes settings
    //{RFM69_REG_AUTOMODES
    // Fifo threshold, Tx start condition
    // Start Transmission if FIFO not empty
    {RFM69_REG_FIFOTHRESH,  RFM69_FIFOTHRESH_TXSTART_FIFONOTEMPTY | RFM69_FIFOTHRESH_VALUE},
    // Packet mode settings
    //  Rx automatically restarted after InterPacketRxDelay.
    {RFM69_REG_PACKETCONFIG2, RFM69_PACKET2_RXRESTARTDELAY_2BITS |
                              RFM69_PACKET2_AUTORXRESTART_ON |      // ?
                              RFM69_PACKET2_AES_OFF},
    // 16 bytes of the cypher key
    //{RFM69_REG_AESKEY1
    //{RFM69_REG_AESKEY16
    // run DAGC continuously in RX mode for Fading Margin Improvement, 
    // recommended default for AfcLowBetaOn=0
    {RFM69_REG_TESTDAGC, RFM69_DAGC_IMPROVED_LOWBETA0}
};

static uint8_t          rfm69_NodeID;
static Queue_t          rfm69_tx_queue = {NULL, NULL, 4, 0};

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

static void rfm69_setFreq(uint8_t chan)
{
    // More accurate calculation
    int64_t frf;
    int32_t offset = chan & 0x7F;
    offset *= 25000;

    frf = RFM69_RFBASE;

    if(chan & 0x80)
    {
        frf -= offset;
    }
    else
    {
        frf += offset;
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
    ReadOD(objRFChannel, MQTTSN_FL_TOPICID_PREDEF, &Len, &Channel);
    // Load Group ID(Synchro)
    Len = sizeof(uint16_t);
    ReadOD(objRFGroup, MQTTSN_FL_TOPICID_PREDEF,  &Len, (uint8_t *)&GroupID);

    // Init Hardware

    // Configure NSS Pin
    hal_dio_configure(RFM69_NSS_PIN, DIO_MODE_OUT_PP_HS);
    RFM69_RELEASE();

    // Configure IRQ Pin
    hal_dio_configure(RFM69_IRQ_PIN, DIO_MODE_IN_PU);

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
