/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

#include "../../config.h"

#ifdef WIZNET_EN

// node MAC & IP addresse
uint8_t macaddr[6];
uint8_t ipaddr[4];

#include "w5200.c"
#include "socket.c"

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
  // Initialize PHY
}

void PHY_Start(void)
{
}

MQ_t * PHY_GetBuf(void)
{
  return NULL;
}

void PHY_Send(MQ_t * pBuf)
{
  mqRelease(pBuf);
}

void PHY_Pool(void)
{
}

uint8_t PHY_BuildName(uint8_t * pBuf)
{
  sprinthex(&pBuf[0], ipaddr[0]);
  sprinthex(&pBuf[2], ipaddr[1]);
  sprinthex(&pBuf[4], ipaddr[2]);
  sprinthex(&pBuf[6], ipaddr[3]);
  return 8;
}

#endif  //  WIZNET_EN