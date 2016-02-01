#include "config.h"

#if ((defined EXTDIO_USED) && (defined EXTPWM_USED) && (defined STM32F0))

/*
static TIM_TypeDef * hal_pwm_timers[] =
{
    NULL,       // TIM0
#ifdef  TIM1
    TIM1,
#else
    NULL,
#endif  //  TIM1

#ifdef  TIM2
    TIM2,
#else
    NULL,
#endif  //  TIM2

#ifdef  TIM3
    TIM3,
#else
    NULL,
#endif  //  TIM3
}
*/

// Mapping physical DIO channel to configuration
// bits 11-8 - AF Number, bits 7-3 Timer, bits 2-0 Channel
static const uint16_t hal_pwm_port2cfg[] = EXTPWM_PORT2CFG;
static const uint8_t  hal_pwm_port2dio[] = EXTPWM_PORT2DIO;

uint8_t hal_pwm_base2dio(uint16_t base)
{
#ifdef EXTPWM_BASE_OFFSET
    if(base < EXTPWM_BASE_OFFSET)
        return 0xFF;
    base -= EXTPWM_BASE_OFFSET;
#endif

    if(base >= (const uint16_t)(sizeof(hal_pwm_port2dio)))
        return 0xFF;

    return hal_pwm_port2dio[base];
}

void hal_pwm_configure(uint16_t base, bool inv)
{
#ifdef EXTPWM_BASE_OFFSET
    base -= EXTPWM_BASE_OFFSET;
#endif
    uint16_t cfg = hal_pwm_port2cfg[base];
    uint8_t  pin = hal_pwm_port2dio[base];
    hal_dio_configure(pin, DIO_MODE_AF_PP | (cfg & 0x0F00));
    
    TIM_TypeDef * TIMx;

    switch((cfg >> 3) & 0x001F)
    {
        case 1:     // Timer 1
            RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
            TIMx = TIM1;
            break;

        case 2:     // Timer 2
            RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
            TIMx = TIM2;
            break;
            
        case 3:     // Timer 3
            RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
            TIMx = TIM3;
            break;
            
        case 14:
            RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;
            TIMx = TIM14;
            break;
            
        case 15:
            RCC->APB2ENR |= RCC_APB2ENR_TIM15EN;
            TIMx = TIM15;
            break;

        case 16:
            RCC->APB2ENR |= RCC_APB2ENR_TIM16EN;
            TIMx = TIM16;
            break;

        case 17:
            RCC->APB2ENR |= RCC_APB2ENR_TIM17EN;
            TIMx = TIM17;
            break;

        default:
            return;
    }

    TIMx->CR1 = TIM_CR1_ARPE;       // Disable Timer
    TIMx->PSC = 13;                 // Set Prescaler N + 1
    TIMx->ARR = 0xFFFE;             // Set Autoreload Register (up value)
    
    switch(cfg & 0x07)
    {
        case 0:     // Channel 1
            TIMx->CCR1 = 0;
            TIMx->CCMR1 &= 0xFF00;
            TIMx->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE;   // Select PWM mode 1 on OC1  (OC1M = 110), enable preload register on OC1 (OC1PE = 1)
            TIMx->CCER |= TIM_CCER_CC1E;                                            // Enable the output on OC1 (CC1E = 1)
            if(inv)
                TIMx->CCER |= TIM_CCER_CC1P;                                        // Select active low polarity on OC1
            else
                TIMx->CCER &= ~TIM_CCER_CC1P;                                       // Select active high polarity on OC1
            break;
            
        case 1:     // Channel 2
            TIMx->CCR2 = 0;
            TIMx->CCMR1 &= 0x00FF;
            TIMx->CCMR1 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2PE;   // Select PWM mode 1 on OC2  (OC2M = 110), enable preload register on OC2 (OC2PE = 1)
            TIMx->CCER |= TIM_CCER_CC2E;                                            // Enable the output on OC1 (CC2E = 1)
            if(inv)
                TIMx->CCER |= TIM_CCER_CC2P;                                        // Select active low polarity on OC2
            else
                TIMx->CCER &= ~TIM_CCER_CC2P;                                       // Select active high polarity on OC2
            break;
            
        case 2:     // Channel 3
            TIMx->CCR3 = 0;
            TIMx->CCMR2 &= 0xFF00;
            TIMx->CCMR2 |= TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3PE;   // Select PWM mode 1 on OC3  (OC3M = 110), enable preload register on OC3 (OC3PE = 1)
            TIMx->CCER |= TIM_CCER_CC3E;                                            // Enable the output on OC3 (CC3E = 1)
            if(inv)
                TIMx->CCER |= TIM_CCER_CC3P;                                        // Select active low polarity on OC3
            else
                TIMx->CCER &= ~TIM_CCER_CC3P;                                       // Select active high polarity on OC3
            break;
            
        case 3:     // Channel 4
            TIMx->CCR4 = 0;
            TIMx->CCMR2 &= 0x00FF;
            TIMx->CCMR2 |= TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4PE;   // Select PWM mode 1 on OC4  (OC4M = 110), enable preload register on OC4 (OC4PE = 1)
            TIMx->CCER |= TIM_CCER_CC4E;                                            // Enable the output on OC4 (CC4E = 1)
            if(inv)
                TIMx->CCER |= TIM_CCER_CC4P;                                        // Select active low polarity on OC4
            else
                TIMx->CCER &= ~TIM_CCER_CC4P;                                       // Select active high polarity on OC4
            break;

        default:
            return;
    }

    TIMx->BDTR |= TIM_BDTR_MOE;     // Enable output (MOE = 1)
    TIMx->CR1 |= TIM_CR1_CEN;       // Enable Timer
    TIMx->EGR |= TIM_EGR_UG;        // Force update generation
}

