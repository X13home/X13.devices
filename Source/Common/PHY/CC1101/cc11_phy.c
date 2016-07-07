/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// CC11xx RF interface

#include "../../config.h"

#ifdef CC11_PHY

//#define CC11_ANAREN

#include "cc11_reg.h"

#ifndef CC11_ANAREN     // Fosc = 26M

// 433 MHz
#if (RF_BASE_FREQ > 433050000UL) && (RF_BASE_FREQ < 434790000UL)
#define CC11_DEFVAL_FREQ2       0x10
#define CC11_DEFVAL_FREQ1       0xA7
#define CC11_DEFVAL_FREQ0       0x62
// 868 MHz
#elif (RF_BASE_FREQ > 868000000UL) && (RF_BASE_FREQ < 870000000UL)
#define CC11_DEFVAL_FREQ2       0x21
#define CC11_DEFVAL_FREQ1       0x62
#define CC11_DEFVAL_FREQ0       0x76
#endif  // RF_BASE_FREQ

#define CC11_MDMCFG3_VAL        0x83    // Data Rate = 38,383 kBaud
#define CC11_DEVIATN_VAL        0x33    // Deviation 17,46 kHz

#else   // Fosc = 27M

// 433 MHz
#if (RF_BASE_FREQ > 433050000UL) && (RF_BASE_FREQ < 434790000UL)
    #define CC11_DEFVAL_FREQ2       0x10
    #define CC11_DEFVAL_FREQ1       0x09
    #define CC11_DEFVAL_FREQ0       0x7B
// 868 MHz
#elif (RF_BASE_FREQ > 868000000UL) && (RF_BASE_FREQ < 870000000UL)
    #define CC11_DEFVAL_FREQ2       0x20
    #define CC11_DEFVAL_FREQ1       0x25
    #define CC11_DEFVAL_FREQ0       0xED
#endif  //  RF_BASE_FREQ

#define CC11_MDMCFG3_VAL        0x75    // Data Rate = 38,4178 kBaud
#define CC11_DEVIATN_VAL        0x32    // Deviation 16,5 kHz

#endif  //  CC11_ANAREN

#if (CC11_PHY == 1)
    #define CC11_RF_POWER           0x50
#elif (CC11_PHY == 2)
    #define CC11_RF_POWER           0xC0
#else
    #error CC11_PHY unknown inteface
#endif  // CC11_PHY

static const uint8_t cc11config[][2] =
{
    {CC11_IOCFG0,   CC11_GDO_DISABLE},  // High impedance (3-State)
    {CC11_IOCFG2,   CC11_GDO_DISABLE},  // High impedance (3-State)
    {CC11_FIFOTHR,  0x47},              // ADC_RETENTION, RX Attenuation: 0 dB, 
                                        // FIFO Threshold 33/32 bytes 
    {CC11_PKTLEN,   0x3D},              // default packet length 61 byte
    {CC11_PKTCTRL1, 0x06},              // Append Status, Check Address and Broadcast
    {CC11_PKTCTRL0, 0x05},              // CRC calculation: enable, variable packet length
    {CC11_FSCTRL1,  0x08},              // IF = 100 kHz
    {CC11_FREQ2,    CC11_DEFVAL_FREQ2}, // Set default carrier frequency
    {CC11_FREQ1,    CC11_DEFVAL_FREQ1},
    {CC11_FREQ0,    CC11_DEFVAL_FREQ0},
    {CC11_MDMCFG4,  0xCA},              // RX filter BW 101,6 kHz
    {CC11_MDMCFG3,  CC11_MDMCFG3_VAL},  // Data Rate
    {CC11_MDMCFG2,  0x93},              // Current optimized, GFSK, sync word 30/32 bit detected
    {CC11_MDMCFG1,  0x00},              // Channel spacing 25 kHz
    {CC11_MDMCFG0,  0x00},
    {CC11_DEVIATN,  CC11_DEVIATN_VAL},  // Deviation 20 kHz
    {CC11_MCSM0,    0x18},              // Automatically calibrate when going from IDLE to RX or TX,
                                        //      PO_TIMEOUT: 150uS
    {CC11_FOCCFG,   0x16},              // Frequency offset compensation 67,5 kHz
    {CC11_AGCCTRL2, 0x43},              // The highest gain setting can not be used,
                                        //  Target amplitude from channel filter: 33 dB 
    {CC11_WORCTRL,  0xFB},
    {CC11_FSCAL3,   0xE9},
    {CC11_FSCAL2,   0x2A},
    {CC11_FSCAL1,   0x00},
    {CC11_FSCAL0,   0x1F},
    {CC11_TEST2,    0x81},
    {CC11_TEST1,    0x35},
    {CC11_TEST0,    0x09}
};

