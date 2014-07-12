/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _EXTSER_H
#define _EXTSER_H

void serClean(void);
uint8_t serCheckIdx(subidx_t * pSubidx);
uint8_t serRegisterOD(indextable_t *pIdx);
void serDeleteOD(subidx_t * pSubidx);

#endif
