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

#include "stm32f3xx.h"

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
#define FEE_SIZE                0x2000                  // 8K
#define FEE_TRANSFER_PAGES      2                       // Reserved Pages for Transfer

#if defined (__STM32F303x8_H)
// 64k Version
#define FEE_PAGE_SIZE           (uint32_t)0x00000800    // Size of FLASH Page - 2k
#define FEE_BASE                (((uint32_t)0x08010000) - FEE_SIZE)
#endif  //  uC

#define FLASH_SR_WRPRTERR       FLASH_SR_WRPERR
#define FEE_EraseTimeout        (uint32_t)0x000B0000
#define FEE_ProgramTimeout      (uint32_t)0x00002000
// End EEPROM Emulation Section
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// SPI Section
#if (defined HAL_USE_SPI1)

#define DIO_MODE_SPI1               ((5<<DIO_AF_OFFS) | DIO_MODE_AF_PP_HS)

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
// End SPI Section
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
//  UART Section
// STM32F303/STM32F334

// USART 1
#define USART1_TX_DMA           DMA1_Channel4
#define USART1_RX_DMA           DMA1_Channel5
#define HAL_USART1_AF           ((7<<DIO_AF_OFFS) | DIO_MODE_AF_PP)

#if   (defined HAL_USART1_REMAP)
#define HAL_USART1_PIN_TX       22                      // PB6
#define HAL_USART1_PIN_RX       23                      // PB7

#else   //  HAL_USART1_REMAP - default
#define HAL_USART1_PIN_TX       9                       // PA9
#define HAL_USART1_PIN_RX       10                      // PA10

#endif  //  HAL_USART1_REMAP

// USART 2
#define USART2_RX_DMA           DMA1_Channel6
#define USART2_TX_DMA           DMA1_Channel7
#define HAL_USART2_AF           ((7<<DIO_AF_OFFS) | DIO_MODE_AF_PP)

#ifndef HAL_USART2_REMAP
#define HAL_USART2_REMAP        1
#endif  //  HAL_USART2_REMAP

#if   (HAL_USART2_REMAP == 2)
#define HAL_USART2_PIN_TX       19                      // PB3
#define HAL_USART2_PIN_RX       20                      // PB4

#elif (HAL_USART2_REMAP == 3)
#define HAL_USART2_PIN_TX       14                      // PA14
#define HAL_USART2_PIN_RX       15                      // PA15

#elif (HAL_USART2_REMAP == 13)
#define HAL_USART2_PIN_TX       2                       // PA2
#define HAL_USART2_PIN_RX       15                      // PA15

#else   //  HAL_USART2_REMAP - default
#define HAL_USART2_PIN_TX       2                       // PA2
#define HAL_USART2_PIN_RX       3                       // PA3

#endif  //  HAL_USART2_REMAP
//  End UART Section
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// HAL AIn Section
#if defined(STM32F303x8)
// PA0 - PA7, PB0 - PB1
#define HAL_AIN_AIN2DPIN    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x10, 0x11}

// apin to configuration conversion
// bits 7,6 : ADC1 - ADC3, bits 4-0 : Channel 1 - 18
// PA0 - PA3 : ADC1, In1 - In4
// PA4 - PA7 : ADC2, In1 - In4
// PB0 - PB1 : ADC1, In11 - In12
#define HAL_AIN_AIN2CFG     {0x01, 0x02, 0x03, 0x04, 0x41, 0x42, 0x43, 0x44, 0x0B, 0x0C}
#endif  //  STM32F303x8

// HAL AIn Section
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// PWM Section
#if defined(STM32F303x8)
#define HAL_PWM_TIMERS      {NULL, TIM1, TIM2, TIM3,  NULL,  NULL,  \
                             NULL, NULL, NULL, NULL,  NULL,  NULL,  \
                             NULL, NULL, NULL, TIM15, TIM16, TIM17}
#endif
// PWM Section
//////////////////////////////////////////////////////////////

#endif  //  HW_STM32F3_H
