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
// UNodeV3, STM32F303K8
#elif   (defined CFG_S4Cn11)
#include "S4xxxx/S4Cn11.h"    // CC1101
#elif   (defined CFG_S4SC11)
#include "S4xxxx/S4SC11.h"    // UART + CC1101
// S4xx13, STM32F334K8
#elif   (defined CFG_S4Mn13)
#include "S4xxxx/S4Mn13.h"    // RS485
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

#if (defined (STM32F303x8) || defined (STM32F334x8) || defined (STM32F303xC))
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
#if (defined HAL_USE_SPI1) && (HAL_USE_SPI1 == 1)
// SPI1, Config 1, PA5, PA6, PA7, AF5
#define DIO_MODE_SPI1           ((5<<DIO_AF_OFFS) | DIO_MODE_AF_PP_HS)
#define SPI1_PORT               GPIOA
#define SPI1_SCK_PIN            GPIO_Pin_5
#define SPI1_MISO_PIN           GPIO_Pin_6
#define SPI1_MOSI_PIN           GPIO_Pin_7

#elif (defined HAL_USE_SPI1) && (HAL_USE_SPI1 == 2)
// SPI1, Config 2, PB3, PB4, PB5, AF5
#define DIO_MODE_SPI1           ((5<<DIO_AF_OFFS) | DIO_MODE_AF_PP_HS)
#define SPI1_PORT               GPIOB
#define SPI1_SCK_PIN            GPIO_Pin_3
#define SPI1_MISO_PIN           GPIO_Pin_4
#define SPI1_MOSI_PIN           GPIO_Pin_5

#endif  //  HAL_USE_SPI1

#if (defined HAL_USE_SPI2) && (HAL_USE_SPI2 == 1)
// SPI2, Config 1, PB13, PB14, PB15, AF5
#define DIO_MODE_SPI2           ((5<<DIO_AF_OFFS) | DIO_MODE_AF_PP_HS)
#define SPI2_PORT               GPIOB
#define SPI2_SCK_PIN            GPIO_Pin_13
#define SPI2_MISO_PIN           GPIO_Pin_14
#define SPI2_MOSI_PIN           GPIO_Pin_15

#endif  //  HAL_USE_SPI2

#if (defined HAL_USE_SPI3) && (HAL_USE_SPI3 == 1)
#if (defined HAL_USE_SPI1) && (HAL_USE_SPI1 == 2)
#error SPI1 and SPI3 Conflict, used PB3-PB5
#endif  //  HAL_USE_SPI1, Config 2
// SPI3, Config 1, PB3, PB4, PB5, AF6
#define DIO_MODE_SPI3           ((6<<DIO_AF_OFFS) | DIO_MODE_AF_PP_HS)
#define SPI2_PORT               GPIOB
#define SPI2_SCK_PIN            GPIO_Pin_3
#define SPI2_MISO_PIN           GPIO_Pin_4
#define SPI2_MOSI_PIN           GPIO_Pin_5

#elif (defined HAL_USE_SPI3) && (HAL_USE_SPI3 == 2)
// SPI3, Config 2, PC10, PC11, PC12, AF6
#define DIO_MODE_SPI3           ((6<<DIO_AF_OFFS) | DIO_MODE_AF_PP_HS)
#define SPI2_PORT               GPIOC
#define SPI2_SCK_PIN            GPIO_Pin_10
#define SPI2_MISO_PIN           GPIO_Pin_11
#define SPI2_MOSI_PIN           GPIO_Pin_12

#endif  //  HAL_USE_SPI3
// End SPI Section
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
//  UART Section
#if (defined HAL_USE_USART1)
#define USART1_TX_DMA           DMA1_Channel4
#define USART1_RX_DMA           DMA1_Channel5
#define HAL_USART1_AF           ((7<<DIO_AF_OFFS) | DIO_MODE_AF_PP)

#if (defined HAL_USART1_REMAP) && (HAL_USART1_REMAP == 1)
// USART1, Config 1
#define HAL_USART1_PIN_TX       22                      // PB6
#define HAL_USART1_PIN_RX       23                      // PB7

