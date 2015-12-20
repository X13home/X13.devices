/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _MQTTSN_H
#define _MQTTSN_H

#ifdef __cplusplus
extern "C" {
#endif

// Current Status
typedef enum e_MQTTSN_STATUS
{
  MQTTSN_STATUS_DISCONNECTED = 0,
  MQTTSN_STATUS_DHCP,
  MQTTSN_STATUS_SEARCHGW,
  MQTTSN_STATUS_OFFLINE,
  MQTTSN_STATUS_PRE_CONNECT,
  MQTTSN_STATUS_CONNECT,
#ifdef ASLEEP
  MQTTSN_STATUS_ASLEEP_DISCONNECT,
  MQTTSN_STATUS_ASLEEP,
  MQTTSN_STATUS_AWAKE
#endif  //  ASLEEP
}e_MQTTSN_STATUS_t;

typedef enum e_TRACE_LEVEL
{
    lvlDEBUG    = 0,
    lvlINFO,
    lvlWARNING,
    lvlERROR
}TRACE_LEVEL_t;

void mqttsn_trace_msg(uint8_t Level, MQ_t * pMessage);

void mqttsn_parser_phy1(MQ_t * pPHY1outBuf);
void mqttsn_parser_phy2(MQ_t * pPHY2outBuf);

void MQTTSN_Init(void);
#ifdef ASLEEP
void MQTTSN_Set_ASleep(uint16_t val);
#endif  //  ASLEEP
e_MQTTSN_STATUS_t MQTTSN_GetStatus(void);

void MQTTSN_Poll(void);

bool MQTTSN_CanSend(void);
void MQTTSN_Send(e_MQTTSN_MSGTYPE_t      MsgType,
                 uint8_t                 Flags,
                 uint16_t                TopicId);

#ifdef __cplusplus
}
#endif

#endif
