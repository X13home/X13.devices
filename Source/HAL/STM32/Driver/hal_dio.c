#include "config.h"

// HAL GPIO Functions
#if ((defined STM32F0) || (defined STM32F3))
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

#elif (defined STM32F1)
void hal_gpio_cfg(GPIO_TypeDef * GPIOx, uint16_t Mask, uint16_t Mode)
{
    uint16_t pinpos;
    uint32_t pos;

    for(pinpos = 0; pinpos < 0x10; pinpos++)
    {
        pos = ((uint32_t)0x01) << pinpos;
        if(Mask & pos)
        {
            uint32_t gpio_cr;
        
            switch(Mode & 0x00FF)
            {
                case DIO_MODE_IN_PU:
                    gpio_cr = GPIO_CRL_CNF0_1;  // Input with pull-up / pull-down
                    GPIOx->BSRR = pos;
                    break;
                case DIO_MODE_IN_PD:
                    gpio_cr = GPIO_CRL_CNF0_1;  // Input with pull-up / pull-down
                    GPIOx->BRR = pos;
                    break;
                case DIO_MODE_OUT_PP:
                    gpio_cr = GPIO_CRL_MODE0_1; // General purpose output push-pull, Low Speed (2MHz)
                    break;
                case DIO_MODE_AF_PP:
                    gpio_cr = GPIO_CRL_CNF0_1 | GPIO_CRL_MODE0_1;   // AF Push-Pull out, Low Speed
                    break;
                case DIO_MODE_AF_OD:
                    gpio_cr = GPIO_CRL_CNF0 | GPIO_CRL_MODE0_1;     // AF Open-Drain out, Low Speed
                    break;
                case DIO_MODE_AIN:
                    gpio_cr = 0;
                    break;
                case DIO_MODE_AF_PP_HS:
                    gpio_cr = GPIO_CRL_CNF0_1 | GPIO_CRL_MODE0;     // AF Push-Pull out, HS
                    break;
                case DIO_MODE_OUT_PP_HS:
                    gpio_cr = GPIO_CRL_MODE0;                       // General purpose output push-pull, High Speed(50MHz)
                    break;
                    
                default:                                            // Floating digital input
                    gpio_cr = GPIO_CRL_CNF0_0;
                    break;
            }

            pos = (pinpos & 0x07) << 2;
            gpio_cr <<= pos;

            if(pinpos < 0x08)
            {
                GPIOx->CRL &= ~(((uint32_t)0x0F) << pos);
                GPIOx->CRL |= gpio_cr;
            }
            else
            {
                GPIOx->CRH &= ~(((uint32_t)0x0F) << pos);
                GPIOx->CRH |= gpio_cr;
            }
        }
    }
}

#else
#error hal_dio.c Unknown uC Family
#endif
// End HAL GPIO Functions

// HAL_DIO Shared functions
static const GPIO_TypeDef * dio_pGPIOx[] = 
{
    GPIOA,
    GPIOB,
    GPIOC,
    GPIOD,
#ifdef GPIOE
    GPIOE,
#else
    NULL,
#endif  //  GPIOE
#ifdef GPIOF
    GPIOF,
#else
    NULL,
#endif  //  GPIOF
    NULL
};

static GPIO_TypeDef * hal_dio_PortNr2GPIOx(uint8_t PortNr)
{
    if(PortNr < (const uint8_t)(sizeof(dio_pGPIOx) / sizeof(dio_pGPIOx[0])))
        return (GPIO_TypeDef *)dio_pGPIOx[PortNr];

    return NULL;
}

void hal_dio_configure(uint8_t Pin, uint16_t Mode)
{
    GPIO_TypeDef * pGPIO = hal_dio_PortNr2GPIOx(Pin >> 4);
    if(pGPIO == NULL)
        return;

    uint16_t Mask = 1UL<<(Pin & 0x0F);
    hal_gpio_cfg(pGPIO, Mask, Mode);
}
// End HAL_DIO Shared functions

#ifdef EXTDIO_USED
static const __attribute__ ((aligned (4))) uint8_t hal_dio_sBase2Base[] = HAL_DIO_MAPPING;

uint8_t hal_dio_base2pin(uint16_t base)
{
    if(base >= sizeof(hal_dio_sBase2Base))
        return 0xFF;
    
    return hal_dio_sBase2Base[base];
}

uint16_t hal_dio_read(uint8_t PortNr)
{
    GPIO_TypeDef * GPIOx = hal_dio_PortNr2GPIOx(PortNr);
    if(GPIOx == NULL)
        return 0;

    return ((uint16_t)GPIOx->IDR);
}

void hal_dio_set(uint8_t PortNr, uint16_t Mask)
{
    GPIO_TypeDef * GPIOx = hal_dio_PortNr2GPIOx(PortNr);
    if(GPIOx != NULL)
    {
        GPIOx->BSRR = Mask;
    }
}

void hal_dio_reset(uint8_t PortNr, uint16_t Mask)
{
    GPIO_TypeDef * GPIOx = hal_dio_PortNr2GPIOx(PortNr);
    if(GPIOx != NULL)
    {
        GPIOx->BRR = Mask;
    }
}
#endif  //  EXTDIO_USED

#if ((defined HAL_USE_EXTI) && (defined STM32F0))
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
        EXTI->FTSR |= u32_tmp;
    else
        EXTI->FTSR &= ~u32_tmp;

    if(Trigger & 2)                     // Rising trigger
        EXTI->RTSR |= u32_tmp;
    else
        EXTI->RTSR &= ~u32_tmp;
    
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
#endif  //  (defined HAL_USE_EXTI) && (defined STM32F0)
