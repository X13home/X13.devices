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

// EEPROM Emulation Section
#define FEE_EraseTimeout        ((uint32_t)0x000B0000)
#define FEE_ProgramTimeout      ((uint32_t)0x00002000)

// HD + CL Devices
#if defined(STM32F100xE) ||                         \
    defined(STM32F101xE) || defined(STM32F101xG) || \
    defined(STM32F103xE) || defined(STM32F103xG) || \
    defined(STM32F105xC) || defined(STM32F107xC)
#define FEE_SECTOR_SIZE         (uint32_t)0x00000800                // Size of FLASH Sectors - 2k
#else                                                               // LD + MD
#define FEE_SECTOR_SIZE         (uint32_t)0x00000400                // Size of FLASH Sectors - 1k
#endif

#define FEE_SECTORS             8                                   // Size of 'EEPROM' in sectors
#define FEE_TRANSIT_SECTORS     2                                   // Reserved Sectors for Transfer

// WARNING !! does not check overlapping with program memory
#define FEE_BASE ((FLASH_BANK1_END + 1) - (FEE_SECTORS * FEE_SECTOR_SIZE))
// End EEPROM Emulation Section

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
