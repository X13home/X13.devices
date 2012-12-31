/*
Copyright (c) 2011-2012 <comparator@gmx.de>

This file is part of the X13.Home project.
https://github.com/X13home

BSD License
See LICENSE.txt file for license details.
*/

// Memory manager

#include "config.h"

MQ_t  * memPnt[MQMEM_SIZEOF_QUEUE];
MQ_t    memRaw[MQMEM_SIZEOF_QUEUE];

//#define mqInit()
void mqInit(void)
{
    uint8_t i;
    for(i = 0; i < MQMEM_SIZEOF_QUEUE; i++)
        memPnt[i] = NULL;
}

//#define mqAssert() malloc(sizeof(MQ_t))
MQ_t * mqAssert(void)
{
    uint8_t i;
    for(i = 0; i < MQMEM_SIZEOF_QUEUE; i++)
    {
        if(memPnt[i] == NULL)
        {
            memPnt[i] = &memRaw[i];
            return memPnt[i];
        }
    }
    return NULL;
}

//#define mqRelease(pBuf)  free(pBuf)
void mqRelease(MQ_t * pBuf)
{
    uint8_t i;
    for(i = 0; i < MQMEM_SIZEOF_QUEUE; i++)
    {
        if(pBuf == &memRaw[i])
        {
            memPnt[i] = NULL;
            break;
        }
    }
}
