/*
Copyright (c) 2011-2016 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// Global configuration settings

#ifndef _CONFIG_H
#define _CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// System Settings
#define POLL_TMR_FREQ                   100     // System Tick Frequenz

// MQTT-SN Section
#define MQTTSN_MSG_SIZE                 48      // Size of payload(base for all buffers)
#define MQTTSN_USE_DHCP                 1       // Use Automatic address resolution
                                                //  Not Standard Messages
//#define MQTTSN_REBOOT_ON_LOST           1       // Reboot on connection lost

// Object Dictionary
#define OD_DEV_SWVERSH                  '3'     // Software Version
#define OD_DEV_SWVERSM                  '1'
#define OD_DEV_SWVERSL                  '4'

#include "hal.h"
#include "mqMEM.h"
#include "mqTypes.h"
#include "mqttsn.h"
#include "ObjDict.h"
#include "ext.h"

#if (defined PHY2_ADDR_t) && (defined ASLEEP)
#error Incompatible options ASLEEP for Gateway
#endif  //

// Common HAL Section
void        HAL_Init(void);
void        HAL_StartSystemTick(void);
uint16_t    HAL_RNG(void);
uint32_t    HAL_RNG32(void);
uint32_t    HAL_get_ms(void);
uint32_t    HAL_get_sec(void);
void        HAL_ASleep(uint16_t duration);
void        HAL_Reboot(void);

// Main Tick's
void        SystemTick(void);

#ifdef __cplusplus
}
#endif

#endif  //  _CONFIG_H
