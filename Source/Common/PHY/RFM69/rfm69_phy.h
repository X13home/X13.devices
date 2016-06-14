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

#if ((RF_BASE_FREQ >= 290000000UL) && (RF_BASE_FREQ < 340000000UL))
#define RFM69_FRQ_MIN               290000000UL
#define RFM69_FRQ                   315000000UL
#define RFM69_FRQ_MAX               340000000UL
#elif ((RF_BASE_FREQ >= 424000000UL) && (RF_BASE_FREQ < 510000000UL))
#define RFM69_FRQ_MIN               424000000UL
#define RFM69_FRQ                   433000000UL
#define RFM69_FRQ_MAX               510000000UL
#elif ((RF_BASE_FREQ >= 862000000UL) && (RF_BASE_FREQ < 890000000UL))
#define RFM69_FRQ_MIN               862000000UL
#define RFM69_FRQ                   868000000UL
#define RFM69_FRQ_MAX               890000000UL
#elif ((RF_BASE_FREQ >= 890000000UL) && (RF_BASE_FREQ < 1020000000UL))
#define RFM69_FRQ_MIN               890000000UL
#define RFM69_FRQ                   915000000UL
#define RFM69_FRQ_MAX               1020000000UL
#else
#error RF_BASE_FREQ this frequency is impossible
#endif  //  RF_BASE_FREQ

#define OD_DEFAULT_CHANNEL          ((RF_BASE_FREQ - RFM69_FRQ)/5000)
#define OD_DEFAULT_GROUP            0x2DD4

#if (RFM69_PHY == 1)

#define PHY1_ADDR_t                 uint8_t
#define ADDR_BROADCAST_PHY1         (PHY1_ADDR_t)0x00
#define ADDR_UNDEF_PHY1             (PHY1_ADDR_t)0xFF

#define PHY1_Init                   RFM69_Init
#define PHY1_Send                   RFM69_Send
#define PHY1_Get                    RFM69_Get
#define PHY1_GetAddr                RFM69_GetAddr
#define PHY1_NodeId                 objRFNodeId
#define PHY1_GateId                 objRFGateId
    
#elif   (RFM69_PHY == 2)

#define PHY2_ADDR_t                 uint8_t
#define ADDR_BROADCAST_PHY2         (PHY2_ADDR_t)0x00
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
