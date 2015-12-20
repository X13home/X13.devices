#include "../../config.h"

extern uint32_t SystemCoreClock;

static uint32_t CriticalNesting = 0;

void INIT_SYSTEM(void)
{
    SystemInit();

    // Enable GPIO clock
#if   (defined __STM32F0XX_H)
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN;
#elif (defined __STM32F10x_H)
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN;
     // disable JTAG
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;
    // TIM4 used in delay_us
    //RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;    
#elif (defined STM32F4XX)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIODEN;
#else
    #error unknown uC Family
#endif  //  STM32F0XX_MD

    CriticalNesting = 0;
}

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
        while(1);               // Error

    CriticalNesting--;
    if(CriticalNesting == 0 )
        __enable_irq();
}

void StartSheduler(void)
{
    if(SysTick_Config((SystemCoreClock / 1000) - 1UL))
        while(1);

    // Enable Global interrupts
    __enable_irq();
}

// IRQ handlers.

static volatile uint16_t hal_ms_counter = 0;
static volatile uint32_t hal_sec_counter = 0;    // Max Uptime 136 Jr.

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
        ticks_counter++;
    else
    {
        SystemTick();
        ticks_counter = 1;
    }
    
//    __set_PRIMASK(PreviousMask);
}

void HardFault_Handler( void ) __attribute__( ( naked ) );
void HardFault_Handler(void)
{
    __disable_irq();
    while(1);
}

uint16_t hal_get_ms(void)
{
    return hal_ms_counter;
}

uint32_t hal_get_sec(void)
{
    return hal_sec_counter;
}

void _delay_ms(uint16_t ms)
{
    uint32_t new_ms;
    
    if(SysTick->CTRL & SysTick_CTRL_ENABLE_Msk)
    {
        new_ms = hal_ms_counter + ms;
        while(hal_ms_counter != new_ms);
    }
    else
    {
        new_ms = (const uint32_t)(SystemCoreClock / 12000UL);
        new_ms *= ms;
        while(new_ms > 0)
            new_ms--;
    }
}

void _delay_us(uint16_t us)
{
/*
    TIM4->CR1 = TIM_CR1_OPM;
    
    TIM4->PSC = (const uint16_t)((SystemCoreClock / 1000000) - 1);
    TIM4->CNT = 0;
    TIM4->ARR = us;

    TIM4->EGR = TIM_EGR_UG;
    TIM4->CR1 |= TIM_CR1_CEN;
    
    while((TIM4->CR1 & TIM_CR1_CEN) != 0);
*/
    uint32_t ticks = (const uint32_t)(SystemCoreClock / 12000000UL);
    ticks *= us;

    while(ticks > 0)
        ticks--;
}

// Generate pseudo random uint16
uint16_t halRNG(void)
{
    static uint16_t rand16 = 0xA15E;

    // Galois LFSRs
    if(rand16 & 1)
    {
        rand16 >>= 1;
        rand16 ^= 0xB400;
    }
    else
        rand16 >>= 1;
  
    return rand16;
}

#ifdef ASLEEP
void hal_ASleep(uint16_t duration)
{
    // ToDo ASleep dummy function

    uint32_t sec_cnt = hal_get_sec() + duration;
    while(sec_cnt != hal_get_sec());
}
#endif  // ASLEEP

// Reboot system on exception
void hal_reboot(void)
{
    NVIC_SystemReset();
}
