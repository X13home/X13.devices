/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#include "../../config.h"

#if (defined ENC_PHY)

#include "enc28j60_def.h"
#include "enc28j60_hw.h"

static uint8_t Enc28j60Bank;
static int16_t gNextPacketPtr;

// Generic SPI write command
static void enc28j60WriteOp(uint8_t op, uint8_t address, uint8_t data)
{
    //taskENTER_CRITICAL();
    ENC_SELECT();
    op |= (address & ADDR_MASK);
    hal_spi_exch8(ENC_USE_SPI, op);       // issue write command
    hal_spi_exch8(ENC_USE_SPI, data);
    ENC_RELEASE();
    //taskEXIT_CRITICAL();
}

// Set register bank
static void enc28j60SetBank(uint8_t address)
{
    uint8_t tmp;
    tmp = (address & BANK_MASK);

    if(tmp != Enc28j60Bank) // set the bank (if needed)
    {
        enc28j60WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, (ECON1_BSEL1 | ECON1_BSEL0));
        enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, tmp>>5);
        Enc28j60Bank = tmp;
    }
}

// Read register
static uint8_t enc28j60Read(uint8_t address)
{
    //taskENTER_CRITICAL();
    if((address & ADDR_MASK) < EIE)
    {
        enc28j60SetBank(address); // set the bank
    }
    uint8_t data;
    ENC_SELECT();
    hal_spi_exch8(ENC_USE_SPI, (ENC28J60_READ_CTRL_REG | (address & ADDR_MASK)));     // issue read command
    if(address & 0x80)                                                                // do dummy read if needed
    {
        hal_spi_exch8(ENC_USE_SPI, 0);                                                  // (for mac and mii, see datasheet page 29)
    }
    data = hal_spi_exch8(ENC_USE_SPI, 0);                                             // read data
    ENC_RELEASE();
    //taskEXIT_CRITICAL();
    return data;
}

// Write register
static void enc28j60Write(uint8_t address, uint8_t data)
{
    if((address & ADDR_MASK) < EIE)
    {
        enc28j60SetBank(address); // set the bank
    }
    enc28j60WriteOp(ENC28J60_WRITE_CTRL_REG, address, data);  // do the write
}

// Write register pair
static void enc28j60WriteW(uint8_t address, uint16_t data)
{
    enc28j60SetBank(address); // set the bank
    enc28j60WriteOp(ENC28J60_WRITE_CTRL_REG, address, data & 0xFF);  // do the write
    enc28j60WriteOp(ENC28J60_WRITE_CTRL_REG, address + 1, data>>8);  // do the write
}

// Read Rx/Tx buffer (at ERDPT)
static void enc28j60ReadBuffer(uint16_t len, uint8_t* data)
{
    //taskENTER_CRITICAL();
    ENC_SELECT();
    hal_spi_exch8(ENC_USE_SPI, ENC28J60_READ_BUF_MEM);            // issue read command
    while(len--)
    {
        *(data++) = hal_spi_exch8(ENC_USE_SPI, 0);                  // read data
    }
    ENC_RELEASE();
    //taskEXIT_CRITICAL();
}

// Write Rx/Tx buffer (at EWRPT)
static void enc28j60WriteBuffer(uint16_t len, uint8_t* data)
{
    //taskENTER_CRITICAL();
    ENC_SELECT();
    hal_spi_exch8(ENC_USE_SPI, ENC28J60_WRITE_BUF_MEM);           // issue write command
    while(len--)
    {
        hal_spi_exch8(ENC_USE_SPI, *(data++));                      // write data
    }
    ENC_RELEASE();
    //taskEXIT_CRITICAL();
}

// Read PHY register
static uint8_t enc28j60PhyRead(uint8_t address)
{
    // Set the right address and start the register read operation
    enc28j60Write(MIREGADR, address);
    enc28j60Write(MICMD, MICMD_MIIRD);
    // wait until the PHY read completes
    while(enc28j60Read(MISTAT) & MISTAT_BUSY);
    // reset reading bit
    enc28j60Write(MICMD, 0x00);
    // get data value from MIRDH
    return enc28j60Read(MIRDH);
}

// Write to PHY register
static void enc28j60PhyWrite(uint8_t address, uint16_t data)
{
    // set the PHY register address
    enc28j60Write(MIREGADR, address);
    // write the PHY data
    enc28j60WriteW(MIWR, data);
    // wait until the PHY write completes
    while(enc28j60Read(MISTAT) & MISTAT_BUSY)
    {
        _delay_us(10);
    }
}

