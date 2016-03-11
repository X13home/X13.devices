/*
Copyright (c) 2011-2016 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// Sensor Si1143

#include "../config.h"

#ifdef EXT_OPS_USED

#include "exttwi.h"

volatile TWI_QUEUE_t * pTWI;

typedef struct
{
    uint32_t        ms;
    TWI_QUEUE_t   * pTWI;
}OPS_QUEUE_t;

//static volatile OPS_QUEUE_t ops_data[4];
//static volatile uint8_t     ops_head = 0;
//static uint8_t              ops_tail = 0;

static void opsProc(void)
{

}

///////////////////////////////////////////////////////////////////////
// OPS API
void opsInit()
{
    uint8_t scl, sda;

    hal_twi_get_pins(&scl, &sda);
    dioTake(scl);
    dioTake(sda);

    if(!hal_twi_configure(1))           // Enable
    {
        dioRelease(scl);
        dioRelease(sda);
        return;
    }
    
    
    
    
    extRegProc(&opsProc);
}
#endif  //  EXT_OPS_USED
