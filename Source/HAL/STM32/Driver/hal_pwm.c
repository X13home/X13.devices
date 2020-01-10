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

#if ((defined EXTDIO_USED) && (defined EXTPWM_USED))

// Mapping physical DIO channel to configuration
// bits 11-8 - AF Number, bits 7-3 Timer, bits 2-0 Channel
static const uint16_t       hal_pwm_port2cfg[]  = HAL_PWM_PORT2CFG;
static const TIM_TypeDef *  hal_pwm_timers[]    = HAL_PWM_TIMERS;

bool hal_pwm_checkbase(uint16_t base)
{
#ifdef HAL_PWM_BASE_OFFSET
    if(base < HAL_PWM_BASE_OFFSET)
    {
        return false;
    }
    base -= HAL_PWM_BASE_OFFSET;
#endif
    if((base >= sizeof(hal_pwm_port2cfg)) ||
       (hal_pwm_port2cfg[base] == 0xFF))
    {
        return false;
    }

    return true;
}

bool hal_pwm_busy(uint16_t base)
{
#ifdef HAL_PWM_BASE_OFFSET
    base -= HAL_PWM_BASE_OFFSET;
#endif
    uint16_t cfg = hal_pwm_port2cfg[base];

    TIM_TypeDef * TIMx = (TIM_TypeDef *)hal_pwm_timers[(cfg >> 3) & 0x001F];
    if(TIMx == NULL)
    {
        return true;
    }

    return ((TIMx->CCER & ((TIM_CCER_CC1E | TIM_CCER_CC1P) << ((cfg & 0x03) * 4))) != 0);
}

