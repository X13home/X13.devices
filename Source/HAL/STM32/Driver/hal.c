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

static uint32_t CriticalNesting = 0;
static volatile uint32_t hal_ms_counter = 0;
static volatile uint32_t hal_sec_counter = 0;    // Max Uptime 136 Jr.

void halEnterCritical(void)
{
    __disable_irq();

    CriticalNesting++;

    __DSB();    //  Data Synchronization Barrier
    __ISB();    //  Instruction Synchronization Barrier
}

void halLeaveCritical(void)
{
    if(CriticalNesting == 0)
    {
        while(1);               // Error
    }

    CriticalNesting--;
    if(CriticalNesting == 0)
    {
        __enable_irq();
    }
}

void HAL_Init(void)
{
    // Set Core Clock
    hal_SetSysClock();
#ifdef HAL_USE_RTC
    hal_rtc_init();
#endif  //  HAL_USE_RTC
    // Enable GPIO clock
    hal_prepare_gpio();
    // Enable CRC, Used for RNG
    RCC->AHBENR |= RCC_AHBENR_CRCEN;
    CRC->CR = CRC_CR_RESET;

#if (__CORTEX_M >= 0x03U)   // core_cm3, core_cm4
    // delay
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;     // Enable DWT
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;                // Start counter
#endif  // core_cm?

    CriticalNesting = 0;
}

void HAL_StartSystemTick(void)
{
    if(SysTick_Config((hal_hclk / 1000) - 1UL))
    {
        while(1);
    }

    // Enable Global interrupts
    __enable_irq();
}

uint32_t HAL_RNG32(void)
{
#if (__CORTEX_M >= 0x03U)   // core_cm3, core_cm4
    CRC->DR = DWT->CYCCNT;
#else                       // core_cm0
    CRC->DR = SysTick->VAL;
#endif
    return CRC->DR;
}

uint16_t HAL_RNG(void)
{
#if (__CORTEX_M >= 0x03U)   // core_cm3, core_cm4
    CRC->DR = DWT->CYCCNT;
#else                       // core_cm0
    CRC->DR = SysTick->VAL;
#endif
    return (CRC->DR & 0x0000FFFF);
}

uint32_t HAL_get_ms(void)
{
    return hal_ms_counter;
}

uint32_t HAL_get_sec(void)
{
    return hal_sec_counter;
}

void _delay_us(uint16_t us)
{
#if (__CORTEX_M >= 0x03U)   // core_cm3, core_cm4
    uint32_t timestamp = DWT->CYCCNT;
    uint32_t ticks = (const uint32_t)(hal_hclk / 1000000UL);
    ticks *= us;

    while(ticks > (DWT->CYCCNT - timestamp));
#else                       // core_cm0
    uint32_t ticks = (const uint32_t)(hal_hclk / 12000000UL);
    ticks *= us;

    while(ticks > 0)
    {
        ticks--;
    }
#endif  //  core_cm?
}

// IRQ handlers.

// 1ms Ticks
void SysTick_Handler(void)
{
//    PreviousMask = __get_PRIMASK();
//    __disable_irq();

    hal_ms_counter++;

    static uint16_t ms_counter  = 0;
    ms_counter++;
    if(ms_counter >= 999)
    {
        ms_counter = 0;
        hal_sec_counter++;
    }

    static uint16_t ticks_counter = 0;

    if(ticks_counter < (const uint16_t)(1000/POLL_TMR_FREQ))
    {
        ticks_counter++;
    }
    else
    {
        SystemTick();
        ticks_counter = 1;
    }

//    __set_PRIMASK(PreviousMask);
}
