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

#if ((defined EXTDIO_USED) && (defined EXTCNT_USED))

// Mapping physical DIO channel to configuration
// bits 11-8 - AF Number, bits 7-3 Timer, bits 2-0 Channel
static const uint16_t       hal_cnt_port2cfg[]  = HAL_CNT_PORT2CFG;
static const TIM_TypeDef *  hal_cnt_timers[]    = HAL_CNT_TIMERS;

bool hal_cnt_checkbase(uint16_t base)
{
#ifdef HAL_CNT_BASE_OFFSET
    if(base < HAL_CNT_BASE_OFFSET)
    {
        return false;
    }
    base -= HAL_CNT_BASE_OFFSET;
#endif
    if((base >= (sizeof(hal_cnt_port2cfg)/2)) ||
       (hal_cnt_port2cfg[base] == 0xFF))
    {
        return false;
    }

    return true;
}

bool hal_cnt_busy(uint16_t base)
{
#ifdef HAL_CNT_BASE_OFFSET
    base -= HAL_CNT_BASE_OFFSET;
#endif
    uint16_t cfg = hal_cnt_port2cfg[base];

    TIM_TypeDef * TIMx = (TIM_TypeDef *)hal_cnt_timers[(cfg >> 3) & 0x001F];
    if(TIMx == NULL)
    {
        return true;
    }

    return ((TIMx->CR1 & TIM_CR1_CEN) != 0);
}

void hal_cnt_configure(uint16_t base, bool inv)
{
    uint8_t pin = hal_dio_base2pin(base);

#ifdef HAL_CNT_BASE_OFFSET
    base -= HAL_CNT_BASE_OFFSET;
#endif
    uint16_t cfg = hal_cnt_port2cfg[base];
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
        default:
            return;
    }

    TIMx->CR1 = (2<<TIM_CR1_CKD_Pos);   // t_dts = 4 * t_ck
    TIMx->SMCR = TIM_SMCR_ECE |         // External clock enable
                                        // Prescaller off
                 TIM_SMCR_ETF |         // External trigger filter - Max
                 0x07;                  // External Clock Mode
    if(inv)
    {
        TIMx->SMCR |= TIM_SMCR_ETP;
    }

    TIMx->CR1 |= TIM_CR1_CEN;
}

void hal_cnt_delete(uint16_t base)
{
    hal_dio_configure(hal_dio_base2pin(base), DIO_MODE_IN_FLOAT);
#ifdef HAL_CNT_BASE_OFFSET
    base -= HAL_CNT_BASE_OFFSET;
#endif
    uint16_t cfg = hal_cnt_port2cfg[base];

    TIM_TypeDef * TIMx = (TIM_TypeDef *)hal_cnt_timers[(cfg >> 3) & 0x001F];
    if(TIMx == NULL)
    {
        return;
    }

    TIMx->CR1 = 0;
    TIMx->SMCR = 0;

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
    }
}

uint16_t hal_cnt_read(uint16_t base)
{
#ifdef HAL_CNT_BASE_OFFSET
    base -= HAL_CNT_BASE_OFFSET;
#endif
    if(base > (const uint16_t)(sizeof(hal_cnt_port2cfg)/2))
    {
        return 0;
    }

    uint16_t cfg = hal_cnt_port2cfg[base];
    if(cfg == 0xFF)
    {
        return 0;
    }

    TIM_TypeDef * TIMx = (TIM_TypeDef *)hal_cnt_timers[(cfg >> 3) & 0x001F];
    if(TIMx == NULL)
    {
        return 0;
    }

    return TIMx->CNT;
}

/*
void hal_cnt_write(uint16_t base, uint16_t value)
{
#ifdef HAL_CNT_BASE_OFFSET
    base -= HAL_CNT_BASE_OFFSET;
#endif
    if(base > (const uint16_t)(sizeof(hal_cnt_port2cfg)/2))
    {
        return;
    }

    uint16_t cfg = hal_cnt_port2cfg[base];
    if(cfg == 0xFF)
    {
        return;
    }

    TIM_TypeDef * TIMx = (TIM_TypeDef *)hal_cnt_timers[(cfg >> 3) & 0x001F];
    if(TIMx == NULL)
    {
        return;
    }
	
	TIMx->CNT = value;
}
*/

#endif //  EXTDIO_USED && EXTCNT_USED