#elif (defined HAL_USART1_REMAP) && (HAL_USART1_REMAP == 2)
// Config 2
#define HAL_USART1_PIN_TX       36                      // PC4
#define HAL_USART1_PIN_RX       37                      // PC5

#else   //  HAL_USART1_REMAP - default
// Config 0
#define HAL_USART1_PIN_TX       9                       // PA9
#define HAL_USART1_PIN_RX       10                      // PA10
#define HAL_USART1_PIN_DE       12                      // PA12

#endif  //  HAL_USART1_REMAP
#endif  //  HAL_USE_USART1

#if (defined HAL_USE_USART2)
#define USART2_TX_DMA           DMA1_Channel7
#define USART2_RX_DMA           DMA1_Channel6
#define HAL_USART2_AF           ((7<<DIO_AF_OFFS) | DIO_MODE_AF_PP)

#if (defined HAL_USART2_REMAP) && (HAL_USART2_REMAP == 1)
// USART2, Config 1
#define HAL_USART2_PIN_TX       14                      // PA14
#define HAL_USART2_PIN_RX       15                      // PA15

#elif (defined HAL_USART2_REMAP) && (HAL_USART2_REMAP == 2)
// USART2, Config 2
#define HAL_USART2_PIN_TX       19                      // PB3
#define HAL_USART2_PIN_RX       20                      // PB4

#elif (defined HAL_USART2_REMAP) && (HAL_USART2_REMAP == 10)
// USART2, Config 10
#define HAL_USART2_PIN_TX       2                       // PA2
#define HAL_USART2_PIN_RX       15                      // PA15

#else   //  HAL_USART2_REMAP - default
// USART2, Config 0
#define HAL_USART2_PIN_DE       1                       // PA1
#define HAL_USART2_PIN_TX       2                       // PA2
#define HAL_USART2_PIN_RX       3                       // PA3

#endif  //  HAL_USART2_REMAP
#endif  //  HAL_USE_USART2

#if (defined HAL_USE_USART3)
#define USART3_TX_DMA           DMA1_Channel2
#define USART3_RX_DMA           DMA1_Channel3
#define HAL_USART3_AF           ((7<<DIO_AF_OFFS) | DIO_MODE_AF_PP)

#if (defined HAL_USART3_REMAP) && (HAL_USART3_REMAP == 1)
// USART3, Config 1
#define HAL_USART3_PIN_TX       42                      // PC10
#define HAL_USART3_PIN_RX       43                      // PC11

#elif (defined HAL_USART3_REMAP) && (HAL_USART3_REMAP == 2)
// USART3, Config 2
#define HAL_USART3_PIN_TX       24                      // PB8
#define HAL_USART3_PIN_RX       25                      // PB9

#else   //  HAL_USART3_REMAP - default
// USART3, Config 0
#define HAL_USART3_PIN_TX       26                      // PB10
#define HAL_USART3_PIN_RX       27                      // PB11
#define HAL_USART3_PIN_DE       30                      // PB14

#endif  //  HAL_USART3_REMAP
#endif  //  HAL_USE_USART3
//  End UART Section
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// TWI Section
// STM32F303K8 / STM32F334K8, Only I2C1
#if (defined HAL_TWI_BUS) && (HAL_TWI_BUS == 1)
// Alternative function, AF = 4, Open Drain
#define DIO_MODE_TWI    ((4<<DIO_AF_OFFS) | DIO_MODE_AF_OD)

#if (defined HAL_TWI_REMAP) && (HAL_TWI_REMAP == 1)
// Config 1
// I2C1, PB8 - SCL, PB9 - SDA
#define I2C_PIN_SCL     GPIO_Pin_8
#define I2C_PIN_SDA     GPIO_Pin_9
#define I2C_GPIO        GPIOB
#define I2C_DIO_SCL     24
#define I2C_DIO_SDA     25

