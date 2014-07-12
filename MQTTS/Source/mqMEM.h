/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _MQMEM_H
#define _MQMEM_H

void mqInit(void);
MQ_t * mqAssert(void);
void mqRelease(MQ_t * pBuf);

#endif  //  _MQMEM_H

