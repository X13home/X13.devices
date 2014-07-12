/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#include "config.h"

volatile uint8_t iPoll;
#define IPOLL_USR   0x01
#define IPOLL_LED_ONL   0x10
#define IPOLL_LED_CONN  0x20
#define IPOLL_LED_ACT   0x40

__attribute__((OS_main)) int main(void)
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
  // Initialise MQTTS
  MQTTS_Init();
  // Initialise PHY
  PHY_Init();
  // Initialise  variables
  iPoll = 0;

  uint8_t objLen;
  uint16_t pollIdx = 0xFFFF;

  // Initialize Task Planer
  InitTimer();
  // configure Sleep controller & enable interrupts
  set_sleep_mode(SLEEP_MODE_IDLE);    // Standby, Idle
  sei();                              // Enable global interrupts
  
  PHY_Start();
  iPoll |= IPOLL_LED_ONL;
  
  while(1)
  {
    MQ_t * pRBuf;                   // RF Buffer
    MQ_t * pMBuf;                   // MQTTS Buffer
  
    if((pRBuf = PHY_GetBuf()) != NULL)
    {
      iPoll |= IPOLL_LED_ACT;
      if(MQTTS_Parser(pRBuf) == 0)
        mqRelease(pRBuf);
    }

    if((pMBuf = MQTTS_Get()) != NULL)
    {
      iPoll |= IPOLL_LED_ACT;
      PHY_Send(pMBuf);
    }
    
    if(iPoll & IPOLL_USR)
    {
      iPoll &= ~IPOLL_USR;

      if(MQTTS_GetStatus() == MQTTS_STATUS_CONNECT)
      {
        iPoll |= IPOLL_LED_CONN;
        if(pollIdx == 0xFFFF)
          pollIdx = PollOD(0);

        if(pollIdx != 0xFFFF)
        {
          // Publish
          uint8_t * pPBuf;          // Publish Buffer
          pPBuf = (uint8_t *)mqAssert();
          if(pPBuf != NULL)
          {
            objLen = (MQTTS_MSG_SIZE - MQTTS_SIZEOF_MSG_PUBLISH);
              
            ReadOD(pollIdx, MQTTS_FL_TOPICID_NORM | 0x80, &objLen, pPBuf);
            MQTTS_Publish(pollIdx, MQTTS_FL_QOS1, objLen, pPBuf);
            mqRelease((MQ_t *)pPBuf);
            pollIdx = 0xFFFF;
          }
        }
      }
      else
        iPoll &= ~IPOLL_LED_CONN;

      MQTTS_Poll(pollIdx != 0xFFFF);
    }
  }
}

ISR(TIMER_ISR)
{
  static uint8_t led_cnt = 0;

  iPoll |= IPOLL_USR;
  
  PHY_Poll();

  if(led_cnt)
  {
    led_cnt--;
  }
  else
  {
    if(iPoll & IPOLL_LED_ONL)
    {
      if(iPoll & IPOLL_LED_CONN)
      {
        if(iPoll & IPOLL_LED_ACT)     // Led blinks on Activity
        {
          LED_OFF();
          iPoll &= ~IPOLL_LED_ACT;
        }
        else
          LED_ON();

        led_cnt = (POLL_TMR_FREQ/32);  // 125mS Period
      }
      else                            // LED blinks slow when not connected to broker
      {
        led_cnt = (POLL_TMR_FREQ/4);  // 500mS Period
        LED_TGL();
      }
    }
    else                              // LED blinks fast if not connected to Net or/and DHCP
    {
      led_cnt = (POLL_TMR_FREQ/32);  // 125mS Period
      LED_TGL();
    }
  }
}
