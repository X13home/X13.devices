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

#ifndef RF_BASE_FREQ
#define RF_BASE_FREQ                868300000UL
#endif  //  RF_BASE_FREQ

// 433 - ISM band
#if ((RF_BASE_FREQ > 433000000UL) && (RF_BASE_FREQ < 434800000UL))
#define RFM69_BASE_FREQ             433000000UL
#define RFM69_MIN_FREQ              433050000UL
#define RFM69_MAX_FREQ              434790000UL

// Extended 868 - ISM band
#elif ((RF_BASE_FREQ >= 865000000UL) && (RF_BASE_FREQ < 868000000UL))
#define RFM69_BASE_FREQ             865000000UL
#define RFM69_MIN_FREQ              865000000UL
#define RFM69_MAX_FREQ              870000000UL

// Standard 868 - ISM Band
#elif ((RF_BASE_FREQ >= 868000000UL) && (RF_BASE_FREQ <= 870000000UL))
#define RFM69_BASE_FREQ             868000000UL
#define RFM69_MIN_FREQ              868000000UL
#define RFM69_MAX_FREQ              870000000UL

// ARRL Band - Digital communications
#elif ((RF_BASE_FREQ >= 903000000UL) && (RF_BASE_FREQ <= 906000000UL))
#define RFM69_BASE_FREQ             903000000UL
#define RFM69_MIN_FREQ              903000000UL
#define RFM69_MAX_FREQ              906000000UL

#elif ((RF_BASE_FREQ >= 915000000UL) && (RF_BASE_FREQ <= 918000000UL))
#define RFM69_BASE_FREQ             915000000UL
#define RFM69_MIN_FREQ              915000000UL
#define RFM69_MAX_FREQ              918000000UL

#else
#error RF_BASE_FREQ does not belond to ISM band
#endif  //  RF_BASE_FREQ

#define OD_DEFAULT_CHANNEL          ((RF_BASE_FREQ - RFM69_BASE_FREQ)/25000)
#define OD_DEFAULT_GROUP            0x2DD4
#define OD_DEFAULT_RF_PWR           10
#define OD_DEFAULT_RF_KEY           {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}

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
#define PHY2_GetRSSI                RFM69_GetRSSI

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
int8_t  RFM69_GetRSSI(void);

#endif  //  _RFM69_PHY_H
