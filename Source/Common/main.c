/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#include "config.h"
#include "diag.h"

static volatile uint8_t SystemTickCnt;

int main(void)
{
    // Initialise System Hardware
    HAL_Init();
    // Initialise Memory manager
    mqInit();
    // Initialise Object's Dictionary
    InitOD();
    // Initialise PHY's
    PHY1_Init();
#ifdef PHY2_ADDR_t
    PHY2_Init();
#endif  //  PHY2_ADDR_t
    // Initialize MQTTSN
    MQTTSN_Init();
    // Initialise optional components
#ifdef  LED_Init
    LED_Init();
#endif  //  LEDsInit

    SystemTickCnt = 0;

    HAL_StartSystemTick();
  
    for(;;)
    {
        if(SystemTickCnt)
        {
            SystemTickCnt--;
            OD_Poll();

            MQTTSN_Poll();
#ifdef DIAG_USED
            DIAG_Poll();
#endif  //  USE_DIAG
#ifdef LED_Off
            LED_Off();
#endif  //  LED_Off
        }

        MQ_t * pBuf;
        pBuf = PHY1_Get();
        if(pBuf != NULL)
        {
            mqttsn_parser_phy1(pBuf);
        }

#ifdef PHY2_Get
        pBuf = PHY2_Get();
        if(pBuf != NULL)
        {
            mqttsn_parser_phy2(pBuf);
        }
#endif  //  PHY2_Get
    }
}

void SystemTick(void)
{
    SystemTickCnt++;
}
