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

// RTC Section
#if (defined HAL_USE_RTC) && ((defined STM32F0) | (defined STM32F3))

void hal_rtc_init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR |= PWR_CR_DBP;                          // Disable Backup Domain write protection

    // Select RTC Clock
    uint32_t rtc_presc;
#if (defined HAL_RTC_USE_HSE)
    if((RCC->BDCR & (RCC_BDCR_RTCSEL | RCC_BDCR_RTCEN)) != (RCC_BDCR_RTCSEL_HSE | RCC_BDCR_RTCEN))
    {
        // Reset BackUp Domain
        RCC->BDCR |= RCC_BDCR_BDRST;
        RCC->BDCR &= ~RCC_BDCR_BDRST;
        RCC->BDCR |= RCC_BDCR_RTCSEL_HSE;           // HSE/32 clock used as RTC clock
        RCC->BDCR |= RCC_BDCR_RTCEN;                // RTC clock enable
    }
    rtc_presc = 0x007C07CF;                         // 8000000/32 = 250000/125 = 2000
#else   // use LSI or LSE
    if((RCC->BDCR & RCC_BDCR_RTCEN) == 0)
    {
#if (defined HAL_RTC_CHECK_LSE)
        RCC->BDCR |= RCC_BDCR_LSEON;                // External Low Speed oscillator enable
        uint16_t tmp = 0;
        while(tmp < 0xFFFF)                         // Wait till LSE is ready
        {
            if(RCC->BDCR & RCC_BDCR_LSERDY)
            {
                break;
            }
            tmp++;
        }

        if(RCC->BDCR & RCC_BDCR_LSERDY)
        {
            RCC->BDCR |= RCC_BDCR_RTCSEL_LSE;       // LSE oscillator clock used as RTC clock
        }
        else                                        // LSE Not Connected, use LSI
#endif  //  HAL_RTC_CHECK_LSE
        {
#if (defined HAL_RTC_CHECK_LSE)
hal_rtc_start_lsi:
#endif  //  HAL_RTC_CHECK_LSE
            if((RCC->BDCR & RCC_BDCR_LSEON) || 
              ((RCC->BDCR & RCC_BDCR_RTCSEL) != RCC_BDCR_RTCSEL_LSI))
            {
                // LSE enabled, or used another clock, Reset DBP
                RCC->BDCR |= RCC_BDCR_BDRST;
                RCC->BDCR &= ~RCC_BDCR_BDRST;
            }

            RCC->CSR |= RCC_CSR_LSION;                  // Enable LSI
            while((RCC->CSR & RCC_CSR_LSIRDY) == 0);    // Wait till LSI is ready
            RCC->BDCR |= RCC_BDCR_RTCSEL_LSI;           // LSI oscillator clock used as RTC clock
        }
        RCC->BDCR |= RCC_BDCR_RTCEN;            // RTC clock enable
    }

#if (defined HAL_RTC_CHECK_LSE)
    if((RCC->BDCR & RCC_BDCR_RTCSEL) == RCC_BDCR_RTCSEL_LSE)
    {
        uint16_t tmp = 0;
        while(tmp < 0xFFFF)                         // Wait till LSE is ready
        {
            if((RCC->BDCR & (RCC_BDCR_LSERDY | RCC_BDCR_LSEON)) == (RCC_BDCR_LSERDY | RCC_BDCR_LSEON))
            {
                break;
            }
            tmp++;
        }

        if((RCC->BDCR & (RCC_BDCR_LSERDY | RCC_BDCR_LSEON)) != (RCC_BDCR_LSERDY | RCC_BDCR_LSEON))
        {
            goto hal_rtc_start_lsi;
        }
        rtc_presc = 0x007F00FF;                     // 32768/128 = 256
    }
    else
#endif  //  HAL_RTC_CHECK_LSE
    {
        RCC->CSR |= RCC_CSR_LSION;                  // Enable LSI
        while((RCC->CSR & RCC_CSR_LSIRDY) == 0);    // Wait till LSI is ready
        rtc_presc = 0x007C013F;                     // 40000/125 = 320/320
    }
#endif  //  HAL_RTC_USE_HSE

    RTC->WPR = 0xCA;                                // Disable RTC write protection
    RTC->WPR = 0x53;

    if((RTC->ISR & RTC_ISR_INITS) == 0)             // RTC not Initialised
    {
        // Initialise RTC
        RTC->ISR |= RTC_ISR_INIT;
        while((RTC->ISR & RTC_ISR_INITF) != RTC_ISR_INITF);
        RTC->PRER = rtc_presc;
        // RTC->TR
        // RTC->DR
        // RTC->CR
        RTC->ISR &= ~RTC_ISR_INIT;
        while((RTC->ISR & RTC_ISR_INIT) != 0);
    }
}

