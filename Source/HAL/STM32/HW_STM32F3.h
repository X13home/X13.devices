/*
Copyright (c) 2011-2016 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef HW_STM32F3_H
#define HW_STM32F3_H

// EEPROM Emulation Section

#define FEE_SIZE                0x2000                  // 8K
#define FEE_TRANSFER_PAGES      2                       // Reserved Pages for Transfer

#if defined (__STM32F303x8_H)
// 64k Version
#define FEE_PAGE_SIZE           (uint32_t)0x00000800    // Size of FLASH Page - 2k
#define FEE_BASE                (((uint32_t)0x08010000) - FEE_SIZE)
#endif  //  uC

// End EEPROM Emulation Section

// STM32F303K8T6 - Modified Nucleo
#if   (defined CFG_S4Sn10)
#include "S4xxxx/S4Sn10.h"    // UART
// UNodeV3, STM32F303K8 / STM32F334K8
#elif   (defined CFG_S4Cn11)
#include "S4xxxx/S4Cn11.h"    // CC1101
#elif   (defined CFG_S4SC11)
#include "S4xxxx/S4SC11.h"    // UART + CC1101
#else
#error Unknown configuration
#endif  //  Configuration

#endif  //  HW_STM32F3_H
