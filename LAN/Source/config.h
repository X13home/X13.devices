/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

// Global configuration settings

#ifndef _CONFIG_H
#define _CONFIG_H

// Memory Manager
#define MQMEM_SIZEOF_QUEUE      10      // Allocated memory buffers

// MQTT-S Section
#define MQTTS_MSG_SIZE          30      // Size of payload(base for all buffers)
#define MQTTS_SIZEOF_SEND_FIFO  4       // Size of MQTTS Send Buffer
#define MQTTS_SIZEOF_POOL_FIFO  4       // Size of MQTTS Pool Buffer

// Object Dictionary
#define OD_MAX_INDEX_LIST       12      // Size of identificators list
#define OD_DEFAULT_TASLEEP      0       // Sleep Time, default - always online.

#define OD_DEV_SWVERSH          '0'     // Software Version
#define OD_DEV_SWVERSM          '0'
#define OD_DEV_SWVERSL          '1'

#if (defined ENC28J60)
 #include "HWconfigENC.h"
#endif  //  (defined ENC28J60)

#include "mqtts.h"
#include "mqMEM.h"
/*
#include "objdict.h"
#include "rf.h"
*/
#include "phy.h"

#endif  // _CONFIG_H
