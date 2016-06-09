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
#if (defined HAL_USE_RTC) && (defined STM32F1)

typedef struct
{
    uint8_t wdu;
    uint8_t day;
    uint8_t mon;
    uint8_t yrs;
}TM_t;

static const uint16_t lmos[] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335};
static const uint16_t  mos[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

#define MONTAB(year)    ((((year) & 03) || ((year) == 0)) ? mos : lmos)
#define Daysto32(year, mon) (((year - 1) / 4) + MONTAB(year)[mon])

// Convert Scalar Time in days to Day/Month/Year/Week Day
static void scd2dmyw(uint32_t days, TM_t * pTM)
{
    uint32_t mon, year, i;

    days += 25567;

    // determine year
    for(year = days / 365; days < (i = Daysto32(year, 0) + 365 * year);)
    {
        --year;
    }
    days -= i;

    // determine month
    const uint16_t * pm;
    pm = MONTAB(year);
    for(mon = 12; days < pm[--mon]; );

    days -= pm[mon];
    days++;
    mon++;

    // determine WDU
    int16_t a = (14 - mon) / 12;
    int16_t y = (year + 1900) - a;
            y += y/4 - y/100 + y/400;
    uint8_t wdu = (7000 + (days + y + (31 * (mon + 12 * a - 2)) / 12)) % 7;
    if(wdu == 0)
    {
        wdu = 7;
    }
    
    pTM->wdu = wdu;
    pTM->day = days & 0x1F;
    pTM->mon = mon & 0x0F;
    pTM->yrs = (year - 100);
}

void hal_rtc_init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_BKPEN | RCC_APB1ENR_PWREN;
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
    rtc_presc = 250000UL - 1;
#else   // use LSI or LSE
#if (defined HAL_RTC_CHECK_LSE)
    RCC->BDCR |= RCC_BDCR_LSEON;                    // External Low Speed oscillator enable
    uint16_t tmp = 0;
    while(tmp < 0xFFFF)                             // Wait till LSE is ready
    {
        if((RCC->BDCR & (RCC_BDCR_LSERDY | RCC_BDCR_LSEON)) == (RCC_BDCR_LSERDY | RCC_BDCR_LSEON))
        {
            break;
        }
        tmp++;
    }

    if((RCC->BDCR & (RCC_BDCR_LSERDY | RCC_BDCR_LSEON)) == (RCC_BDCR_LSERDY | RCC_BDCR_LSEON))
    {
        if((RCC->BDCR & RCC_BDCR_RTCSEL) != RCC_BDCR_RTCSEL_LSE)
        {
            // Reset BackUp Domain
            RCC->BDCR |= RCC_BDCR_BDRST;
            RCC->BDCR &= ~RCC_BDCR_BDRST;
            RCC->BDCR |= RCC_BDCR_RTCSEL_LSE;       // LSI oscillator clock used as RTC clock
            RCC->BDCR |= RCC_BDCR_RTCEN;            // RTC clock enable
        }
        rtc_presc = 32768 - 1;
    }
    else                                            // LSE Not Connected, use LSI
#endif  //  HAL_RTC_CHECK_LSE
    {
        RCC->CSR |= RCC_CSR_LSION;                  // Enable LSI
        while((RCC->CSR & RCC_CSR_LSIRDY) == 0);    // Wait till LSI is ready

        if((RCC->BDCR & (RCC_BDCR_RTCSEL | RCC_BDCR_RTCEN)) != 
                        (RCC_BDCR_RTCSEL_LSI | RCC_BDCR_RTCEN))
        {
            RCC->BDCR |= RCC_BDCR_BDRST;
            RCC->BDCR &= ~RCC_BDCR_BDRST;
            RCC->BDCR |= RCC_BDCR_RTCSEL_LSI;       // LSI oscillator clock used as RTC clock
            RCC->BDCR |= RCC_BDCR_RTCEN;            // RTC clock enable
        }
        rtc_presc = 40000 - 1;
    }
#endif  //  HAL_RTC_USE_HSE

    while((RTC->CRL & RTC_CRL_RTOFF) == 0);
    RTC->CRL |= RTC_CRL_CNF;                        // Enter to Configuration Mode
    
    RTC->PRLH = rtc_presc >> 16;
    RTC->PRLL = rtc_presc & 0xFFFF;
    
    RTC->CRL &= ~RTC_CRL_CNF;
    while((RTC->CRL & RTC_CRL_RTOFF) == 0);
}

