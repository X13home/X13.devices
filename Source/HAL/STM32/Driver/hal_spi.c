/*
Copyright (c) 2011-2016 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#include "config.h"

#if (defined HAL_USE_SPI1) || \
    (defined HAL_USE_SPI2) || \
    (defined HAL_USE_SPI3)
/*
        Config 1                Config 2
SPIx    MOSI    MISO    SCK     MOSI    MISO    SCK     APB
SPI1    PA7     PA6     PA5     PB5     PB4     PB3     2
SPI2    PB15    PB14    PB13                            1
SPI3    PB5     PB4     PB3     PC12    PC11    PC10    1
*/

#ifndef SPI2
#define SPI2    NULL
#endif  //  SPI2

#ifndef SPI3
#define SPI3    NULL
#endif  //  SPI3

static const SPI_TypeDef * hal_spi_port2spi[4] = {NULL, SPI1, SPI2, SPI3};

void hal_spi_cfg(uint8_t port, uint8_t mode, uint32_t speed)
{
    SPI_TypeDef * SPIx;

    switch(port)
    {
#if (defined HAL_USE_SPI1)
        case 1:
            RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
            RCC->APB2RSTR |= RCC_APB2RSTR_SPI1RST;
            RCC->APB2RSTR &= ~RCC_APB2RSTR_SPI1RST;
#if ((defined STM32F1) && (HAL_USE_SPI1 == 2))
            AFIO->MAPR |= AFIO_MAPR_SPI1_REMAP;     // Remap SPI1
#endif  //  STM32F1
            hal_gpio_cfg(SPI1_PORT, (SPI1_SCK_PIN | SPI1_MISO_PIN | SPI1_MOSI_PIN), DIO_MODE_SPI1);
            SPIx = SPI1;
            break;
#endif  //  HAL_USE_SPI1

#if (defined HAL_USE_SPI2)
        case 2:
            RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
            RCC->APB1RSTR |= RCC_APB1RSTR_SPI2RST;
            RCC->APB1RSTR &= ~RCC_APB1RSTR_SPI2RST;
            hal_gpio_cfg(SPI2_PORT, (SPI2_SCK_PIN | SPI2_MISO_PIN | SPI2_MOSI_PIN), DIO_MODE_SPI2);
            SPIx = SPI2;
            break;
#endif  //  DIO_MODE_SPI2

#if (defined HAL_USE_SPI3)
        case 3:
            RCC->APB1ENR |= RCC_APB1ENR_SPI3EN;
            RCC->APB1RSTR |= RCC_APB1RSTR_SPI3RST;
            RCC->APB1RSTR &= ~RCC_APB1RSTR_SPI3RST;
            hal_gpio_cfg(SPI3_PORT, (SPI3_SCK_PIN | SPI3_MISO_PIN | SPI3_MOSI_PIN), DIO_MODE_SPI3);
            SPIx = SPI3;
            break;
#endif  //  HAL_USE_SPI3
        default:
            while(1);      // Unknown SPI port
    }

    if(speed > 18000000UL)
    {
        speed = 18000000UL;
    }

    uint16_t div = 0;
    uint32_t spi_clock = hal_GetClock((uint32_t)SPIx)/2;
    
    while((spi_clock > speed) && (div < 7))
    {
        div++;
        spi_clock /= 2;
    }

    // Configure SPI
    SPIx->CR1 = (uint16_t)(SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_MSTR) | (div<<3);
#if (defined STM32F0) || (defined STM32F3)
    if(mode & HAL_SPI_16B)
    {
        SPIx->CR2 = (uint16_t)(SPI_CR2_DS_3 | SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0); // 16 bit
    }
    else
    {
        SPIx->CR2 = (uint16_t)(SPI_CR2_FRXTH | SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0); // 8 bit
    }
#elif (defined STM32F1) || (defined STM32L0)
    if(mode & HAL_SPI_16B)
    {
        SPIx->CR1 |= SPI_CR1_DFF;
    }
#else
    #error hal_spi_cfg unknown uC Family
#endif  // CPU
    if(mode & HAL_SPI_LSB)
    {
        SPIx->CR1 |= SPI_CR1_LSBFIRST;
    }

    if(mode & 1)        // Mode 1/3
    {
        SPIx->CR1 |= SPI_CR1_CPHA;
    }
    if(mode & 2)        // Mode 2/3
    {
        SPIx->CR1 |= SPI_CR1_CPOL;
    }

    SPIx->CRCPR =  7;

#if defined (SPI_I2SCFGR_I2SMOD)
    SPIx->I2SCFGR &= (uint16_t)~((uint16_t)SPI_I2SCFGR_I2SMOD);
#endif  //  SPI_I2SCFGR_I2SMOD

    SPIx->CR1 |= SPI_CR1_SPE;       // SPI enable
}

uint8_t hal_spi_exch8(uint8_t port, uint8_t data)
{
    SPI_TypeDef * SPIx = (SPI_TypeDef *)hal_spi_port2spi[port];

#if (defined STM32F0) || (defined STM32F3)
    *((__IO uint8_t *)&SPIx->DR) = data;
    while((SPIx->SR & SPI_SR_RXNE) == 0);
    data = *((__IO uint8_t *)&SPIx->DR);
    return data;
#elif (defined STM32F1) || (defined STM32L0)
    SPIx->DR = data;
    while((SPIx->SR & SPI_SR_RXNE) == 0);
    return (SPIx->DR & 0xFF);
#else
    #error hal_spi_exch8 unknown uC Family
#endif
}

uint16_t hal_spi_exch16(uint8_t port, uint16_t data)
{
    SPI_TypeDef * SPIx = (SPI_TypeDef *)hal_spi_port2spi[port];
    SPIx->DR = data;
    while((SPIx->SR & SPI_SR_RXNE) == 0);
    return SPIx->DR;
}

#endif  //  HAL_USE_SPIx
