/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _CC11_PHY_H
#define _CC11_PHY_H

#define OD_DEFAULT_GROUP        0x2DD4

// 433 MHz
#if (RF_BASE_FREQ > 433050000UL) && (RF_BASE_FREQ < 434790000UL)
#define OD_DEFAULT_CHANNEL      ((RF_BASE_FREQ - 433000000UL)/25000)

// 868 MHz
#elif (RF_BASE_FREQ > 868000000UL) && (RF_BASE_FREQ < 870000000UL)
#define OD_DEFAULT_CHANNEL      ((RF_BASE_FREQ - 868000000UL)/25000)

// Bad Frequency
#else
#error  RF_BASE_FREQ does not belond to ISM band
#endif  // RF_BASE_FREQ

#if (CC11_PHY == 1)

#define PHY1_ADDR_t                 uint8_t
#define ADDR_BROADCAST_PHY1         (PHY1_ADDR_t)0x00
#define ADDR_UNDEF_PHY1             (PHY1_ADDR_t)0xFF

#define PHY1_Init                   CC11_Init
#define PHY1_Send                   CC11_Send
#define PHY1_Get                    CC11_Get
#define PHY1_GetAddr                CC11_GetAddr
#define PHY1_NodeId                 objRFNodeId
#define PHY1_GateId                 objRFGateId
#define PHY1_ASleep                 CC11_ASleep
#define PHY1_AWake                  CC11_AWake
    
#elif   (CC11_PHY == 2)

#define PHY2_ADDR_t                 uint8_t
#define ADDR_BROADCAST_PHY2         (PHY2_ADDR_t)0x00
#define ADDR_UNDEF_PHY2             (PHY2_ADDR_t)0xFF

#define PHY2_Init                   CC11_Init
#define PHY2_Send                   CC11_Send
#define PHY2_Get                    CC11_Get
#define PHY2_GetRSSI                CC11_GetRSSI
#define PHY2_GetAddr                CC11_GetAddr
#define PHY2_NodeId                 objRFNodeId

#else
#error CC11_PHY unknown interface
#endif  // CC11_PHY

#ifndef RF_ADDR_t

#define RF_ADDR_t                   uint8_t
#define RF_ADDR_TYPE                objUInt8
#define ADDR_UNDEF_RF               (RF_ADDR_t)0xFF

#endif  //  RF_ADDR_t

// API Section
void    CC11_Init(void);
void    CC11_Send(void *pBuf);
void  * CC11_Get(void);
uint8_t CC11_GetRSSI(void);
void *  CC11_GetAddr(void);

void    CC11_ASleep(void);
void    CC11_AWake(void);

#endif  //  _CC11_PHY_H
