/*
Copyright (c) 2011-2012 <comparator@gmx.de>

This file is part of the X13.Home project.
https://github.com/X13home

BSD License
See LICENSE.txt file for license details.
*/

// RF Prototypes

#include "config.h"

#if (defined _RFM12_H)
#include "rf/rfm12/rfm12.c"
#elif (defined _CC11_H)
#include "rf/cc11/cc11.c"
#else
#error RF TRV not defined
#endif  //  HWCONFIG
