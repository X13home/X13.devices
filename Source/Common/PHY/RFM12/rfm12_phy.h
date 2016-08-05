/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _RFM12_PHY_H
#define _RFM12_PHY_H

#include <stdbool.h>

#if (RFM12_PHY == 1)

#define PHY1_ADDR_t                 uint8_t
#define ADDR_BROADCAST_PHY1         (PHY1_ADDR_t)0x00
#define ADDR_UNDEF_PHY1             (PHY1_ADDR_t)0xFF

#define PHY1_Init                   RFM12_Init
#define PHY1_Send                   RFM12_Send
#define PHY1_Get                    RFM12_Get
#define PHY1_GetAddr                RFM12_GetAddr
#define PHY1_NodeId                 objRFNodeId
#define PHY1_GateId                 objRFGateId
#define PHY1_ASleep                 RFM12_ASleep
#define PHY1_AWake                  RFM12_AWake

#elif (RFM12_PHY == 2)
    
#define PHY2_ADDR_t                 uint8_t
#define ADDR_BROADCAST_PHY2         (PHY2_ADDR_t)0x00
#define ADDR_UNDEF_PHY2             (PHY2_ADDR_t)0xFF

#define PHY2_Init                   RFM12_Init
#define PHY2_Send                   RFM12_Send
#define PHY2_Get                    RFM12_Get
#define PHY2_GetAddr                RFM12_GetAddr
#define PHY2_NodeId                 objRFNodeId

#else
#error RFM12_PHY unknown interface
#endif

#ifndef RF_ADDR_t

#define RF_ADDR_t                   uint8_t
#define RF_ADDR_TYPE                objUInt8
#define ADDR_UNDEF_RF               (RF_ADDR_t)0xFF

#endif  //  RF_ADDR_t

///////////////////////////////////////////////////////////////
// Configuration settings

#define OD_DEFAULT_GROUP        0x2DD4

#ifndef RF_BASE_FREQ
#define RF_BASE_FREQ                    868300000UL
#endif  //  RF_BASE_FREQ

// 433 MHz
#if (RF_BASE_FREQ > 433050000UL) && (RF_BASE_FREQ < 434790000UL)
#define RFM12_BAND          RFM12_BAND_433
#define OD_DEFAULT_CHANNEL  ((RF_BASE_FREQ - 433000000UL)/25000)
// 868 MHz
#elif (RF_BASE_FREQ > 868000000UL) && (RF_BASE_FREQ < 870000000UL)
#define RFM12_BAND          RFM12_BAND_868
#define OD_DEFAULT_CHANNEL  ((RF_BASE_FREQ - 868000000UL)/25000)
// 915 MHz
#elif (RF_BASE_FREQ > 902000000UL) && (RF_BASE_FREQ < 928000000UL)
#define RFM12_BAND          RFM12_BAND_915
#define OD_DEFAULT_CHANNEL  ((RF_BASE_FREQ - 902000000UL)/25000)
#else
#error  RF_BASE_FREQ does not belond to ISM band
#endif  // RF_BASE_FREQ


// HAL section
void        hal_rfm12_init_hw(void);
uint16_t    hal_rfm12_spiExch(uint16_t data);
bool        hal_rfm12_irq_stat(void);
void        hal_rfm12_enable_irq(void);

void        rfm12_irq(void);

// API Section
void        RFM12_Init(void);
void        RFM12_Send(void *pBuf);
void      * RFM12_Get(void);
void      * RFM12_GetAddr(void);

void        RFM12_ASleep(void);
void        RFM12_AWake(void);

#endif  //  _RFM12_PHY_H
