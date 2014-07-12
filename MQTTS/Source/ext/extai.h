/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _EXTAI_H
#define _EXTAI_H

void aiClean(void);
uint8_t aiCheckIdx(subidx_t * pSubidx);
uint8_t aiRegisterOD(indextable_t *pIdx);
void aiDeleteOD(subidx_t * pSubidx);

#endif
