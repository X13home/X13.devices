/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

// EEPROM and FLASH access

#include "config.h"
#include "eeprom.h"

void eepromReadListOD(uint16_t addr, subidx_t *pSubidx)
{
    eeprom_read_block(pSubidx, (const void *)addr, sizeof(subidx_t));
}

void eepromWriteListOD(uint16_t addr, subidx_t *pSubidx)
{
    eeprom_write_block(pSubidx, (void *)addr, sizeof(subidx_t));
}

uint8_t eepromReadOD(subidx_t *pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
    uint16_t Base = pSubidx->Base;

    switch(pSubidx->Type)
    {
        case objBool:
        case objInt8:
        case objUInt8:
            *pLen = sizeof(uint8_t);
            *pBuf = eeprom_read_byte((const void *)Base);
            return MQTTS_RET_ACCEPTED;
        case objInt16:
        case objUInt16:
            *pLen = sizeof(uint16_t);
            break;
        case objInt32:
        case objUInt32:
            *pLen = sizeof(uint32_t);
            break;
        case objString:
        case objArray:
            {
            uint8_t Len = eeprom_read_byte((const void *)Base);
            *pLen = Len < *pLen ? Len : *pLen;
            Base++;
            break;
            }
        default:
            return MQTTS_RET_REJ_NOT_SUPP;
    }
    eeprom_read_block(pBuf, (const void *)Base, *pLen);
    return MQTTS_RET_ACCEPTED;
}

uint8_t progmemReadOD(subidx_t *pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
    uint16_t Base = pSubidx->Base;

    switch(pSubidx->Type)
    {
        case objBool:
        case objInt8:
        case objUInt8:
            *pLen = sizeof(uint8_t);
            *pBuf = pgm_read_byte((const void *)Base);
            return MQTTS_RET_ACCEPTED;
        case objInt16:
        case objUInt16:
            *pLen = sizeof(uint16_t);
            *(uint16_t *)pBuf = pgm_read_word((const void *)Base);
            return MQTTS_RET_ACCEPTED;
        case objInt32:
        case objUInt32:
            *pLen = sizeof(uint32_t);
            *(uint32_t *)pBuf = pgm_read_dword((const void *)Base);
            return MQTTS_RET_ACCEPTED;
        case objString:
        case objArray:
            {
            uint8_t Len = pgm_read_byte((const void *)Base);
            if(*pLen < Len)
                Len = *pLen;
            Base++;
            memcpy_P((void *)pBuf, (const void *)Base, Len);
            *pLen = Len;
            }
            break;
    }
    return MQTTS_RET_REJ_NOT_SUPP;
}

uint8_t eepromWriteOD(subidx_t *pSubidx, uint8_t Len, uint8_t *pBuf)
{
    uint16_t Base = pSubidx->Base;

    switch(pSubidx->Type)
    {
        case objBool:
        case objInt8:
        case objUInt8:
            Len = sizeof(uint8_t);
            break;
        case objInt16:
        case objUInt16:
            Len = sizeof(uint16_t);
            break;
        case objInt32:
        case objUInt32:
            Len = sizeof(uint32_t);
            break;
        case objString:
        case objArray:
            eeprom_write_byte((uint8_t *)Base, Len);
            Base++;
            break;
        default:
            return MQTTS_RET_REJ_NOT_SUPP;
    }
    eeprom_write_block(pBuf, (void *)Base, Len);
    return MQTTS_RET_ACCEPTED;
}
