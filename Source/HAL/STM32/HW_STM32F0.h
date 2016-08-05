/*
Copyright (c) 2011-2016 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef HW_STM32F0_H
#define HW_STM32F0_H

#include "stm32f0xx.h"

// STM32F051R8, Discovery F0
#if   (defined CFG_S2Sn10)
#include "S2xxxx/S2Sn10.h"      // UART
#elif   (defined CFG_S2SR10)
#include "S2xxxx/S2SR10.h"      // UART + RFM12

// STM32F091RC, Nucleo F091RC
#elif   (defined CFG_S2Sn11)
#include "S2xxxx/S2Sn11.h"      // UART

// STM32F051C8
// Board S2EC13/S2ER13 without RF
#elif   (defined CFG_S2En13)
#include "S2xxxx/S2En13.h"      // ENC28J60
#elif   (defined CFG_S2Sn13)
#include "S2xxxx/S2Sn13.h"      // UART ALT
#elif   (defined CFG_S2EM13)
#include "S2xxxx/S2EM13.h"      // ENC28J60 + EXT_RS485
#elif   (defined CFG_S2SM13)
#include "S2xxxx/S2SM13.h"      // UART ALT + EXT_RS485
#elif   (defined CFG_S2Mn13)
#include "S2xxxx/S2Mn13.h"      // EXT_RS485
// Board S2EC13 + CC1101
#elif   (defined CFG_S2EC13)
#include "S2xxxx/S2EC13.h"      // ENC28J60 + CC1101
#elif   (defined CFG_S2Cn13)
#include "S2xxxx/S2Cn13.h"      // CC1101
#elif   (defined CFG_S2SC13)
#include "S2xxxx/S2SC13.h"      // UART ALT + CC1101
// Board S2ER13 + RFM69CW
#elif   (defined CFG_S2Qn13)
#include "S2xxxx/S2Qn13.h"      // RFM69
#elif   (defined CFG_S2SQ13)
#include "S2xxxx/S2SQ13.h"      // UART ALT + RFM69

// UNode V3.0 - STM32F051K8
#elif   (defined CFG_S2SC15)    
#include "S2xxxx/S2SC15.h"      // UART + CC1101
#elif   (defined CFG_S2Cn15)    
#include "S2xxxx/S2Cn15.h"      // CC1101

// S2Mn10
#elif   (defined CFG_S2Mn16)
#include "S2xxxx/S2Mn16.h"      // EXT_RS485

#else
#error Unknown configuration
#endif  //  Configuration

//////////////////////////////////////////////////////////////
// PLC Section
#define EXTPLC_USED             1
#define EXTPLC_SIZEOF_PRG       2048
#define EXTPLC_SIZEOF_PRG_CACHE 32      // Must be 2^n, bytes
#define EXTPLC_SIZEOF_RAM       256     // size in uint32_t
#define EXTPLC_SIZEOF_RW        16      // size in uint32_t
// PLC Section
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
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

#define FEE_EraseTimeout        (uint32_t)0x000B0000
#define FEE_ProgramTimeout      (uint32_t)0x000B0000
// End EEPROM Emulation Section
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// SPI Section
#if (defined HAL_USE_SPI1) && (HAL_USE_SPI1 == 1)
// SPI1, Config 1, PA5, PA6, PA7
#define DIO_MODE_SPI1           DIO_MODE_AF_PP_HS
#define SPI1_PORT               GPIOA
#define SPI1_SCK_PIN            GPIO_Pin_5
#define SPI1_MISO_PIN           GPIO_Pin_6
#define SPI1_MOSI_PIN           GPIO_Pin_7

#elif (defined HAL_USE_SPI1) && (HAL_USE_SPI1 == 2)
// SPI1, Config 2, PB3, PB4, PB5
#define DIO_MODE_SPI1           DIO_MODE_AF_PP_HS
#define SPI1_PORT               GPIOB
#define SPI1_SCK_PIN            GPIO_Pin_3
#define SPI1_MISO_PIN           GPIO_Pin_4
#define SPI1_MOSI_PIN           GPIO_Pin_5

#endif  //  HAL_USE_SPI1

#if (defined HAL_USE_SPI2) && (HAL_USE_SPI2 == 1)
// SPI2, Config 1, PB13, PB14, PB15
#define DIO_MODE_SPI2           DIO_MODE_AF_PP_HS
#define SPI2_PORT               GPIOB
#define SPI2_SCK_PIN            GPIO_Pin_13
#define SPI2_MISO_PIN           GPIO_Pin_14
#define SPI2_MOSI_PIN           GPIO_Pin_15

#endif  //  HAL_USE_SPI2
// End SPI Section
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
//  UART Section
#if (defined HAL_USE_USART1)
#define USART1_TX_DMA           DMA1_Channel2
#define USART1_RX_DMA           DMA1_Channel3

#if (defined HAL_USART1_REMAP) && (HAL_USART1_REMAP == 1)
// USART1, Config 1
#define HAL_USART1_AF           DIO_MODE_AF_PP
#define HAL_USART1_PIN_TX       22                      // PB6
#define HAL_USART1_PIN_RX       23                      // PB7

#else   //  HAL_USART1_REMAP - default
// USART1, Config 0
#define HAL_USART1_AF           ((1<<DIO_AF_OFFS) | DIO_MODE_AF_PP)
#define HAL_USART1_PIN_TX       9                       // PA9
#define HAL_USART1_PIN_RX       10                      // PA10
#define HAL_USART1_PIN_DE       12                      // PA12

#endif  //  HAL_USART1_REMAP
#endif  //  HAL_USE_USART1

#if (defined HAL_USE_USART2)
#define USART2_TX_DMA           DMA1_Channel4
#define USART2_RX_DMA           DMA1_Channel5

#if (defined HAL_USART2_REMAP) && (HAL_USART2_REMAP == 1)
// USART2, Config 1
#define HAL_USART2_AF           ((1<<DIO_AF_OFFS) | DIO_MODE_AF_PP)
#define HAL_USART2_PIN_TX       14                      // PA14
#define HAL_USART2_PIN_RX       15                      // PA15

#else   //  HAL_USART2_REMAP - default
// USART2, Config 0
#define HAL_USART2_AF           ((1<<DIO_AF_OFFS) | DIO_MODE_AF_PP)
#define HAL_USART2_PIN_DE       1                       // PA1
#define HAL_USART2_PIN_TX       2                       // PA2
#define HAL_USART2_PIN_RX       3                       // PA3

#endif  //  HAL_USART2_REMAP
#endif  //  HAL_USE_USART2

#if (defined HAL_USE_USART3)
#define USART3_TX_DMA           DMA1_Channel7
#define USART3_RX_DMA           DMA1_Channel6

// STM32F091
#if (defined HAL_USART3_REMAP) && (HAL_USART3_REMAP == 1)
// USART3, Config 1
#define HAL_USART3_AF           ((1<<DIO_AF_OFFS) | DIO_MODE_AF_PP)
#define HAL_USART3_PIN_TX       42                      // PC10
#define HAL_USART3_PIN_RX       43                      // PC11
#define HAL_USART3_PIN_DE       50                      // PD2

#elif (defined HAL_USART3_REMAP) && (HAL_USART3_REMAP == 2)
// USART3, Config 2
#define HAL_USART3_AF           ((1<<DIO_AF_OFFS) | DIO_MODE_AF_PP)
#define HAL_USART3_PIN_TX       36                      // PC4
#define HAL_USART3_PIN_RX       37                      // PC5

#else   //  HAL_USART3_REMAP - default
// USART3, Config 0
#define HAL_USART3_AF           ((4<<DIO_AF_OFFS) | DIO_MODE_AF_PP)
#define HAL_USART3_PIN_DE       17                      // PB1
#define HAL_USART3_PIN_TX       26                      // PB10
#define HAL_USART3_PIN_RX       27                      // PB11

#endif  //  HAL_USART3_REMAP
#endif  //  HAL_USE_USART3
//  End UART Section
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
//  TWI Section
#if (defined HAL_TWI_BUS) && (HAL_TWI_BUS == 1)
#if (defined HAL_TWI_REMAP) && (HAL_TWI_REMAP == 1)
// I2C1, Config 1, PB8 - SCL, PB9 - SDA, AF1
#define DIO_MODE_TWI            ((1<<DIO_AF_OFFS) | DIO_MODE_AF_OD)
#define I2C_PIN_SCL             GPIO_Pin_8
#define I2C_PIN_SDA             GPIO_Pin_9
#define I2C_GPIO                GPIOB
#define I2C_DIO_SCL             24
#define I2C_DIO_SDA             25

#elif (defined HAL_TWI_REMAP) && (HAL_TWI_REMAP == 2)
// I2C1, Config 2, PA9 - SCL, PA10 - SDA, AF4, F091
#define DIO_MODE_TWI            ((4<<DIO_AF_OFFS) | DIO_MODE_AF_OD)
#define I2C_PIN_SCL             GPIO_Pin_9
#define I2C_PIN_SDA             GPIO_Pin_10
#define I2C_GPIO                GPIOA
#define I2C_DIO_SCL             9
#define I2C_DIO_SDA             10

#elif (defined HAL_TWI_REMAP) && (HAL_TWI_REMAP == 3)
// I2C1, Config 3, PF1 - SCL, PF0 - SDA, AF1, F091
#define DIO_MODE_TWI            ((1<<DIO_AF_OFFS) | DIO_MODE_AF_OD)
#define I2C_PIN_SCL             GPIO_Pin_1
#define I2C_PIN_SDA             GPIO_Pin_0
#define I2C_GPIO                GPIOF
#define I2C_DIO_SCL             81
#define I2C_DIO_SDA             80

#else   //  HAL_TWI_REMAP - default
// I2C1, Config 0, PB6 - SCL, PB7 - SDA, AF1
#define DIO_MODE_TWI            ((1<<DIO_AF_OFFS) | DIO_MODE_AF_OD)
#define I2C_PIN_SCL             GPIO_Pin_6
#define I2C_PIN_SDA             GPIO_Pin_7
#define I2C_GPIO                GPIOB
#define I2C_DIO_SCL             22
#define I2C_DIO_SDA             23

#endif  //  HAL_TWI_REMAP

#elif (defined HAL_TWI_BUS) && (HAL_TWI_BUS == 2)
#if (defined HAL_TWI_REMAP) && (HAL_TWI_REMAP == 1)
// I2C2, Config 1, PB13 - SCL, PB14 - SDA, AF5, F091
#define DIO_MODE_TWI    ((5<<DIO_AF_OFFS) | DIO_MODE_AF_OD)
#define I2C_PIN_SCL     GPIO_Pin_13
#define I2C_PIN_SDA     GPIO_Pin_14
#define I2C_GPIO        GPIOB
#define I2C_DIO_SCL     29
#define I2C_DIO_SDA     30

#elif (defined HAL_TWI_REMAP) && (HAL_TWI_REMAP == 2)
// I2C2, Config 2, PA11 - SCL, PA12 - SDA, AF5, F091
#define DIO_MODE_TWI    ((5<<DIO_AF_OFFS) | DIO_MODE_AF_OD)
#define I2C_PIN_SCL     GPIO_Pin_11
#define I2C_PIN_SDA     GPIO_Pin_12
#define I2C_GPIO        GPIOA
#define I2C_DIO_SCL     11
#define I2C_DIO_SDA     12

#elif (defined HAL_TWI_REMAP) && (HAL_TWI_REMAP == 3)
// I2C2, Config 3, PF6 - SCL, PF7 - SDA, AF0, F051
#define DIO_MODE_TWI    DIO_MODE_AF_OD
#define I2C_PIN_SCL     GPIO_Pin_6
#define I2C_PIN_SDA     GPIO_Pin_7
#define I2C_GPIO        GPIOF
#define I2C_DIO_SCL     86
#define I2C_DIO_SDA     87

#else   //  HAL_TWI_REMAP - default
// I2C2, Config 0, PB10 - SCL, PB11 - SDA, AF1
#define DIO_MODE_TWI    ((1<<DIO_AF_OFFS) | DIO_MODE_AF_OD)
#define I2C_PIN_SCL     GPIO_Pin_10
#define I2C_PIN_SDA     GPIO_Pin_11
#define I2C_GPIO        GPIOB
#define I2C_DIO_SCL     26
#define I2C_DIO_SDA     27

#endif  //  HAL_TWI_REMAP
#endif  //  HAL_TWI_BUS
//  End TWI Section
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// PWM Section
#if defined(STM32F051x8) || defined(STM32F091xC)
#define HAL_PWM_TIMERS      {NULL, TIM1, TIM2,  TIM3,  NULL,  NULL,     /* 0  - 5   */ \
                             NULL, NULL, NULL,  NULL,  NULL,  NULL,     /* 6  - 11  */ \
                             NULL, NULL, TIM14, TIM15, TIM16, TIM17}    /* 12 - 17  */
#else
#error unknown uC
#endif
// PWM Section
//////////////////////////////////////////////////////////////

#endif  //  HW_STM32F0_H
