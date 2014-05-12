/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// Memory manager

#include "config.h"

//#define MQMEM_TTL   64

MQ_t    memRaw[MQMEM_SIZEOF_QUEUE];
static uint8_t memTTL[MQMEM_SIZEOF_QUEUE];  
#ifdef MQ_DEBUG
uint8_t memFreeCnt;
#endif // MQ_DEBUG

//#define mqInit()
void mqInit(void)
{
  uint8_t i;
  for(i = 0; i < MQMEM_SIZEOF_QUEUE; i++)
    memTTL[i] = 0;
#ifdef MQ_DEBUG
  memFreeCnt = MQMEM_SIZEOF_QUEUE;
#endif  //  MQ_DEBUG
}

//#define mqAssert() malloc(sizeof(MQ_t))
MQ_t * mqAssert(void)
{
  static uint8_t pnt = 0xFF;
  uint8_t i = MQMEM_SIZEOF_QUEUE;

  while(i > 0)
  {
    i--;
    pnt++;
    if(pnt >= MQMEM_SIZEOF_QUEUE)
      pnt = 0;
    
    if(memTTL[pnt] == 0)
    {
#ifdef MQ_DEBUG
      memFreeCnt--;
#endif  //  MQ_DEBUF
#ifdef  MQMEM_TTL
      memTTL[pnt] = MQMEM_TTL;
#else   //  !MQMEM_TTL
      memTTL[pnt] = 0xFF;
#endif  //  MQMEM_TTL
      return &memRaw[pnt];
    }
  };

  return NULL;
}

//#define mqRelease(pBuf)  free(pBuf)
void mqRelease(MQ_t * pBuf)
{
  uint8_t i;
  for(i = 0; i < MQMEM_SIZEOF_QUEUE; i++)
    if(pBuf == &memRaw[i])
    {
      memTTL[i] = 0;
#ifdef MQ_DEBUG
      memFreeCnt++;
#endif  //  MQ_DEBUG
#ifndef MQMEM_TTL
      break;
#endif  //  MQMEM_TTL
    }
#ifdef  MQMEM_TTL
    else if(memTTL[i] > 0)
    {
      memTTL[i]--;
#ifdef MQ_DEBUG
      if(memTTL[i] == 0)
        memFreeCnt++;
#endif  //  MQ_DEBUG
    }
#endif  // MQMEM_TTL
}

#ifdef MQ_DEBUG
uint8_t mqGetFreeCnt(void)
{
  return memFreeCnt;
}
#endif  //  MQ_DEBUG