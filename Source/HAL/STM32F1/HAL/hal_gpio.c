#include "config.h"

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