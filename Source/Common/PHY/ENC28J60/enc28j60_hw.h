/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _ENC28J60_HW_H
#define _ENC28J60_HW_H

#include <stdbool.h>

// functions
void enc28j60Init(uint8_t* macaddr);
void enc28j60EnableBroadcast(void);
void enc28j60DisableBroadcast(void);
bool enc28j60linkup(void);

void enc28j60_SendPrep(uint16_t len);
void enc28j60_PutData(uint16_t len, uint8_t* pBuf);
void enc28j60_Fill(uint8_t len);
void enc28j60_Send(void);
//void enc28j60PacketSend(uint16_t len, uint8_t* packet);

bool en28j60_DataRdy(void);
//uint16_t enc28j60PacketReceive(uint16_t maxlen, uint8_t* packet);
uint16_t enc28j60_GetPacketLen(void);
void enc28j60_GetPacket(uint8_t * pBuf, uint16_t len);
void enc28j60_Skip(uint16_t len);
void enc28j60_ClosePacket(void);

#endif  //  _ENC28J60_API_H
