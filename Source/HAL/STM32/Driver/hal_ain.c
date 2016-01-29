#include "config.h"

#if (defined EXTAIN_USED)

static const __attribute__ ((aligned (4))) uint8_t hal_ainBase2Apin[] = EXTAIN_BASE_2_APIN;
static const __attribute__ ((aligned (4))) uint8_t hal_ain2dpin[] = {0,1,2,3,4,5,6,7,       // PA0 - PA7
                                                                     16,17,                 // PB0 - PB1
                                                                     32,33,34,35,36,37};    // PC0 - PC5
uint8_t hal_ain_apin2dio(uint8_t apin)
{
    if(apin >= sizeof(hal_ainBase2Apin))
        return 0xFF;
    
    uint8_t base = hal_ainBase2Apin[apin];

    if(base == 0xFF)        // pin not exist
        return 0xFF;
    else if(base > 15)      // DIO not used
        return 0xFE;
    return hal_ain2dpin[base];
}

void hal_ain_configure(uint8_t apin, uint8_t aref)
{
    uint8_t base = hal_ainBase2Apin[apin];
    uint8_t dpin = hal_ain_apin2dio(base);

    if(aref == 0xFF)
    {
        if(dpin < 0xFE)
            hal_dio_configure(dpin, DIO_MODE_IN_FLOAT);
        return;
    }

    if(dpin < 0xFE)
        hal_dio_configure(dpin, DIO_MODE_AIN);

    if((RCC->APB2ENR & RCC_APB2ENR_ADC1EN) == 0)
    {
        RCC->APB2ENR |= RCC_APB2ENR_ADC1EN; // Enable ADC Clock
#if (defined STM32F0)
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
#elif (defined STM32F1)
        ADC1->CR1 = 0;
        ADC1->CR2 = ADC_CR2_EXTTRIG |       // Enable conversion on external trigger
                    ADC_CR2_EXTSEL |        // external trigger on SWSTART.
                    ADC_CR2_ALIGN;          // Left Aligned
        // Sampling 239,5 ADC Clock cycles
        ADC1->SMPR1 = ADC_SMPR1_SMP10 | ADC_SMPR1_SMP11 | ADC_SMPR1_SMP12 | 
                      ADC_SMPR1_SMP13 | ADC_SMPR1_SMP14 | ADC_SMPR1_SMP15 |
                      ADC_SMPR1_SMP16 | ADC_SMPR1_SMP17;
        ADC1->SMPR2 = ADC_SMPR2_SMP0 | ADC_SMPR2_SMP1 | ADC_SMPR2_SMP2 |
                      ADC_SMPR2_SMP3 | ADC_SMPR2_SMP4 | ADC_SMPR2_SMP5 |
                      ADC_SMPR2_SMP6 | ADC_SMPR2_SMP7 | ADC_SMPR2_SMP8 |
                      ADC_SMPR2_SMP9;

        ADC1->SQR1 = 0;                     // 1 Conversion

        ADC1->CR2 |= ADC_CR2_ADON;          // ADC enabled

        ADC1->CR2 |= ADC_CR2_CAL;           // ADC calibration
        while (ADC1->CR2 & ADC_CR2_CAL);
#else
    #error hal_ain_configure unknown uC Family
#endif
    }
}

// ignore some GCC warnings
#if defined ( __GNUC__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

void hal_ain_select(uint8_t apin, uint8_t unused)
{
#if (defined STM32F0)
    uint32_t mux;
    mux = hal_ainBase2Apin[apin];
    mux = (1 << mux);

    ADC1->CHSELR = mux;
#elif (defined STM32F1)
    ADC1->SQR3 = hal_ainBase2Apin[apin];
#else
    #error hal_ain_select unknown uC Family
#endif
}

#if defined ( __GNUC__ )
#pragma GCC diagnostic pop
#endif

int16_t hal_ain_get(void)
{
    int16_t retval;
#if (defined STM32F0)
    retval = (ADC1->DR) >> 1;
    // Start Conversion
    ADC1->CR |= ADC_CR_ADSTART;
#elif (defined STM32F1)
    retval = ((ADC1->DR) & 0x0000FFFF) >> 1;
    // Start Conversion
    ADC1->CR2 |= ADC_CR2_SWSTART;
#else
    #error hal_ain_get unknown uC Family
#endif
    return retval;
}
#endif  //EXTAIN_USED
