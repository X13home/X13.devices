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

// HAL_DIO Shared functions
static const GPIO_TypeDef * dio_pGPIOx[] =
{
    GPIOA,
    GPIOB,
    GPIOC,
    GPIOD,
#ifdef GPIOE
    GPIOE,
#else
    NULL,
#endif  //  GPIOE
#ifdef GPIOF
    GPIOF,
#else
    NULL,
#endif  //  GPIOF
    NULL
};

static GPIO_TypeDef * hal_dio_PortNr2GPIOx(uint8_t PortNr)
{
    if(PortNr < (const uint8_t)(sizeof(dio_pGPIOx) / sizeof(dio_pGPIOx[0])))
    {
        return (GPIO_TypeDef *)dio_pGPIOx[PortNr];
    }
    return NULL;
}

void hal_dio_configure(uint8_t Pin, uint16_t Mode)
{
    GPIO_TypeDef * pGPIO = hal_dio_PortNr2GPIOx(Pin >> 4);
    if(pGPIO == NULL)
    {
        return;
    }
    uint16_t Mask = 1UL<<(Pin & 0x0F);
    hal_gpio_cfg(pGPIO, Mask, Mode);
}
// End HAL_DIO Shared functions

#ifdef EXTDIO_USED
static const __attribute__ ((aligned (4))) uint8_t hal_dio_sBase2Base[] = HAL_DIO_MAPPING;

uint8_t hal_dio_base2pin(uint16_t base)
{
    if(base >= sizeof(hal_dio_sBase2Base))
    {
        return 0xFF;
    }

    return hal_dio_sBase2Base[base];
}

uint16_t hal_dio_read(uint8_t PortNr)
{
    GPIO_TypeDef * GPIOx = hal_dio_PortNr2GPIOx(PortNr);
    if(GPIOx == NULL)
    {
        return 0;
    }

    return ((uint16_t)GPIOx->IDR);
}

void hal_dio_set(uint8_t PortNr, uint16_t Mask)
{
    GPIO_TypeDef * GPIOx = hal_dio_PortNr2GPIOx(PortNr);
    if(GPIOx != NULL)
    {
        GPIOx->BSRR = Mask;
    }
}

void hal_dio_reset(uint8_t PortNr, uint16_t Mask)
{
    GPIO_TypeDef * GPIOx = hal_dio_PortNr2GPIOx(PortNr);
    if(GPIOx != NULL)
    {
#if (defined BRR)
        GPIOx->BRR = Mask;
#else   // BRR Not defined
        GPIOx->BSRR = Mask << 16;
#endif  // BRR
    }
}
#endif  //  EXTDIO_USED
