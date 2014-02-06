/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

// Memory manager

#include "config.h"

MQ_t    memRaw[MQMEM_SIZEOF_QUEUE];
static uint8_t memTTL[MQMEM_SIZEOF_QUEUE];  

//#define mqInit()
void mqInit(void)
{
  uint8_t i;
  for(i = 0; i < MQMEM_SIZEOF_QUEUE; i++)
    memTTL[i] = 0;
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
    if(pnt == MQMEM_SIZEOF_QUEUE)
      pnt = 0;
    
    if(memTTL[pnt] == 0)
    {
      memTTL[pnt] = 255;
      return &memRaw[pnt];
    }
    else
      memTTL[pnt]--;
  };

  return NULL;
}

//#define mqRelease(pBuf)  free(pBuf)
void mqRelease(MQ_t * pBuf)
{
  uint8_t i;
  for(i = 0; i < MQMEM_SIZEOF_QUEUE; i++)
    if(pBuf == &memRaw[i])
      memTTL[i] = 0;
}
