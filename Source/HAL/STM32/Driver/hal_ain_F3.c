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

#if (defined EXTAIN_USED) && (defined STM32F3)

static const __attribute__ ((aligned (4))) uint8_t hal_ainBase2Apin[] = HAL_AIN_BASE2APIN;
static const __attribute__ ((aligned (4))) uint8_t hal_ain2dpin[]     = HAL_AIN_AIN2DPIN;
static const __attribute__ ((aligned (4))) uint8_t hal_ain2cfg[]      = HAL_AIN_AIN2CFG;

static uint8_t halain_adc = 0;

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

    if((RCC->AHBENR & RCC_AHBENR_ADC12EN) == 0)
    {
        RCC->AHBENR |= RCC_AHBENR_ADC12EN;      // Enable ADC Clock
        ADC1_2_COMMON->CCR = ADC12_CCR_CKMODE | // ADC12 Clock = HCLK/4
                             ADC12_CCR_TSEN;    // Temperature sensor enable

        ADC1->CR &= ~ADC_CR_ADVREGEN;           // Enable ADC1 Voltage Regulator
        ADC1->CR |= ADC_CR_ADVREGEN_0;

        ADC2->CR &= ~ADC_CR_ADVREGEN;           // Enable ADC2 Voltage Regulator
        ADC2->CR |= ADC_CR_ADVREGEN_0;

        _delay_us(10);                          // Regulator Worst Case StarUp Time is 10 uS.

        ADC1->CR |= ADC_CR_ADCAL;               // ADC1 calibration
        while(ADC1->CR & ADC_CR_ADCAL);

        ADC2->CR |= ADC_CR_ADCAL;               // ADC2 calibration
        while(ADC2->CR & ADC_CR_ADCAL);

        ADC1->CFGR =                            // Analog Watchdog disabled
                                                // Regular Channels
                            ADC_CFGR_AUTOFF |   // ADC Auto power OFF
                                                // Single conversion
                            ADC_CFGR_OVRMOD |   // Overrun mode
                                                // Hardware trigger disabled
                            ADC_CFGR_ALIGN;     // Left Aligned 12 bit
                                                // DMA Disabled

        ADC2->CFGR =                            // Analog Watchdog disabled
                                                // Regular Channels
                            ADC_CFGR_AUTOFF |   // ADC Auto power OFF
                                                // Single conversion
                            ADC_CFGR_OVRMOD |   // Overrun mode
                                                // Hardware trigger disabled
                            ADC_CFGR_ALIGN;     // Left Aligned 12 bit
                                                // DMA Disabled

        // Sampling 19,5 ADC Clock cycles
        ADC1->SMPR1 = ADC_SMPR1_SMP1_2 | ADC_SMPR1_SMP2_2 | ADC_SMPR1_SMP3_2 |
                      ADC_SMPR1_SMP4_2 | ADC_SMPR1_SMP5_2 | ADC_SMPR1_SMP6_2 |
                      ADC_SMPR1_SMP7_2 | ADC_SMPR1_SMP8_2 | ADC_SMPR1_SMP9_2;

        ADC1->SMPR2 = ADC_SMPR2_SMP10_2 | ADC_SMPR2_SMP11_2 | ADC_SMPR2_SMP12_2 |
                      ADC_SMPR2_SMP13_2 | ADC_SMPR2_SMP14_2 | ADC_SMPR2_SMP15_2 |
                      ADC_SMPR2_SMP16_2 | ADC_SMPR2_SMP17_2 | ADC_SMPR2_SMP18_2;

        ADC2->SMPR1 = ADC_SMPR1_SMP1_2 | ADC_SMPR1_SMP2_2 | ADC_SMPR1_SMP3_2 |
                      ADC_SMPR1_SMP4_2 | ADC_SMPR1_SMP5_2 | ADC_SMPR1_SMP6_2 |
                      ADC_SMPR1_SMP7_2 | ADC_SMPR1_SMP8_2 | ADC_SMPR1_SMP9_2;

        ADC2->SMPR2 = ADC_SMPR2_SMP10_2 | ADC_SMPR2_SMP11_2 | ADC_SMPR2_SMP12_2 |
                      ADC_SMPR2_SMP13_2 | ADC_SMPR2_SMP14_2 | ADC_SMPR2_SMP15_2 |
                      ADC_SMPR2_SMP16_2 | ADC_SMPR2_SMP17_2 | ADC_SMPR2_SMP18_2;

        ADC1->SQR1 = 0;                         // 1 Conversion
        ADC2->SQR1 = 0;

        ADC1->CR |= ADC_CR_ADEN;                // ADC enabled
        ADC2->CR |= ADC_CR_ADEN;                // ADC enabled
    }
}

void hal_ain_select(uint8_t apin, uint8_t aref __attribute__ ((unused)))
{
    uint8_t cfg = hal_ain2cfg[apin];
    halain_adc = cfg & 0xC0;

    if(halain_adc != 0)   // ADC2
    {
        ADC2->SQR1 = ((cfg & 0x1F) << 6);
    }
    else                    // ADC1
    {
        ADC1->SQR1 = ((cfg & 0x1F) << 6);
    }
}

int16_t hal_ain_get(void)
{
    int16_t retval;
    if(halain_adc != 0)     // ADC2
    {
        retval = (ADC2->DR) >> 1;
        // Start Conversion
        ADC2->CR |= ADC_CR_ADSTART;
    }
    else                    // ADC1
    {
        retval = (ADC1->DR) >> 1;
        // Start Conversion
        ADC1->CR |= ADC_CR_ADSTART;
    }

    return retval;
}
#endif  //EXTAIN_USED
