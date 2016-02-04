#include "config.h"

#if (defined STM32F0)
void hal_gpio_cfg(GPIO_TypeDef * GPIOx, uint16_t Mask, uint16_t Mode)
{
    uint16_t pinpos;
    uint32_t pos;
    uint32_t pupd = Mode & 0x03;            // Float / Pull-Up / Pull-Down
    uint32_t ppod = (Mode & 0x04) >> 2;     // Push-Pull / Open Drain
    uint32_t mod  = (Mode & 0x18) >> 3;     // Input / Output / AF / Analog
    uint32_t spd  = (Mode & 0x60) >> 5;     // Low / Medim / Fast / High Speed
    uint32_t afr  = (Mode & 0x0F00) >> 8;   // Alternative function

    for(pinpos = 0; pinpos < 0x10; pinpos++)
    {
        pos = ((uint32_t)0x01) << pinpos;
        if(Mask & pos)
        {
            pos = pinpos << 1;
        
            // default state input
            GPIOx->MODER  &= ~(GPIO_MODER_MODER0 << pos);
            GPIOx->MODER |= mod << pos;
            
            if((mod == 1) || (mod == 2))
            {
                // Low Speed
                GPIOx->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR0 << pos);
                GPIOx->OSPEEDR |= spd << pos;

                // Output type - push/pull
                GPIOx->OTYPER &= ~((GPIO_OTYPER_OT_0) << pinpos);
                GPIOx->OTYPER |= ppod << pinpos;
            }
            // without PullUp/Down
            GPIOx->PUPDR &= ~(GPIO_PUPDR_PUPDR0 << pos);
            GPIOx->PUPDR |= pupd << pos;

            if(mod == 2)
            {
                // Set Alternative function
                if(pinpos < 8)              // AFR0
                {
                    pos = (pinpos << 2);
                    GPIOx->AFR[0] &= ~((uint32_t)0x0000000F << pos);
                    GPIOx->AFR[0] |= ((uint32_t)afr << pos);
                }
                else                        // AFR1
                {
                    pos = ((pinpos - 8) << 2);
                    GPIOx->AFR[1] &= ~((uint32_t)0x0000000F << pos);
                    GPIOx->AFR[1] |= ((uint32_t)afr << pos);
                }
            }
        }
    }
}

#elif (defined STM32F1)
void hal_gpio_cfg(GPIO_TypeDef * GPIOx, uint16_t Mask, uint16_t Mode)
{
    uint16_t pinpos;
    uint32_t pos;

    for(pinpos = 0; pinpos < 0x10; pinpos++)
    {
        pos = ((uint32_t)0x01) << pinpos;
        if(Mask & pos)
        {
            uint32_t gpio_cr;
        
            switch(Mode & 0x00FF)
            {
                case DIO_MODE_IN_PU:
                    gpio_cr = GPIO_CRL_CNF0_1;  // Input with pull-up / pull-down
                    GPIOx->BSRR = pos;
                    break;
                case DIO_MODE_IN_PD:
                    gpio_cr = GPIO_CRL_CNF0_1;  // Input with pull-up / pull-down
                    GPIOx->BRR = pos;
                    break;
                case DIO_MODE_OUT_PP:
                    gpio_cr = GPIO_CRL_MODE0_1; // General purpose output push-pull, Low Speed (2MHz)
                    break;
                case DIO_MODE_AF_PP:
                    gpio_cr = GPIO_CRL_CNF0_1 | GPIO_CRL_MODE0_1;   // AF Push-Pull out, Low Speed
                    break;
                case DIO_MODE_AF_OD:
                    gpio_cr = GPIO_CRL_CNF0 | GPIO_CRL_MODE0_1;     // AF Open-Drain out, Low Speed
                    break;
                case DIO_MODE_AIN:
                    gpio_cr = 0;
                    break;
                case DIO_MODE_AF_PP_HS:
                    gpio_cr = GPIO_CRL_CNF0_1 | GPIO_CRL_MODE0;     // AF Push-Pull out, HS
                    break;
                case DIO_MODE_OUT_PP_HS:
                    gpio_cr = GPIO_CRL_MODE0;                       // General purpose output push-pull, High Speed(50MHz)
                    break;
                    
                default:                                            // Floating digital input
                    gpio_cr = GPIO_CRL_CNF0_0;
                    break;
            }

            pos = (pinpos & 0x07) << 2;
            gpio_cr <<= pos;

            if(pinpos < 0x08)
            {
                GPIOx->CRL &= ~(((uint32_t)0x0F) << pos);
                GPIOx->CRL |= gpio_cr;
            }
            else
            {
                GPIOx->CRH &= ~(((uint32_t)0x0F) << pos);
                GPIOx->CRH |= gpio_cr;
            }
        }
    }
}

#else
#error hal_dio.c Unknown uC Family
#endif

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
        return (GPIO_TypeDef *)dio_pGPIOx[PortNr];

    return NULL;
}

void hal_dio_configure(uint8_t Pin, uint16_t Mode)
{
    GPIO_TypeDef * pGPIO = hal_dio_PortNr2GPIOx(Pin >> 4);
    if(pGPIO == NULL)
        return;

    uint16_t Mask = 1UL<<(Pin & 0x0F);
    hal_gpio_cfg(pGPIO, Mask, Mode);
}

#ifdef EXTDIO_USED

static const __attribute__ ((aligned (4))) uint8_t hal_dio_sBase2Base[] = HAL_DIO_MAPPING;

uint8_t hal_dio_base2pin(uint16_t base)
{
    if(base >= sizeof(hal_dio_sBase2Base))
        return 0xFF;
    
    return hal_dio_sBase2Base[base];
}

uint16_t hal_dio_read(uint8_t PortNr)
{
    GPIO_TypeDef * GPIOx = hal_dio_PortNr2GPIOx(PortNr);
    if(GPIOx == NULL)
        return 0;

    return ((uint16_t)GPIOx->IDR);
}

void hal_dio_set(uint8_t PortNr, uint16_t Mask)
{
    GPIO_TypeDef * GPIOx = hal_dio_PortNr2GPIOx(PortNr);
    if(GPIOx != NULL)
        hal_gpio_set(GPIOx, Mask);
}

void hal_dio_reset(uint8_t PortNr, uint16_t Mask)
{
    GPIO_TypeDef * GPIOx = hal_dio_PortNr2GPIOx(PortNr);
    if(GPIOx != NULL)
        hal_gpio_reset(GPIOx, Mask);
}

#endif  //  EXTDIO_USED