void enc28j60Init(uint8_t* macaddr)
{
    // Init Hardware
    hal_dio_configure(ENC_NSS_PIN, DIO_MODE_OUT_PP_HS);
    ENC_RELEASE();
    hal_spi_cfg(ENC_USE_SPI, (HAL_SPI_MODE_0 | HAL_SPI_MSB | HAL_SPI_8B), 10000000UL);

#ifdef ENC_IRQ_PIN
    // Not Used, but configured
    hal_dio_configure(ENC_IRQ_PIN, DIO_MODE_IN_PU);
#endif  // ENC_IRQ_PIN

    // perform system reset
#ifdef ENC_RST_PIN
    // Hardware Reset
    hal_dio_configure(ENC_RST_PIN, DIO_MODE_OUT_PP);
    ENC_RST_SET();
    _delay_us(500);
    ENC_RST_CLR();
#else
    // Software Reset
    enc28j60WriteOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
#endif  // ENC_RST_PIN

    _delay_us(50000);
    Enc28j60Bank = 0;

    // check CLKRDY bit to see if reset is complete
    uint8_t timeout = 50;
    while(((enc28j60Read(ESTAT) & ESTAT_CLKRDY) == 0) && (timeout > 0))
    {
        timeout--;
        if(timeout == 0)
        {
            break;
        }
        _delay_us(10000);
    }

    // change clkout from 6.25MHz to 12.5MHz
    // Check connection
    do
    {
        _delay_us(2000);
        enc28j60Write(ECOCON, 2);
        _delay_us(2000);
    }while(enc28j60Read(ECOCON) != 2);

    // reset PHY
    enc28j60PhyWrite(PHCON1, PHCON1_PRST);
    while((enc28j60PhyRead(PHCON1) & PHCON1_PRST) != 0);
    // PHY Power Down - disconnect from switch
    enc28j60PhyWrite(PHCON1, PHCON1_PPWRSV);
    _delay_us(50000);
    // reset MAC
    enc28j60Write(ECON1, (ECON1_TXRST | ECON1_RXRST));
    _delay_us(50000);
    enc28j60Write(ECON1, 0);
    _delay_us(50000);


    // do bank 0 stuff
    // initialize receive buffer
    // 16-bit transfers, must write low byte first
    // set receive buffer start address
    gNextPacketPtr = RXSTART_INIT;
    // Rx start
    enc28j60WriteW(ERXST, RXSTART_INIT);
    // set receive pointer address
    enc28j60WriteW(ERXRDPT, RXSTART_INIT);
    // RX end
    enc28j60WriteW(ERXND, RXSTOP_INIT);
    // TX start
    enc28j60WriteW(ETXST, TXSTART_INIT);
    // TX end
    enc28j60WriteW(ETXND, TXSTOP_INIT);
    // do bank 1 stuff, packet filter:
    enc28j60Write(ERXFCON, (ERXFCON_UCEN | ERXFCON_CRCEN | ERXFCON_PMEN));
    enc28j60Write(EPMM0, 0x3f);
    enc28j60Write(EPMM1, 0x30);
    enc28j60Write(EPMCSL, 0xf9);
    enc28j60Write(EPMCSH, 0xf7);
    // do bank 2 stuff
    // enable MAC receive
    enc28j60Write(MACON1, (MACON1_MARXEN | MACON1_TXPAUS | MACON1_RXPAUS));
    // bring MAC out of reset
    //enc28j60Write(MACON2, 0x00);
    // enable automatic padding to 60bytes and CRC operations
    enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, MACON3, (MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FRMLNEN | MACON3_FULDPX));
    // set inter-frame gap (non-back-to-back)
    enc28j60WriteW(MAIPGL, 0x0C12);
    //enc28j60Write(MAIPGL, 0x12);
    //enc28j60Write(MAIPGH, 0x0C);
    // set inter-frame gap (back-to-back)
    enc28j60Write(MABBIPG, 0x12);
    // Set the maximum packet size which the controller will accept
    // Do not send packets longer than MAX_FRAMELEN:
    enc28j60WriteW(MAMXFL, MAX_FRAMELEN);
    // do bank 3 stuff
    // write MAC address
    // NOTE: MAC address in ENC28J60 is byte-backward
    enc28j60Write(MAADR5, macaddr[0]);
    enc28j60Write(MAADR4, macaddr[1]);
    enc28j60Write(MAADR3, macaddr[2]);
    enc28j60Write(MAADR2, macaddr[3]);
    enc28j60Write(MAADR1, macaddr[4]);
    enc28j60Write(MAADR0, macaddr[5]);
    // Setup PHY
    enc28j60PhyWrite(PHCON1, PHCON1_PDPXMD);  // Force full-duplex mode
    enc28j60PhyWrite(PHCON2, PHCON2_HDLDIS);  // Disable loopback
    // Magjack leds configuration, LEDA=links status(yellow, LEDB=receive/transmit(green)
    enc28j60PhyWrite(PHLCON, (PHLCON_LACFG2 | 
                              PHLCON_LBCFG2 | PHLCON_LBCFG1 | PHLCON_LBCFG0 |
                              PHLCON_LFRQ0 | PHLCON_STRCH));
                              
    // Enable Rx packets
    enc28j60SetBank(0);         // switch to bank 0