static uint8_t          cc11s_NodeID;
static Queue_t          cc11_tx_queue = {NULL, NULL, 4, 0};
static uint8_t          cc11_rssi;

// Send command strobe to the CC1101 IC via SPI
static void cc11_cmdStrobe(uint8_t cmd) 
{
    CC11_SELECT();                      // Select CC1101
    CC11_WAIT_LOW_MISO();               // Wait until MISO goes low
    hal_spi_exch8(CC11_USE_SPI, cmd);
    CC11_RELEASE();                     // Release CC1101
}

// Write single register into the CC1101 IC via SPI
static void cc11_writeReg(uint8_t Addr, uint8_t value) 
{
    CC11_SELECT();                      // Select CC1101
    CC11_WAIT_LOW_MISO();               // Wait until MISO goes low
    hal_spi_exch8(CC11_USE_SPI, Addr);  // Send register address
    hal_spi_exch8(CC11_USE_SPI, value); // Send value
    CC11_RELEASE();                     // Release CC1101
}

// Read single CC1101 register via SPI
static uint8_t cc11_readReg(uint8_t Addr)
{
    uint8_t retval;

    CC11_SELECT();                      // Select CC1101
    CC11_WAIT_LOW_MISO();               // Wait until MISO goes low
    hal_spi_exch8(CC11_USE_SPI, Addr);  // Send register address
    // Read result
    retval = hal_spi_exch8(CC11_USE_SPI, 0);
    CC11_RELEASE();                     // Release CC1101
    return retval;
}

#define CC11_TX_RETRYS  5   // delay 15 - 50 ms

static void cc11_tx_task(void)
{
    static uint8_t cc11_tx_delay = 0;
    static uint8_t cc11_tx_retry = CC11_TX_RETRYS;

#ifdef LED_On
    LED_On();
#endif  //  LED_On
    
    // CDMA
    if(cc11_tx_delay > 0)
    {
        static uint8_t cc11_ms = 0;
        uint8_t act_ms = HAL_get_ms() & 0xFF;

        if(cc11_ms == act_ms)
        {
            return;
        }
        cc11_ms = act_ms;
        cc11_tx_delay--;
        return;
    }

    // Channel Busy ?
    if((cc11_readReg(CC11_PKTSTATUS | CC11_STATUS_REGISTER) & CC11_PKTSTATUS_CCA) == 0)
    {
        if(cc11_tx_retry > 0)
        {
            cc11_tx_retry--;
            cc11_tx_delay = (HAL_RNG() & 0x7) + 3;
            return;
        }
        //cc11_cmdStrobe(CC11_SIDLE);     // Enter to the IDLE state
        //cc11_cmdStrobe(CC11_SFRX);
    }

    cc11_tx_delay = 0;
    cc11_tx_retry = CC11_TX_RETRYS;

    MQ_t * pTxBuf = mqDequeue(&cc11_tx_queue);
    if(pTxBuf == NULL)      // Queue Busy
    {
        return;
    }

    // Fill Buffer
    uint8_t i, len;

    len = pTxBuf->Length;
    // Send burst
    CC11_SELECT();                                              // Select CC1101
    CC11_WAIT_LOW_MISO();                                       // Wait until MISO goes low
    hal_spi_exch8(CC11_USE_SPI, (CC11_BIT_BURST | CC11_TXFIFO));
    hal_spi_exch8(CC11_USE_SPI, len + 2);                       // Set data length at the
                                                                //  first position of the TX FIFO
    hal_spi_exch8(CC11_USE_SPI, pTxBuf->a.phy1addr[0]);         // Send destination address
    hal_spi_exch8(CC11_USE_SPI, cc11s_NodeID);                  // Send Source address
    for(i = 0; i < len; i++)                                    // Send Payload
    {
        hal_spi_exch8(CC11_USE_SPI, pTxBuf->m.raw[i]);
    }
    CC11_RELEASE();                                             // Release CC1101

    mqFree(pTxBuf);

    cc11_cmdStrobe(CC11_STX);                                   // Switch to TX state
}

