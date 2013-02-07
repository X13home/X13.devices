/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

#ifndef _RF_H
#define _RF_H

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

void rf_LoadCfg(uint8_t Channel, uint16_t Group, uint8_t ID);
void rf_Initialize(void);
void rf_SetState(uint8_t state);
#ifdef RF_USE_RSSI
uint8_t rf_GetRSSI(void);
#endif  //  RF_USE_RSSI
uint8_t * rf_GetBuf(void);
uint8_t rf_GetNodeID(void);
void rf_Send(uint8_t * pBuf);
void rf_Pool(void);

#endif  //  _RF_H