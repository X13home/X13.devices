/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

#error "Sorry, but its doesn't work, wait a little bit."

#include "config.h"

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
    InitUART(USART_BAUD);   //  Buad = 38400, fosc = 16M/ (16 * baud)  - 1

    // Initialise  variables
    iPool = 0;
    
    //MQ_t * pUBuf = NULL;        // USART Buffer
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
#endif  //  USE_RTC_OSC
  {
    iPool |= IPOOL_USR;
    PHY_Pool();
  }
}
