/*
Copyright (c) 2011-2012 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

#ifndef _RFM12_H
#define _RFM12_H

#include "rfm12reg.h"

#define     OD_DEFAULT_GROUP    0x2DD4

// 433 MHz
#if (RF_BASE_FREQ > 433050000UL) && (RF_BASE_FREQ < 434790000UL)
#define RFM12_BAND          RFM12_BAND_433
#define OD_DEFAULT_CHANNEL  ((RF_BASE_FREQ - 433000000UL)/25000)
// 868 MHz
#elif (RF_BASE_FREQ > 868000000UL) && (RF_BASE_FREQ < 870000000UL)
#define RFM12_BAND          RFM12_BAND_868
#define OD_DEFAULT_CHANNEL  ((RF_BASE_FREQ - 868000000UL)/25000)
// 915 MHz
#elif (RF_BASE_FREQ > 902000000UL) && (RF_BASE_FREQ < 928000000UL)
#define RFM12_BAND          RFM12_BAND_915
#define OD_DEFAULT_CHANNEL  ((RF_BASE_FREQ - 902000000UL)/25000)
#else
#error  RF_BASE_FREQ does not belond to ISM band
#endif  // RF_BASE_FREQ

// Mode definition
#define RFM12_SLEEP_MODE    (RFM12_CMD_PWRMGT | RFM12_PWRMGT_DC)
#define RFM12_IDLE_MODE     (RFM12_CMD_PWRMGT | \
                             RFM12_PWRMGT_ES | RFM12_PWRMGT_EX | RFM12_PWRMGT_DC)
#define RFM12_RECEIVE_MODE  (RFM12_CMD_PWRMGT | RFM12_PWRMGT_ER | RFM12_PWRMGT_EBB | \
                             RFM12_PWRMGT_ES | RFM12_PWRMGT_EX | RFM12_PWRMGT_DC)
#define RFM12_TRANSMIT_MODE (RFM12_CMD_PWRMGT | RFM12_PWRMGT_ET | \
                             RFM12_PWRMGT_ES | RFM12_PWRMGT_EX | RFM12_PWRMGT_DC)

#define RFM12_TXFIFO_DIS    (RFM12_CMD_CFG | RFM12_CFG_EF | RFM12_BAND | RFM12_XTAL_12PF)
#define RFM12_TXFIFO_ENA    (RFM12_CMD_CFG | RFM12_CFG_EL | RFM12_CFG_EF | RFM12_BAND | \
                             RFM12_XTAL_12PF)

#define RFM12_RXFIFO_DIS    (RFM12_CMD_FIFORESET | (0x08<<RFM12_FIFOITLVL_OFS) | RFM12_FIFORESET_DR)
#define RFM12_RXFIFO_ENA    (RFM12_CMD_FIFORESET | (0x08<<RFM12_FIFOITLVL_OFS) | \
                             RFM12_FIFORESET_FF  | RFM12_FIFORESET_DR)
// Chip configuration
#define RFM12_BAND          RFM12_BAND_868
#define RFM12_BAUD          RFM12_BAUD_38K4         // Follow FSK shift & bandwidth
// Quartz +-50 ppm
#define RFM12_BANDWIDTH     RFM12_RXCTRL_BW_270
#define RFM12_FSKWIDTH      RFM12_TXCONF_FS_135

//#define RFM12_BANDWIDTH     RFM12_RXCTRL_BW_134
//#define RFM12_FSKWIDTH      RFM12_TXCONF_FS_90

#define RFM12_GAIN          RFM12_RXCTRL_LNA_6
#define RFM12_DRSSI         RFM12_RXCTRL_RSSI_91
#define RFM12_POWER         RFM12_TXCONF_POWER_0

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

#endif  //  _RFM12_H
