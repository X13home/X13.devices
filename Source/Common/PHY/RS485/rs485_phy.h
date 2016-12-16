/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _RS485_PHY_H
#define _RS485_PHY_H

#define HAL_USE_SUBMSTICK           1

#if (RS485_PHY == 1)

#define PHY1_ADDR_t                 uint8_t
#define ADDR_BROADCAST_PHY1         (PHY1_ADDR_t)0x00
#define ADDR_UNDEF_PHY1             (PHY1_ADDR_t)0xFF

#define PHY1_Init                   RS485_Init
#define PHY1_Send                   RS485_Send
#define PHY1_Get                    RS485_Get
#define PHY1_GetAddr                RS485_GetAddr
#define PHY1_NodeId                 objRFNodeId
#define PHY1_GateId                 objRFGateId

#elif (RS485_PHY == 2)
    
#define PHY2_ADDR_t                 uint8_t
#define ADDR_BROADCAST_PHY2         (PHY2_ADDR_t)0x00
#define ADDR_UNDEF_PHY2             (PHY2_ADDR_t)0xFF

#define PHY2_Init                   RS485_Init
#define PHY2_Send                   RS485_Send
#define PHY2_Get                    RS485_Get
#define PHY2_GetAddr                RS485_GetAddr
#define PHY2_NodeId                 objRFNodeId

#else
#error unknown RS485_PHY interface
#endif

#ifndef RF_ADDR_t

#define RF_ADDR_t                   uint8_t
#define RF_ADDR_TYPE                objUInt8
#define ADDR_UNDEF_RF               (RF_ADDR_t)0xFF

#endif  //  RF_ADDR_t

// API Section
void    RS485_Init(void);
void    RS485_Send(void *pBuf);
void  * RS485_Get(void);
void  * RS485_GetAddr(void);

#endif  //  _RS485_PHY_H

