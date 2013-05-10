/*
Copyright (c) 2011-2012 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

#ifndef _CC11_H
#define _CC11_H

#define     OD_DEFAULT_GROUP    0x2DD4

// 433 MHz
#if (RF_BASE_FREQ > 433050000UL) && (RF_BASE_FREQ < 434790000UL)
#define OD_DEFAULT_CHANNEL  ((RF_BASE_FREQ - 433000000UL)/25000)
#define CC11_DEFVAL_FREQ2     0x10
#define CC11_DEFVAL_FREQ1     0xA7
#define CC11_DEFVAL_FREQ0     0x62

// 868 MHz
#elif (RF_BASE_FREQ > 868000000UL) && (RF_BASE_FREQ < 870000000UL)
#define OD_DEFAULT_CHANNEL  ((RF_BASE_FREQ - 868000000UL)/25000)
#define CC11_DEFVAL_FREQ2     0x21
#define CC11_DEFVAL_FREQ1     0x62
#define CC11_DEFVAL_FREQ0     0x76

// 915 MHz
#elif (RF_BASE_FREQ > 902000000UL) && (RF_BASE_FREQ < 928000000UL)
#define OD_DEFAULT_CHANNEL  ((RF_BASE_FREQ - 902000000UL)/25000)
#define CC11_DEFVAL_FREQ2     0x22
#define CC11_DEFVAL_FREQ1     0xB1
#define CC11_DEFVAL_FREQ0     0x3B

// Bad Frequency
#else
#error  RF_BASE_FREQ does not belond to ISM band
#endif  // RF_BASE_FREQ

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

void cc11_LoadCfg(uint8_t Channel, uint16_t Group, uint8_t ID);
void cc11_Initialize(void);
void cc11_SetState(uint8_t state);
uint8_t * cc11_GetBuf(uint8_t *pAddr);
uint8_t cc11_GetNodeID(void);
void cc11_Send(uint8_t * pBuf, uint8_t * pAddr);
void cc11_Pool(void);

#endif
