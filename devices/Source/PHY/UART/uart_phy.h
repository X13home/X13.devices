/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _UART_PHY_H
#define _UART_PHY_H

#if (UART_PHY == 1)

#define PHY1_ADDR_t                 uint8_t
#define ADDR_BROADCAST_PHY1         (PHY1_ADDR_t)0x00
#define ADDR_UNDEF_PHY1             (PHY1_ADDR_t)0xFF

#define PHY1_Init                   UART_Init
#define PHY1_Send                   UART_Send
#define PHY1_Get                    UART_Get
#define PHY1_GetAddr                UART_GetAddr
#define PHY1_NodeId                 objRFNodeId
#define PHY1_GateId                 objRFGateId

#elif (UART_PHY == 2)
    
#define PHY2_ADDR_t                 uint8_t
#define ADDR_BROADCAST_PHY2         (PHY2_ADDR_t)0x00
#define ADDR_UNDEF_PHY2             (PHY2_ADDR_t)0xFF

#define PHY2_Init                   UART_Init
#define PHY2_Send                   UART_Send
#define PHY2_Get                    UART_Get
#define PHY2_GetAddr                UART_GetAddr
#define PHY2_NodeId                 objRFNodeId

#else
#error unknown UART_PHY interface
#endif

#ifndef RF_ADDR_t

#define RF_ADDR_t                   uint8_t
#define RF_ADDR_TYPE                objUInt8
#define ADDR_UNDEF_RF               (RF_ADDR_t)0xFF

#endif  //  RF_ADDR_t

// API Section
void UART_Init(void);
void UART_Send(void *pBuf);
void * UART_Get(void);
void * UART_GetAddr(void);

#endif  //  _UART_PHY_H
