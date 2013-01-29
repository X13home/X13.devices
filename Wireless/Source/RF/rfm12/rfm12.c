/*
Copyright (c) 2011-2012 <comparator@gmx.de>

This file is part of the X13.Home project.
https://github.com/X13home

BSD License
See LICENSE.txt file for license details.
*/

// MRF49XA/RFM12 RF Tranceiver

// Constants
static uint16_t             rfm12s_Channel = 0;
static uint16_t             rfm12s_Group = 0;
static uint8_t              rfm12s_NodeID = 0;

// ISR Section
volatile static uint8_t     rfm12v_RfLen;       // Packet Length
volatile static uint16_t    rfm12v_RfCRC;       // actual CRC
volatile static uint8_t   * rfm12v_pRfBuf;      // RF buffer
volatile static uint8_t     rfm12v_Pos;         // Position

// Process variables
volatile static uint8_t     rfm12v_State;       // Actual Status
    
// Pool variables
volatile static uint8_t     rfm12v_rxHead;
static uint8_t              rfm12v_rxTail;
volatile static uint8_t   * rfm12v_pRxBuf[RF_RX_BUF_SIZE];

static uint8_t              rfm12v_ChanBusy;
static uint8_t              rfm12v_txHead;
static uint8_t              rfm12v_txTail;
static uint8_t            * rfm12v_pTxPool[RF_TX_POOL_SIZE];

#ifdef RF_USE_RSSI
extern void ai_Sel_RSSI(void);
extern uint8_t ai_Get_RSSI(void);
#endif  //  RF_USE_RSSI

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

// Send Command( fast)
static void rfm12_control(uint16_t cmd)
{
    RF_SELECT();
    RF_SPI_DATA = cmd >> 8;
    while(RF_SPI_BISY);
    RF_SPI_DATA = cmd & 0xFF;
    while(RF_SPI_BISY);
    RF_RELEASE();
}

// Get Status
static uint16_t rfm12_get_status(void)
{
    uint16_t reply;
    RF_SELECT();
    RF_SPI_DATA = 0;
    while(RF_SPI_BISY);
    reply = RF_SPI_DATA<<8;
    RF_SPI_DATA = 0;
    while(RF_SPI_BISY);
    reply |= RF_SPI_DATA;
    RF_RELEASE();
    return reply;
}

// Load byte from RX FIFO
static uint8_t rfm12_get_fifo(void)
{
    uint8_t data;
    RF_SPI_SLOW();
    RF_SELECT();
    RF_SPI_DATA = (RFM12_CMD_READ>>8);
    while(RF_SPI_BISY);
    RF_SPI_DATA = (RFM12_CMD_READ & 0xFF);
    while(RF_SPI_BISY);
    data = RF_SPI_DATA;
    RF_RELEASE();
    RF_SPI_FAST();
    return data;
}

// Switch to TX State
static void rfm12_send(volatile uint8_t * pBuf)
{
    TxLEDon();
    rfm12_control(RFM12_IDLE_MODE);
    rfm12_control(RFM12_TXFIFO_ENA);
    rfm12v_pRfBuf = pBuf;
    rfm12v_RfLen = ((MQ_t *)pBuf)->mq.Length + 2;
    rfm12v_Pos = 0;
    rfm12v_State = RF_TRVTXHDR;
    rfm12_control(RFM12_TRANSMIT_MODE);
}

