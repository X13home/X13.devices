/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// API
// Load/Change configuration parameters

#include "../../config.h"

#ifdef DUMMY

static uint8_t dummy_NodeID;


void PHY_LoadConfig(void)
{
  uint8_t Len = sizeof(dummy_NodeID);
  ReadOD(objRFNodeId, MQTTS_FL_TOPICID_PREDEF,  &Len, &dummy_NodeID);
}

void PHY_Init(void)
{}

#ifdef ASLEEP
// Change state
void rf_SetState(uint8_t state)
{}
#endif

uint8_t rf_GetNodeID(void)
{
    return dummy_NodeID;
}

MQ_t * PHY_GetBuf(void)
{
  return NULL;
}

// Can Send ? Then switch to TX mode
uint8_t PHY_CanSend(void)
{
  return 1;
}

void PHY_Send(MQ_t * pBuf)
{
  mqRelease(pBuf);
}

void PHY_Poll(void)
{}

uint8_t PHY_BuildName(uint8_t * pBuf)
{
  sprinthex(&pBuf[0], dummy_NodeID);
  return 2;
}
#endif  // DUMMY