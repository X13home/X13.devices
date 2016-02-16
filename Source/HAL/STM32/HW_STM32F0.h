/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef HW_STM32F0_H
#define HW_STM32F0_H

// EEPROM Emulation Section
#if defined (STM32F070x6) || defined (STM32F070xB) ||   \
    defined (STM32F071xB) || defined (STM32F072xB) ||   \
    defined (STM32F078xx) || defined (STM32F091xC) ||   \
    defined (STM32F098xx)
#define FEE_PAGE_SIZE           (uint32_t)0x00000800    // Size of FLASH Page - 2k
#else
#define FEE_PAGE_SIZE           (uint32_t)0x00000400    // Size of FLASH Page - 1k
#endif

#define FEE_SIZE                0x2000                  // 8K
#define FEE_TRANSFER_PAGES      2                       // Reserved Pages for Transfer

// WARNING !! does not check overlapping with program memory
#define FEE_BASE ((FLASH_BANK1_END + 1) - FEE_SIZE)
// End EEPROM Emulation Section

// STM32F051C8
#if   (defined CFG_S2EC13)
#include "S2xxxx/S2EC13.h"    // ENC28J60 + CC1101
#elif   (defined CFG_S2En13)
#include "S2xxxx/S2En13.h"    // ENC28J60
#elif   (defined CFG_S2Cn13)
#include "S2xxxx/S2Cn13.h"    // CC1101
#elif   (defined CFG_S2Sn13)
#include "S2xxxx/S2Sn13.h"    // UART_ALT

// STM32F051R8
#elif   (defined CFG_S2Sn10)
#include "S2xxxx/S2Sn10.h"    // UART

// STM32F091RC
#elif   (defined CFG_S2Sn11)
#include "S2xxxx/S2Sn11.h"    // UART

#else
#error Unknown configuration
#endif  //  Configuration

#endif  //  HW_STM32F0_H
