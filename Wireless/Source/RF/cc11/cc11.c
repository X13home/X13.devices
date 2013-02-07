/*
Copyright (c) 2011-2012 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

// CC1101 RF Tranceiver

#include <util/delay.h>
#include "CC11_Reg.h"

#define RF_WAIT_LOW_MISO()  while(RF_PIN & (1<<RF_PIN_MISO))

#define CC11_RF_POWER   0x50

// Constants
static uint8_t              cc11s_Channel = 0xFF;
static uint16_t             cc11s_Group = 0;
static uint8_t              cc11s_NodeID = 0;
// ISR Variables
volatile static uint8_t     cc11v_State;        // Internal State
volatile static uint8_t   * cc11v_pRxBuf;       // Pointer to received data
#ifdef RF_USE_RSSI
static uint8_t              cc11v_RSSI;         // Actual RSSI
#endif  //  RF_USE_RSSI
// Pool variables
static uint8_t              cc11v_ChanBusy;
static uint8_t              cc11v_txHead;
static uint8_t              cc11v_txTail;
static uint8_t            * cc11v_pTxPool[RF_TX_POOL_SIZE];

const PROGMEM uint8_t cc11config[][2] =
{
    {CC11_IOCFG2,   CC11_GDO_DISABLE},  // GDO2 - High impedance (3-State)
    {CC11_IOCFG0,   CC11_GDO_SYNC},     // GDO0 - Asserts when sync word has been sent/received, and de-asserts at the end of the packet.
    {CC11_FIFOTHR,  0x47},              // ADC_RETENTION, RX Attenuation: 0 dB, FIFO Threshold 33/32 bytes 
    {CC11_PKTLEN,   0x3D},              // default packet length 61 byte
    {CC11_PKTCTRL1, 0x06},              // Append Status, Check Address and Broadcast
    {CC11_PKTCTRL0, 0x05},              // CRC calculation: enable, variable packet length
    {CC11_FSCTRL1,  0x06},              // IF = 152,3 kHz
    {CC11_FREQ2,    CC11_DEFVAL_FREQ2}, // Set default carrier frequency
    {CC11_FREQ1,    CC11_DEFVAL_FREQ1},
    {CC11_FREQ0,    CC11_DEFVAL_FREQ0},
    {CC11_MDMCFG4,  0xCA},              // RX filter BW 101,6 kHz
    {CC11_MDMCFG3,  0x83},              // Data Rate = 38,383 kBaud
    {CC11_MDMCFG2,  0x13},              // Sensitivity optimized, GFSK, sync word 30/32 bit detected
    {CC11_DEVIATN,  0x35},              // Deviation 20 kHz
    {CC11_MCSM0,    0x18},              // Automatically calibrate when going from IDLE to RX or TX,  PO_TIMEOUT: 150uS
    {CC11_FOCCFG,   0x16},              // Frequency offset compensation 67,5 kHz
    {CC11_AGCCTRL2, 0x43},              // The highest gain setting can not be used, Target amplitude from channel filter: 33 dB 
    {CC11_WORCTRL,  0xFB},
    {CC11_FSCAL3,   0xE9},
    {CC11_FSCAL2,   0x2A},
    {CC11_FSCAL1,   0x00},
    {CC11_FSCAL0,   0x1F},
    {CC11_TEST2,    0x81},
    {CC11_TEST1,    0x35},
    {CC11_TEST0,    0x09}
/* Fosc = 27MHz,868M  !!! Not tested
  {CC1101_FREQ2,       0x20},
  {CC1101_FREQ1,       0x25},
  {CC1101_FREQ0,       0xED},
  {CC1101_MDMCFG3,     0x75},
  {CC1101_MDMCFG0,     0xE2},
  {CC1101_DEVIATN,     0x34},
*/
};

// low level SPI exchange
static uint8_t cc11_spiExch(uint8_t data)
{
    RF_SPI_DATA = data;
    while(RF_SPI_BISY);             // Wait until SPI operation is terminated
    return RF_SPI_DATA;
}

