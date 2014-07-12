/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _TWIM_H
#define _TWIM_H

#define TW_SUCCESS      0xFF

#define TWIM_READ       1               // Read Data
#define TWIM_WRITE      2               // Write Data
#define TWIM_BUSY       8               // Bus Busy
#define TWIM_ERROR      0x80            // Bus Error

typedef void (*cbTWI)(void);            // TWI ISR ready, Callback function

void twiClean();
void twiConfig(void);

uint8_t twimExch(uint8_t addr, uint8_t access, uint8_t write, uint8_t read, uint8_t *pBuf);
void twimExch_ISR(uint8_t addr, uint8_t access, uint8_t write, uint8_t read, uint8_t *pBuf, cbTWI pCallback);

#endif
