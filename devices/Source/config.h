/*
Copyright (c) 2011-2015 <comparator@gmx.de>

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
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// System Settings
#define POLL_TMR_FREQ                   100     // System Tick Frequenz

// MQTT-SN Section
#define MQTTSN_MSG_SIZE                 48      // Size of payload(base for all buffers)
#define MQTTSN_USE_DHCP                 1       // Use Automatic address resolution, not standard messages
#define MQTTSN_REBOOT_ON_LOST           1       // Reboot on connection lost

// Object Dictionary
#define OD_DEV_SWVERSH                  '3'     // Software Version
#define OD_DEV_SWVERSM                  '1'
#define OD_DEV_SWVERSL                  '0'

#define OD_DEF_ADC_AVERAGE              80

#define UART_BaudRate                   38400
#define RF_BASE_FREQ                    868300000UL

// Include Hardware definitions
// Atmel
// ATM328P
#if (defined __AVR_MEGA__)
#include "HAL/HW_ATMega.h"
#elif (defined STM32F0XX_MD)
#include "HAL/HW_STM32F0.h"
#else
#error unknown uC Family
#endif

#include "HAL/HW_Common.h"

#include "mqMEM.h"
#include "mqTypes.h"
#include "mqttsn.h"
#include "ObjDict.h"

#if (defined PHY2_ADDR_t) && (defined ASLEEP)
#error Incompatible options ASLEEP for Gateway
#endif  //  

#ifdef __cplusplus
}
#endif

#endif  //  _CONFIG_H
