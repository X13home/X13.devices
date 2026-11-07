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

#if (defined STM32F0)

#define HSE_STARTUP_TIME            0x00005000UL

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

    // Enable Prefetch Buffer and set Flash Latency
    FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY;

    RCC->CFGR &= ~(RCC_CFGR_PLLMUL |                // Reset PLL Multiplication factor
                   RCC_CFGR_HPRE |                  // HCLK not divided
                   RCC_CFGR_PPRE);                  // SYSCLK not divided

    if(StartUpCounter < HSE_STARTUP_TIME)
    {
        RCC->CFGR |= (RCC_CFGR_PLLMUL6 |            // PLL multiplication factor = 6
                      RCC_CFGR_PLLSRC_HSE_PREDIV);  // HSE/PREDIV clock selected
                                                    //      as PLL entry clock source
    }
    else
    {
        RCC->CR &= ~RCC_CR_HSEON;                   // Disble HSE
        RCC->CFGR &= ~RCC_CFGR_PLLSRC_HSE_PREDIV;   // HSI clock divided by 2 selected
                                                    //      as PLL entry clock source
        RCC->CFGR |= RCC_CFGR_PLLMUL12;             // PLL multiplication factor = 12
    }

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

uint32_t hal_GetClock(uint32_t base __attribute__ ((unused)))
{
    return SystemCoreClock;
}

uint32_t hal_GetDeviceID(void)
{
    return *(__IO uint32_t*)0x1FFFF7AC;
}

#endif  //  STM32F0
