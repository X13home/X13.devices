/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.

Based on IPstack for AVR from Guido Socher and Pascal Stang
*/

#include "../../config.h"


#ifdef ENC28J60_EN
static uint8_t Enc28j60Bank;
static int16_t gNextPacketPtr;

static uint8_t enc28j60ReadOp(uint8_t op, uint8_t address)
{
  ENC_SELECT();
  ENC_SPI_DATA = op | (address & ADDR_MASK);  // issue read command
  while(ENC_SPI_BISY);
  ENC_SPI_DATA = 0x00;                        // read data
  while(ENC_SPI_BISY);
  if(address & 0x80)                          // do dummy read if needed
  {                                           // (for mac and mii, see datasheet page 29)
    ENC_SPI_DATA = 0x00;
    while(ENC_SPI_BISY);
  }
  ENC_RELEASE();
  return(ENC_SPI_DATA);
}

static void enc28j60WriteOp(uint8_t op, uint8_t address, uint8_t data)
{
  ENC_SELECT();
  ENC_SPI_DATA = op | (address & ADDR_MASK);  // issue write command
  while(ENC_SPI_BISY);
  ENC_SPI_DATA = data;                        // write data
  while(ENC_SPI_BISY);
  ENC_RELEASE();
}

static void enc28j60ReadBuffer(uint16_t len, uint8_t* data)
{
  ENC_SELECT();
  ENC_SPI_DATA = ENC28J60_READ_BUF_MEM;   // issue read command
  while(ENC_SPI_BISY);
  while(len)
  {
    len--;
    ENC_SPI_DATA = 0x00;
    while(ENC_SPI_BISY);
    *data = ENC_SPI_DATA;   // read data
    data++;
  }
  ENC_RELEASE();
}

static void enc28j60WriteBuffer(uint16_t len, uint8_t* data)
{
  ENC_SELECT();
  ENC_SPI_DATA = ENC28J60_WRITE_BUF_MEM;  // issue write command
  while(ENC_SPI_BISY);
  while(len)
  {
    len--;
    SPDR = *data; // write data
    data++;
    while(ENC_SPI_BISY);
  }
  ENC_RELEASE();
}

static void enc28j60SetBank(uint8_t address)
{
  uint8_t tmp = address & BANK_MASK;
  if(tmp != Enc28j60Bank)               // set the bank (if needed)
  {
    enc28j60WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, (ECON1_BSEL1 | ECON1_BSEL0));
    enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, tmp>>5);
    Enc28j60Bank = tmp;
  }
}

static uint8_t enc28j60Read(uint8_t address)
{
  enc28j60SetBank(address); // set the bank
  return enc28j60ReadOp(ENC28J60_READ_CTRL_REG, address); // do the read
}

static void enc28j60Write(uint8_t address, uint8_t data)
{
  enc28j60SetBank(address); // set the bank
  enc28j60WriteOp(ENC28J60_WRITE_CTRL_REG, address, data);  // do the write
}

// read 8 bits
static uint8_t enc28j60PhyRead8(uint8_t address)
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

static void enc28j60PhyWrite(uint8_t address, uint16_t data)
{
  // set the PHY register address
  enc28j60Write(MIREGADR, address);
  // write the PHY data
  enc28j60Write(MIWRL, data);
  enc28j60Write(MIWRH, data>>8);
  // wait until the PHY write completes
  while(enc28j60Read(MISTAT) & MISTAT_BUSY)
    _delay_us(10);
}

