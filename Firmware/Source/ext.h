/*
Copyright (c) 2011-2012 <comparator@gmx.de>

This file is part of the X13.Home project.
https://github.com/X13home

BSD License
See LICENSE.txt file for license details.
*/

#ifndef _EXT_H_
#define _EXT_H_

void    extClean(void);                                     // Init extensions
void    extConfig(void);                                    // Configure extensions
uint8_t extRegisterOD(indextable_t * pIdx);                 // Register Object
uint8_t extCheckIdx(subidx_t * pSubidx);                    // Check Index ->frei/busy/invalid
void extDeleteOD(subidx_t * pSubidx);                       // Delete Object
uint8_t extCvtIdx2TopicId(subidx_t * pSubidx, uint8_t * pPnt);  // Convert Index to Topic ID

#endif  //  _EXT_H_
