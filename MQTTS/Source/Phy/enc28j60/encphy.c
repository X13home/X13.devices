/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

#include "../../config.h"

#ifdef ENC28J60_EN

#include "ip_arp_udp.h"
#include "enc28j60.h"
#include "net.h"

#include "enc28j60.c"
#include "ip_arp_udp.c"

// node MAC & IP addresse
uint8_t macaddr[6];
uint8_t ipaddr[4];

// eth/ip buffer:
#define MAX_FRAME_BUF 350
static uint8_t buf[MAX_FRAME_BUF];

void PHY_LoadConfig(void)
{
  uint8_t Len;

  Len = 6;
  ReadOD(objMACAddr, MQTTS_FL_TOPICID_PREDEF, &Len, (uint8_t *)macaddr);
  Len = 4;
  ReadOD(objIPAddr, MQTTS_FL_TOPICID_PREDEF, &Len, (uint8_t *)ipaddr);
//  ReadOD(objIPBroker, MQTTS_FL_TOPICID_PREDEF, &Len, (uint8_t *)gwip);
}

void PHY_Init(void)
{
  //initialize enc28j60
  enc28j60Init(macaddr);
}

void PHY_Start(void)
{
#ifdef  DHCP_client
  if(ipaddr[0] == 0xFF)
  {
    // DHCP handling. Get the initial IP
    uint8_t i;
    uint16_t plen;

    i = 0;
    while(i == 0)
    {
      plen = enc28j60PacketReceive(MAX_FRAME_BUF, buf);
      i = packetloop_dhcp_initial_ip_assignment(buf, plen);
    }
  }
#else
  while (enc28j60linkup()==0)
    _delay_ms(250);
#endif  //  DHCP_client
}

MQ_t * PHY_GetBuf(void)
{
  uint16_t plen;
  MQ_t * pTmp;

  plen = enc28j60PacketReceive(MAX_FRAME_BUF, buf);
  if(packetloop_lan(buf, plen))    // 1 - Unicast, 2 - Broadcast
  {
    if((buf[UDP_DST_PORT_H_P] == (MQTTS_UDP_PORT >> 8)) &&
       (buf[UDP_DST_PORT_L_P] == (MQTTS_UDP_PORT & 0xFF)))
    {
      plen -= UDP_DATA_P;
      if((plen > sizeof(MQ_t)) || ((pTmp = mqAssert()) == NULL))
        return NULL;
        
      memcpy(&pTmp->addr.mac, &buf[ETH_SRC_MAC], 6);
      memcpy(&pTmp->addr.ip,  &buf[IP_SRC_P], 4);
        
      memcpy(&pTmp->mq, &buf[UDP_DATA_P], plen);
      return pTmp;
    }
  }
  return NULL;
}

void PHY_Send(MQ_t * pBuf)
{
  send_udp_prepare(buf, MQTTS_UDP_PORT, pBuf->addr.ip, MQTTS_UDP_PORT, pBuf->addr.mac);
  
  uint16_t datalen = pBuf->mq.Length;

  memcpy(&buf[UDP_DATA_P], &pBuf->mq, datalen);
  mqRelease(pBuf);
  
  send_udp_transmit(buf, datalen);
}

void PHY_Pool(void)
{
  static uint8_t PoolCnt = POOL_TMR_FREQ - 1;

  if(PoolCnt)
    PoolCnt--;
  else
  {
    PoolCnt = POOL_TMR_FREQ - 1;
    sec_tick_lan();
  }
}

uint8_t PHY_BuildName(uint8_t * pBuf)
{
  sprinthex(&pBuf[0], ipaddr[0]);
  sprinthex(&pBuf[2], ipaddr[1]);
  sprinthex(&pBuf[4], ipaddr[2]);
  sprinthex(&pBuf[6], ipaddr[3]);
  return 8;
}

#endif  //  _ENCPHY_H
