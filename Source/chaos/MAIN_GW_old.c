/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

#include "config.h"

volatile uint8_t iPool;
#define IPOOL_USR   0x01
#define IPOOL_CALIB 0x02
#define IPOOL_SLEEP 0x04

int main(void) __attribute__((naked));
int main(void)
{
// Watchdog Stop
    cli();
    wdt_reset();
    wdt_disable();
    // Configure Power Reduction
    CONFIG_PRR();
    // Initialize Memory manager
    mqInit();
    // Initialize Object's Dictionary
    InitOD();
    InitUART(USART_BAUD);   //  Buad = 38400, fosc = 16M/ (16 * baud)  - 1
    // Init MQTTS
    MQTTS_Init();
    // Init RF
    rf_Initialize();
    // Initialise  variables
    iPool = 0;
    
    MQ_t * pUBuf = NULL;        // USART Buffer
    MQ_t * pRBuf = NULL;        // RF Buffer
    MQ_t * pMBuf = NULL;        // MQTTS Buffer
    uint8_t * pPBuf = NULL;     // Publish Buffer
    
    uint8_t bTmp;
    uint16_t poolIdx = 0xFFFF;

    // Initialize Task Planer
    InitTimer();
    // configure Sleep controller & enable interrupts
    set_sleep_mode(SLEEP_MODE_IDLE);    // Standby, Idle
    sei();                              // Enable global interrupts

    while(1)
    {
        pUBuf = (MQ_t *)uGetBuf();
        if(pUBuf != NULL)
        {
            bTmp = pUBuf->addr;
            if((bTmp == rf_GetNodeID()) || (bTmp == 0))
            {
                if(MQTTS_Parser(pUBuf) == 0)
                {
                    if((MQTTS_GetStatus() == MQTTS_STATUS_CONNECT) && (bTmp == 0))  // broadcast
                        rf_Send((uint8_t *)pUBuf);
                    else
                        mqRelease(pUBuf);
                }
            }
            else if(MQTTS_GetStatus() == MQTTS_STATUS_CONNECT)
                rf_Send((uint8_t *)pUBuf);
        }

        pRBuf = (MQ_t *)rf_GetBuf();
        if(pRBuf != NULL)
        {
            if(MQTTS_GetStatus() == MQTTS_STATUS_CONNECT)
            {
                uPutBuf((uint8_t *)pRBuf);
            }
            else
                mqRelease(pRBuf);
        }

        pMBuf = MQTTS_Get();
        if(pMBuf != NULL)
            uPutBuf((uint8_t *)pMBuf);

        if(iPool & IPOOL_USR)
        {
            iPool &= ~IPOOL_USR;
            
            rf_Pool();

            bTmp = MQTTS_GetStatus();
            if(bTmp == MQTTS_STATUS_CONNECT)
            {
                if(poolIdx == 0xFFFF)
                    poolIdx = PoolOD();
            
                if(poolIdx != 0xFFFF)
                {
                    // Publish
                    pPBuf = (uint8_t *)mqAssert();
                    if(pPBuf != NULL)
                    {
                        bTmp = (MQTTS_MSG_SIZE - MQTTS_SIZEOF_MSG_PUBLISH);

                        ReadOD(poolIdx, MQTTS_FL_TOPICID_NORM | 0x80, &bTmp, pPBuf);
                        MQTTS_Publish(0, poolIdx, MQTTS_FL_QOS1, bTmp, pPBuf);
                        mqRelease((MQ_t *)pPBuf);
                        poolIdx = 0xFFFF;
                    }
                }
            }

            bTmp = MQTTS_Pool(poolIdx != 0xFFFF);
        }
    }
}

ISR(TIMER_ISR)
{
#ifdef USE_RTC_OSC
#define BASE_TICK       (F_CPU/8/POOL_TMR_FREQ)
#define BASE_TICK_MIN   (uint16_t)(BASE_TICK/1.005)
#define BASE_TICK_MAX   (uint16_t)(BASE_TICK*1.005)

//  Calibrate internal RC Osc
// !!!! for ATMEGA xx8P only, used Timer 1
    if(iPool & IPOOL_CALIB)
    {
        uint16_t tmp = TCNT1;
        TCCR1B = 0;

        if(tmp < BASE_TICK_MIN)         // Clock is running too slow
            OSCCAL++;
        else if(tmp > BASE_TICK_MAX)    // Clock is running too fast
            OSCCAL--;

        iPool &= ~IPOOL_CALIB;
    }
    else if(!(iPool & IPOOL_SLEEP))
    {
        TCNT1 = 0;
        TCCR1B = (2<<CS10);
        iPool |= IPOOL_CALIB;
    }
#else   //  !USE_RTC_OSC
    if(!(iPool & IPOOL_SLEEP))
#endif  //  USE_RTC_OSC
        iPool |= IPOOL_USR;
}
