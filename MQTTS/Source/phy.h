/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
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

// PHY API
void PHY_LoadConfig(void);                  // Load PHY Configuration from database
void PHY_Init(void);                        // Initialise PHY
MQ_t * PHY_GetBuf(void);                    // Get Data from PHY, return pointer to data or NULL
void PHY_Send(MQ_t * pBuf);                 // Send data via PHY
void PHY_Poll(void);                        // PHY polling
uint8_t PHY_BuildName(uint8_t * pBuf);      // Build Node name
#endif  //  _RF_H