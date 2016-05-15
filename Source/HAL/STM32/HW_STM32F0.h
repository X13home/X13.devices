/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef HW_STM32F0_H
#define HW_STM32F0_H

// STM32F051C8
#if   (defined CFG_S2EC13)
#include "S2xxxx/S2EC13.h"      // ENC28J60 + CC1101
#elif   (defined CFG_S2En13)
#include "S2xxxx/S2En13.h"      // ENC28J60
#elif   (defined CFG_S2Cn13)
#include "S2xxxx/S2Cn13.h"      // CC1101
#elif   (defined CFG_S2Sn13)
#include "S2xxxx/S2Sn13.h"      // UART_ALT

// STM32F051R8
#elif   (defined CFG_S2Sn10)
#include "S2xxxx/S2Sn10.h"      // UART
#elif   (defined CFG_S2SR10)
#include "S2xxxx/S2SR10.h"      // UART + RFM12

// STM32F091RC
#elif   (defined CFG_S2Sn11)
#include "S2xxxx/S2Sn11.h"      // UART

// STM32F051K8
#elif   (defined CFG_S2SC14)
#include "S2xxxx/S2SC14.h"      // UART + CC1101

// UNode V3.0 - STM32F051K8
#elif   (defined CFG_S2SC15)    
#include "S2xxxx/S2SC15.h"      // UART + CC1101
#elif   (defined CFG_S2Cn15)    
#include "S2xxxx/S2Cn15.h"      // UART + CC1101

#else
#error Unknown configuration
#endif  //  Configuration

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

#if (defined HAL_USART1_ALT)
#define HAL_USART1_AF           DIO_MODE_AF_PP
#define HAL_USART1_PIN_RX       23                      // GPIOB PIN7
#define HAL_USART1_PIN_TX       22                      // GPIOB PIN6

#else
#define HAL_USART1_AF           ((1<<DIO_AF_OFFS) | DIO_MODE_AF_PP)
#define HAL_USART1_PIN_RX       10                      // GPIOA PIN10
#define HAL_USART1_PIN_TX       9                       // GPIOA PIN9

#endif  //  HAL_USART1_ALT

#define HAL_USART2_AF           ((1<<DIO_AF_OFFS) | DIO_MODE_AF_PP)
#define HAL_USART2_PIN_RX       3                       // GPIOA PIN3
#define HAL_USART2_PIN_TX       2                       // GPIOA PIN2

//  End UART Section
//////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////
// TWI Section
#if (defined HAL_TWI_BUS)

#ifndef HAL_TWI_CONFIG
#define HAL_TWI_CONFIG  1
#endif  //  HAL_TWI_CONFIG

#define DIO_MODE_TWI    ((1<<DIO_AF_OFFS) | DIO_MODE_AF_OD)       // Alternative function, AF = 1, Open Drain

#if (HAL_TWI_BUS == 1)

void I2C1_IRQHandler(void);
#define ISR_TWI_HANDLER() I2C1_IRQHandler(void)

#define I2C_BUS         I2C1
#define I2C_IRQn        I2C1_IRQn

#if (HAL_TWI_CONFIG == 1)
// I2C1, PB6 - SCL, PB7 - SDA
#define I2C_PIN_SCL     GPIO_Pin_6
#define I2C_PIN_SDA     GPIO_Pin_7
#define I2C_GPIO        GPIOB

#define I2C_DIO_SCL     22
#define I2C_DIO_SDA     23

#elif (HAL_TWI_CONFIG == 2)
// I2C1, PB8 - SCL, PB9 - SDA
#define I2C_PIN_SCL     GPIO_Pin_8
#define I2C_PIN_SDA     GPIO_Pin_9
#define I2C_GPIO        GPIOA

#define I2C_DIO_SCL     24
#define I2C_DIO_SDA     25

#else
#error Unknown HAL_TWI_CONFIG
#endif  //  HAL_TWI_CONFIG

#elif (HAL_TWI_BUS == 2)

void I2C2_IRQHandler(void);
#define ISR_TWI_HANDLER() I2C2_IRQHandler(void)

#define I2C_BUS         I2C2
#define I2C_IRQn        I2C2_IRQn

// I2C2, PB10 - SCL, PB11 - SDA
#define I2C_PIN_SCL     GPIO_Pin_10
#define I2C_PIN_SDA     GPIO_Pin_11
#define I2C_GPIO        GPIOB

#define I2C_DIO_SCL     26
#define I2C_DIO_SDA     27

#endif  //  HAL_TWI_BUS - config
#endif  //  HAL_TWI_BUS

// TWI Section
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// HAL AIn Section
#define EXTAIN_REF                  0x02        // Bit0 - Ext, Bit1 - Vcc, Bit2 - Int1, Bit3 - Int2
// HAL AIn Section
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// PWM Section

#if defined(STM32F051x8) || defined(STM32F091xC)
#define HAL_PWM_TIMERS      {NULL, TIM1, TIM2, TIM3, NULL,  \
                             NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,   /* TIM5 - TIM13 */ \
                             TIM14, TIM15, TIM16, TIM17}
#else
#error unknown uC
#endif

// PWM Section
//////////////////////////////////////////////////////////////

#endif  //  HW_STM32F0_H