void HAL_RTC_Set(uint8_t *pBuf)
{
    // Convert BCD to Binary
    uint8_t sec = pBuf[0];
    sec = (sec & 0x0F) + ((sec >> 4) * 10);
    uint8_t min = pBuf[1];
    min = (min & 0x0F) + ((min >> 4) * 10);
    uint8_t hrs = pBuf[2];
    hrs = (hrs & 0x0F) + ((hrs >> 4) * 10);

    uint8_t day = pBuf[3];
    day = (day & 0x0F) + ((day >> 4) * 10);
    uint8_t mon = pBuf[4] & 0x1F;                   // Week day ignored.
    mon = (mon & 0x0F) + ((mon >> 4) * 10) - 1;
    uint8_t yrs = pBuf[5];
    yrs = (yrs & 0x0F) + ((yrs >> 4) * 10) + 100;   //  Years after 2000

    uint32_t cnt;
    // Convert DMY to scalar time
    cnt  = Daysto32(yrs, mon) - 1;
    cnt += 365 * yrs;
    cnt += day;
    cnt -= 25567;
    cnt *= 86400UL;

    // Add Now Seconds
    cnt += (3600 * hrs);
    cnt += (60 * min);
    cnt += sec;

    while((RTC->CRL & RTC_CRL_RTOFF) == 0);
    RTC->CRL |= RTC_CRL_CNF;                    // Enter to Configuration Mode

    if((RCC->BDCR & RCC_BDCR_RTCSEL) == RCC_BDCR_RTCSEL_LSI)
    {
        uint32_t uTmp;
        uTmp = (RTC->CNTH << 16);
        uTmp |= (RTC->CNTL & 0xFFFF);
        
        if(((uTmp + 5) < cnt) && (RTC->PRLL > 33250))
        {
            RTC->PRLL--;
        }
        else if((uTmp > (cnt + 5)) && (RTC->PRLL < 44000))
        {
            RTC->PRLL++;
        }
    }

    RTC->CNTH = cnt >> 16;
    RTC->CNTL = cnt & 0xFFFF;

    RTC->CRL &= ~RTC_CRL_CNF;
    while((RTC->CRL & RTC_CRL_RTOFF) == 0);
}

uint8_t HAL_RTC_Get(uint8_t *pBuf)
{
    uint32_t uTmp;
    uTmp = (RTC->CNTH << 16);
    uTmp |= (RTC->CNTL & 0xFFFF);

    uint8_t sec = uTmp % 60;
    uTmp /= 60;
    uint8_t min = uTmp % 60;
    uTmp /= 60;
    uint8_t hrs = uTmp % 24;
    
    pBuf[0] = (sec % 10) + ((sec / 10) << 4);   //  SU + ST
    pBuf[1] = (min % 10) + ((min / 10) << 4);
    pBuf[2] = (hrs % 10) + ((hrs / 10) << 4);
    
    TM_t tm;
    scd2dmyw(uTmp / 24, &tm);
    uint8_t day = tm.day;
    uint8_t mon = tm.mon;
    uint8_t yrs = tm.yrs;

    pBuf[3] = (day % 10) + ((day / 10) << 4);
    pBuf[4] = ((mon % 10) + ((mon / 10) << 4)) | (tm.wdu << 5);
    pBuf[5] = (yrs % 10) + ((yrs / 10) << 4);

    return 6;
}

// Get Now Seconds from 00:00:00
uint32_t HAL_RTC_SecNow(void)
{
    uint32_t tmp;
    tmp = (RTC->CNTH << 16);
    tmp |= (RTC->CNTL & 0xFFFF);
    tmp %= 86400;
    return tmp;
}

// Get Date 4 bytes: YRS [31-24], MNT[23-16], DAY[15-8], WDU[7-0]
// Years
// Month:    1 - Jan, 12 - Dez
// Day:      1 - 31
// Week day: 1 - Monday, 7 - Sunday
uint32_t HAL_RTC_DateNow(void)
{
    uint32_t secs;
    secs = (RTC->CNTH << 16);
    secs |= (RTC->CNTL & 0xFFFF);
    
    TM_t tm;
    scd2dmyw(secs / 86400, &tm);
    return (tm.yrs<<24) | (tm.mon<<16) | (tm.day<<8) | tm.wdu;
}

#endif  //  HAL_USE_RTC