void hal_pwm_delete(uint16_t base)
{
#ifdef EXTPWM_BASE_OFFSET
    base -= EXTPWM_BASE_OFFSET;
#endif
    uint16_t cfg = hal_pwm_port2cfg[base];

    uint8_t  pin = hal_pwm_port2dio[base];
    hal_dio_configure(pin, DIO_MODE_IN_FLOAT);

    TIM_TypeDef * TIMx;

    switch((cfg >> 3) & 0x001F)
    {
        case 1:
            TIMx = TIM1;
            break;

        case 2:
            TIMx = TIM2;
            break;

        case 3:
            TIMx = TIM3;
            break;

        case 14:
            TIMx = TIM14;
            break;
            
        case 15:
            TIMx = TIM15;
            break;
            
        case 16:
            TIMx = TIM16;
            break;

        case 17:
            TIMx = TIM17;
            break;

        default:
            return;
    }

    switch(cfg & 0x07)
    {
        case 0:     // Channel 1
            TIMx->CCER &= ~(TIM_CCER_CC1E | TIM_CCER_CC1P);     // Disable Output OC1
            break;
            
        case 1:     // Channel 2
            TIMx->CCER &= ~(TIM_CCER_CC2E | TIM_CCER_CC2P);     // Disable Output OC2
            break;
            
        case 2:     // Channel 3
            TIMx->CCER &= ~(TIM_CCER_CC3E | TIM_CCER_CC3P);     // Disable Output OC3
            break;
            
        case 3:     // Channel 4
            TIMx->CCER &= ~(TIM_CCER_CC4E | TIM_CCER_CC4P);     // Disable Output OC4
            break;

        default:
            return;
    }
    
    if((TIMx->CCER & (TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E)) == 0)
    {
        TIMx->BDTR &= ~TIM_BDTR_MOE;        // Disable master output
        TIMx->CR1  &= ~TIM_CR1_CEN;         // Stop Timer
        
        
        switch((cfg >> 3) & 0x001F)
        {
            case 1:     // Timer 1
                RCC->APB2ENR &= ~RCC_APB2ENR_TIM1EN;
                break;

            case 2:     // Timer 2
                RCC->APB1ENR &= ~RCC_APB1ENR_TIM2EN;
                break;
            
            case 3:     // Timer 3
                RCC->APB1ENR &= ~RCC_APB1ENR_TIM3EN;
                break;
            
            case 14:
                RCC->APB1ENR &= ~RCC_APB1ENR_TIM14EN;
                break;
            
            case 15:
                RCC->APB2ENR &= ~RCC_APB2ENR_TIM15EN;
                break;

            case 16:
                RCC->APB2ENR &= ~RCC_APB2ENR_TIM16EN;
                break;

            case 17:
                RCC->APB2ENR &= ~RCC_APB2ENR_TIM17EN;
                break;
        }
    }
}

void hal_pwm_write(uint16_t base, uint16_t value)
{
#ifdef EXTPWM_BASE_OFFSET
    base -= EXTPWM_BASE_OFFSET;
#endif
    uint16_t cfg = hal_pwm_port2cfg[base];
    TIM_TypeDef * TIMx;

    switch((cfg >> 3) & 0x001F)
    {
        case 1:
            TIMx = TIM1;
            break;

        case 2:
            TIMx = TIM2;
            break;

        case 3:
            TIMx = TIM3;
            break;

        case 14:
            TIMx = TIM14;
            break;
            
        case 15:
            TIMx = TIM15;
            break;
            
        case 16:
            TIMx = TIM16;
            break;

        case 17:
            TIMx = TIM17;
            break;

        default:
            return;
    }
    
    switch(cfg & 0x07)
    {
        case 0:     // Channel 1
            TIMx->CCR1 = value;
            break;
            
        case 1:     // Channel 2
            TIMx->CCR2 = value;
            break;
            
        case 2:     // Channel 3
            TIMx->CCR3 = value;
            break;
            
        case 3:     // Channel 4
            TIMx->CCR4 = value;
            break;

        default:
            break;
    }
}

#endif  //  (defined EXTDIO_USED) && (defined EXTPWM_USED)