// Interrupt
ISR(RF_INT_vect)
{
    if(RF_STAT_IRQ)                         // Pin Change Interrupt, activated on BOTH edges
        return;
        
    uint16_t intstat = rfm12_get_status();
    uint8_t ch;

    if(intstat & RFM12_STATUS_POR)          // Power-on reset
    {
        rfm12_control(RFM12_SLEEP_MODE);
        rfm12v_State = RF_TRVPOR;
        return;
    }
    
    if(intstat & RFM12_STATUS_RGIT)         // Rx/Tx FIFO events
    {
        switch(rfm12v_State)
        {
            // Start Rx Section
            case RF_TRVRXIDLE:              // Get Packet Length
                ch = rfm12_get_fifo();
                if((ch < 4) || (ch > (MQTTS_MSG_SIZE + 3)))
                    break;
                rfm12v_RfCRC = 0xFFFF;
                rfm12_CalcCRC(ch, (uint16_t *)&rfm12v_RfCRC);
                rfm12v_State = RF_TRVRXHDR;
#ifndef RF_MONITOR
                rfm12v_RfLen = ch - 1;      // Packet length(hdr + data), w/o CRC
#else   //  RF_MONITOR
                rfm12v_RfLen = ch + 1;
#endif  //  RF_MONITOR
                rfm12v_Pos = 0;
                return;
            case RF_TRVRXHDR:               // Destination Addr
                ch = rfm12_get_fifo();
#ifndef RF_MONITOR
                if((ch == 0) || (ch == rfm12s_NodeID))
#endif
                {
                    uint8_t * pTmp;
                    pTmp = (uint8_t *)mqAssert();
                    if(pTmp == NULL)        // No Memory
                        break;

                    rfm12v_pRfBuf = pTmp;
#ifdef RF_MONITOR
                    rfm12v_pRfBuf[rfm12v_Pos++] = rfm12v_RfLen;
                    rfm12v_pRfBuf[rfm12v_Pos++] = ch;
#endif  //  RF_MONITOR
                    RxLEDon();
                    rfm12_CalcCRC(ch, (uint16_t *)&rfm12v_RfCRC);
                    rfm12v_State = RF_TRVRXDATA;
#ifdef RF_USE_RSSI
                    ai_Sel_RSSI();
#endif  //  RF_USE_RSSI
                    return;
                }
                break;
            case RF_TRVRXDATA:
                ch = rfm12_get_fifo();
                if(rfm12v_Pos < rfm12v_RfLen)
                {
                    rfm12_CalcCRC(ch, (uint16_t *)&rfm12v_RfCRC);
                    rfm12v_pRfBuf[rfm12v_Pos++] = ch;
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
                else                                    // 2nd CRC byte
                {
                    uint8_t bTmp = rfm12v_RfCRC & 0xFF;
                    if(ch == bTmp)
                    {
//                        rfm12v_Foffs = (uint8_t)(intstat & 0x1F)<<3;	// int8_t frequency offset
#ifdef RF_MONITOR
                        rfm12v_pRfBuf[--rfm12v_Pos] = 0x80;  // CRC Ok
#endif  //  RF_MONITOR

                        rfm12v_pRxBuf[rfm12v_rxHead] = rfm12v_pRfBuf;
                        rfm12v_rxHead++;
                        if(rfm12v_rxHead >= RF_RX_BUF_SIZE)
                            rfm12v_rxHead -=RF_RX_BUF_SIZE;
                        break;
                    }
#ifndef RF_MONITOR
                }
                // Bad CRC
                mqRelease((MQ_t *)rfm12v_pRfBuf);
                break;
#else   //  RF_MONITOR
                    rfm12v_Pos--;
                }
                rfm12v_pRfBuf[rfm12v_Pos] = 0;      // Bad CRC

                rfm12v_pRxBuf[rfm12v_rxHead] = rfm12v_pRfBuf;
                rfm12v_rxHead++;
                if(rfm12v_rxHead >= RF_RX_BUF_SIZE)
                    rfm12v_rxHead -=RF_RX_BUF_SIZE;

                break;
#endif  //  RF_MONITOR
                // End Rx Section
                // Start Tx Section
            case RF_TRVTXHDR:
                if(rfm12v_Pos == 0)             // Send preamble, preamble length 3 bytes
                    ch = 0xAA;
                else if(rfm12v_Pos == 1)        // Send Group ID, MSB
                    ch = rfm12s_Group>>8;
                else if(rfm12v_Pos == 2)        // Send Group ID, LSB
                    ch = rfm12s_Group & 0xFF;
                else if(rfm12v_Pos == 3)        // Send packet length
                {
                    ch = rfm12v_RfLen;
                    rfm12v_RfCRC = 0xFFFF;
                    rfm12_CalcCRC(ch, (uint16_t *)&rfm12v_RfCRC);
                    rfm12v_RfLen -= 1;
                }
                else if(rfm12v_Pos == 4)        // Send destination addr
                {
                    ch = rfm12v_pRfBuf[0];
                    rfm12_CalcCRC(ch, (uint16_t *)&rfm12v_RfCRC);
                }
                else                            // Send Source addr;
                {
                    ch = rfm12s_NodeID;
                    rfm12_CalcCRC(ch, (uint16_t *)&rfm12v_RfCRC);
                    rfm12v_Pos = 0;
                    rfm12v_State = RF_TRVTXDATA;
                }
                rfm12v_Pos++;
                rfm12_control(RFM12_CMD_TX | ch);
                return;
            case RF_TRVTXDATA:                  // Send Payload
                ch = rfm12v_pRfBuf[rfm12v_Pos++];
                rfm12_control(RFM12_CMD_TX | ch);
                rfm12_CalcCRC(ch, (uint16_t *)&rfm12v_RfCRC);
                if(rfm12v_Pos >= rfm12v_RfLen)
                {
                    rfm12v_State = RF_TRVTXDONE;
                    rfm12v_Pos = 0;
                    mqRelease((MQ_t *)rfm12v_pRfBuf);
                }
                return;
            case RF_TRVTXDONE:
                if(rfm12v_Pos++ == 0)       // Send CRC MSB
                    ch = rfm12v_RfCRC >>8;
                else if(rfm12v_Pos == 2)    // Send CRC LSB
                    ch = rfm12v_RfCRC & 0xFF;
                else if(rfm12v_Pos == 3)    // Send dummy byte, byte is not send to RF
                    ch = 0;
                else                        // Return to Receiver mode
                    break;
                rfm12_control(RFM12_CMD_TX | ch);
                return;
        }
    }
    
    LEDsOff();
    // Switch to Receive mode
    rfm12_control(RFM12_IDLE_MODE);
    rfm12_control(RFM12_RXFIFO_DIS);
    rfm12_control(RFM12_TXFIFO_DIS);
    
    rfm12_control(RFM12_RECEIVE_MODE);
    rfm12_control(RFM12_RXFIFO_ENA);
    rfm12v_State = RF_TRVRXIDLE;
}