#ifdef ENC_IRQ_PIN
    // enable interrupts
    //enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE | EIE_PKTIE);
#endif // ENC_IRQ_PIN

    // enable packet reception
    enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);
}

// A number of utility functions to enable/disable broadcast 
void enc28j60EnableBroadcast(void)
{
  uint8_t erxfcon;
  erxfcon = enc28j60Read(ERXFCON);
  erxfcon |= ERXFCON_BCEN;
  enc28j60Write(ERXFCON, erxfcon);
}

void enc28j60DisableBroadcast(void)
{
  uint8_t erxfcon;
  erxfcon=enc28j60Read(ERXFCON);
  erxfcon &= ~ERXFCON_BCEN;
  enc28j60Write(ERXFCON, erxfcon);
}


// link status
bool enc28j60linkup(void)
{
  // The non latching version is LSTAT.
  // PHSTAT2 LSTAT (= bit 10 in upper reg)
  return ((enc28j60PhyRead(PHSTAT2) & (1<<2)) != 0);
}


void enc28j60_SendPrep(uint16_t len)
{
    // ToDo Timeout
    // Check no transmit in progress
    while(enc28j60Read(ECON1) & ECON1_TXRTS)
    {
        // Reset the transmit logic problem. See Rev. B4 Silicon Errata point 12.
        if((enc28j60Read(EIR) & EIR_TXERIF))
        {
            enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRST);
            enc28j60WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRST);
        }
    }

    // Set the write pointer to start of transmit buffer area
    enc28j60WriteW(EWRPT, TXSTART_INIT);
    // Set the TXND pointer to correspon to the packet size given
    enc28j60WriteW(ETXND, (TXSTART_INIT + len));
    // write per-packet control byte (0x00 means use macon3 settings)
    enc28j60WriteOp(ENC28J60_WRITE_BUF_MEM, 0, 0x00);
}

void enc28j60_PutData(uint16_t len, uint8_t* pBuf)
{
    // copy the packet into the transmit buffer
    enc28j60WriteBuffer(len, pBuf);
}

void enc28j60_Fill(uint8_t len)
{
    ENC_SELECT();
    hal_spi_exch8(ENC_USE_SPI, ENC28J60_WRITE_BUF_MEM);             // issue write command
    while(len--)
        hal_spi_exch8(ENC_USE_SPI, 0);
    ENC_RELEASE();
}

void enc28j60_Send(void)
{
    // send the contents of the transmit buffer onto the network
    enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
}


/*
void enc28j60PacketSend(uint16_t len, uint8_t* packet)
{
  // Check no transmit in progress
  while(enc28j60Read(ECON1) & ECON1_TXRTS)
  {
    // Reset the transmit logic problem. See Rev. B4 Silicon Errata point 12.
    if((enc28j60Read(EIR) & EIR_TXERIF))
    {
      enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRST);
      enc28j60WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRST);
    }
  }
  // Set the write pointer to start of transmit buffer area
  enc28j60WriteW(EWRPT, TXSTART_INIT);
  // Set the TXND pointer to correspon to the packet size given
  enc28j60WriteW(ETXND, (TXSTART_INIT + len));
  // write per-packet control byte (0x00 means use macon3 settings)
  enc28j60WriteOp(ENC28J60_WRITE_BUF_MEM, 0, 0x00);
  // copy the packet into the transmit buffer
  enc28j60WriteBuffer(len, packet);
  // send the contents of the transmit buffer onto the network
  enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
}
*/

bool en28j60_DataRdy(void)
{
  return (enc28j60Read(EPKTCNT) != 0);
}

