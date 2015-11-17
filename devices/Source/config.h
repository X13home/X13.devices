/*
Copyright (c) 2011-2014 <comparator@gmx.de>

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

// MQTT-SN Section
#define MQTTSN_MSG_SIZE         48      // Size of payload(base for all buffers)
#define MQTTSN_USE_DHCP         1       // Use Automatic address resolution

// Object Dictionary
#define OD_MAX_INDEX_LIST       16      // Size of identificators list
#define OD_DEV_SWVERSH          '3'     // Software Version
#define OD_DEV_SWVERSM          '0'
#define OD_DEV_SWVERSL          '2'

#define POLL_TMR_FREQ           100     // System Tick Period

#define UART_BaudRate           38400
#define RF_BASE_FREQ            868300000UL

//Optional modules
//#define EXTDIO_USED             1       // Use DIO
//#define EXTPWM_USED             1       // Use PWM
//#define EXTAIN_USED             1       // Use Analog inputs
//#define EXTTWI_USED             1       // Use TWI/I2C Devices
//#define EXTSER_USED             1       // Number of used serial ports
//#define EXTPLC_USED             1       // Internal PLC

//#define DIAG_USED               1       // Enable diagnostic messages

#define ASLEEP                  1       // Enable ASleep mode
#define OD_DEFAULT_TASLEEP      0

// Include Hardware definitions
// Atmel
// ATM328P
#if (defined __AVR_MEGA__)
#include "HAL/HW_ATMega.h"
#elif (defined STM32F0XX_MD)
#include "HAL/HW_STM32F0.h"
#elif (defined STM32F10X_MD)
#include "HAL/HW_STM32F1.h"
#elif (defined STM32F4XX)
#include "HAL/HW_STM32F4.h"
#else
#error unknown uC Family
#endif

#include "HAL/HW_Common.h"

#include "mqMEM.h"
#include "mqTypes.h"
#include "mqttsn.h"
#include "ObjDict.h"

#ifdef __cplusplus
}
#endif

#endif  //  _CONFIG_H