void hal_pwm_configure(uint16_t base, bool inv)
{
    uint8_t pin = hal_dio_base2pin(base);

#ifdef HAL_PWM_BASE_OFFSET
    base -= HAL_PWM_BASE_OFFSET;
#endif
    uint16_t cfg = hal_pwm_port2cfg[base];
    hal_dio_configure(pin, DIO_MODE_AF_PP | (cfg & 0x0F00));

    TIM_TypeDef * TIMx;


    switch((cfg >> 3) & 0x001F)
    {
#ifdef TIM1
        case 1:     // Timer 1
            RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
            TIMx = TIM1;
            break;
#endif  //  TIM1
#ifdef TIM2
        case 2:     // Timer 2
            RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
            TIMx = TIM2;
            break;
#endif  //  TIM2
#ifdef TIM3
        case 3:     // Timer 3
            RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
            TIMx = TIM3;
            break;
#endif  //  TIM3
#ifdef TIM4
        case 4:     // Timer 4
            RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
            TIMx = TIM4;
            break;
#endif  // TIM4
#ifdef TIM8
        case 8:     // Timer 8
            RCC->APB2ENR |= RCC_APB2ENR_TIM8EN;
            TIMx = TIM8;
            break;
#endif  // TIM8
#ifdef TIM9
        case 9:     // Timer 9
            RCC->APB2ENR |= RCC_APB2ENR_TIM9EN;
            TIMx = TIM9;
            break;
#endif  // TIM9
#ifdef TIM10
        case 10:    // Timer 10
            RCC->APB2ENR |= RCC_APB2ENR_TIM10EN;
            TIMx = TIM10;
            break;
#endif  // TIM10
#ifdef TIM11
        case 11:    // Timer 11
            RCC->APB2ENR |= RCC_APB2ENR_TIM11EN;
            TIMx = TIM11;
            break;
#endif  // TIM11
#ifdef TIM14
        case 14:
            RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;
            TIMx = TIM14;
            break;
#endif  //  TIM14
#ifdef TIM15
        case 15:
            RCC->APB2ENR |= RCC_APB2ENR_TIM15EN;
            TIMx = TIM15;
            break;
#endif  //  TIM15
#ifdef  TIM16
        case 16:
            RCC->APB2ENR |= RCC_APB2ENR_TIM16EN;
            TIMx = TIM16;
            break;
#endif  //  TIM16
#ifdef  TIM17
        case 17:
            RCC->APB2ENR |= RCC_APB2ENR_TIM17EN;
            TIMx = TIM17;
            break;
#endif  //  TIM17
#ifdef  TIM21
        case 21:
            RCC->APB2ENR |= RCC_APB2ENR_TIM21EN;
            TIMx = TIM21;
            break;
#endif  //  TIM21
#ifdef  TIM22
        case 22:
            RCC->APB2ENR |= RCC_APB2ENR_TIM22EN;
            TIMx = TIM22;
            break;
#endif  //  TIM22
        default:
            return;
    }

    uint32_t tim_clk = hal_GetClock((uint32_t)TIMx);
    tim_clk /= 6553500;                         // Fpwm >= 100 Hz
    if(tim_clk > 0)
    {
        tim_clk--;
    }

    TIMx->CR1 = TIM_CR1_ARPE;                   // Disable Timer
    TIMx->PSC = tim_clk;                        // Set Prescaler N + 1
    TIMx->ARR = 0xFFFE;                         // Set Autoreload Register (up value)

    switch(cfg & 0x03)
    {
        case 0:     // Channel 1
            TIMx->CCR1 = 0;
            TIMx->CCMR1 &= 0xFF00;
            // Select PWM mode 1 on OC1  (OC1M = 110), enable preload register on OC1 (OC1PE = 1)
            TIMx->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE;
            TIMx->CCER |= TIM_CCER_CC1E;        // Enable the output on OC1 (CC1E = 1)
            if(inv)
            {
                TIMx->CCER |= TIM_CCER_CC1P;    // Select active low polarity on OC1
            }
            else
            {
                TIMx->CCER &= ~TIM_CCER_CC1P;   // Select active high polarity on OC1
            }
            break;

        case 1:     // Channel 2
            TIMx->CCR2 = 0;
            TIMx->CCMR1 &= 0x00FF;
            // Select PWM mode 1 on OC2  (OC2M = 110), enable preload register on OC2 (OC2PE = 1)
            TIMx->CCMR1 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2PE;
            TIMx->CCER |= TIM_CCER_CC2E;        // Enable the output on OC1 (CC2E = 1)
            if(inv)
            {
                TIMx->CCER |= TIM_CCER_CC2P;    // Select active low polarity on OC2
            }
            else
            {
                TIMx->CCER &= ~TIM_CCER_CC2P;   // Select active high polarity on OC2
            }
            break;

        case 2:     // Channel 3
            TIMx->CCR3 = 0;
            TIMx->CCMR2 &= 0xFF00;
            // Select PWM mode 1 on OC3  (OC3M = 110), enable preload register on OC3 (OC3PE = 1)
            TIMx->CCMR2 |= TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3PE;
            TIMx->CCER |= TIM_CCER_CC3E;        // Enable the output on OC3 (CC3E = 1)
            if(inv)
            {
                TIMx->CCER |= TIM_CCER_CC3P;    // Select active low polarity on OC3
            }
            else
            {
                TIMx->CCER &= ~TIM_CCER_CC3P;   // Select active high polarity on OC3
            }
            break;

        case 3:     // Channel 4
            TIMx->CCR4 = 0;
            TIMx->CCMR2 &= 0x00FF;
            // Select PWM mode 1 on OC4  (OC4M = 110), enable preload register on OC4 (OC4PE = 1)
            TIMx->CCMR2 |= TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4PE;
            TIMx->CCER |= TIM_CCER_CC4E;        // Enable the output on OC4 (CC4E = 1)
            if(inv)
            {
                TIMx->CCER |= TIM_CCER_CC4P;    // Select active low polarity on OC4
            }
            else
            {
                TIMx->CCER &= ~TIM_CCER_CC4P;   // Select active high polarity on OC4
            }
            break;
    }
#ifdef TIM_BDTR_MOE
    TIMx->BDTR |= TIM_BDTR_MOE;                 // Enable output (MOE = 1)
#endif  //  TIM_BDTR_MOE
    TIMx->CR1 |= TIM_CR1_CEN;                   // Enable Timer
    TIMx->EGR |= TIM_EGR_UG;                    // Force update generation
}

