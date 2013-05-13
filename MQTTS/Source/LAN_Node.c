/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

#include "config.h"

volatile uint8_t iPool;
#define IPOOL_USR       0x01
#define IPOOL_LED_ONL   0x10
#define IPOOL_LED_CONN  0x20
#define IPOOL_LED_ACT   0x40

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

  // Initialise  variables
  iPool = 0;

  MQ_t * pRBuf = NULL;        // LAN Buffer
  MQ_t * pMBuf = NULL;        // MQTTS Buffer
  uint8_t * pPBuf = NULL;     // Publish Buffer
    
  uint8_t objLen;
  uint16_t poolIdx = 0xFFFF;

  s_Addr sAddr;

  // Initialize Task Planer
  InitTimer();
  // configure Sleep controller & enable interrupts
  set_sleep_mode(SLEEP_MODE_IDLE);    // Standby, Idle
  sei();                              // Enable global interrupts

  PHY_Start();
  iPool |= IPOOL_LED_ONL;

  while(1)
  {
    if((pRBuf = PHY_GetBuf(&sAddr)) != NULL)
    {
      iPool |= IPOOL_LED_ACT;
      if(MQTTS_Parser(pRBuf, &sAddr) == 0)
        mqRelease(pRBuf);
    }

    if((pMBuf = MQTTS_Get(&sAddr)) != NULL)
    {
      iPool |= IPOOL_LED_ACT;
      PHY_Send(pMBuf, &sAddr);
    }

    if(iPool & IPOOL_USR)
    {
      iPool &= ~IPOOL_USR;

      if(MQTTS_GetStatus() == MQTTS_STATUS_CONNECT)
      {
        iPool |= IPOOL_LED_CONN;
        if(poolIdx == 0xFFFF)
          poolIdx = PoolOD();

        if(poolIdx != 0xFFFF)
        {
          // Publish
          pPBuf = (uint8_t *)mqAssert();
          if(pPBuf != NULL)
          {
            objLen = (MQTTS_MSG_SIZE - MQTTS_SIZEOF_MSG_PUBLISH);
              
            ReadOD(poolIdx, MQTTS_FL_TOPICID_NORM | 0x80, &objLen, pPBuf);
            MQTTS_Publish(poolIdx, MQTTS_FL_QOS1, objLen, pPBuf);
            mqRelease((MQ_t *)pPBuf);
            poolIdx = 0xFFFF;
          }
        }
      }
      else
        iPool &= ~IPOOL_LED_CONN;

      MQTTS_Pool(poolIdx != 0xFFFF);
    }
  }
}

ISR(TIMER_ISR)
{
  static uint8_t led_cnt = 0;

  iPool |= IPOOL_USR;
  
  PHY_Pool();

  if(led_cnt)
  {
    led_cnt--;
  }
  else
  {
    if(iPool & IPOOL_LED_ONL)
    {
      if(iPool & IPOOL_LED_CONN)
      {
        if(iPool & IPOOL_LED_ACT)     // Led blinks on Activity
        {
          LED_OFF();
          iPool &= ~IPOOL_LED_ACT;
        }
        else
          LED_ON();

        led_cnt = (POOL_TMR_FREQ/32);  // 125mS Period
      }
      else                            // LED blinks slow when not connected to broker
      {
        led_cnt = (POOL_TMR_FREQ/4);  // 500mS Period
        LED_TGL();
      }
    }
    else                              // LED blinks fast wenn not connected to Net or/and DHCP
    {
      led_cnt = (POOL_TMR_FREQ/32);  // 125mS Period
      LED_TGL();
    }
  }
}
