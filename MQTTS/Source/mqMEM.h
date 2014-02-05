/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

#ifndef _MQMEM_H
#define _MQMEM_H

void mqInit(void);
MQ_t * mqAssert(void);
void mqRelease(MQ_t * pBuf);
uint16_t mqFreeCnt(void);

#endif  //  _MQMEM_H

