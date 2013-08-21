/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

#ifndef _PHY_H
#define _PHY_H

#ifdef RF_NODE

enum e_RF_TRVSTATE
{
    RF_TRVPOR = 0,
    RF_TRVSLEEP,
    RF_TRVIDLE,
    RF_TRVRXIDLE,
    RF_TRVRXHDR,
    RF_TRVRXDATA,
    RF_TRVRXDONE,
    RF_TRVTXHDR,
    RF_TRVTXDATA,
    RF_TRVTXDONE,
    RF_TRVASLEEP,
    RF_TRVWKUP
};

#ifdef ASLEEP
void rf_SetState(uint8_t state);
#endif  //  ASLEEP
#ifdef RF_USE_RSSI
uint8_t rf_GetRSSI(void);
#endif  //  RF_USE_RSSI

uint8_t rf_GetNodeID(void);

uint8_t PHY_CanSend(void);

#else   // not defined RF_NODE

#define rf_GetNodeID()  0
void PHY_Start(void);

#endif  //  RF_NODE

// New Section
void PHY_LoadConfig(void);
void PHY_Init(void);
MQ_t * PHY_GetBuf(void);
void PHY_Send(MQ_t * pBuf);
void PHY_Pool(void);
uint8_t PHY_BuildName(uint8_t * pBuf);

#endif  //  _RF_H