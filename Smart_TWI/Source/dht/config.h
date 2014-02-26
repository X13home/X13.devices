/*
Copyright (c) 2013-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org

BSD New License
See LICENSE.txt file for license details.
 */ 

#ifndef CONFIG_H_
#define CONFIG_H_

#define SENS3     PB1
#define SENS1     PB3
#define SENS2     PB4

/*
#define PORT_USI  PORTB
#define DDR_USI   DDRB
#define PIN_USI   PINB

#define DHT_DDR   DDRB
#define DHT_PORT  PORTB
#define DHT_PIN   PINB
*/

//#define DHT_USE_WATCHDOG    1     // Enable watchdog on pool

#define DHT_REQ_PER         20    // Time between requests, T = Val*0,25, 20 - 5Sec

#define EE_ADDR_I2CADDR     0
#define DEF_I2C_ADDR        11

/*
// Smart Version
#define CFG_VENDOR_ID       1
#define CFG_DEVICE_ID_L     1
#define CFG_DEVICE_ID_H     0
#define CFG_REVISION_ID     0
*/

#endif // CONFIG_H_
