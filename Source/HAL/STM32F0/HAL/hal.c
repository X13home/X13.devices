#include "config.h"

static uint32_t CriticalNesting = 0;
static volatile uint16_t hal_ms_counter = 0;
static volatile uint32_t hal_sec_counter = 0;    // Max Uptime 136 Jr.

static void SetSysClock(void)
{
    uint32_t StartUpCounter = 0;

    // Enable Prefetch Buffer and set Flash Latency
    FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY;

    // Enable HSE
    RCC->CR |= ((uint32_t)RCC_CR_HSEON);

    // Wait till HSE is ready and if Time out is reached exit
    while(((RCC->CR & RCC_CR_HSERDY) == 0) && (StartUpCounter < 0x00005000))
        StartUpCounter++;

    RCC->CFGR &= ~(RCC_CFGR_PLLMUL |                // Reset PLL Multiplication factor
                   RCC_CFGR_HPRE |                  // HCLK not divided
                   RCC_CFGR_PPRE);                  // SYSCLK not divided

    if(StartUpCounter < 0x00005000) 
    {
        RCC->CFGR |= (RCC_CFGR_PLLMUL6 |            // PLL multiplication factor = 6
                      RCC_CFGR_PLLSRC_HSE_PREDIV);  // HSE/PREDIV clock selected as PLL entry clock source
    }
    else
    {
        RCC->CR &= ~RCC_CR_HSEON;                   // Disble HSE
        RCC->CFGR &= ~RCC_CFGR_PLLSRC_HSE_PREDIV;   // HSI clock divided by 2 selected as PLL entry clock source
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

    // Update SystemCoreClock variable according to Clock Register Values
    SystemCoreClockUpdate();
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

void INIT_SYSTEM(void)
{
    // Set Core Clock
    SetSysClock();
    // Enable GPIO clock
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN;

    CriticalNesting = 0;
}

void StartSheduler(void)
{
    if(SysTick_Config((SystemCoreClock / 1000) - 1UL))
        while(1);

    // Enable Global interrupts
    __enable_irq();
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
uint16_t hal_RNG(void)
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
