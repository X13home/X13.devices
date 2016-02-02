/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// Extensions digital inputs/outputs

#include "../config.h"

#ifdef EXTDIO_USED

#ifndef EXTDIO_PORT_OFFSET
#define EXTDIO_PORT_OFFSET 0
#endif  //  EXTDIO_PORT_OFFSET

// DIO Variables

static DIO_PORT_TYPE dio_write_mask[EXTDIO_MAXPORT_NR];
static DIO_PORT_TYPE dio_read_mask[EXTDIO_MAXPORT_NR];
static DIO_PORT_TYPE dio_state_flag[EXTDIO_MAXPORT_NR];
static DIO_PORT_TYPE dio_change_flag[EXTDIO_MAXPORT_NR];
static DIO_PORT_TYPE dio_status[EXTDIO_MAXPORT_NR];

// DIO local subroutines

static void dioPin2hw(uint8_t pin, uint8_t *pPort, DIO_PORT_TYPE *pMask)
{
    uint8_t port = (pin >> DIO_PORT_POS);
#if (EXTDIO_PORT_OFFSET > 0)
    port -= EXTDIO_PORT_OFFSET;
#endif  //  EXTDIO_PORT_OFFSET
    *pPort = port;
    *pMask = (DIO_PORT_TYPE)1 << (pin & DIO_PORT_MASK);
}

// ignore some GCC warnings
#if defined ( __GNUC__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
// Write DIO Object's
static e_MQTTSN_RETURNS_t dioWriteOD(subidx_t * pSubidx, uint8_t unused, uint8_t *pBuf)
{
    uint8_t port;
    DIO_PORT_TYPE mask;
    uint8_t pin = hal_dio_base2pin(pSubidx->Base);
    dioPin2hw(pin, &port, &mask);

    uint8_t state = *pBuf;
    dio_change_flag[port] &= ~mask;

    if(pSubidx->Type == objPinNPN)
        state = ~state;

    if(state & 1)
        dio_status[port] |= mask;
    else
        dio_status[port] &= ~mask;

    return MQTTSN_RET_ACCEPTED;
}
#if defined ( __GNUC__ )
#pragma GCC diagnostic pop
#endif

// Poll Procedure
static uint8_t dioPollOD(subidx_t * pSubidx)
{
    uint8_t port;
    DIO_PORT_TYPE mask;
    uint8_t pin = hal_dio_base2pin(pSubidx->Base);
    dioPin2hw(pin, &port, &mask);

    return ((dio_change_flag[port] & mask) != 0);
}

// Read digital Inputs
static e_MQTTSN_RETURNS_t dioReadOD(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
    uint8_t port;
    DIO_PORT_TYPE mask;
    uint8_t pin = hal_dio_base2pin(pSubidx->Base);
    dioPin2hw(pin, &port, &mask);

    DIO_PORT_TYPE state = dio_status[port];
    dio_change_flag[port] &= ~mask;
  
    if(pSubidx->Type == objPinNPN)
        state = ~state;
    *pLen = 1;
    *pBuf = ((state & mask) != 0) ? 1 : 0;
    return MQTTSN_RET_ACCEPTED;
}

///////////////////////////////////////////////////////////////////////
// DIO API

// Preinit digital IO
void dioInit()
{
    uint8_t port;
    for(port = 0; port < EXTDIO_MAXPORT_NR; port++)
    {
        dio_write_mask[port] = 0;
        dio_read_mask[port] = 0;
        dio_state_flag[port] = 0;
        dio_change_flag[port] = 0;
        dio_status[port] = 0;
    }
}

// Check Subindex digital input/output
bool dioCheckSubidx(subidx_t * pSubidx)
{
    return (((pSubidx->Type == objPinNPN) || (pSubidx->Type == objPinPNP)) &&
            (hal_dio_base2pin(pSubidx->Base) != 0xFF));
}

