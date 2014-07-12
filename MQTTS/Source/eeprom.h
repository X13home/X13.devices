/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _EEPROM_H
#define _EEPROM_H

void eepromReadListOD(uint16_t addr, subidx_t *pIdx);
void eepromWriteListOD(uint16_t addr, subidx_t *pIdx);
uint8_t eepromReadOD(subidx_t *pIdx, uint8_t *pLen, uint8_t *pBuf);
uint8_t progmemReadOD(subidx_t *pIdx, uint8_t *pLen, uint8_t *pBuf);
uint8_t eepromWriteOD(subidx_t *pIdx, uint8_t Len, uint8_t *pBuf);

#endif