/*
// Gets a packet from the network receive buffer, if one is available.
// The packet will by headed by an ethernet header.
//      maxlen  The maximum acceptable length of a retrieved packet.
//      packet  Pointer where packet data should be stored.
// Returns: Packet length in bytes if a packet was retrieved, zero otherwise.
uint16_t enc28j60PacketReceive(uint16_t maxlen, uint8_t* packet)
{
  uint16_t rxstat;
  uint16_t len;
  // check if a packet has been received and buffered
  if(enc28j60Read(EPKTCNT) == 0)
    return(0);

  // Set the read pointer to the start of the received packet
  enc28j60WriteW(ERDPT, gNextPacketPtr);
  // read the next packet pointer
  enc28j60ReadBuffer(sizeof(uint16_t), (uint8_t *)&gNextPacketPtr);
  // read the packet length (see datasheet page 43)
  enc28j60ReadBuffer(sizeof(uint16_t), (uint8_t *)&len);
  // read the receive status (see datasheet page 43)
  enc28j60ReadBuffer(sizeof(uint16_t), (uint8_t *)&rxstat);

  // check CRC and symbol errors (see datasheet page 44, table 7-3):
  // The ERXFCON.CRCEN is set by default. Normally we should not
  // need to check this.
  if((rxstat & 0x80) == 0)
  {
    len = 0;  // invalid
  }
  else
  {
    len -= 4;               // remove the CRC count
    if(len > maxlen)        // limit retrieve length
      len = maxlen;
    // copy the packet from the receive buffer
    enc28j60ReadBuffer(len, packet);
  }

  // Move the RX read pointer to the start of the next received packet
  // This frees the memory we just read out. 
  // However, compensate for the errata point 13, rev B4: never write an even address!
  // gNextPacketPtr is always an even address if RXSTOP_INIT is odd.
  if((gNextPacketPtr - 1) > RXSTOP_INIT) // RXSTART_INIT is zero, no test for gNextPacketPtr less than RXSTART_INIT.
  {
    enc28j60WriteW(ERXRDPT, RXSTOP_INIT);
  }
  else
  {
    enc28j60WriteW(ERXRDPT, (gNextPacketPtr - 1));
  }
  // decrement the packet counter indicate we are done with this packet
  enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
  return(len);
}
*/

uint16_t enc28j60_GetPacketLen(void)
{
  uint16_t len, rxstat;

  // Set the read pointer to the start of the received packet
  enc28j60WriteW(ERDPT, gNextPacketPtr);
  // read the next packet pointer
  enc28j60ReadBuffer(sizeof(uint16_t), (uint8_t *)&gNextPacketPtr);
  // read the packet length (see datasheet page 43)
  enc28j60ReadBuffer(sizeof(uint16_t), (uint8_t *)&len);
  // read the receive status (see datasheet page 43)
  enc28j60ReadBuffer(sizeof(uint16_t), (uint8_t *)&rxstat);

  // check CRC and symbol errors (see datasheet page 44, table 7-3):
  // The ERXFCON.CRCEN is set by default. Normally we should not
  // need to check this.
  if(((rxstat & 0x80) == 0) || (len < 18))    // SizeOf(EthFrame) + 4
  {
    len = 0;  // invalid
  }
  else
    len -= 4;               // remove the CRC count

  return len;
}

void enc28j60_GetPacket(uint8_t * pBuf, uint16_t len)
{
  enc28j60ReadBuffer(len, pBuf);
}

void enc28j60_Skip(uint16_t len)
{
    ENC_SELECT();
    hal_spi_exch8(ENC_USE_SPI, ENC28J60_READ_BUF_MEM);   // issue read command
    while(len--)
        hal_spi_exch8(ENC_USE_SPI, 0);                   // skip data
    ENC_RELEASE();
}

void enc28j60_ClosePacket(void)
{
  // Move the RX read pointer to the start of the next received packet
  // This frees the memory we just read out. 
  // However, compensate for the errata point 13, rev B4: never write an even address!
  // gNextPacketPtr is always an even address if RXSTOP_INIT is odd.
  if((gNextPacketPtr - 1) > RXSTOP_INIT) // RXSTART_INIT is zero, no test for gNextPacketPtr less than RXSTART_INIT.
  {
    enc28j60WriteW(ERXRDPT, RXSTOP_INIT);
  }
  else
  {
    enc28j60WriteW(ERXRDPT, (gNextPacketPtr - 1));
  }
  // decrement the packet counter indicate we are done with this packet
  enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
}

#endif  // ENC_PHY