#elif (defined HAL_TWI_REMAP) && (HAL_TWI_REMAP == 2)
// Config 2
// I2C1, PA15 - SCL, PA14 - SDA
#define I2C_PIN_SCL     GPIO_Pin_15
#define I2C_PIN_SDA     GPIO_Pin_14
#define I2C_GPIO        GPIOA
#define I2C_DIO_SCL     15
#define I2C_DIO_SDA     14

#else   //  HAL_TWI_REMAP - default
// Config 0
// I2C1, PB6 - SCL, PB7 - SDA
#define I2C_PIN_SCL     GPIO_Pin_6
#define I2C_PIN_SDA     GPIO_Pin_7
#define I2C_GPIO        GPIOB
#define I2C_DIO_SCL     22
#define I2C_DIO_SDA     23

#endif  //  HAL_TWI_REMAP

#elif (defined HAL_TWI_BUS) && (HAL_TWI_BUS == 2)
#if (defined HAL_TWI_REMAP) && (HAL_TWI_REMAP == 1)
// STM32F303VC
// I2C2, Config 1, PF1 - SCL, PF0 - SDA, AF4
#define DIO_MODE_TWI    ((4<<DIO_AF_OFFS) | DIO_MODE_AF_OD)
#define I2C_PIN_SCL     GPIO_Pin_1
#define I2C_PIN_SDA     GPIO_Pin_0
#define I2C_GPIO        GPIOF
#define I2C_DIO_SCL     81
#define I2C_DIO_SDA     80

#else   //  HAL_TWI_REMAP - default
// I2C2, Config 0, PA9 - SCL, PA10 - SDA, AF4
#define DIO_MODE_TWI    ((4<<DIO_AF_OFFS) | DIO_MODE_AF_OD)
#define I2C_PIN_SCL     GPIO_Pin_9
#define I2C_PIN_SDA     GPIO_Pin_10
#define I2C_GPIO        GPIOA
#define I2C_DIO_SCL     9
#define I2C_DIO_SDA     10

#endif  //  HAL_TWI_REMAP
#endif  //  HAL_TWI_BUS
// End TWI Section
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// HAL AIn Section
// PA0 - PA7, PB0 - PB1, PC0 - PC5, PB2, PB12 - PB15
#define HAL_AIN_AIN2DPIN    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    \
                             0x10, 0x11, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,    \
                             0x12, 0x1C, 0x1D, 0x1E, 0x1F}

// apin to configuration conversion
// bits 7,6     : ADC1 - ADC3, bits 4-0 : Channel 1 - 18
#define HAL_AIN_AIN2CFG     {0x01, 0x02, 0x03, 0x04, /* PA0 - PA3 : 1.1 - 1.4 */    \
                             0x41, 0x42, 0x43, 0x44, /* PA4 - PA7 : 2.1 - 2.4 */    \
                             0x0B, 0x0C,             /* PB0 - PB1 : 1.11 - 1.12 */  \
                             0x06, 0x07, 0x08, 0x09, /* PC0 - PC3 : 1.6 - 1.9 */    \
                             0x45, 0x4B,             /* PC4, PC5  : 2.5, 2.11 */    \
                             0x4C, 0x4D,             /* PB2, PB12 : 2.12, 2.13 */   \
                             0x0E,                   /* PB13      : 1.13 */         \
                             0x4E, 0x4F}             /* PB14 - PB15 : 2.14 - 2.15 */\
// HAL AIn Section
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// PWM Section

#if (defined __STM32F303xC_H)
#define HAL_PWM_TIMERS      {NULL, TIM1, TIM2, TIM3,  TIM4,  NULL,  \
                             NULL, NULL, TIM8, NULL,  NULL,  NULL,  \
                             NULL, NULL, NULL, TIM15, TIM16, TIM17}
#else
#define HAL_PWM_TIMERS      {NULL, TIM1, TIM2, TIM3,  NULL,  NULL,  \
                             NULL, NULL, NULL, NULL,  NULL,  NULL,  \
                             NULL, NULL, NULL, TIM15, TIM16, TIM17}
#endif  //  __STM32F303xC_H
// PWM Section
//////////////////////////////////////////////////////////////

#endif  //  HW_STM32F3_H
