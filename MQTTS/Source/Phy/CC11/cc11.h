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

#endif
