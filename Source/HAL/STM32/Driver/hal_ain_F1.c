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

#if (defined EXTAIN_USED) && (defined STM32F1)

static const __attribute__ ((aligned (4))) uint8_t hal_ainBase2Apin[] = HAL_AIN_BASE2APIN;
static const __attribute__ ((aligned (4))) uint8_t hal_ain2dpin[] = {0,1,2,3,4,5,6,7,   // PA0 - PA7
                                                                     16,17,             // PB0 - PB1
                                                                     32,33,34,35,36,37};// PC0 - PC5
uint8_t hal_ain_base2apin(uint16_t base)
{
    if(base > sizeof(hal_ainBase2Apin))
    {
        return 0xFF;
    }
    return hal_ainBase2Apin[base];
}

uint8_t hal_ain_apin2dio(uint8_t apin)
{
    if(apin >= sizeof(hal_ain2dpin))
    {
        return 0xFF;
    }
    return hal_ain2dpin[apin];
}

void hal_ain_configure(uint8_t apin, uint8_t aref)
{
    uint8_t dpin = hal_ain_apin2dio(apin);

    if(aref == 0xFF)
    {
        if(dpin < 0xFE)
        {
            hal_dio_configure(dpin, DIO_MODE_IN_FLOAT);
        }
        return;
    }

    if(dpin < 0xFE)
    {
        hal_dio_configure(dpin, DIO_MODE_AIN);
    }

    if((RCC->APB2ENR & RCC_APB2ENR_ADC1EN) == 0)
    {
        RCC->APB2ENR |= RCC_APB2ENR_ADC1EN; // Enable ADC Clock

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
    }
}

void hal_ain_select(uint8_t apin, uint8_t aref __attribute__ ((unused)))
{
    ADC1->SQR3 = apin;
}

int16_t hal_ain_get(void)
{
    int16_t retval;
    retval = ((ADC1->DR) & 0x0000FFFF) >> 1;
    // Start Conversion
    ADC1->CR2 |= ADC_CR2_SWSTART;
    return retval;
}
#endif  //EXTAIN_USED