static MQ_t * cc11_rx_task(void)
{
    MQ_t * pRxBuf;
    
#ifdef LED_On
    LED_On();
#endif  //  LED_On

    // read number of bytes in receive FIFO
    // Due a chip bug, the RXBYTES register must read the same value twice in a row 
    //      to guarantee an accurate value.
    uint8_t frameLen = 0xFF, tmp, i = 16;
    do
    {
        tmp = frameLen;
        frameLen = cc11_readReg(CC11_RXBYTES | CC11_STATUS_REGISTER);
        i--;
    }while((tmp != frameLen) && (i != 0));

    if((i == 0) ||                                      // Data invalid
       (tmp & 0x80) ||                                  // or Overflow
       (tmp < 7) ||                                     // Packet is too small
       (tmp > (MQTTSN_MSG_SIZE + 2)))                   // or Packet is too Big
    {
        cc11_cmdStrobe(CC11_SFRX);      // Clear RX Buffer
        cc11_cmdStrobe(CC11_SRX);       // Enter to RX State
        return NULL;
    }
    
    pRxBuf = mqAlloc(sizeof(MQ_t));
    frameLen -= 5;
    pRxBuf->Length = frameLen;

    // Read Burst
    CC11_SELECT();                                                      // Select CC1101
    CC11_WAIT_LOW_MISO();                                               // Wait until MISO goes low
    hal_spi_exch8(CC11_USE_SPI, (CC11_BIT_READ | CC11_BIT_BURST | CC11_RXFIFO));
    hal_spi_exch8(CC11_USE_SPI, 0);                                     // Read Length
    hal_spi_exch8(CC11_USE_SPI, 0);                                     // Read Destination address
    pRxBuf->a.phy1addr[0] = hal_spi_exch8(CC11_USE_SPI, 0);             // Read Source address

    for(i = 0; i < frameLen; i++)                                       // Read Payload
    {
        pRxBuf->m.raw[i] = hal_spi_exch8(CC11_USE_SPI, 0);
    }

    cc11_rssi = hal_spi_exch8(CC11_USE_SPI, 0);                         // Read RSSI
    tmp  = hal_spi_exch8(CC11_USE_SPI, 0);                              // Read LQI 
    CC11_RELEASE();                                                     // Release CC1101

    //int8_t Foffs = cc11_readReg(CC11_FREQEST | CC11_STATUS_REGISTER);   // int8_t frequency offset

    if((tmp & CC11_LQI_CRC_OK) == 0)
    {
        mqFree(pRxBuf);
        pRxBuf = NULL;
    }

    cc11_cmdStrobe(CC11_SFRX);
    cc11_cmdStrobe(CC11_SRX);       // Enter to RX State

    return pRxBuf;
}

