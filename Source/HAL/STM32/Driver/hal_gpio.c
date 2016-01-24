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
        
            switch(Mode)
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
#error hal_gpio.c Unknown uC Family
#endif