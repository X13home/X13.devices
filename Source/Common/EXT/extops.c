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

#define SI1143_ADDR     0x5A
/*
#define ENTER_LEFT      0x01
#define ENTER_RIGHT     0x02
#define ENTER_BOTTOM    0x04
#define ENTER_TOP       0x08
#define ENTER_CENTER    0x0F
#define LEAVE_LEFT      0x10
#define LEAVE_RIGHT     0x20
#define LEAVE_BOTTOM    0x40
#define LEAVE_TOP       0x80
#define LEAVE_CENTER    0xF0
#define OPS_HW_ERROR    0x03
*/

/*
typedef struct
{
    uint32_t        ms;
    TWI_QUEUE_t   * pTWI;
}OPS_QUEUE_t;
*/

static const uint8_t ops_initSeq[] = {
    0x0F, 0x03, 0x03,       // turn on interrupts
          0x10,             // turn on interrupt on PS3
          0x00,
          0x01,             // interrupt on ps3 measurement
          0x17,             //HW_KEY - The system must write the value 0x17 to this register for proper Si114x operation.
          0x6B,             // The device wakes up every 3 1/3 ms (0x6C x 31.25 µs)
          0x32,             // ALS Measurements made every 10 times the device wakes up.
          0x08,             // PS Measurements made every time the device wakes up
          0xFF, 0xFF,
          0xFF, 0xFF,
          0x77,             // LED current for LEDs 1 (red) & 2 (IR1)
          0x07,             // LED current for LED 3 (IR2)
    0x03, 0x17, 0x77, 0xA1, // PARAM_CH_LIST - all measurements on
    0x03, 0x17, 0x21, 0xA2, // PARAM_PSLED12_SELECT - select LEDs on
    0x03, 0x17, 0x04, 0xA3, // PARAM_PSLED3_SELECT - 3 only
    0x03, 0x17, 0x03, 0xA7, // PARAM_PS1_ADCMUX - PS1 photodiode select
    0x03, 0x17, 0x03, 0xA8, // PARAM_PS2_ADCMUX - PS2 photodiode select
    0x03, 0x17, 0x03, 0xA9, // PARAM_PS3_ADCMUX - PS3 photodiode select
    0x03, 0x17, 0x70, 0xAA, // PARAM_PS_ADC_COUNTER - is default
    0x03, 0x17, 0x00, 0xAB, // PARAM_PS_ADC_GAIN -
    0x02, 0x18, 0x0F        // starts an autonomous read loop
};

//static volatile OPS_QUEUE_t ops_data[4];
//static volatile uint8_t     ops_head = 0;
//static uint8_t              ops_tail = 0;

static int8_t ops_state;

volatile TWI_QUEUE_t * pTWI = NULL;

static void opsProc(void)
{
    // ToDo Watchdog
    if(pTWI->frame.access & TWI_BUSY)
        return;

    if(pTWI->frame.access & (TWI_ERROR | TWI_SLANACK))
    {
        ops_state = -100;
        pTWI->frame.access = 0;
        return;
    }
    
    if(ops_state < 0)
    {
        ops_state++;
    }
    else if(ops_state < (int8_t)sizeof(ops_initSeq))
    {
        uint8_t len = ops_initSeq[ops_state++];
        pTWI->frame.address = SI1143_ADDR;
        pTWI->frame.access  = TWI_WRITE;
        pTWI->frame.write   = len;
        pTWI->frame.read    = 0;
        
        memcpy((void *)(pTWI->frame.data), &ops_initSeq[ops_state], len);
        ops_state += len;
        hal_twi_start();
    }
    else
    {
        pTWI->frame.address = SI1143_ADDR;
        pTWI->frame.access  = (TWI_WRITE | TWI_READ);
        pTWI->frame.write   = 1;
        pTWI->frame.read    = 11;       // Read 11 bytes
        pTWI->frame.data[0] = 0x21;     // Start read from register 21
        hal_twi_start();
    }
}

///////////////////////////////////////////////////////////////////////
// OPS API
void opsInit(void)
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

    ops_state = -100;
    
    pTWI = mqAlloc(sizeof(MQ_t));
    pTWI->frame.access = 0;
    
    extRegProc(&opsProc);
}
#endif  //  EXT_OPS_USED
