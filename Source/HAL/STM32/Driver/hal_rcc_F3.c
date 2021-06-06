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

#if !defined  (HSI_VALUE)
    #define HSI_VALUE    ((uint32_t)8000000)
#endif /* HSI_VALUE */


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
    }
    else
    {
        RCC->CR &= ~RCC_CR_HSEON;                   // Disble HSE
        RCC->CFGR |= RCC_CFGR_PLLMUL16;
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

static uint32_t hal_get_pclk1(void)
{
    return (SystemCoreClock >> APBPrescTable[(RCC->CFGR & RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos]);
}

static uint32_t hal_get_pclk2(void)
{
    return (SystemCoreClock >> APBPrescTable[(RCC->CFGR & RCC_CFGR_PPRE2) >> RCC_CFGR_PPRE2_Pos]);
}


uint32_t hal_GetClock(uint32_t base)
{
    switch(base)
    {
#ifdef USART1_BASE
        case USART1_BASE:
            switch(RCC->CFGR3 & RCC_CFGR3_USART1SW)
            {
#if (defined RCC_CFGR3_USART1SW_PCLK2)
                case RCC_CFGR3_USART1SW_PCLK2:
                    return hal_get_pclk2();
#elif (defined RCC_CFGR3_USART1SW_PCLK1)
                case RCC_CFGR3_USART1SW_PCLK1:
                    return hal_get_pclk1();
#endif  //  RCC_CFGR3_USART1SW_PCLK2
                case RCC_CFGR3_USART1SW_SYSCLK:
                    return SystemCoreClock;
#ifdef LSE_VALUE
                case RCC_CFGR3_USART1SW_LSE:
                    return LSE_VALUE;
#endif  //  LSE_VALUE
                case RCC_CFGR3_USART1SW_HSI:
                    return HSI_VALUE;
            }
            break;
#endif  //  USART1_BASE

#ifdef USART2_BASE
        case USART2_BASE:
#if (defined RCC_CFGR3_USART2SW)
            switch(RCC->CFGR3 & RCC_CFGR3_USART2SW)
            {
                case RCC_CFGR3_USART2SW_PCLK:
                    return hal_get_pclk1();
                case RCC_CFGR3_USART2SW_SYSCLK:
                    return SystemCoreClock;
#ifdef LSE_VALUE
                case RCC_CFGR3_USART2SW_LSE:
                    return LSE_VALUE;
#endif  //  LSE_VALUE
                case RCC_CFGR3_USART2SW_HSI:
                    return HSI_VALUE;
            }
            break;
#else
            return hal_get_pclk1();
#endif  //  RCC_CFGR3_USART2SW
#endif  //  USART2_BASE

#ifdef USART3_BASE
        case USART3_BASE:
#ifdef RCC_CFGR3_USART3SW
            switch(RCC->CFGR3 & RCC_CFGR3_USART3SW)
            {
                case RCC_CFGR3_USART3SW_PCLK:
                    return hal_get_pclk1();
                case RCC_CFGR3_USART3SW_SYSCLK:
                    return SystemCoreClock;
#ifdef LSE_VALUE
                case RCC_CFGR3_USART3SW_LSE:
                    return LSE_VALUE;
#endif  //  LSE_VALUE
                case RCC_CFGR3_USART3SW_HSI:
                    return HSI_VALUE;
            }
            break;
#else
            return hal_get_pclk1();
#endif  //  RCC_CFGR3_USART3SW
#endif  //  USART3_BASE

#if (defined SPI1_BASE)
        case SPI1_BASE:
            return hal_get_pclk2();
#endif  //  HAL_USE_SPI1
#if (defined SPI2_BASE)
        case SPI2_BASE:
            return hal_get_pclk1();
#endif  //  HAL_USE_SPI2
#if (defined SPI3_BASE)
        case SPI3_BASE:
            return hal_get_pclk1();
#endif  //  HAL_USE_SPI3

        // Tim1 - Tim3, Tim15-17 - 303x8
        // Tim1 - Tim4, Tim8, Tim15-Tim17, Tim20 - 303xE

#ifdef TIM1
        case TIM1_BASE:
            if((RCC->CFGR3 & RCC_CFGR3_TIM1SW) != 0)
            {
                // PLL VCO Clock
                return SystemCoreClock;
            }
            else
            {
                return hal_get_pclk2();
            }
#endif  // TIM1
#ifdef TIM4
        case TIM4_BASE:
            return hal_get_pclk1();
#endif  // TIM4
#ifdef TIM2
        case TIM2_BASE:
#endif  //  TIM2
#ifdef TIM3
        case TIM3_BASE:
#endif  //  TIM3
        case TIM6_BASE:
            if((RCC->CFGR & RCC_CFGR_PPRE1) == RCC_CFGR_PPRE1_DIV1)
            {
                return hal_get_pclk1();
            }
            else
            {
                return hal_get_pclk1() * 2;
            }
#ifdef TIM8
        case TIM8_BASE:
            return hal_get_pclk2();
#endif  // TIM8
#ifdef TIM15
        case TIM15_BASE:
#endif  //  TIM15
#ifdef  TIM16
        case TIM16_BASE:
#endif  //  TIM16
#ifdef  TIM17
        case TIM17_BASE:
            return hal_get_pclk2();
#endif  //  TIM17

/*
#ifdef TIM9
        case (uint32_t)TIM9:
            return hal_pclk2;
#endif  // TIM9
#ifdef TIM10
        case (uint32_t)TIM10;
            return hal_pclk2;
#endif  // TIM10
#ifdef TIM11
        case (uint32_t)TIM11:
            return hal_pclk2;
#endif  // TIM11
#ifdef TIM14
        case (uint32_t)TIM14:
            return hal_pclk1;
#endif  //  TIM14
#ifdef  TIM21
        case (uint32_t)TIM21:
            return hal_pclk2;
#endif  //  TIM21
#ifdef  TIM22
        case (uint32_t)TIM22:
            return hal_pclk2;
#endif  //  TIM22
*/
        default:
            break;
    }

    while(1);

    return 0;
}

uint32_t hal_GetDeviceID(void)
{
    return *(__IO uint32_t*)0x1FFFF7AC;
}

#endif  // STM32F3
