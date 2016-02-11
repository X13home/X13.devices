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

// EEPROM Section
#define FEE_EraseTimeout    (uint32_t)0x000B0000
#define FEE_ProgramTimeout  (uint32_t)0x000B0000


#if (defined STM32F051x8)

#define FEE_BASE_ADDRESS    (uint32_t)0x0800F000    // Base for 'EEPROM'
#define FEE_EEPROM_SIZE     (uint32_t)0x00000800    // Size of 'EEPROM'
#define FEE_SECTOR_SIZE     (uint32_t)0x00000400    // Size of FLASH Sectors
#define FEE_MIRROR_NUMBER   (uint8_t)2

#endif  //  STM32F051x8
// End EEPROM Section

// STM32F051C8T6
#if   (defined CFG_S2EC13)
#include "S2xxxx/S2EC13.h"    // ENC28J60 + CC1101
#elif   (defined CFG_S2En13)
#include "S2xxxx/S2En13.h"    // ENC28J60
#elif   (defined CFG_S2Cn13)
#include "S2xxxx/S2Cn13.h"    // CC1101
#elif   (defined CFG_S2Sn13)
#include "S2xxxx/S2Sn13.h"    // UART_ALT

// STM32F051R8T6
#elif   (defined CFG_S2Sn10)
#include "S2xxxx/S2Sn10.h"    // UART

#else
#error Unknown configuration
#endif  //  Configuration

#endif  //  HW_STM32F0_H
