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

#if (defined STM32F3)

#define HSE_STARTUP_TIME            0x00005000UL

uint32_t    hal_hclk  = 8000000UL;
uint32_t    hal_pclk1 = 8000000UL;
uint32_t    hal_pclk2 = 8000000UL;

void hal_SetSysClock(void)
{
    uint32_t StartUpCounter = 0;

#if (defined HSE_CRYSTAL_BYPASS)
    // HSE crystal oscillator bypassed with external clock
    RCC->CR |= RCC_CR_HSEBYP;
#endif  //  HSE_CRYSTAL_BYPASS

    // Enable HSE
    RCC->CR |= RCC_CR_HSEON;

    // Wait till HSE is ready and if Time out is reached exit
    while(((RCC->CR & RCC_CR_HSERDY) == 0) && (StartUpCounter < HSE_STARTUP_TIME))
    {
        StartUpCounter++;
    }

    // Enable Prefetch Buffer
    FLASH->ACR |= FLASH_ACR_PRFTBE;

    // Set Flash Latency, Flash 2 wait state
    FLASH->ACR &= ~FLASH_ACR_LATENCY;
    FLASH->ACR |= FLASH_ACR_LATENCY_1;

    // PLL->CFGR reset in SystemInit()
    // SYSCLK = 72, on PLL
    // AHB Prescaler = 1, HCLK = SYSCLK
    // APB2 Prescaler = 1, PCLK2 = HCLK
    // APB1 Prescaler = 2, PCLK1 = HCLK/2
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;

    if(StartUpCounter < HSE_STARTUP_TIME)
    {
        // PLL configuration: PLLCLK = HSE * 9 = 72 MHz
        RCC->CFGR |= (RCC_CFGR_PLLSRC | RCC_CFGR_PLLMUL9);
        hal_hclk  = 72000000UL;
    }
    else
    {
        RCC->CR &= ~RCC_CR_HSEON;                   // Disble HSE
        RCC->CFGR |= RCC_CFGR_PLLMUL16;
        hal_hclk  = 64000000UL;
    }

    hal_pclk2 = hal_hclk;
    hal_pclk1 = hal_hclk/2;

    // Enable PLL
    RCC->CR |= RCC_CR_PLLON;

    // Wait till PLL is ready
    while((RCC->CR & RCC_CR_PLLRDY) == 0);

    // Select PLL as system clock source
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL;

    // Wait till PLL is used as system clock source
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
}

void hal_prepare_gpio(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN |
                   RCC_AHBENR_GPIODEN |
#if defined RCC_AHBENR_GPIOEEN
                    RCC_AHBENR_GPIOEEN |
#endif
                    RCC_AHBENR_GPIOFEN;
}

// HAL GPIO Functions
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
// End HAL GPIO Functions

#endif  //  STM32F3
