/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _EXTTWI_H
#define _EXTTWI_H

#ifdef __cplusplus
extern "C" {
#endif

// TWI Access Flags
#define TWI_WRITE       0x01    // Write access
#define TWI_READ        0x02    // Read access
#define TWI_WD_ARMED    0x04    // Watchdog started
#define TWI_BUSY        0x08    // Bus busy
#define TWI_RDY         0x10    // Access complete
#define TWI_WD          0x20    // Timeout
#define TWI_SLANACK     0x40    // Slave Addr NACK received
#define TWI_ERROR       0x80    // Unknown error


typedef struct
{
    uint8_t     address;
    uint8_t     access;
    uint8_t     write;
    uint8_t     read;
    uint8_t     data[];
} TWI_FRAME_t;

typedef struct sTWI_QUEUE
{
    struct  sTWI_QUEUE * pNext;
    TWI_FRAME_t frame;
}TWI_QUEUE_t;

void twiInit(void);

#ifdef __cplusplus
}
#endif

#endif  //  _EXTTWI_H
