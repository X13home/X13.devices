/*
Copyright (c) 2013-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org

BSD New License
See LICENSE.txt file for license details.
 */ 

#ifndef __DHT_H_
#define __DHT_H_

void dht_start_conversion(uint8_t pin);
uint8_t dht_get_data(uint16_t * pBuf);

#endif  //  __DHT_H_
