/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _ENC28J60_PHY_H
#define _ENC28J60_PHY_H

#ifdef __cplusplus
extern "C" {
#endif

#define LAN_NODE

#if (ENC28J60_PHY == 1)

#define PHY1_ADDR_t                 uint32_t
#define ADDR_BROADCAST_PHY1         (PHY1_ADDR_t)inet_addr(255,255,255,255)
#define ADDR_UNDEF_PHY1             (PHY1_ADDR_t)inet_addr(255,255,255,255)

#define PHY1_Init                   ENC28J60_Init
#define PHY1_Send                   ENC28J60_Send
#define PHY1_Get                    ENC28J60_Get
#define PHY1_GetAddr                ENC28J60_GetAddr
#define PHY1_NodeId                 objIPAddr
#define PHY1_GateId                 objIPBroker

#if (defined LED1_On) && (defined LED1_Off)
#define ENC_LED_ON                  LED1_On
#define ENC_LED_OFF                 LED1_Off
#endif  //  (defined LED1_On) && (defined LED1_Off)

#elif (ENC28J60_PHY == 2)

#define PHY2_ADDR_t                 uint32_t
#define ADDR_BROADCAST_PHY2         (PHY1_ADDR_t)inet_addr(255,255,255,255)
#define ADDR_UNDEF_PHY2             (PHY1_ADDR_t)inet_addr(255,255,255,255)

#define PHY2_Init                   ENC28J60_Init
#define PHY2_Send                   ENC28J60_Send
#define PHY2_Get                    ENC28J60_Get
#define PHY2_GetAddr                ENC28J60_GetAddr
#define PHY2_NodeId                 objIPAddr

#if (defined LED2_On) && (defined LED2_Off)
#define ENC_LED_ON                  LED2_On
#define ENC_LED_OFF                 LED2_Off
#endif  //  (defined LED1_On) && (defined LED1_Off)

#else
#error ENC28J60_PHY unknown inteface
#endif


#define inet_addr(d,c,b,a)          (((uint32_t)a<<24) | ((uint32_t)b << 16) | ((uint32_t)c<<8)  | ((uint32_t)d))

// API Section
void ENC28J60_Init(void);
void ENC28J60_Send(void *pBuf);
void * ENC28J60_Get(void);
void * ENC28J60_GetAddr(void);

#ifdef __cplusplus
}
#endif

#endif  //  _UART_PHY_H
