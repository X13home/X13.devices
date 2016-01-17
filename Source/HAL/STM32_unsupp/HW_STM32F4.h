/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef HW_STM32F4_H
#define HW_STM32F4_H

// STM32F401RE
#if     (defined CFG_S4SN12)
#include "HWconfig_S4Sn12.h"    // UART
#elif   (defined CFG_S4ES12)
#include "HWconfig_S4ES12.h"    // ENC28J60 + UART
// STM32F405RG
#elif   (defined CFG_S4SN13)
#include "HWconfig_S4Sn13.h"    // UART
#else
#error Unknown configuration
#endif  //  Configuration

#endif  //  HW_STM32F0_H
