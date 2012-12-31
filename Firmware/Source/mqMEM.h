/*
Copyright (c) 2011-2012 <comparator@gmx.de>

This file is part of the X13.Home project.
https://github.com/X13home

BSD License
See LICENSE.txt file for license details.
*/

#ifndef _MQMEM_H
#define _MQMEM_H

void mqInit(void);
MQ_t * mqAssert(void);
void mqRelease(MQ_t * pBuf);

#endif  //  _MQMEM_H

