/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

#include "config.h"

static volatile uint8_t iPool;
#define IPOOL_USR   0x01
#define IPOOL_CALIB 0x02
#define IPOOL_SLEEP 0x04

#ifdef ASLEEP
static void goToSleep(void);
static void wakeUp(void);
#endif  //  ASLEEP

#ifdef GATEWAY
#include "uart.h"
#endif  //  GATEWAY

int main(void)
{
    MQ_t *  pRBuf;              // RF Buffer
    uint8_t * pPBuf;            // Publish Buffer
#ifdef GATEWAY
    MQ_t *  pUBuf;              // USART Buffer
    s_Addr  iAddr;
#endif  // GATEWAY
    uint8_t bTmp;
    uint16_t poolIdx = 0xFFFF;

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
#ifdef GATEWAY
    InitUART(USART_BAUD);   //  Buad = 38400, fosc = 16M/ (16 * baud)  - 1
#endif  // GATEWAY
    // Init MQTTS
    MQTTS_Init();
    // Init PHY
    PHY_Init();
    // Initialise  variables
    iPool = 0;
    
    // Initialize Task Planer
    InitTimer();
    // configure Sleep controller & enable interrupts
    set_sleep_mode(SLEEP_MODE_IDLE);    // Standby, Idle
    sei();                              // Enable global interrupts

    while(1)
    {
#ifdef GATEWAY
        pUBuf = (MQ_t *)uGetBuf();
        if(pUBuf != NULL)
        {
          iAddr = pUBuf->addr;
          if(iAddr == rf_GetNodeID())
          {
            if(MQTTS_Parser(pUBuf) == 0)
              mqRelease(pUBuf);              
          }
          else if((iAddr != AddrBroadcast) || (MQTTS_Parser(pUBuf) == 0))
          {
            if((MQTTS_GetStatus() == MQTTS_STATUS_CONNECT) && PHY_CanSend())
              PHY_Send(pUBuf);
            else
              mqRelease(pUBuf);
          }
        }

        pRBuf = PHY_GetBuf();
        if(pRBuf != NULL)
        {
            if(MQTTS_GetStatus() == MQTTS_STATUS_CONNECT)
                uPutBuf((uint8_t *)pRBuf);
            else
                mqRelease(pRBuf);
        }
        
        if(MQTTS_DataRdy())
          uPutBuf((uint8_t *)MQTTS_Get());
#else   // NODE
        pRBuf = PHY_GetBuf();
        if((pRBuf != NULL) && (MQTTS_Parser(pRBuf) == 0))
            mqRelease(pRBuf);
            
        if(MQTTS_DataRdy() && PHY_CanSend())
          PHY_Send(MQTTS_Get());
#endif  //  GATEWAY
        if(iPool & IPOOL_USR)
        {
            iPool &= ~IPOOL_USR;
            
            PHY_Pool();

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
                        MQTTS_Publish(poolIdx, MQTTS_FL_QOS1, bTmp, pPBuf);
                        mqRelease((MQ_t *)pPBuf);
                        poolIdx = 0xFFFF;
                    }
                }
            }
#ifdef ASLEEP
            else if(bTmp == MQTTS_STATUS_AWAKE)
            {
                if(poolIdx == 0xFFFF)
                    poolIdx = PoolOD();
            }
#endif  //  ASLEEP

            bTmp = MQTTS_Pool(poolIdx != 0xFFFF);
#ifdef ASLEEP
            if(bTmp == MQTTS_POOL_STAT_ASLEEP)       // Sweet dreams
                goToSleep();
            else if(bTmp == MQTTS_POOL_STAT_AWAKE)   // Wake UP
                wakeUp();
#endif  //  ASLEEP
        }
        sleep_mode();
    }
}

ISR(TIMER_ISR)
{
#ifdef USE_RTC_OSC
#define BASE_TICK       (F_CPU/8/POOL_TMR_FREQ)
#define BASE_TICK_MIN   (uint16_t)(BASE_TICK/1.005)
#define BASE_TICK_MAX   (uint16_t)(BASE_TICK*1.005)

    uint16_t tmp;

//  Calibrate internal RC Osc
// !!!! for ATMEGA xx8P only, used Timer 1
    if(iPool & IPOOL_CALIB)
    {
        tmp = TCNT1;
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

#ifdef ASLEEP
#ifndef USE_RTC_OSC
ISR(WDT_vect)
{
    wdt_reset();
    if(iPool & IPOOL_SLEEP)
        iPool |= IPOOL_USR;
}
#endif  //  !USE_RTC_OSC

static void goToSleep(void)
{
    iPool = IPOOL_SLEEP;
    rf_SetState(RF_TRVASLEEP);
#ifdef USE_RTC_OSC
    config_sleep_rtc();
#else   // Use watchdog
    config_sleep_wdt();
#endif  //  USE_RTC_OSC
    set_sleep_mode(SLEEP_MODE_PWR_SAVE);    // Standby, Power Save
}

static void wakeUp(void)
{
    set_sleep_mode(SLEEP_MODE_IDLE);        // Standby, Idle
    rf_SetState(RF_TRVWKUP);
#ifdef USE_RTC_OSC
    InitTimer();
#else   // Use watchdog
    wdt_reset();
    wdt_disable();
#endif  //  USE_RTC_OSC
    iPool = 0;
}
#endif  //  ASLEEP