// Register digital inp/out Object
e_MQTTSN_RETURNS_t dioRegisterOD(indextable_t *pIdx)
{
    uint8_t port;
    DIO_PORT_TYPE mask;
    uint8_t pin = hal_dio_base2pin(pIdx->sidx.Base);
    dioPin2hw(pin, &port, &mask);

    if((dio_read_mask[port] & mask) || (dio_write_mask[port] & mask))
        return MQTTSN_RET_REJ_INV_ID; // Port busy

    dio_change_flag[port] &= ~mask;

    if(pIdx->sidx.Place == objDout)
    {
        pIdx->cbWrite = &dioWriteOD;
        hal_dio_configure(pin, DIO_MODE_OUT_PP);
        dio_write_mask[port] |= mask;

        if(pIdx->sidx.Type == objPinPNP)
        {
            hal_dio_reset(port, mask);
            dio_status[port] &= ~mask;
        }
        else    // NPN
        {
            hal_dio_set(port, mask);
            dio_status[port] |= mask;
        }
    }
    else
    {
        pIdx->cbRead = &dioReadOD;
        pIdx->cbWrite = &dioWriteOD;
        pIdx->cbPoll = &dioPollOD;
        dio_read_mask[port] |= mask;

        if(pIdx->sidx.Type == objPinPNP)
        {
            hal_dio_configure(pin, DIO_MODE_IN_PD);
            dio_state_flag[port] &= ~mask;
            dio_status[port] &= ~mask;
        } // else NPN
        else
        {
            hal_dio_configure(pin, DIO_MODE_IN_PU);
            dio_state_flag[port] |= mask;
            dio_status[port] |= mask;
        }
    }

    return MQTTSN_RET_ACCEPTED;
}

void dioDeleteOD(subidx_t * pSubidx)
{
    uint8_t port;
    DIO_PORT_TYPE mask;
    uint8_t pin = hal_dio_base2pin(pSubidx->Base);
    dioPin2hw(pin, &port, &mask);
  
    dio_write_mask[port] &= ~mask;
    dio_read_mask[port] &= ~mask;

    hal_dio_configure(pin, DIO_MODE_IN_FLOAT);
}

void dioProc(void)
{
    uint8_t port, port_hal = EXTDIO_PORT_OFFSET;
    DIO_PORT_TYPE state, mask;

    for(port = 0; port < EXTDIO_MAXPORT_NR; port++)
    {
        mask = dio_read_mask[port] & ~dio_write_mask[port];
        if(mask != 0)
        {
            state = hal_dio_read(port_hal) & mask;

            DIO_PORT_TYPE maskp = 1;
            while(maskp)
            {
                DIO_PORT_TYPE maski = mask & maskp;
                if(maski)
                {
                    if((dio_state_flag[port] ^ state) & maski)
                    {
                        dio_state_flag[port] &= ~maski;
                        dio_state_flag[port] |= state & maski;
                    }
                    else
                    {
                        DIO_PORT_TYPE staterd =  ((dio_status[port] ^ state) & maski);
                        if(staterd)
                        {
                            dio_status[port] ^= staterd;
                            dio_change_flag[port] |= staterd;
                        }
                    }
                }
                maskp <<= 1;
            }
        }

        mask = dio_write_mask[port] & ~dio_read_mask[port];
        if(mask != 0)
        {
            state = dio_status[port] & mask;
            if(state)
                hal_dio_set(port_hal, state);
        
            state = ~dio_status[port] & mask;
            if(state)
                hal_dio_reset(port_hal, state);
        }

        port_hal++;
    }
}

// Is Pin free
uint8_t dioCheckBase(uint8_t pin)
{
    uint8_t port;
    DIO_PORT_TYPE mask;
    dioPin2hw(pin, &port, &mask);

    if((dio_read_mask[port] & mask) || (dio_write_mask[port] & mask))
        return 1; // Port busy

    return 0; // Port free
}

// Take Pin
void dioTake(uint8_t pin)
{
    uint8_t port;
    DIO_PORT_TYPE mask;
    dioPin2hw(pin, &port, &mask);

    dio_write_mask[port] |= mask;
    dio_read_mask[port] |= mask;
}

// Release Pin
void dioRelease(uint8_t pin)
{
    uint8_t port;
    DIO_PORT_TYPE mask;
    dioPin2hw(pin, &port, &mask);

    dio_write_mask[port] &= ~mask;
    dio_read_mask[port] &= ~mask;
}

#ifdef EXTPLC_USED
bool dioRead(subidx_t * pSubidx)
{
    uint8_t pin = hal_dio_base2pin(pSubidx->Base);
    if(pin == 0xFF)
        return false;

    uint8_t port;
    DIO_PORT_TYPE mask;
    dioPin2hw(pin, &port, &mask);
    
    bool sr = (dio_status[port] & mask) != 0;
    if(pSubidx->Type == objPinNPN)
        sr = !sr;
    return sr;
}

void dioWrite(subidx_t * pSubidx, bool sr)
{
    uint8_t pin = hal_dio_base2pin(pSubidx->Base);
    if(pin == 0xFF)
        return;

    uint8_t port;
    DIO_PORT_TYPE mask;
    dioPin2hw(pin, &port, &mask);

    if(pSubidx->Type == objPinNPN)
        sr = !sr;

    if(sr)
        dio_status[port] |= mask;
    else
        dio_status[port] &= ~mask;
}
#endif  //  EXTPLC_USED

#endif  //  EXTDIO_USED