void CC11_Init(void)
{
    uint8_t     Channel;
    uint16_t    GroupID;
    
    MQ_t * pBuf;
    while((pBuf = mqDequeue(&cc11_tx_queue)) != NULL)
    {
        mqFree(pBuf);
    }

    // Load Device ID
    uint8_t Len = sizeof(uint8_t);
    ReadOD(objRFNodeId, MQTTSN_FL_TOPICID_PREDEF,  &Len, &cc11s_NodeID);
    // Load Frequency channel
    ReadOD(objRFChannel, MQTTSN_FL_TOPICID_PREDEF, &Len, &Channel);
    // Load Group ID(Synchro)
    Len = sizeof(uint16_t);
    ReadOD(objRFGroup, MQTTSN_FL_TOPICID_PREDEF,  &Len, (uint8_t *)&GroupID);

    // Init Hardware
    hal_dio_configure(CC11_NSS_PIN, DIO_MODE_OUT_PP_HS);
    CC11_RELEASE();
    hal_spi_cfg(CC11_USE_SPI, (HAL_SPI_MODE_0 | HAL_SPI_MSB | HAL_SPI_8B), 6500000UL);

    // Reset CC1101
    _delay_us(5);
    CC11_SELECT();
    _delay_us(10);
    CC11_RELEASE();
    _delay_us(40);
    CC11_SELECT();
    CC11_WAIT_LOW_MISO();                   // Wait until MISO goes low
    hal_spi_exch8(CC11_USE_SPI, CC11_SRES); // Reset CC1101 chip
    CC11_WAIT_LOW_MISO();                   // Wait until MISO goes low
    CC11_RELEASE();

    // verify that SPI is working and the correct radio is installed

    uint8_t pn = 0xFF, vers = 0xFF;
    uint16_t cnt = 0;
    
    while((pn != 0) && (vers < 3))
    {
        if((cnt & 0xFF00) == 0)
        {
            pn = cc11_readReg(CC11_PARTNUM | CC11_STATUS_REGISTER);
            vers = cc11_readReg(CC11_VERSION | CC11_STATUS_REGISTER);
        }
        cnt++;
    }

    // Configure CC1101
    uint8_t i;
    for (i=0; i<(sizeof(cc11config)/sizeof(cc11config[0])); i++)
    {
        cc11_writeReg(cc11config[i][0], cc11config[i][1]);
    }

    // Load Device ID
    cc11_writeReg(CC11_ADDR, cc11s_NodeID);
    // Load Group ID(Synchro)
    cc11_writeReg(CC11_SYNC1, GroupID>>8);
    cc11_writeReg(CC11_SYNC0, GroupID & 0xFF);
    // Load Frequency channel
    cc11_writeReg(CC11_CHANNR, Channel);
    // Configure PATABLE, No Ramp
    cc11_writeReg(CC11_PATABLE, CC11_RF_POWER);
}

void CC11_Send(void *pBuf)
{
    if(!mqEnqueue(&cc11_tx_queue, pBuf))
    {
        mqFree(pBuf);
    }
}

void * CC11_Get(void)
{
    uint8_t marcs = cc11_readReg(CC11_MARCSTATE | CC11_STATUS_REGISTER);
    
    if(marcs == CC11_MARCSTATE_IDLE)
    {
        if(cc11_readReg(CC11_PKTSTATUS | CC11_STATUS_REGISTER) & CC11_PKTSTATUS_CRC_OK)
        {
            return cc11_rx_task();
        }

        cc11_cmdStrobe(CC11_SFTX);          // Clear TX Buffer
        cc11_cmdStrobe(CC11_SFRX);          // Clear RX Buffer
        cc11_cmdStrobe(CC11_SRX);           // Enter to RX State
    }
    else if((marcs == CC11_MARCSTATE_RX) && (cc11_tx_queue.Size != 0))
    {
        cc11_tx_task();
    }

    return NULL;
}

#if ((CC11_PHY == 2) && (defined PHY2_GetRSSI))
uint8_t CC11_GetRSSI(void)
{
    if(cc11_rssi & 0x80)
    {
        cc11_rssi = ~cc11_rssi;
        cc11_rssi++;
        cc11_rssi >>= 1;
        cc11_rssi += 74;
    }
    else
    {
        cc11_rssi >>= 1;
        cc11_rssi = 74 - cc11_rssi;
    }
    return (~cc11_rssi + 1);
}
#endif  // GetRSSI

void * CC11_GetAddr(void)
{
    return &cc11s_NodeID;
}

#ifdef ASLEEP
void CC11_ASleep(void)
{
    // We need to enter the IDLE state first
    cc11_cmdStrobe(CC11_SIDLE);
    // Enter Power-down state
    cc11_cmdStrobe(CC11_SPWD);
}

void CC11_AWake(void)
{
    cc11_cmdStrobe(CC11_SIDLE);
}
#endif  //  ASLEEP

#endif  //  CC11_PHY
