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

#if (defined EXTAIN_USED) && ((defined STM32F0) || (defined STM32L0))

static const __attribute__ ((aligned (4))) uint8_t hal_ainBase2Apin[] = HAL_AIN_BASE2APIN;
static const __attribute__ ((aligned (4))) uint8_t hal_ain2dpin[] = {0,1,2,3,4,5,6,7,   // PA0 - PA7
                                                                     16,17,             // PB0 - PB1
                                                                     32,33,34,35,36,37, // PC0 - PC5
                                                                     0xFE};             // RefInt
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

#ifdef STM32L0
        ADC->CCR = 
                        ADC_CCR_LFMEN |     // Low Frequency Mode enable
                        ADC_CCR_PRESC_2 |   // ADC Prescaler = 8
                        ADC_CCR_VREFEN;     // Vrefint Enable
#endif  //  STM32L0

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

        ADC1->SMPR = ADC_SMPR_SMP;          // Sampling 239,5 ADC Clock cycles

        ADC1->CR |= ADC_CR_ADEN;            // ADC enabled
    }
}

void hal_ain_select(uint8_t apin, uint8_t aref __attribute__ ((unused)))
{
    ADC1->CHSELR = (1UL << apin);
}

int16_t hal_ain_get(void)
{
    int16_t retval = (int16_t)(ADC1->DR);
    retval >>= 1;
    // Start Conversion
    ADC1->CR |= ADC_CR_ADSTART;
    return retval;
}
#endif  //EXTAIN_USED
