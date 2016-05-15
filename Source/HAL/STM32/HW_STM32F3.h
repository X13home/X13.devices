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
#define HAL_USART1_AF           ((7<<DIO_AF_OFFS) | DIO_MODE_AF_PP)

#if   (defined HAL_USART1_ALT)
#define HAL_USART1_PIN_TX       22                      // PB6
#define HAL_USART1_PIN_RX       23                      // PB7

#else   //  HAL_USART1_ALT - default
#define HAL_USART1_PIN_TX       9                       // PA9
#define HAL_USART1_PIN_RX       10                      // PA10

#endif  //  HAL_USART1_ALT

// USART 2
#define HAL_USART2_AF           ((7<<DIO_AF_OFFS) | DIO_MODE_AF_PP)

#ifndef HAL_USART2_ALT
#define HAL_USART2_ALT          1
#endif  //  HAL_USART2_ALT

#if   (HAL_USART2_ALT == 2)
#define HAL_USART2_PIN_TX       19                      // PB3
#define HAL_USART2_PIN_RX       20                      // PB4

#elif (HAL_USART2_ALT == 3)
#define HAL_USART2_PIN_TX       14                      // PA14
#define HAL_USART2_PIN_RX       15                      // PA15

#elif (HAL_USART2_ALT == 13)
#define HAL_USART2_PIN_TX       2                       // PA2
#define HAL_USART2_PIN_RX       15                      // PA15

#else   //  HAL_USART2_ALT - default
#define HAL_USART2_PIN_TX       2                       // PA2
#define HAL_USART2_PIN_RX       3                       // PA3

#endif  //  HAL_USART2_ALT

//  End UART Section
//////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////
// TWI Section
// STM32F303K8 / STM32F334K8
#if (defined HAL_TWI_BUS) && (HAL_TWI_BUS == 1)

#ifndef HAL_TWI_CONFIG
#define HAL_TWI_CONFIG  1
#endif  //  HAL_TWI_CONFIG

void I2C1_EV_IRQHandler(void);
#define ISR_TWI_HANDLER() I2C1_EV_IRQHandler(void)

#define DIO_MODE_TWI    ((4<<DIO_AF_OFFS) | DIO_MODE_AF_OD)       // Alternative function, AF = 4, Open Drain

#define I2C_BUS         I2C1
#define I2C_IRQn        I2C1_EV_IRQn

#if (HAL_TWI_CONFIG == 1)
// I2C1, PB6 - SCL, PB7 - SDA
#define I2C_PIN_SCL     GPIO_Pin_6
#define I2C_PIN_SDA     GPIO_Pin_7
#define I2C_GPIO        GPIOB

#define I2C_DIO_SCL     22
#define I2C_DIO_SDA     23

#elif (HAL_TWI_CONFIG == 2)
// I2C1, PA15 - SCL, PA14 - SDA
#define I2C_PIN_SCL     GPIO_Pin_15
#define I2C_PIN_SDA     GPIO_Pin_14
#define I2C_GPIO        GPIOA

#define I2C_DIO_SCL     15
#define I2C_DIO_SDA     14

#else
#error Unknown HAL_TWI_CONFIG
#endif  //  HAL_TWI_CONFIG

#endif  //  HAL_TWI_BUS

// TWI Section
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// HAL AIn Section
#define EXTAIN_REF                  0x02        // Bit0 - Ext, Bit1 - Vcc, Bit2 - Int1, Bit3 - Int2

#if defined(STM32F303x8)
// PA0 - PA7, PB0 - PB1, Temp
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
