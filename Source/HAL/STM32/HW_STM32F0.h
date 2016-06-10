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

// STM32F051R8
#if   (defined CFG_S2Sn10)
#include "S2xxxx/S2Sn10.h"      // UART
#elif   (defined CFG_S2SR10)
#include "S2xxxx/S2SR10.h"      // UART + RFM12

// STM32F091RC
#elif   (defined CFG_S2Sn11)
#include "S2xxxx/S2Sn11.h"      // UART

// STM32F051C8
#elif   (defined CFG_S2EC13)
#include "S2xxxx/S2EC13.h"      // ENC28J60 + CC1101
#elif   (defined CFG_S2En13)
#include "S2xxxx/S2En13.h"      // ENC28J60
#elif   (defined CFG_S2Cn13)
#include "S2xxxx/S2Cn13.h"      // CC1101
#elif   (defined CFG_S2Sn13)
#include "S2xxxx/S2Sn13.h"      // UART ALT
#elif   (defined CFG_S2SC13)
#include "S2xxxx/S2SC13.h"      // UART ALT + CC1101

// UNode V3.0 - STM32F051K8
#elif   (defined CFG_S2SC15)    
#include "S2xxxx/S2SC15.h"      // UART + CC1101
#elif   (defined CFG_S2Cn15)    
#include "S2xxxx/S2Cn15.h"      // CC1101

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

#if (defined HAL_USE_SPI1)

#define DIO_MODE_SPI1               DIO_MODE_AF_PP_HS    

#if (HAL_USE_SPI1 == 1)         // Config 1, GPIOA PA5, PA6, PA7
#define SPI1_PORT                   GPIOA
#define SPI1_SCK_PIN                GPIO_Pin_5
#define SPI1_MISO_PIN               GPIO_Pin_6
#define SPI1_MOSI_PIN               GPIO_Pin_7

#elif (HAL_USE_SPI1 == 2)       // Config 2, GPIOB PB3, PB4, PB5
#define SPI1_PORT                   GPIOB
#define SPI1_SCK_PIN                GPIO_Pin_3
#define SPI1_MISO_PIN               GPIO_Pin_4
#define SPI1_MOSI_PIN               GPIO_Pin_5

#else
#error Unknown HAL_USE_SPI1 config
#endif  //  HAL_USE_SPI1 - config
#endif  //  HAL_USE_SPI1

#if (defined HAL_USE_SPI2)

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

#if (defined HAL_USE_USART1)
#if (defined HAL_USART1_REMAP)
#define HAL_USART1_AF           DIO_MODE_AF_PP
#define HAL_USART1_PIN_RX       23                      // GPIOB PIN7
#define HAL_USART1_PIN_TX       22                      // GPIOB PIN6

#else
#define HAL_USART1_AF           ((1<<DIO_AF_OFFS) | DIO_MODE_AF_PP)
#define HAL_USART1_PIN_RX       10                      // GPIOA PIN10
#define HAL_USART1_PIN_TX       9                       // GPIOA PIN9

#endif  //  HAL_USART1_REMAP
#endif  //  HAL_USE_USART1

#if (defined HAL_USE_USART2)
#define HAL_USART2_AF           ((1<<DIO_AF_OFFS) | DIO_MODE_AF_PP)

#if (defined HAL_USART2_REMAP)
#define HAL_USART2_PIN_RX       15                      // GPIOA PIN15
#define HAL_USART2_PIN_TX       14                      // GPIOA PIN14

#else
#define HAL_USART2_PIN_RX       3                       // GPIOA PIN3
#define HAL_USART2_PIN_TX       2                       // GPIOA PIN2

#endif  //  HAL_USART2_REMAP
#endif  //  HAL_USE_USART2

//  End UART Section
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