void enc28j60Init(uint8_t* macaddr)
{
  // initialize I/O
  ENC_PORT_INIT();
  ENC_SPI_INIT();

  // perform system reset
  enc28j60WriteOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
  _delay_ms(20);

  // change clkout from 6.25MHz to 12.5MHz
  enc28j60Write(ECOCON, 2);
  _delay_ms(20);
  
  // do bank 0 stuff
  // initialize receive buffer
  // 16-bit transfers, must write low byte first
  // set receive buffer start address
  gNextPacketPtr = RXSTART_INIT;
  // Rx start
  enc28j60Write(ERXSTL, RXSTART_INIT & 0xFF);
  enc28j60Write(ERXSTH, RXSTART_INIT >> 8);
  // set receive pointer address
  enc28j60Write(ERXRDPTL, RXSTART_INIT & 0xFF);
  enc28j60Write(ERXRDPTH, RXSTART_INIT >> 8);
  // RX end
  enc28j60Write(ERXNDL, RXSTOP_INIT & 0xFF);
  enc28j60Write(ERXNDH, RXSTOP_INIT >> 8);
  // TX start
  enc28j60Write(ETXSTL, TXSTART_INIT & 0xFF);
  enc28j60Write(ETXSTH, TXSTART_INIT >> 8);
  // TX end
  enc28j60Write(ETXNDL, TXSTOP_INIT & 0xFF);
  enc28j60Write(ETXNDH, TXSTOP_INIT >> 8);
 // do bank 1 stuff, packet filter:
  enc28j60Write(ERXFCON, ERXFCON_UCEN | ERXFCON_CRCEN | ERXFCON_PMEN);
  enc28j60Write(EPMM0, 0x3f);
  enc28j60Write(EPMM1, 0x30);
  enc28j60Write(EPMCSL, 0xf9);
  enc28j60Write(EPMCSH, 0xf7);
  // do bank 2 stuff
  // enable MAC receive
  enc28j60Write(MACON1, MACON1_MARXEN|MACON1_TXPAUS|MACON1_RXPAUS);
  // bring MAC out of reset
  enc28j60Write(MACON2, 0x00);
  // enable automatic padding to 60bytes and CRC operations
  enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FRMLNEN);
  // set inter-frame gap (non-back-to-back)
  enc28j60Write(MAIPGL, 0x12);
  enc28j60Write(MAIPGH, 0x0C);
  // set inter-frame gap (back-to-back)
  enc28j60Write(MABBIPG, 0x12);
  // Set the maximum packet size which the controller will accept
  // Do not send packets longer than MAX_FRAMELEN:
  enc28j60Write(MAMXFLL, MAX_FRAMELEN & 0xFF);
  enc28j60Write(MAMXFLH, MAX_FRAMELEN >> 8);
  // do bank 3 stuff
  // write MAC address
  // NOTE: MAC address in ENC28J60 is byte-backward
  enc28j60Write(MAADR5, macaddr[0]);
  enc28j60Write(MAADR4, macaddr[1]);
  enc28j60Write(MAADR3, macaddr[2]);
  enc28j60Write(MAADR2, macaddr[3]);
  enc28j60Write(MAADR1, macaddr[4]);
  enc28j60Write(MAADR0, macaddr[5]);
  // no loopback of transmitted frames
  enc28j60PhyWrite(PHCON2, PHCON2_HDLDIS);
  // switch to bank 0
  enc28j60SetBank(ECON1);
  // enable interrutps
  // enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE | EIE_PKTIE);
  // enable packet reception
  enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);
  // Magjack leds configuration, 0x476 is PHLCON LEDA=links status(yellow, LEDB=receive/transmit(green)
  enc28j60PhyWrite(PHLCON, 0x476);
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
uint8_t enc28j60linkup(void)
{
  // The non latching version is LSTAT.
  // PHSTAT2 LSTAT (= bit 10 in upper reg)
  if(enc28j60PhyRead8(PHSTAT2) & (1<<2))
    return(1);
  return(0);
}

void enc28j60PacketSend(uint16_t len, uint8_t* packet)
{
  // Check no transmit in progress
  while(enc28j60ReadOp(ENC28J60_READ_CTRL_REG, ECON1) & ECON1_TXRTS)
  {
    // Reset the transmit logic problem. See Rev. B4 Silicon Errata point 12.
    if((enc28j60Read(EIR) & EIR_TXERIF))
    {
      enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRST);
      enc28j60WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRST);
    }
  }
  // Set the write pointer to start of transmit buffer area
  enc28j60Write(EWRPTL, TXSTART_INIT & 0xFF);
  enc28j60Write(EWRPTH, TXSTART_INIT >> 8);
  // Set the TXND pointer to correspon to the packet size given
  enc28j60Write(ETXNDL, (TXSTART_INIT + len) & 0xFF);
  enc28j60Write(ETXNDH, (TXSTART_INIT + len) >> 8);
  // write per-packet control byte (0x00 means use macon3 settings)
  enc28j60WriteOp(ENC28J60_WRITE_BUF_MEM, 0, 0x00);
  // copy the packet into the transmit buffer
  enc28j60WriteBuffer(len, packet);
  // send the contents of the transmit buffer onto the network
  enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
}

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
  enc28j60Write(ERDPTL, gNextPacketPtr & 0xFF);
  enc28j60Write(ERDPTH, gNextPacketPtr >> 8);
  // read the next packet pointer
  gNextPacketPtr  = enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
  gNextPacketPtr |= enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0) << 8;
  // read the packet length (see datasheet page 43)
  len  = enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
  len |= enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0) << 8;
  len -= 4; //remove the CRC count
  // read the receive status (see datasheet page 43)
  rxstat  = enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
  rxstat |= ((uint16_t)enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0))<<8;

  // check CRC and symbol errors (see datasheet page 44, table 7-3):
  // The ERXFCON.CRCEN is set by default. Normally we should not
  // need to check this.
  if((rxstat & 0x80) == 0)
  {
    len=0;  // invalid
  }
  else
  {
    if(len > maxlen)  // limit retrieve length
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
    enc28j60Write(ERXRDPTL, RXSTOP_INIT & 0xFF);
    enc28j60Write(ERXRDPTH, RXSTOP_INIT >> 8);
  }
  else
  {
    enc28j60Write(ERXRDPTL, (gNextPacketPtr - 1) & 0xFF);
    enc28j60Write(ERXRDPTH, (gNextPacketPtr - 1) >> 8);
  }
  // decrement the packet counter indicate we are done with this packet
  enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
  return(len);
}
#endif  //  ENC28J60_EN
