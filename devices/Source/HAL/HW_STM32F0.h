/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef HW_STM32F0_H
#define HW_STM32F0_H

// STM32F051R8T6
#if   (defined CFG_S2EC13)
#include "HWconfig_S2EC13.h"    // ENC28J60 + CC1101
#elif   (defined CFG_S2En13)
#include "HWconfig_S2En13.h"    // ENC28J60
#elif   (defined CFG_S2Cn13)
#include "HWconfig_S2Cn13.h"    // CC1101
#else
#error Unknown configuration
#endif  //  Configuration

#endif  //  HW_STM32F0_H
