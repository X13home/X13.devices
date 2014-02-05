/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

// Memory manager

#include "config.h"

MQ_t  * memPnt[MQMEM_SIZEOF_QUEUE];
MQ_t    memRaw[MQMEM_SIZEOF_QUEUE];

static uint16_t memMqCnt = 0;

//#define mqInit()
void mqInit(void)
{
  uint8_t i;
  for(i = 0; i < MQMEM_SIZEOF_QUEUE; i++)
    memPnt[i] = NULL;

  memMqCnt = (MQMEM_SIZEOF_QUEUE<<8) | MQMEM_SIZEOF_QUEUE;
}

//#define mqAssert() malloc(sizeof(MQ_t))
MQ_t * mqAssert(void)
{
  memMqCnt -= 256;

  uint8_t i;
  for(i = 0; i < MQMEM_SIZEOF_QUEUE; i++)
  {
    if(memPnt[i] == NULL)
    {
      memMqCnt--;
      memPnt[i] = &memRaw[i];
      return memPnt[i];
    }
  }
  return NULL;
}

//#define mqRelease(pBuf)  free(pBuf)
void mqRelease(MQ_t * pBuf)
{
  memMqCnt += 256;

  uint8_t i;
  for(i = 0; i < MQMEM_SIZEOF_QUEUE; i++)
  {
    if(pBuf == &memRaw[i])
    {
      memMqCnt++;
      memPnt[i] = NULL;
      break;
    }
  }
}

// Debug
uint16_t mqFreeCnt(void)
{
  return memMqCnt;
}
