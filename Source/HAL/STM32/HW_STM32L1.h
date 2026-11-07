/*
Copyright (c) 2011-2016 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef HW_STM32L1_H
#define HW_STM32L1_H

#include "stm32l1xx.h"

#ifdef CONFIG
#define CONFIG_INCLUDE < CONFIG.h >
#include CONFIG_INCLUDE
#else
#error Unknown configuration
#endif

//////////////////////////////////////////////////////////////
//  UART Section

#define RDR     DR
#define TDR     DR

#if (defined HAL_USE_USART1)
#define USART1_TX_DMA           DMA1_Channel4
#define USART1_RX_DMA           DMA1_Channel5
#define HAL_USART1_AF           ((7<<DIO_AF_OFFS) | DIO_MODE_AF_PP)

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
#define USART2_TX_DMA           DMA1_Channel7
#define USART2_RX_DMA           DMA1_Channel6
#define HAL_USART2_AF           ((7<<DIO_AF_OFFS) | DIO_MODE_AF_PP)

// USART2, Config 0
#define HAL_USART2_PIN_TX       2                       // PA2
#define HAL_USART2_PIN_RX       3                       // PA3

#endif  //  HAL_USE_USART2

#if (defined HAL_USE_USART3)
#define USART3_TX_DMA           DMA1_Channel2
#define USART3_RX_DMA           DMA1_Channel3
#define HAL_USART3_AF           ((7<<DIO_AF_OFFS) | DIO_MODE_AF_PP)

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

// End UART Section
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// PWM Section
#define HAL_PWM_TIMERS      {NULL, NULL, TIM2, TIM3, TIM4, NULL, NULL, NULL,    /* 0 - 7 */ \
                             NULL, TIM9, TIM10, TIM11}                          /* 8 - 11 */
// PWM Section
//////////////////////////////////////////////////////////////

#endif  //  HW_STM32L0_H
