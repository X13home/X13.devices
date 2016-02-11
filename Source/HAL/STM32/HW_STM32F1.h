/*
Copyright (c) 2011-2016 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef HW_STM32F1_H
#define HW_STM32F1_H

// EEPROM Section
#define FEE_EraseTimeout    ((uint32_t)0x000B0000)
#define FEE_ProgramTimeout  ((uint32_t)0x00002000)

#if (defined STM32F103xB)
#define FEE_BASE_ADDRESS    (uint32_t)0x0801F000    // Base for 'EEPROM'
#define FEE_EEPROM_SIZE     (uint32_t)0x00000800    // Size of 'EEPROM'
#define FEE_SECTOR_SIZE     (uint32_t)0x00000400    // Size of FLASH Sectors
#define FEE_MIRROR_NUMBER   (uint8_t)2
#endif
// End EEPROM Section

// STM32F103CBT6, Maple Mini
#if   (defined CFG_S3Sn10)
#include "S3xxxx/S3Sn10.h"    // UART
#elif (defined CFG_S3SC10)
#include "S3xxxx/S3SC10.h"    // UART + CC1101
#elif (defined CFG_S3En10)
#include "S3xxxx/S3En10.h"    // ENC28J60
#elif (defined CFG_S3Cn10)
#include "S3xxxx/S3Cn10.h"    // CC1101
#else
#error Unknown configuration
#endif  //  Configuration

#endif  //  HW_STM32F1_H
