/*
Copyright (c) 2011-2012 <comparator@gmx.de>

This file is part of the X13.Home project.
https://github.com/X13home

BSD License
See LICENSE.txt file for license details.
*/

#include <avr/pgmspace.h>
#include "util.h"
/*
uint8_t sprintstrP(uint8_t *pBuf, const char *s)
{
    uint8_t len = 0;
    uint8_t c;
    while((c = pgm_read_byte(s++)) != 0)
    {
        *(pBuf++) = c;
        len++;
    }
    return len;
}
*/
uint8_t sprinthex(uint8_t *pBuf, uint8_t val)
{
    uint8_t ch = val>>4;
    *(pBuf++) = '0' + (ch > 9 ? ch + 7 : ch);
    ch = val & 0x0F;
    *pBuf = '0' + (ch > 9 ? ch + 7 : ch);
    return 2;
}

uint8_t sprintdec(uint8_t *pBuf, uint16_t val)
{
    uint16_t div = 10000;
    uint8_t ch = 0, fl = 0, len = 1;
    
    while(div >= 10)
    {
        while(val >= div)
        {
            val -=div;
            ch++;
            fl = 1;
        }

        switch(div)
        {
            case 10000:
                div = 1000;
                break;
            case 1000:
                div = 100;
                break;
            case 100:
                div = 10;
                break;
            default:
                div = 1;
                break;
        }
//        div = div/10;

        if(fl)
        {
            *(pBuf++) = ch + '0';
            ch = 0;
            len++;
        }
    }
    *pBuf = val + '0';
    
    return len;
}