void HAL_RTC_Set(uint8_t *pBuf)
{
    uint32_t oldTR = RTC->TR;
    uint32_t oldDR = RTC->DR;

    uint32_t newTR = 0, newDR = 0;
    memcpy(&newTR, pBuf, 3);
    pBuf += 3;
    memcpy(&newDR, pBuf, 3);

    // Initialise RTC
    RTC->ISR |= RTC_ISR_INIT;
    while((RTC->ISR & RTC_ISR_INITF) != RTC_ISR_INITF);

    if(((RCC->BDCR & RCC_BDCR_RTCSEL) == RCC_BDCR_RTCSEL_LSI) && (oldDR == newDR))
    {
        uint32_t tmp = RTC->PRER;
        tmp &= 0x7FFF;

        // BCD to binary
        uint32_t oSec = (oldTR & 0x0F) +                    // SU
                        (((oldTR >> 4)  & 0x07) * 10)    +  // ST
                        (((oldTR >> 8)  & 0x0F) * 60)    +  // MNU
                        (((oldTR >> 12) & 0x07) * 600)   +  // MNT
                        (((oldTR >> 16) & 0x0F) * 3600)  +  // HU
                        (((oldTR >> 20) & 0x03) * 36000);   // HT
                        
        // BCD to binary
        uint32_t nSec = (newTR & 0x0F) +                    // SU
                        (((newTR >> 4)  & 0x07) * 10)    +  // ST
                        (((newTR >> 8)  & 0x0F) * 60)    +  // MNU
                        (((newTR >> 12) & 0x07) * 600)   +  // MNT
                        (((newTR >> 16) & 0x0F) * 3600)  +  // HU
                        (((newTR >> 20) & 0x03) * 36000);   // HT

        if(((oSec + 5) < nSec) && (tmp > 266))      // LSI > 33250 Hz
        {
            RTC->PRER--;
        }
        else if((oSec > (nSec + 5)) && (tmp < 352)) // LSI < 44000 Hz
        {
            RTC->PRER++;
        }
    }

    RTC->DR = newDR;
    RTC->TR = newTR;

    RTC->ISR &= ~RTC_ISR_INIT;
    while((RTC->ISR & RTC_ISR_INIT) != 0);
}

uint8_t HAL_RTC_Get(uint8_t *pBuf)
{
    uint32_t uTmp;
    uTmp = RTC->TR;
    memcpy(pBuf, &uTmp, 3);
    pBuf += 3;
    uTmp = RTC->DR;
    memcpy(pBuf, &uTmp, 3);
    return 6;
}

// Get Now Seconds from 00:00:00
uint32_t HAL_RTC_SecNow(void)
{
    uint32_t tmp;
    tmp = RTC->TR;
    tmp = (tmp & 0x0F)                  +   // SU
        (((tmp >> 4)  & 0x07) * 10)     +   // ST
        (((tmp >> 8)  & 0x0F) * 60)     +   // MNU
        (((tmp >> 12) & 0x07) * 600)    +   // MNT
        (((tmp >> 16) & 0x0F) * 3600)   +   // HU
        (((tmp >> 20) & 0x03) * 36000);     // HT
    return tmp;
}

// Get Date 4 bytes: YRS [31-24], MNT[23-16], DAY[15-8], WDU[7-0]
// Years
// Month:    1 - Jan, 12 - Dez
// Day:      1 - 31
// Week day: 1 - Monday, 7 - Sunday
uint32_t HAL_RTC_DateNow(void)
{
    uint32_t tmp = RTC->DR;

    uint32_t day = (tmp & 0x0F) + (((tmp >> 4) & 0x03) * 10);
    uint32_t month = ((tmp >> 8) & 0x0F) + ((tmp & 0x01000) != 0 ? 10 : 0);
    uint32_t wdu = (tmp >> 13) & 0x07;
    uint32_t yrs = ((tmp >> 16) & 0x0F) + (((tmp >> 20) & 0x0F) * 10);
    
    tmp = wdu | (day << 8) | (month << 16) | (yrs << 24);
    return tmp;
}

#endif  //  HAL_USE_RTC
