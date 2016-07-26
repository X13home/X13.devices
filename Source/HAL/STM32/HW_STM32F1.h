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

#include "stm32f1xx.h"

// STM32F103CBT6, Maple Mini
#if   (defined CFG_S3Sn10)
#include "S3xxxx/S3Sn10.h"    // UART
#elif (defined CFG_S3SC10)
#include "S3xxxx/S3SC10.h"    // UART + CC1101
#elif (defined CFG_S3En10)
#include "S3xxxx/S3En10.h"    // ENC28J60
#elif (defined CFG_S3Cn10)
#include "S3xxxx/S3Cn10.h"    // CC1101
#elif (defined CFG_S3SQ10)
#include "S3xxxx/S3SQ10.h"    // UART + RFM69
#else
#error Unknown configuration
#endif  //  Configuration

//////////////////////////////////////////////////////////////
// PLC Section
#define EXTPLC_USED                     1
#define EXTPLC_SIZEOF_PRG               2048
#define EXTPLC_SIZEOF_PRG_CACHE         32      // Must be 2^n, bytes
#define EXTPLC_SIZEOF_RAM               256     // size in uint32_t
#define EXTPLC_SIZEOF_RW                16      // size in uint32_t
// PLC Section
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// EEPROM Emulation Section
// HD + CL Devices
#if defined(STM32F100xE) ||                         \
    defined(STM32F101xE) || defined(STM32F101xG) || \
    defined(STM32F103xE) || defined(STM32F103xG) || \
    defined(STM32F105xC) || defined(STM32F107xC)
#define FEE_PAGE_SIZE           (uint32_t)0x00000800    // Size of FLASH Page - 2k
#else                                                   // LD + MD Devices
#define FEE_PAGE_SIZE           (uint32_t)0x00000400    // Size of FLASH Page - 1k
#endif

#define FEE_SIZE                0x2000                  // 8K
#define FEE_TRANSFER_PAGES      2                       // Reserved Pages for Transfer

// WARNING !! does not check overlapping with program memory
#ifndef FLASH_BANK2_END
#define FEE_BASE ((FLASH_BANK1_END + 1) - FEE_SIZE)
#else   //  FLASH_BANK2_END
#define FEE_BASE ((FLASH_BANK2_END + 1) - FEE_SIZE)
#endif  //  FLASH_BANK2_END

#define FEE_EraseTimeout        (uint32_t)0x000B0000
#define FEE_ProgramTimeout      (uint32_t)0x00002000
// End EEPROM Emulation Section
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// SPI Section
#if (defined HAL_USE_SPI1) && (HAL_USE_SPI1 == 1)
// SPI1, Config 1, PA5, PA6, PA7
#define DIO_MODE_SPI1               DIO_MODE_AF_PP_HS
#define SPI1_PORT                   GPIOA
#define SPI1_SCK_PIN                GPIO_Pin_5
#define SPI1_MISO_PIN               GPIO_Pin_6
#define SPI1_MOSI_PIN               GPIO_Pin_7

#elif (defined HAL_USE_SPI1) && (HAL_USE_SPI1 == 2)
// SPI1, Config 2, PB3, PB4, PB5
#define DIO_MODE_SPI1               DIO_MODE_AF_PP_HS
#define SPI1_PORT                   GPIOB
#define SPI1_SCK_PIN                GPIO_Pin_3
#define SPI1_MISO_PIN               GPIO_Pin_4
#define SPI1_MOSI_PIN               GPIO_Pin_5

#endif  //  HAL_USE_SPI1

#if (defined HAL_USE_SPI2) && (HAL_USE_SPI2 == 1)
// SPI2, Config 1, PB13, PB14, PB15
#define DIO_MODE_SPI2               DIO_MODE_AF_PP_HS
#define SPI2_PORT                   GPIOB
#define SPI2_SCK_PIN                GPIO_Pin_13
#define SPI2_MISO_PIN               GPIO_Pin_14
#define SPI2_MOSI_PIN               GPIO_Pin_15

#endif  //  HAL_USE_SPI2
// End SPI Section
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
//  UART Section

