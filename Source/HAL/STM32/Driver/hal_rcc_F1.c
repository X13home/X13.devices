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

#if (defined STM32F1)

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

    // Enable Prefetch Buffer
    FLASH->ACR |= FLASH_ACR_PRFTBE;

    // Flash 2 wait state
    FLASH->ACR &= ~FLASH_ACR_LATENCY;
    FLASH->ACR |= FLASH_ACR_LATENCY_2;

    // PLL->CFGR reset in SystemInit()
    // SYSCLK = 72, on PLL
    // AHB Prescaler = 1, HCLK = SYSCLK
    // APB2 Prescaler = 1, PCLK2 = HCLK
    // APB1 Prescaler = 2, PCLK1 = HCLK/2
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;
    // ADC Prescaler = 6;
    RCC->CFGR |= RCC_CFGR_ADCPRE_DIV6;

    if(StartUpCounter < HSE_STARTUP_TIME)
    {
        // PLL configuration: PLLCLK = HSE * 9 = 72 MHz
        RCC->CFGR |= (RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL9);
    }
    else
    {
        // PLL configuration: PLLCLK = HSI * 16 = 64 MHz, Not acceptabel for USB
        // HSI clock divided by 2 selected as PLL entry clock source
        // PLL multiplication factor = 16
        RCC->CR &= ~RCC_CR_HSEON;       // Disble HSE
        RCC->CFGR |= RCC_CFGR_PLLMULL16;
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

uint32_t hal_GetClock(uint32_t base)
{
    #error hal_GetClock
    return SystemCoreClock;
}

#endif  //  STM32F1