// API
// Load/Change configuration parameters
void rf_LoadCfg(uint8_t Channel, uint16_t Group, uint8_t ID)
{
    uint16_t chn = Channel;
#if (RFM12_BAND == RFM12_BAND_433)
    chn += 120;
    chn *= 10;
#elif (RFM12_BAND == RFM12_BAND_868)
    chn += 320;
    chn *= 5;
#else   // 915 MHz
    chn += 80;
    chn *= 10;
    chn /= 3;
#endif  //  RFM12_BAND

    if((rfm12s_Channel != 0) && (rfm12s_Channel != chn))
        rfm12_control(RFM12_CMD_FREQUENCY | chn);
    rfm12s_Channel = chn;

    if((rfm12s_Group != 0) && (rfm12s_Group != Group))
        rfm12_control(RFM12_CMD_SYNCPATTERN | (Group & 0xFF));
    rfm12s_Group = Group;

    rfm12s_NodeID = ID;
}

// Initialize Hardware & configure
void rf_Initialize(void)
{
    // HW Initialise
    RF_DISABLE_IRQ();
    RF_PORT_INIT();                         // Ports Init
    RF_SPI_INIT();                          // init SPI controller
    RF_IRQ_CFG();                           // init IRQ input 
    // HW End

    // init Internal register from RFM12
    rfm12_get_status();                     // intitial SPI transfer added to avoid power-up problem
    rfm12_control(RFM12_SLEEP_MODE);

    // wait until RFM12B is out of power-up reset, this takes several *seconds*
    rfm12_control(RFM12_CMD_TX);            // in case we're still in OOK mode
    while (RF_STAT_IRQ == 0)
        rfm12_get_status();

    rfm12_control(RFM12_CMD_CFG |           // Configuration Setting
                  RFM12_CFG_EL |            // Enable TX FIFO
                  RFM12_CFG_EF |            // Enable RX FIFO
                  RFM12_BAND |              // Select Band
                  RFM12_XTAL_12PF);         // Set XTAL capacitor
    rfm12_control(RFM12_SLEEP_MODE);
    rfm12_control(RFM12_CMD_FREQUENCY |     // Frequency Setting Command
                  rfm12s_Channel);
    rfm12_control(RFM12_CMD_DATARATE |      // Data Rate Command
                  RFM12_BAUD);
    rfm12_control(RFM12_CMD_RXCTRL |        // Receiver Control Command
                  RFM12_RXCTRL_P16_VDI |    // Pin16 - VDI output
                  RFM12_RXCTRL_VDI_MEDIUM | // VDI response - Medium
                  RFM12_GAIN |              // gain select
                  RFM12_BANDWIDTH |         // Receiver baseband bandwidth
                  RFM12_DRSSI);             // RSSI detector threshold
    rfm12_control(RFM12_CMD_DATAFILTER |    // Data Filter Command
                  RFM12_DATAFILTER_AL |     // Clock recovery (CR) auto lock control, Slow mode,
                                            //  Digital Filter
                  RFM12_DQD_THRESH_4);      // DQD threshold = 4( good)
    rfm12_control(RFM12_RXFIFO_DIS);        // FIFO and Reset Mode Command
    rfm12_control(RFM12_CMD_SYNCPATTERN |   // Synchro Pattern = 0x2D[grp]
                  (rfm12s_Group & 0xFF));
    rfm12_control(RFM12_CMD_AFC |           // AFC Command
#ifdef GATEWAY
                  RFM12_AFC_AUTO_VDI |      // Keep the foffset only during receiving (VDI=high)
#else   // GATEWAY
                  RFM12_AFC_AUTO_KEEP |     // Keep the foffset value independently from the state of the VDI signal
#endif  // GATEWAY
                  RFM12_AFC_LIMIT_16 |       // Limit  +75 -80 kHz
                  RFM12_AFC_FI |            // accuracy (fine) mode
                  RFM12_AFC_OE |            // Enables the frequency offset register
                  RFM12_AFC_EN);            // Enables the calculation of the offset frequency 
                                            //  by the AFC circuit.
    rfm12_control(RFM12_CMD_TXCONF |        // TX Configuration Control Command
                  RFM12_FSKWIDTH |          // FSK =  fo + offset
                  RFM12_POWER);             // Relative Output Power
    rfm12_control(RFM12_CMD_PLL |           // PLL Setting Command
                  RFM12_CLK_FRQ_HIGH |      // uC CLK Freq >= 5MHz
                  RFM12_PLL_DDIT);          // disable the dithering in the PLL loop.
    rfm12_control(RFM12_CMD_WAKEUP);        // Wake-Up Timer Command, not used
    rfm12_control(RFM12_CMD_DUTYCYCLE);     // Low Duty-Cycle Command, not used
    rfm12_control(RFM12_CMD_LBDMCD |        // Low Battery Detector and 
                                            //  Microcontroller Clock Divider Command
                  RFM12_MCD_DIV5);          // CLK Out = 2 MHz

    rfm12v_State = RF_TRVIDLE;
    rfm12_control(RFM12_IDLE_MODE);
    
    rfm12v_rxHead = 0;
    rfm12v_rxTail = 0;
    rfm12v_txHead = 0;
    rfm12v_txTail = 0;
    rfm12v_ChanBusy = 0;

    RF_SPI_FAST();                          // Fsck up to <= 10MHz
    RF_ENABLE_IRQ();                        // configure interrupt controller
}

