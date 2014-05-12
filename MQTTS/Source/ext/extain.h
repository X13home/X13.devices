/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _EXTAIN_H
#define _EXTAIN_H

void ainClean(void);
uint8_t ainCheckIdx(subidx_t * pSubidx);
uint8_t ainRegisterOD(indextable_t *pIdx);
void ainDeleteOD(subidx_t * pSubidx);

#endif  //  _EXTAIN_H