#define RDR     DR
#define TDR     DR

#if (defined HAL_USE_USART1)
#define USART1_TX_DMA           DMA1_Channel4
#define USART1_RX_DMA           DMA1_Channel5
#define HAL_USART1_AF           DIO_MODE_AF_PP

#if (defined HAL_USART1_REMAP) && (HAL_USART1_REMAP == 1)
// USART1, Config 1
#define HAL_USART1_PIN_TX       22                      // PB6
#define HAL_USART1_PIN_RX       23                      // PB7

#else   //  HAL_USART1_REMAP - default
// USART1, Config 0
#define HAL_USART1_PIN_TX       9                       // PA9
#define HAL_USART1_PIN_RX       10                      // PA10

#endif  //  HAL_USART1_REMAP
#endif  //  HAL_USE_USART1

#if (defined HAL_USE_USART2)
#define USART2_RX_DMA           DMA1_Channel6
#define USART2_TX_DMA           DMA1_Channel7
#define HAL_USART2_AF           DIO_MODE_AF_PP

#if (defined HAL_USART2_REMAP) && (HAL_USART2_REMAP == 1)
// USART2, Config 1
#define HAL_USART1_PIN_TX       53                      // PD5
#define HAL_USART1_PIN_RX       54                      // PD6

#else   //  HAL_USART2_REMAP - default
// USART2, Config 0
#define HAL_USART2_PIN_TX       2                       // PA2
#define HAL_USART2_PIN_RX       3                       // PA3

#endif  //  HAL_USART2_REMAP
#endif  //  HAL_USE_USART2

#if (defined HAL_USE_USART3)
#define USART3_TX_DMA           DMA1_Channel2
#define USART3_RX_DMA           DMA1_Channel3
#define HAL_USART3_AF           DIO_MODE_AF_PP

#if (defined HAL_USART3_REMAP) && (HAL_USART3_REMAP == 1)
// USART3, Config 1
#define HAL_USART3_PIN_TX       42                      // PC10
#define HAL_USART3_PIN_RX       43                      // PC11

#else   //  HAL_USART3_REMAP - default
// USART3, Config 0
#define HAL_USART3_PIN_TX       26                      // PB10
#define HAL_USART3_PIN_RX       27                      // PB11

#endif  //  HAL_USART3_REMAP
#endif  //  HAL_USE_USART3
//  End UART Section
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
//  TWI Section
#define DIO_MODE_TWI    DIO_MODE_AF_OD          // Alternative function, Open Drain
#define I2C_GPIO        GPIOB

#if (defined HAL_TWI_BUS) && (HAL_TWI_BUS == 1)
#if (defined HAL_TWI_REMAP) && (HAL_TWI_REMAP == 1)
// I2C1, Config 1, PB8 - SCL, PB9 - SDA
#define I2C_PIN_SCL     GPIO_Pin_8
#define I2C_PIN_SDA     GPIO_Pin_9
#define I2C_DIO_SCL     24
#define I2C_DIO_SDA     25

#else   //  HAL_TWI_REMAP - default
// I2C1, Config 0, PB6 - SCL, PB7 - SDA
#define I2C_PIN_SCL     GPIO_Pin_6
#define I2C_PIN_SDA     GPIO_Pin_7
#define I2C_DIO_SCL     22
#define I2C_DIO_SDA     23

#endif  //  HAL_TWI_REMAP

#elif (defined HAL_TWI_BUS) && (HAL_TWI_BUS == 2)
// I2C2, PB10 - SCL, PB11 - SDA
#define I2C_PIN_SCL     GPIO_Pin_10
#define I2C_PIN_SDA     GPIO_Pin_11
#define I2C_DIO_SCL     26
#define I2C_DIO_SDA     27

#endif  //  HAL_TWI_BUS
//  End TWI Section
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// PWM Section

#if defined(STM32F103xB)
#define HAL_PWM_TIMERS      {NULL, TIM1, TIM2, TIM3, TIM4}
#else
#error unknown uC
#endif

// PWM Section
//////////////////////////////////////////////////////////////

#endif  //  HW_STM32F1_H
