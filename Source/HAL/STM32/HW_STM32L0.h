/*
Copyright (c) 2011-2016 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef HW_STM32L0_H
#define HW_STM32L0_H

#include "stm32l0xx.h"

// UNode V3.0 - STM32L051K8
#if   (defined CFG_S5Cn10)
#include "S5xxxx/S5Cn10.h"      // CC1101

#else
#error Unknown configuration
#endif  //  Configuration

//////////////////////////////////////////////////////////////
// EEPROM Emulation Section
#define FEE_PAGE_SIZE           (uint32_t)0x00000400    // Size of FLASH Page - 1k

#define FEE_SIZE                0x2000                  // 8K
#define FEE_TRANSFER_PAGES      2                       // Reserved Pages for Transfer

// WARNING !! does not check overlapping with program memory
#define FEE_BASE ((FLASH_BANK1_END + 1) - FEE_SIZE)

#define FEE_EraseTimeout        (uint32_t)0x000B0000
#define FEE_ProgramTimeout      (uint32_t)0x000B0000
// End EEPROM Emulation Section
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// SPI Section

#define DIO_MODE_SPI1               DIO_MODE_AF_PP_HS    

#define SPI1_PORT                   GPIOB
#define SPI1_SCK_PIN                GPIO_Pin_3
#define SPI1_MISO_PIN               GPIO_Pin_4
#define SPI1_MOSI_PIN               GPIO_Pin_5

// End SPI Section
//////////////////////////////////////////////////////////////


#endif  //  HW_STM32L0_H
