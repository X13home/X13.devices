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

// uC: STM32L051K8
// Board: UNode V3.0
#if   (defined CFG_S5Cn10)
#include "S5xxxx/S5Cn10.h"      // CC1101
// Board: CPUe - prototype
#elif  (defined CFG_S5Qn11)
#include "S5xxxx/S5Qn11.h"      // RFM69 at PHY interface
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
// I2C2, Config 1, PB13 - SCL, PB14 - SDA, AF5
#define DIO_MODE_TWI    ((5<<DIO_AF_OFFS) | DIO_MODE_AF_OD)
#define I2C_PIN_SCL     GPIO_Pin_13
#define I2C_PIN_SDA     GPIO_Pin_14
#define I2C_GPIO        GPIOB
#define I2C_DIO_SCL     29
#define I2C_DIO_SDA     30

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
#define HAL_PWM_TIMERS      {NULL, NULL, TIM2, NULL, NULL, NULL, NULL, NULL,    /* 0 - 7 */ \
                             NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,    /* 8 - 15 */ \
                             NULL, NULL, NULL, NULL, NULL, TIM21, TIM22}        /* 16 - 22 */
// PWM Section
//////////////////////////////////////////////////////////////

#endif  //  HW_STM32L0_H
