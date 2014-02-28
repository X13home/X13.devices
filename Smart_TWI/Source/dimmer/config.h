/*
Copyright (c) 2013-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org

BSD New License
See LICENSE.txt file for license details.
 */ 

#ifndef CONFIG_H_
#define CONFIG_H_

#define SDA     PB0
#define OUT2    PB1
#define SCL     PB2
#define ZCD     PB3
#define OUT1    PB4

//#define DHT_USE_WATCHDOG    1     // Enable watchdog on pool

#define EE_ADDR_I2CADDR     0
#define EE_ADDR_FADE        1

#define DEF_I2C_ADDR        11

#endif // CONFIG_H_