// Change state
void rf_SetState(uint8_t state)
{
    if(state == RF_TRVASLEEP)
    {
        rfm12_control(RFM12_SLEEP_MODE);
        rfm12v_State = RF_TRVASLEEP;
    }
    else if(state == RF_TRVWKUP)
    {
        rfm12v_State = RF_TRVIDLE;
        rfm12_control(RFM12_IDLE_MODE);
    }
}

#ifdef RF_USE_RSSI
uint8_t rf_GetRSSI(void)
{
    return ai_Get_RSSI();
}
#endif

// Get received data
uint8_t * rf_GetBuf(void)
{
    if(rfm12v_rxHead == rfm12v_rxTail)
        return NULL;

    uint8_t tmpTail = rfm12v_rxTail;
    rfm12v_rxTail++;
    if(rfm12v_rxTail >= RF_RX_BUF_SIZE)
        rfm12v_rxTail -= RF_RX_BUF_SIZE;
    return (uint8_t *)rfm12v_pRxBuf[tmpTail];
}

uint8_t rf_GetNodeID(void)
{
    return rfm12s_NodeID;
}

// Send data
void rf_Send(uint8_t * pBuf)
{
    if((rfm12v_txTail == rfm12v_txHead) &&                  // Buffer is empty
       (rfm12v_State == RF_TRVRXIDLE) &&                    // State is RxIdle
       ((rfm12_get_status() & RFM12_STATUS_RSSI) == 0))     // No carrier
    {
        rfm12_send(pBuf);
    }
    else
    {
        uint8_t tmpHead = rfm12v_txHead + 1;
        if(tmpHead >= RF_TX_POOL_SIZE)
            tmpHead -= RF_TX_POOL_SIZE;
        if(tmpHead == rfm12v_txTail)                        // Overflow, packet droped
            mqRelease((MQ_t *)pBuf);
        else
        {
            rfm12v_pTxPool[rfm12v_txHead] = pBuf;
            rfm12v_txHead = tmpHead;
        }
    }
}

// Periodical 
void rf_Pool(void)
{
    if(rfm12v_State == RF_TRVPOR)
        rf_Initialize();
    else if((rfm12v_State == RF_TRVRXIDLE) && (rfm12v_txTail != rfm12v_txHead))
    {
        if(rfm12v_ChanBusy)
        {
            rfm12v_ChanBusy--;
        }
        else if(rfm12_get_status() & RFM12_STATUS_RSSI)
        {
            rfm12v_ChanBusy = rfm12s_NodeID>>4;
        }
        else
        {
            rfm12_send(rfm12v_pTxPool[rfm12v_txTail]);
            if(++rfm12v_txTail >= RF_TX_POOL_SIZE)
                rfm12v_txTail -= RF_TX_POOL_SIZE;
        }
    }
    else if(rfm12v_State == RF_TRVIDLE)
    {
        rfm12v_State = RF_TRVRXIDLE;
        rfm12_control(RFM12_RECEIVE_MODE);
        rfm12_control(RFM12_RXFIFO_ENA);
    }
}
