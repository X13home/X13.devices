/*
Copyright (c) 2011-2012 <comparator@gmx.de>

This file is part of the X13.Home project.
https://github.com/X13home

BSD License
See LICENSE.txt file for license details.
*/

// Test RF Link Software

#include "config.h"

#ifdef GATEWAY
#include "usart.h"
#endif  // GATEWAY

#define DEFAULT_ADDR 0x15

volatile uint8_t iPool;
#define IPOOL_USR   0x01
#define IPOOL_CALIB 0x02

int main(void) __attribute__((naked));
int main(void)
{
// Watchdog Stop
    cli();
    wdt_reset();
    wdt_disable();
#ifdef GATEWAY
    InitUSART(USART_BAUD);   //  Buad = 38400, fosc = 16M/ (16 * baud)  - 1
#endif  //  GATEWAY
    // Init RF
    rf_LoadCfg(OD_DEFAULT_CHANNEL, OD_DEFAULT_GROUP, DEFAULT_ADDR);
    rf_Initialize();

    // Initialize Task Planer
    InitTimer();
    // configure Sleep controller & enable interrupts
    set_sleep_mode(SLEEP_MODE_IDLE);    // Standby, Idle
    sei();                              // Enable global interrupts

    MQ_t * pRBuf;
#ifdef RF_BEACON
    MQ_t * pTBuf;
    uint8_t ticks = POOL_TMR_FREQ - 1;
    uint8_t sCnt = 0;
#endif  //  RF_BEACON
    while(1)
    {
#ifdef GATEWAY
        if(uDataReady())
            uPutChar(uGetChar());
#endif  //  GATEWAY

        pRBuf = (MQ_t *)rf_GetBuf();
        if(pRBuf != NULL)
        {
#ifdef GATEWAY
            uint8_t len;

#ifdef RF_MONITOR
            len = pRBuf->len - 5;
            uPutChar(pRBuf->dest);
#else   //  !RF_MONITOR
            uPutChar(0);
            len = (pRBuf->mq.Length - 2;
#endif  //  RF_MONITOR
            uPutChar(pRBuf->addr);
            uPutChar(pRBuf->mq.Length);
            uPutChar(pRBuf->mq.MsgType);

            for(uint8_t i = 0; i < len; i++)
                uPutChar(pRBuf->mq.m.raw[i]);
            
            uint8_t stat;
#ifndef RF_MONITOR
            stat = 0x80;
#else   //  RF_MONITOR
            stat = pRBuf->mq.m.raw[len];
#endif  //  RF_MONOTOR
#ifdef RF_USE_RSSI
            stat++;
#endif  //  RF_USE_RSSI
            uPutChar(stat);
#ifdef RF_USE_RSSI
            uPutChar(rf_GetRSSI());
#endif  //  RF_USE_RSSI
#endif  //  GATEWAY
            mqRelease(pRBuf);
        }
        
        if(iPool & IPOOL_USR)
        {
            iPool &= ~IPOOL_USR;
#ifdef RF_BEACON
            // Beacon
            if(--ticks == 0)
            {
                ticks = POOL_TMR_FREQ - 1;

                pTBuf = mqAssert();
                if(pTBuf != NULL)
                {
                    pTBuf->addr = 0x00;
#ifdef RF_USE_RSSI
                    pTBuf->mq.Length = 4;
#else   //  !RF_USE_RSSI
                    pTBuf->mq.Length = 3;
#endif  //  RF_USE_RSSI
                    pTBuf->mq.MsgType = MQTTS_MSGTYP_PINGREQ;
                    pTBuf->mq.m.raw[0] = sCnt++;
#ifdef RF_USE_RSSI
                    pTBuf->mq.m.raw[1] = rf_GetRSSI();
#endif  //  RF_USE_RSSI
                    rf_Send((uint8_t *)pTBuf);
                }
            }
#endif  //  BEACON
            rf_Pool();
        }
    }
}

ISR(TIMER_ISR)
{
#ifdef USE_RTC_OSC
//  Calibrate internal RC Osc
// !!!! for ATMEGA xx8P only, used Timer 1
    if(iPool & IPOOL_CALIB)
    {
        uint16_t tmp = TCNT1;
        TCCR1B = 0;
        TCNT1 = 0;

        LEDsOff();
        if(tmp < 62189)         // Clock is running too slow
        {
            OSCCAL++;
        }
        else if(tmp > 62813)    // Clock is running too fast
        {
            OSCCAL--;
        }
        iPool &= ~IPOOL_CALIB;
    }
    else
    {
        TCCR1B = (1<<CS10);
        iPool |= IPOOL_CALIB;
    }
#endif
    iPool |= IPOOL_USR;
}
