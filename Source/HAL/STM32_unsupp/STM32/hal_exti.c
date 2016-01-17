#include "../../config.h"

#ifdef HAL_USE_EXTI

typedef void (*cbEXTI_t)(void);
    
static void * hal_exti_cb[16] = {NULL,};

void hal_exti_config(GPIO_TypeDef *GPIOx, uint16_t Mask, uint8_t Trigger, void * pCallback)
{

    // Enable SYSCFG clock
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;


    uint32_t port;
    uint8_t  pin;

    switch((uint32_t)GPIOx)
    {
#ifdef GPIOA
        case (uint32_t)GPIOA:
            port = 0;
            break;
#endif  //  GPIOA
#ifdef GPIOB
        case (uint32_t)GPIOB:
            port = 1;
            break;
#endif  //  GPIOB
#ifdef GPIOC
        case (uint32_t)GPIOC:
            port = 2;
            break;
#endif  //  GPIOC
#ifdef GPIOD
        case (uint32_t)GPIOD:
            port = 3;
            break;
#endif  //  GPIOD
#ifdef GPIOE
        case (uint32_t)GPIOE:
            port = 4;
            break;
#endif  //  GPIOE
#ifdef GPIOF
        case (uint32_t)GPIOF:
            port = 5;
            break;
#endif  //  GPIOF
#ifdef GPIOG
        case (uint32_t)GPIOG:
            port = 6;
            break;
#endif  //  GPIOG
        default:
            assert(0);      // Unknown Port
    }
    
    uint32_t u32_tmp;
    for(pin = 0; pin < 16; pin++)
    {
        if(Mask & (1 << pin))
        {
            // Connect EXTIx Line to IRQ PIN
            u32_tmp = ((uint32_t)0x0F) << (0x04 * (pin & (uint8_t)0x03));
            SYSCFG->EXTICR[pin >> 0x02] &= ~u32_tmp;
            SYSCFG->EXTICR[pin >> 0x02] |= port << (0x04 * (pin & (uint8_t)0x03));
            
            hal_exti_cb[pin] = pCallback;
        }
    }

    u32_tmp = Mask;
    EXTI->IMR |= u32_tmp;               // Enable Interrupt
    EXTI->EMR &= ~u32_tmp;              // Disable Events
    EXTI->PR = Mask;                    // Reset pending status

    if(Trigger & 1)                     // Falling trigger
        EXTI->FTSR |= u32_tmp;
    else
        EXTI->FTSR &= ~u32_tmp;
    
    if(Trigger & 2)                     // Rising trigger
        EXTI->RTSR |= u32_tmp;
    else
        EXTI->RTSR &= ~u32_tmp;
}

void exti_irq_handler(uint8_t pos, uint8_t max)
{
    uint32_t mask;
    
    for(; pos <= max; pos++)
    {
        mask = 1 << pos;
        
        if((EXTI->PR & mask) != 0)
        {
            if(hal_exti_cb[pos] != NULL)
            {
                cbEXTI_t cb = hal_exti_cb[pos];
                cb();
            }

            EXTI->PR = mask;
        }
    }
}

#if (defined __STM32F0XX_H)

void EXTI0_1_IRQHandler(void)
{
    exti_irq_handler(0,1);
}

void EXTI2_3_IRQHandler(void)
{
    exti_irq_handler(2,3);
}

void EXTI4_15_IRQHandler(void)
{
    exti_irq_handler(4,15);
}
#elif (defined STM32F4XX)

void EXTI0_IRQHandler(void)
{
    exti_irq_handler(0,0);
}

void EXTI1_IRQHandler(void)
{
    exti_irq_handler(1,1);
}

void EXTI2_IRQHandler(void)
{
    exti_irq_handler(2,2);
}

void EXTI3_IRQHandler(void)
{
    exti_irq_handler(3,3);
}

void EXTI4_IRQHandler(void)
{
    exti_irq_handler(4,4);
}

void EXTI9_5_IRQHandler(void)
{
    exti_irq_handler(5,9);
}

void EXTI15_10_IRQHandler(void)
{
    exti_irq_handler(10,15);
}

#else
    #error unknown uC Family
#endif  //  uC Family

#endif  // HAL_USE_EXTI