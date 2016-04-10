#include "config.h"

#define HSE_STARTUP_TIME            0x00005000UL

static uint32_t CriticalNesting = 0;
static volatile uint32_t hal_ms_counter = 0;
static volatile uint32_t hal_sec_counter = 0;    // Max Uptime 136 Jr.

static void SetSysClock(void)
{
    uint32_t StartUpCounter = 0;

    // Enable HSE
    RCC->CR |= RCC_CR_HSEON;

#if (defined HSE_CRYSTAL_BYPASS)
    // HSE crystal oscillator bypassed with external clock
    RCC->CR |= RCC_CR_HSEBYP;
#endif  //  HSE_CRYSTAL_BYPASS

    // Wait till HSE is ready and if Time out is reached exit
    while(((RCC->CR & RCC_CR_HSERDY) == 0) && (StartUpCounter < HSE_STARTUP_TIME))
        StartUpCounter++;

#if (defined STM32F0)
    // Enable Prefetch Buffer and set Flash Latency
    FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY;

    RCC->CFGR &= ~(RCC_CFGR_PLLMUL |                // Reset PLL Multiplication factor
                   RCC_CFGR_HPRE |                  // HCLK not divided
                   RCC_CFGR_PPRE);                  // SYSCLK not divided

    if(StartUpCounter < HSE_STARTUP_TIME)
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
#elif (defined STM32F1)
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
#elif (defined STM32F3)
    // Enable Prefetch Buffer and set Flash Latency
    // Enable Prefetch Buffer
    FLASH->ACR |= FLASH_ACR_PRFTBE;

    // Flash 2 wait state
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
#else
#error SetSysClock Unknown uC Family
#endif

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

void HAL_Init(void)
{
    // Set Core Clock
    SetSysClock();
    // Enable GPIO clock
#if ((defined STM32F0) || (defined STM32F3))
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIODEN | RCC_AHBENR_GPIOFEN;
#elif (defined STM32F1)
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN | RCC_APB2ENR_IOPDEN | RCC_APB2ENR_IOPEEN;
     // disable JTAG
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;
#else
#error HAL_Init Unknown uC Family
#endif

    // Enable CRC, Used for RNG
    RCC->AHBENR |= RCC_AHBENR_CRCEN;
    CRC->CR = CRC_CR_RESET;

    CriticalNesting = 0;
}

void HAL_StartSystemTick(void)
{
    if(SysTick_Config((SystemCoreClock / 1000) - 1UL))
        while(1);

    // Enable Global interrupts
    __enable_irq();
}

uint32_t HAL_RNG32(void)
{
    CRC->DR = SysTick->VAL;
    return CRC->DR;
}
uint16_t HAL_RNG(void)
{
    CRC->DR = SysTick->VAL;
    return (CRC->DR & 0x0000FFFF);
}

/*
// Generate pseudo random uint16
uint16_t HAL_RNG(void)
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
*/


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

#ifdef ASLEEP
void HAL_ASleep(uint16_t duration)
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


void HardFault_Handler(void) __attribute__((naked));
#ifdef STM32F0
void HardFault_Handler(void)
{
    asm volatile(
        " movs r0,#4    \n"
        " mov r1,lr     \n"
        " tst r0,r1     \n"
        " beq 1f        \n"
        " mrs r0,psp    \n"
        " b   2f        \n"
        "1:             \n"
        " mrs r0,msp    \n"
        "2:"
        " mov r1,lr     \n"

        : /* Outputs */
        : /* Inputs */
        : /* Clobbers */
    );
#if defined(DEBUG)
    __DEBUG_BKPT();
#endif
    while(1);
}
#else
void HardFault_Handler(void)
{
    __disable_irq();
    while(1);
}
#endif  //STM32F0
