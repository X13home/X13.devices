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
// PLC Section
#define EXTPLC_USED                     1
#define EXTPLC_SIZEOF_PRG               1024
#define EXTPLC_SIZEOF_PRG_CACHE         32      // Must be 2^n, bytes
#define EXTPLC_SIZEOF_RAM               256     // size in uint32_t
#define EXTPLC_SIZEOF_RW                16      // size in uint32_t
// PLC Section
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

//////////////////////////////////////////////////////////////
// PWM Section
#define HAL_PWM_TIMERS      {NULL, NULL, TIM2, NULL, NULL, NULL, NULL, NULL,    /* 0 - 7 */ \
                             NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,    /* 8 - 15 */ \
                             NULL, NULL, NULL, NULL, NULL, TIM21, TIM22}        /* 16 - 22 */
// PWM Section
//////////////////////////////////////////////////////////////

#endif  //  HW_STM32L0_H