// Send command strobe to the CC1101 IC via SPI
static void cc11_cmdStrobe(uint8_t cmd) 
{
    RF_SELECT();                        // Select CC1101
    RF_WAIT_LOW_MISO();                 // Wait until MISO goes low
    RF_SPI_DATA = cmd;                  // Send strobe command
    while(RF_SPI_BISY);                 // Wait until SPI operation is terminated
    RF_RELEASE();                       // Release CC1101
}

// Write single register into the CC1101 IC via SPI
static void cc11_writeReg(uint8_t Addr, uint8_t value) 
{
    RF_SELECT();                        // Select CC1101
    RF_WAIT_LOW_MISO();                 // Wait until MISO goes low
    RF_SPI_DATA = Addr;                 // Send register address
    while(RF_SPI_BISY);                 // Wait until SPI operation is terminated
    RF_SPI_DATA = value;                // Send value
    while(RF_SPI_BISY);                 // Wait until SPI operation is terminated
    RF_RELEASE();                       // Release CC1101
}

// Read single CC1101 register via SPI
static uint8_t cc11_readReg(uint8_t Addr)
{
    uint8_t retval;

    RF_SELECT();                        // Select CC1101
    RF_WAIT_LOW_MISO();                 // Wait until MISO goes low
    RF_SPI_DATA = Addr;                 // Send register address
    while(RF_SPI_BISY);                 // Wait until SPI operation is terminated
    // Read result
    retval = cc11_spiExch(0);
    RF_RELEASE();                       // Release CC1101
    return retval;
}

// Send data
static void cc11_send(volatile uint8_t * pBuf)
{
    TxLEDon();
    cc11_cmdStrobe(CC11_SIDLE);                     // Switch to Idle state
    cc11_cmdStrobe(CC11_SFTX);                      // Flush the TX FIFO buffer

    // Fill Buffer
    uint8_t i, len;
    uint8_t * ipBuf;
    len = ((MQ_t *)pBuf)->mq.Length;
    ipBuf = (uint8_t *)(&((MQ_t *)pBuf)->mq);
    // Send burst
    RF_SELECT();                                    // Select CC1101
    RF_WAIT_LOW_MISO();                             // Wait until MISO goes low
    RF_SPI_DATA = CC11_BIT_BURST | CC11_TXFIFO;
    while(RF_SPI_BISY);
    RF_SPI_DATA = len + 2;                          // Set data length at the first position of the TX FIFO
    while(RF_SPI_BISY);
    RF_SPI_DATA = ((MQ_t *)pBuf)->addr;             // Send destination addr
    while(RF_SPI_BISY);
    RF_SPI_DATA = cc11s_NodeID;                     // Send Source addr
    while(RF_SPI_BISY);
    for(i = 0; i < len; i++)                        // Send Payload
    {
        RF_SPI_DATA = ipBuf[i];
        while(RF_SPI_BISY);
    }
    RF_RELEASE();                                   // Release CC1101

    mqRelease((MQ_t *)pBuf);

    // CCA enabled: will enter TX state only if the channel is clear
    cc11_cmdStrobe(CC11_STX);
    cc11v_State = RF_TRVTXHDR;
}

