#include "config.h"

#ifdef EXTDIO_USED

static const GPIO_TypeDef * dio_pGPIOx[] = 
{
#ifdef GPIOA
    GPIOA,
#else
    NULL,
#endif  //  GPIOA
#ifdef GPIOB
    GPIOB,
#else
    NULL,
#endif  //  GPIOB
#ifdef GPIOC
    GPIOC,
#else
    NULL,
#endif  //  GPIOC
    NULL
};

static const hal_dio_sBase2Base[] = EXTDIO_MAPPING;

static GPIO_TypeDef * dioPortNr2GPIOx(uint8_t PortNr)
{
    if(PortNr < (const uint8_t)(sizeof(dio_pGPIOx) / sizeof(dio_pGPIOx[0])))
        return (GPIO_TypeDef *)dio_pGPIOx[PortNr];

    return NULL;
}

uint8_t hal_dio_base2pin(uint16_t base)
{
    if(base >= sizeof(hal_dio_sBase2Base))
        return 0xFF;
    
    return hal_dio_sBase2Base[base];
}

void hal_dio_configure(uint8_t PortNr, uint16_t Mask, uint16_t Mode)
{
    GPIO_TypeDef * GPIOx = dioPortNr2GPIOx(PortNr);
    if(GPIOx == NULL)
        return;

    hal_gpio_cfg(GPIOx, Mask, Mode);
}

uint16_t hal_dio_read(uint8_t PortNr)
{
    GPIO_TypeDef * GPIOx = dioPortNr2GPIOx(PortNr);
    if(GPIOx == NULL)
        return 0;

    return ((uint16_t)GPIOx->IDR);
}

void hal_dio_set(uint8_t PortNr, uint16_t Mask)
{
    GPIO_TypeDef * GPIOx = dioPortNr2GPIOx(PortNr);
    if(GPIOx != NULL)
        hal_gpio_set(GPIOx, Mask);
}

void hal_dio_reset(uint8_t PortNr, uint16_t Mask)
{
    GPIO_TypeDef * GPIOx = dioPortNr2GPIOx(PortNr);
    if(GPIOx != NULL)
        hal_gpio_reset(GPIOx, Mask);
}

#endif  //  EXTDIO_USED
