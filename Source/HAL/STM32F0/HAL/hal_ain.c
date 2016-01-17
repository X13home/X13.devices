#include "config.h"

#ifdef EXTAIN_USED

static const __attribute__ ((aligned (4))) uint8_t hal_ainBase2Apin[] = EXTAIN_BASE_2_APIN;

uint8_t hal_ain_apin2dio(uint8_t apin)
{
    if(apin >= sizeof(hal_ainBase2Apin))
        return 0xFF;
    
    uint8_t base = hal_ainBase2Apin[apin];

    if(base == 0xFF)        // pin not exist
        return 0xFF;
    else if(base > 15)      // DIO not used
        return 0xFE;
    else if(base < 8)       // PA0 - PA7
        return base;
    else if(base < 10)      // PB0, PB1
        return (base + 8);

    return (base + 22);     // PC0 - PC5
}

void hal_ain_configure(uint8_t apin, uint8_t aref)
{
    uint8_t base = hal_ainBase2Apin[apin];
    GPIO_TypeDef * GPIOx;
    uint16_t Mask;

    if(base < 8)        // PA0 - PA7
    {
        GPIOx = GPIOA;
        Mask = 1 << base;
    }
    else if(base < 10)  // PB0, PB1
    {
        GPIOx = GPIOB;
        Mask = 1 << (base - 8);
    }
    else if(base < 16)  // PC0 - PC5
    {
        GPIOx = GPIOC;
        Mask = 1 << (base - 10);
    }
    else
        Mask = 0;

    if(aref == 0xFF)
    {
        if(Mask != 0)
            hal_gpio_cfg(GPIOx, Mask, DIO_MODE_IN_FLOAT);
        return;
    }

    if(Mask != 0)
        hal_gpio_cfg(GPIOx, Mask, DIO_MODE_AIN);

    if((RCC->APB2ENR & RCC_APB2ENR_ADC1EN) == 0)
    {
        RCC->APB2ENR |= RCC_APB2ENR_ADC1EN; // Enable ADC Clock

        ADC1->CR |= ADC_CR_ADCAL;           // ADC calibration
        while(ADC1->CR & ADC_CR_ADCAL);
        
        ADC1->CFGR1 =                       // Analog Watchdog disabled
                                            // Regular Channels
                        ADC_CFGR1_AUTOFF |  // auto power off
                                            // Single conversion
                        ADC_CFGR1_OVRMOD |  // Overrun mode
                                            // Hardware trigger disabled
                        ADC_CFGR1_ALIGN;    // Left Aligned 12 bit
                                            // DMA Disabled

        ADC1->SMPR = ADC_SMPR1_SMPR;        // Sampling 239,5 ADC Clock cycles

        ADC1->CR |= ADC_CR_ADEN;            // ADC enabled
    }
}

// ignore some GCC warnings
#if defined ( __GNUC__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

void hal_ain_select(uint8_t apin, uint8_t unused)
{
    uint32_t mux;
    mux = hal_ainBase2Apin[apin];
    mux = (1 << mux);

    ADC1->CHSELR = mux;
}

#if defined ( __GNUC__ )
#pragma GCC diagnostic pop
#endif

int16_t hal_ain_get(void)
{
    int16_t retval;
    retval = (ADC1->DR) >> 1;
    // Start Conversion
    ADC1->CR |= ADC_CR_ADSTART;
    return retval;
}
#endif  //EXTAIN_USED
