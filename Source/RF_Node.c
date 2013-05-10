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

#ifdef ASLEEP
void goToSleep(void);
void wakeUp(void);
#endif  //  ASLEEP

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
    // Init MQTTS
    MQTTS_Init();
    // Init Interconnection Interface & Load Configuration
    PHY_Init();
#ifdef GATEWAY
    InitUART(USART_BAUD);   //  Buad = 38400, fosc = 16M/ (16 * baud)  - 1
#endif  // GATEWAY

    // Initialise  variables
    iPool = 0;
    
#ifdef GATEWAY
    MQ_t * pUBuf = NULL;        // USART Buffer
#endif  // GATEWAY
    MQ_t * pRBuf = NULL;        // RF Buffer
    MQ_t * pMBuf = NULL;        // MQTTS Buffer
    uint8_t * pPBuf = NULL;     // Publish Buffer
    
    uint8_t bTmp;
    uint16_t poolIdx = 0xFFFF;

    s_Addr sAddr;

    // Initialize Task Planer
    InitTimer();
    // configure Sleep controller & enable interrupts
    set_sleep_mode(SLEEP_MODE_IDLE);    // Standby, Idle
    sei();                              // Enable global interrupts

    // 
    PHY_Start();

    while(1)
    {
      pRBuf = PHY_GetBuf(&sAddr);
      if((pRBuf != NULL) && (MQTTS_Parser(pRBuf, &sAddr) == 0))
        mqRelease(pRBuf);
        
      pMBuf = MQTTS_Get(&sAddr);
      if(pMBuf != NULL)
        PHY_Send(pMBuf, &sAddr);
        
      if(iPool & IPOOL_USR)
      {
        iPool &= ~IPOOL_USR;

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
  {
    iPool |= IPOOL_USR;
    PHY_Pool();
  }
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

void goToSleep(void)
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

void wakeUp(void)
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