void hal_pwm_delete(uint16_t base)
{
    hal_dio_configure(hal_dio_base2pin(base), DIO_MODE_IN_FLOAT);
#ifdef HAL_PWM_BASE_OFFSET
    base -= HAL_PWM_BASE_OFFSET;
#endif
    uint16_t cfg = hal_pwm_port2cfg[base];

    TIM_TypeDef * TIMx = (TIM_TypeDef *)hal_pwm_timers[(cfg >> 3) & 0x001F];
    if(TIMx == NULL)
    {
        return;
    }

    TIMx->CCER &= ~((TIM_CCER_CC1E | TIM_CCER_CC1P) << ((cfg & 0x03) * 4));

    if((TIMx->CCER & (TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E)) == 0)
    {
#ifdef TIM_BDTR_MOE
        TIMx->BDTR &= ~TIM_BDTR_MOE;        // Disable master output
#endif  //  TIM_BDTR_MOE
        TIMx->CR1  &= ~TIM_CR1_CEN;         // Stop Timer

        switch((cfg >> 3) & 0x001F)
        {
#ifdef TIM1
            case 1:     // Timer 1
                RCC->APB2ENR &= ~RCC_APB2ENR_TIM1EN;
                break;
#endif  //  TIM1
#ifdef TIM2
            case 2:     // Timer 2
                RCC->APB1ENR &= ~RCC_APB1ENR_TIM2EN;
                break;
#endif  //  TIM2
#ifdef TIM3
            case 3:     // Timer 3
                RCC->APB1ENR &= ~RCC_APB1ENR_TIM3EN;
                break;
#endif  //  TIM3
#ifdef TIM4
            case 4:
                RCC->APB1ENR &= ~RCC_APB1ENR_TIM4EN;
                break;
#endif  //  TIM4
#ifdef TIM8
            case 8:
                RCC->APB2ENR &= ~RCC_APB2ENR_TIM8EN;
                break;
#endif  // TIM8
#ifdef TIM9
            case 9:
                RCC->APB2ENR &= ~RCC_APB2ENR_TIM9EN;
                break;
#endif  // TIM9
#ifdef TIM10
            case 10:
                RCC->APB2ENR &= ~RCC_APB2ENR_TIM10EN;
                break;
#endif  // TIM10
#ifdef TIM11
            case 11:
                RCC->APB2ENR &= ~RCC_APB2ENR_TIM11EN;
                break;
#endif  // TIM11
#ifdef TIM14
            case 14:
                RCC->APB1ENR &= ~RCC_APB1ENR_TIM14EN;
                break;
#endif  //  TIM14
#ifdef TIM15
            case 15:
                RCC->APB2ENR &= ~RCC_APB2ENR_TIM15EN;
                break;
#endif  //  TIM15
#ifdef TIM16
            case 16:
                RCC->APB2ENR &= ~RCC_APB2ENR_TIM16EN;
                break;
#endif  //  TIM16
#ifdef TIM17
            case 17:
                RCC->APB2ENR &= ~RCC_APB2ENR_TIM17EN;
                break;
#endif  //  TIM17
#ifdef  TIM21
            case 21:
                RCC->APB2ENR &= ~RCC_APB2ENR_TIM21EN;
                break;
#endif  //  TIM21
#ifdef  TIM22
            case 22:
                RCC->APB2ENR &= ~RCC_APB2ENR_TIM22EN;
                break;
#endif  //  TIM22
        }
    }
}

void hal_pwm_write(uint16_t base, uint16_t value)
{
#ifdef HAL_PWM_BASE_OFFSET
    base -= HAL_PWM_BASE_OFFSET;
#endif
    if(base > (const uint16_t)(sizeof(hal_pwm_port2cfg)/2))
    {
        return;
    }

    uint16_t cfg = hal_pwm_port2cfg[base];
    if(cfg == 0xFF)
    {
        return;
    }

    TIM_TypeDef * TIMx = (TIM_TypeDef *)hal_pwm_timers[(cfg >> 3) & 0x001F];
    if(TIMx == NULL)
    {
        return;
    }

    uint32_t timbase = (uint32_t)TIMx + 0x34;
    uint32_t offset = cfg & 0x03;
    offset *= 4;
    timbase += offset;
    *(__IO uint32_t *)timbase = value;
}

#endif  //  (defined EXTDIO_USED) && (defined EXTPWM_USED)