ISR(RF_INT_vect)
{
    uint8_t marcs = cc11_readReg(CC11_MARCSTATE | CC11_STATUS_REGISTER);
    switch(cc11v_State)
    {
        case RF_TRVRXIDLE:
            if(marcs == CC11_MARCSTATE_RX)  // Synchro received
            {
                RxLEDon();
                cc11v_State = RF_TRVRXDATA;
                return;
            }
            break;
        case RF_TRVRXDATA:                  // Data received
            if(marcs == CC11_MARCSTATE_IDLE)
            {
                // read number of bytes in receive FIFO
                // Due a chip bug, the RXBYTES register must read the same value twice in a row to guarantee an accurate value.
                uint8_t frameLen, tmp, i;
                frameLen = cc11_readReg(CC11_RXBYTES | CC11_STATUS_REGISTER);
                do
                {
                    tmp = frameLen;
                    frameLen = cc11_readReg(CC11_RXBYTES | CC11_STATUS_REGISTER);
                }while (tmp != frameLen);

                if(((tmp & 0x7F) < 7) ||                        // Packet is too small
                    (tmp & 0x80) ||                             // or Overflow
                    (tmp > (MQTTS_MSG_SIZE + 3)))               // or Packet is too Big
                    break;
                frameLen -= 4;
                uint8_t * pTmp;
                pTmp = (uint8_t *)mqAssert();
                if(pTmp == NULL)                                // No Memory
                    break;
                    
                // Read Burst
                RF_SELECT();                                    // Select CC1101
                RF_WAIT_LOW_MISO();                             // Wait until MISO goes low
                RF_SPI_DATA = CC11_BIT_READ | CC11_BIT_BURST | CC11_RXFIFO;
                while(RF_SPI_BISY);
                cc11_spiExch(0);                                // Read Length
                cc11_spiExch(0);                                // Read Destination addr
                for(i = 0; i < frameLen; i++)                   // Read Payload
                {
                    tmp = cc11_spiExch(0);
                    pTmp[i] = tmp;
                }

                tmp = cc11_spiExch(0);                          // Read RSSI
                i = cc11_spiExch(0);                            // Read LQI 
                RF_RELEASE();                                   // Release CC1101
                
                if(i & CC11_LQI_CRC_OK)                         // is CRC Ok ?
                {
                    //cc11v_Foffs = cc11_readReg(CC11_FREQEST | CC11_STATUS_REGISTER);    // int8_t frequency offset
#ifdef RF_USE_RSSI
                    cc11v_RSSI = tmp;
#endif  //  RF_USE_RSSI
                    cc11v_pRxBuf = pTmp;
                }
                else
                    mqRelease((MQ_t *)pTmp);                    // Bad CRC
            }
            break;
        case RF_TRVTXHDR:
            if(marcs == CC11_MARCSTATE_TX)
            {
                cc11v_State = RF_TRVTXDATA;
                return;
            }
            break;
    }

    LEDsOff();
    cc11_cmdStrobe(CC11_SIDLE);     // Enter to the IDLE state
    cc11_cmdStrobe(CC11_SFTX);
    cc11_cmdStrobe(CC11_SFRX);
    cc11_cmdStrobe(CC11_SRX);       // Enter to RX State
    cc11v_State = RF_TRVRXIDLE;
}

// API
void rf_LoadCfg(uint8_t Channel, uint16_t Group, uint8_t NodeID)
{
    if((cc11s_Group != 0) && (cc11s_Group != Group))
    {
        cc11_writeReg(CC11_SYNC1, Group>>8);
        cc11_writeReg(CC11_SYNC0, Group & 0xFF);
    }
    cc11s_Group = Group;

    if((cc11s_NodeID != 0) && (cc11s_NodeID != NodeID))
        cc11_writeReg(CC11_ADDR, NodeID);
    cc11s_NodeID = NodeID;
    
    // !!!  Channel Space = 200 kHz not 25
    Channel >>= 3;

    if((cc11s_Channel != 0xFF) && (cc11s_Channel != Channel))
        cc11_writeReg(CC11_CHANNR, Channel);
    cc11s_Channel = Channel;
}

