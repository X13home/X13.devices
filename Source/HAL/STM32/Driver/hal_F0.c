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

    hal_hclk  = 48000000UL;
    hal_pclk1 = 48000000UL;
    hal_pclk2 = 48000000UL;

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

#if (defined HAL_USE_EXTI)
typedef void (*cbEXTI_t)(void);

void EXTI0_1_IRQHandler(void);
void EXTI2_3_IRQHandler(void);
void EXTI4_15_IRQHandler(void);

static void * hal_exti_cb[16] = {[0 ... 15] = NULL};

void hal_exti_config(uint8_t pin, uint8_t Trigger, void * pCallback)
{
    // Enable SYSCFG clock
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    uint8_t port = (pin >> 4);
    pin &= 0x0F;
    hal_exti_cb[pin] = pCallback;

    // Connect EXTIx Line to IRQ PIN
    SYSCFG->EXTICR[pin >> 0x02] &= ~(15UL << (0x04 * (pin & (uint8_t)0x03)));
    SYSCFG->EXTICR[pin >> 0x02] |= port << (0x04 * (pin & (uint8_t)0x03));

    uint32_t u32_tmp = (1UL << pin);
    EXTI->IMR |= u32_tmp;               // Enable Interrupt
    EXTI->EMR &= ~u32_tmp;              // Disable Events
    EXTI->PR = (uint16_t)u32_tmp;       // Reset pending status

    if(Trigger & 1)                     // Falling trigger
    {
        EXTI->FTSR |= u32_tmp;
    }
    else
    {
        EXTI->FTSR &= ~u32_tmp;
    }

    if(Trigger & 2)                     // Rising trigger
    {
        EXTI->RTSR |= u32_tmp;
    }
    else
    {
        EXTI->RTSR &= ~u32_tmp;
    }

    // Enable IRQ
    IRQn_Type IRQn;

    if(pin < 2)
    {
        IRQn = EXTI0_1_IRQn;
    }
    else if(pin < 4)
    {
        IRQn = EXTI2_3_IRQn;
    }
    else
    {
        IRQn = EXTI4_15_IRQn;
    }

    NVIC_SetPriority(IRQn, 0);
    NVIC_EnableIRQ(IRQn);
}

// generate IRQ on pin
void hal_exti_trig(uint8_t pin)
{
    EXTI->SWIER |= (1UL << (pin & 0x0F));
}

void EXTI0_1_IRQHandler(void)
{
    if((EXTI->PR & 1) != 0)
    {
        EXTI->PR = 1;
        cbEXTI_t cb = hal_exti_cb[0];
        if(cb != NULL)
        {
            cb();
        }
    }

    if((EXTI->PR & 2) != 0)
    {
        EXTI->PR = 2;
        cbEXTI_t cb = hal_exti_cb[1];
        if(cb != NULL)
        {
            cb();
        }
    }
}

void EXTI2_3_IRQHandler(void)
{
    if((EXTI->PR & 4) != 0)
    {
        EXTI->PR = 4;
        cbEXTI_t cb = hal_exti_cb[2];
        if(cb != NULL)
        {
            cb();
        }
    }

    if((EXTI->PR & 8) != 0)
    {
        EXTI->PR = 8;
        cbEXTI_t cb = hal_exti_cb[3];
        if(cb != NULL)
        {
            cb();
        }
    }
}

void EXTI4_15_IRQHandler(void)
{
    uint8_t pos;
    for(pos = 4; pos < 16; pos++)
    {
        uint32_t mask = 1UL << pos;
        if((EXTI->PR & mask) != 0)
        {
            EXTI->PR = mask;
            cbEXTI_t cb = hal_exti_cb[pos];
            if(cb != NULL)
            {
                cb();
            }
        }
    }
}
#endif  // HAL_USE_EXTI

#endif  //  STM32F0