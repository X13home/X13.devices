/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _RFM69_PHY_H
#define _RFM69_PHY_H

// FXO = 32000000UL
// FSTEP = FXO/2^19
// Baud = 38400
// BITRATE = RFM69_FXO/Baud
#define RFM69_REG_BITRATEMSB_DEF    0x03
#define RFM69_REG_BITRATELSB_DEF    0x41
// Deviation = 20000
// FDEV = Deviation / FSTEP
#define RFM69_REG_FDEVMSB_DEF       0x01
#define RFM69_REG_FDEVLSB_DEF       0x48
// FSK Mode: RxBw = FXOSC / (RxBwMant * 2^(RxBwExp + 2))
// RX filter Bandwidth 100 KHz = 32M /(20 * 2^(2 + 2))
// CuttOff Freq 4%
#define RFM69_REG_RXBW_DEF          (RFM69_RXBW_DCCFREQ_2 | RFM69_RXBW_MANT_20 | RFM69_RXBW_EXP_2)

#define RFM69_REG_PACKETCONFIG2_DEF (RFM69_PACKET2_RXRESTARTDELAY_1BIT |    \
                                     RFM69_PACKET2_AUTORXRESTART_ON |       \
                                     RFM69_PACKET2_AES_OFF)

#if ((RF_BASE_FREQ >= 868000000UL) && (RF_BASE_FREQ <= 870000000UL))
#define RFM69_BASE_FREQ             868000000UL
// Default Frequence = 868,3
#define RFM69_REG_FRFMSB_DEF        0xD9
#define RFM69_REG_FRFMID_DEF        0x13
#define RFM69_REG_FRFLSB_DEF        0x33
#else
#error RF_BASE_FREQ does not belond to ISM band
#endif  //  RF_BASE_FREQ

#define OD_DEFAULT_CHANNEL          ((RF_BASE_FREQ - RFM69_BASE_FREQ)/25000)
#define OD_DEFAULT_GROUP            0x2DD4

#define RFM69_BROADCAST_ADDR        0x00

#if (RFM69_PHY == 1)

#define PHY1_ADDR_t                 uint8_t
#define ADDR_BROADCAST_PHY1         (PHY1_ADDR_t)RFM69_BROADCAST_ADDR
#define ADDR_UNDEF_PHY1             (PHY1_ADDR_t)0xFF

#define PHY1_Init                   RFM69_Init
#define PHY1_Send                   RFM69_Send
#define PHY1_Get                    RFM69_Get
#define PHY1_GetAddr                RFM69_GetAddr
#define PHY1_NodeId                 objRFNodeId
#define PHY1_GateId                 objRFGateId
    
#elif   (RFM69_PHY == 2)

#define PHY2_ADDR_t                 uint8_t
#define ADDR_BROADCAST_PHY2         (PHY2_ADDR_t)RFM69_BROADCAST_ADDR
#define ADDR_UNDEF_PHY2             (PHY2_ADDR_t)0xFF

#define PHY2_Init                   RFM69_Init
#define PHY2_Send                   RFM69_Send
#define PHY2_Get                    RFM69_Get
#define PHY2_GetAddr                RFM69_GetAddr
#define PHY2_NodeId                 objRFNodeId

#else
#error RFM69_PHY unknown interface
#endif  // RFM69_PHY

#ifndef RF_ADDR_t

#define RF_ADDR_t                   uint8_t
#define RF_ADDR_TYPE                objUInt8
#define ADDR_UNDEF_RF               (RF_ADDR_t)0xFF

#endif  //  RF_ADDR_t

// API Section
void    RFM69_Init(void);
void    RFM69_Send(void *pBuf);
void  * RFM69_Get(void);
void  * RFM69_GetAddr(void);

#endif  //  _RFM69_PHY_H
