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

// STM32F1 has no peripheral clock mux: every peripheral is fed by the APB its bus sits on.
// hal_SetSysClock above leaves HCLK = SYSCLK and PCLK2 = HCLK while setting PCLK1 = HCLK/2, but
// the prescalers are read back here, so a board that changes them still gets the right answer.
// APB prescaler field: 0xx = /1, 100 = /2, 101 = /4, 110 = /8, 111 = /16 (RM0008, RCC_CFGR).
static uint32_t hal_get_pclk1(void)
{
    uint32_t ppre = RCC->CFGR & RCC_CFGR_PPRE1;

    if(ppre == RCC_CFGR_PPRE1_DIV2)
        return SystemCoreClock / 2;
    if(ppre == RCC_CFGR_PPRE1_DIV4)
        return SystemCoreClock / 4;
    if(ppre == RCC_CFGR_PPRE1_DIV8)
        return SystemCoreClock / 8;
    if(ppre == RCC_CFGR_PPRE1_DIV16)
        return SystemCoreClock / 16;

    return SystemCoreClock;
}

static uint32_t hal_get_pclk2(void)
{
    uint32_t ppre = RCC->CFGR & RCC_CFGR_PPRE2;

    if(ppre == RCC_CFGR_PPRE2_DIV2)
        return SystemCoreClock / 2;
    if(ppre == RCC_CFGR_PPRE2_DIV4)
        return SystemCoreClock / 4;
    if(ppre == RCC_CFGR_PPRE2_DIV8)
        return SystemCoreClock / 8;
    if(ppre == RCC_CFGR_PPRE2_DIV16)
        return SystemCoreClock / 16;

    return SystemCoreClock;
}

// A timer whose APB prescaler is not /1 is fed twice the APB clock (RM0008, Clock tree).
static uint32_t hal_get_tclk1(void)
{
    uint32_t pclk = hal_get_pclk1();

    if((RCC->CFGR & RCC_CFGR_PPRE1) != RCC_CFGR_PPRE1_DIV1)
        pclk *= 2;

    return pclk;
}

static uint32_t hal_get_tclk2(void)
{
    uint32_t pclk = hal_get_pclk2();

    if((RCC->CFGR & RCC_CFGR_PPRE2) != RCC_CFGR_PPRE2_DIV1)
        pclk *= 2;

    return pclk;
}

uint32_t hal_GetClock(uint32_t base)
{
    switch(base)
    {
        // APB2
#ifdef USART1_BASE
        case USART1_BASE:
#endif  //  USART1_BASE
#ifdef SPI1_BASE
        case SPI1_BASE:
#endif  //  SPI1_BASE
            return hal_get_pclk2();

        // APB1
#ifdef USART2_BASE
        case USART2_BASE:
#endif  //  USART2_BASE
#ifdef USART3_BASE
        case USART3_BASE:
#endif  //  USART3_BASE
#ifdef UART4_BASE
        case UART4_BASE:
#endif  //  UART4_BASE
#ifdef UART5_BASE
        case UART5_BASE:
#endif  //  UART5_BASE
#ifdef SPI2_BASE
        case SPI2_BASE:
#endif  //  SPI2_BASE
#ifdef SPI3_BASE
        case SPI3_BASE:
#endif  //  SPI3_BASE
#ifdef I2C1_BASE
        case I2C1_BASE:
#endif  //  I2C1_BASE
#ifdef I2C2_BASE
        case I2C2_BASE:
#endif  //  I2C2_BASE
            return hal_get_pclk1();

        // Timers on APB2
#ifdef TIM1_BASE
        case TIM1_BASE:
#endif  //  TIM1_BASE
#ifdef TIM8_BASE
        case TIM8_BASE:
#endif  //  TIM8_BASE
#ifdef TIM9_BASE
        case TIM9_BASE:
#endif  //  TIM9_BASE
#ifdef TIM10_BASE
        case TIM10_BASE:
#endif  //  TIM10_BASE
#ifdef TIM11_BASE
        case TIM11_BASE:
#endif  //  TIM11_BASE
            return hal_get_tclk2();

        // Timers on APB1
#ifdef TIM2_BASE
        case TIM2_BASE:
#endif  //  TIM2_BASE
#ifdef TIM3_BASE
        case TIM3_BASE:
#endif  //  TIM3_BASE
#ifdef TIM4_BASE
        case TIM4_BASE:
#endif  //  TIM4_BASE
#ifdef TIM5_BASE
        case TIM5_BASE:
#endif  //  TIM5_BASE
#ifdef TIM6_BASE
        case TIM6_BASE:
#endif  //  TIM6_BASE
#ifdef TIM7_BASE
        case TIM7_BASE:
#endif  //  TIM7_BASE
#ifdef TIM12_BASE
        case TIM12_BASE:
#endif  //  TIM12_BASE
#ifdef TIM13_BASE
        case TIM13_BASE:
#endif  //  TIM13_BASE
#ifdef TIM14_BASE
        case TIM14_BASE:
#endif  //  TIM14_BASE
            return hal_get_tclk1();

        default:
            break;
    }

    while(1);

    return 0;
}

uint32_t hal_GetDeviceID(void)
{
    // Unique device ID register, RM0008: 96 bits at 0x1FFFF7E8
    return *(__IO uint32_t*)0x1FFFF7E8;
}

#endif  //  STM32F1
