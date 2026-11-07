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

static volatile uint8_t SystemTickCnt;

int main(void)
{
    // Initialise System Hardware
    HAL_Init();
    // Initialise Memory manager
    mqInit();
    // Initialise Object's Dictionary
    InitOD();
#ifdef  LED_Init
    LED_Init();
#endif  //  LEDsInit
    // Initialise PHY's
    PHY1_Init();
#ifdef PHY2_ADDR_t
    PHY2_Init();
#endif  //  PHY2_ADDR_t
#ifdef PHY3_ADDR_t
    PHY3_Init();
#endif  //  PHY3_ADDR_t
#ifdef PHY4_ADDR_t
    PHY4_Init();
#endif  //  PHY4_ADDR_t
    // Initialize MQTTSN
    MQTTSN_Init();
    // Initialise optional components

    SystemTickCnt = 0;

    HAL_StartSystemTick();
  
    for(;;)
    {
        if(SystemTickCnt)
        {
#ifdef LED_Off
            LED_Off();
#endif  //  LED_Off

            SystemTickCnt--;
            OD_Poll();

            MQTTSN_Poll();
        }

        MQ_t * pBuf1 = PHY1_Get();
        if(pBuf1 != NULL)
        {
            mqttsn_parser_phyM(pBuf1);
        }

#ifdef PHY2_Get
        MQ_t * pBuf2 = PHY2_Get();
        if(pBuf2 != NULL)
        {
            mqttsn_parser_phyS(pBuf2, 2);
        }
#endif  //  PHY2_Get

#ifdef PHY3_Get
        MQ_t * pBuf3 = PHY3_Get();
        if(pBuf3 != NULL)
        {
            mqttsn_parser_phyS(pBuf3, 3);
        }
#endif  //  PHY3_Get

#ifdef PHY4_Get
        MQ_t * pBuf4 = PHY4_Get();
        if(pBuf4 != NULL)
        {
            mqttsn_parser_phyS(pBuf4, 4);
        }
#endif  //  PHY4_Get
    }
}

void SystemTick(void)
{
    SystemTickCnt++;
}