void rf_Initialize(void)
{
    // HW Initialise
    RF_DISABLE_IRQ();
    RF_PORT_INIT();                     // Ports Init
    RF_SPI_INIT();                      // init SPI controller
    RF_IRQ_CFG();                       // init IRQ input 
    // HW End
    // Reset CC1101
    _delay_us(5);
    RF_SELECT();
    _delay_us(10);
    RF_RELEASE();
    _delay_us(40);
    RF_SELECT();
    RF_WAIT_LOW_MISO();                 // Wait until MISO goes low
    RF_SPI_DATA = CC11_SRES;
    while(RF_SPI_BISY);                 // Wait until SPI operation is terminated
    RF_WAIT_LOW_MISO();                 // Wait until MISO goes low
    RF_RELEASE();

    // Configure CC1101
    uint8_t i;
    for (i=0; i<(sizeof(cc11config)/sizeof(cc11config[0])); i++)
      cc11_writeReg(pgm_read_byte(&cc11config[i][0]), pgm_read_byte(&cc11config[i][1]));

    // Load Group ID(Synchro)
    cc11_writeReg(CC11_SYNC1, cc11s_Group>>8);
    cc11_writeReg(CC11_SYNC0, cc11s_Group & 0xFF);
    // Load Device ID
    cc11_writeReg(CC11_ADDR, cc11s_NodeID);
    // Load Frequency channel
    cc11_writeReg(CC11_CHANNR, cc11s_Channel);
    
    // Configure PATABLE, No Ramp
    cc11_writeReg(CC11_PATABLE, CC11_RF_POWER);

    // Init Internal variables
    cc11v_State = RF_TRVIDLE;
    cc11v_pRxBuf = NULL;
    cc11v_txHead = 0;
    cc11v_txTail = 0;
    cc11v_ChanBusy = 0;

    RF_ENABLE_IRQ();                        // configure interrupt controller
}

// Change state
void rf_SetState(uint8_t state)
{
    if(state == RF_TRVASLEEP)
    {
        // We need to enter the IDLE state first
        cc11_cmdStrobe(CC11_SIDLE);
        // Enter Power-down state
        cc11_cmdStrobe(CC11_SPWD);
        cc11v_State = RF_TRVASLEEP;
    }
    else if(state == RF_TRVWKUP)
    {
        cc11_cmdStrobe(CC11_SIDLE);
        cc11v_State = RF_TRVIDLE;
    }
}

#ifdef RF_USE_RSSI
uint8_t rf_GetRSSI(void)
{
    return cc11v_RSSI;
}
#endif

uint8_t * rf_GetBuf(void)
{
    if(cc11v_pRxBuf == NULL)
        return NULL;

    uint8_t * pRet = (uint8_t *)cc11v_pRxBuf;
    cc11v_pRxBuf = NULL;
    return pRet;
}

uint8_t rf_GetNodeID(void)
{
    return cc11s_NodeID;
}

void rf_Send(uint8_t * pBuf)
{
    if((cc11v_txTail == cc11v_txHead) &&                                            // Buffer is empty
       (cc11v_State == RF_TRVRXIDLE) &&                                             // State is RxIdle
       (cc11_readReg(CC11_PKTSTATUS | CC11_STATUS_REGISTER) & CC11_PKTSTATUS_CCA))  // Channel Clear
    {
        cc11_send(pBuf);
    }
    else
    {
        uint8_t tmpHead = cc11v_txHead + 1;
        if(tmpHead >= RF_TX_POOL_SIZE)
            tmpHead -= RF_TX_POOL_SIZE;
        if(tmpHead == cc11v_txTail)                          // Overflow, packet droped
            mqRelease((MQ_t *)pBuf);
        else
        {
            cc11v_pTxPool[cc11v_txHead] = pBuf;
            cc11v_txHead = tmpHead;
        }
    }
}

void rf_Pool(void)
{
    if((cc11v_State == RF_TRVRXIDLE) && (cc11v_txTail != cc11v_txHead)) // Send Buffer not empty
    {
        if(cc11v_ChanBusy)
        {
            cc11v_ChanBusy--;
        }
        else if(!(cc11_readReg(CC11_PKTSTATUS | CC11_STATUS_REGISTER) & CC11_PKTSTATUS_CCA))
        {
            cc11v_ChanBusy = cc11s_NodeID>>4;
        }
        else
        {
            cc11_send(cc11v_pTxPool[cc11v_txTail]);
            if(++cc11v_txTail >= RF_TX_POOL_SIZE)
                cc11v_txTail -= RF_TX_POOL_SIZE;
        }
    }
    else if(cc11v_State == RF_TRVIDLE)
    {
        cc11_cmdStrobe(CC11_SIDLE);     // Enter to the IDLE state
        cc11_cmdStrobe(CC11_SFTX);
        cc11_cmdStrobe(CC11_SFRX);
        cc11_cmdStrobe(CC11_SRX);       // Enter to RX State
        cc11v_State = RF_TRVRXIDLE;
    }
}
