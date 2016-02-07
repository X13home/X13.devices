/*
Copyright (c) 2011-2016 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef HW_STM32F1_H
#define HW_STM32F1_H

// STM32F103CBT6, Maple Mini
#if   (defined CFG_S3Sn10)
#include "S3xxxx/S3Sn10.h"    // UART
#elif (defined CFG_S3En10)
#include "S3xxxx/S3En10.h"    // ENC28J60
#else
#error Unknown configuration
#endif  //  Configuration

#endif  //  HW_STM32F1_H
