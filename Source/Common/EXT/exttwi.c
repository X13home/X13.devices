/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// Extensions TWI/I2C

#include "../config.h"

#ifdef EXTTWI_USED

#define TWIM_BUS_TIMEOUT            250 // ms

// Global variable used in HAL
volatile TWI_QUEUE_t  * pTwi_exchange = NULL;

#ifndef EXTPLC_USED

// local queues
static Queue_t  twi_tx_queue = {NULL, NULL, 4, 0};      // Max Size = 4 records

static e_MQTTSN_RETURNS_t twiReadOD(subidx_t * pSubidx __attribute__ ((unused)), uint8_t *pLen, uint8_t *pBuf)
{
    if(pTwi_exchange == NULL)
        return MQTTSN_RET_REJ_CONG;

    *pLen = pTwi_exchange->frame.read + sizeof(TWI_FRAME_t);
    memcpy(pBuf, (void *)&pTwi_exchange->frame, *pLen);
    mqFree((void *)pTwi_exchange);
    pTwi_exchange = NULL;
    return MQTTSN_RET_ACCEPTED;
}

static e_MQTTSN_RETURNS_t twiWriteOD(subidx_t * pSubidx __attribute__ ((unused)), uint8_t Len, uint8_t *pBuf)
{
    if(Len < sizeof(TWI_FRAME_t))
        return MQTTSN_RET_REJ_NOT_SUPP;

    TWI_QUEUE_t * pQueue = mqAlloc(sizeof(MQ_t));

    memcpy(&pQueue->frame, pBuf, Len);
    
    pQueue->frame.access &= (TWI_WRITE | TWI_READ);
    pQueue->frame.write = (Len - sizeof(TWI_FRAME_t));
    if(pQueue->frame.write != 0)
    {
        pQueue->frame.access |= TWI_WRITE;
    }
    if(pQueue->frame.read != 0)
    {
        pQueue->frame.access |= TWI_READ;
    }

    if((pQueue->frame.read > (MQTTSN_MSG_SIZE - sizeof(TWI_FRAME_t) - MQTTSN_SIZEOF_MSG_PUBLISH)) ||
       ((pQueue->frame.access & (TWI_WRITE | TWI_READ)) == 0))
    {
        mqFree(pQueue);
        return MQTTSN_RET_REJ_NOT_SUPP;
    }

    if(!mqEnqueue(&twi_tx_queue, pQueue))
    {
        mqFree(pQueue);
        return MQTTSN_RET_REJ_CONG;
    }

    return MQTTSN_RET_ACCEPTED;
}

static uint8_t twiPollOD(subidx_t * pSubidx __attribute__ ((unused)))
{
    if(pTwi_exchange != NULL)
    {
        uint8_t access = pTwi_exchange->frame.access;
        if((access & (TWI_ERROR | TWI_SLANACK | TWI_WD)) != 0)      // Error state
        {
            return 1;
        }
        else if(access & TWI_RDY)
        {
            if(pTwi_exchange->frame.read != 0)
            {
                return 1;
            }
            else
            {
                mqFree((void *)pTwi_exchange);
                pTwi_exchange = NULL;
            }
        }
        else
        {
            static uint16_t twi_ms = 0;
            uint16_t act_ms = HAL_get_ms() & 0x0000FFFF;

            if((access & TWI_WD_ARMED) == 0)
            {
                pTwi_exchange->frame.access |= TWI_WD_ARMED;
                twi_ms = act_ms;
            }
            else if((act_ms - twi_ms) > TWIM_BUS_TIMEOUT)
            {
                if(access & TWI_BUSY)
                    hal_twi_stop();
                
                pTwi_exchange->frame.access |= TWI_WD;
                return 1;
            }

            if((access & TWI_BUSY) == 0)
                hal_twi_start();
        }
    }
    else if(twi_tx_queue.Size != 0)
    {
        pTwi_exchange = mqDequeue(&twi_tx_queue);
    }

    return 0;
}

#endif  //  EXTPLC_USED
void twiInit()
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

    if(pTwi_exchange != NULL)
    {
        mqFree((void *)pTwi_exchange);
        pTwi_exchange = NULL;
    }

#ifndef EXTPLC_USED
    // Register variable Ta0
    indextable_t * pIndex = getFreeIdxOD();
    if(pIndex == NULL)
    {
        dioRelease(scl);
        dioRelease(sda);
        hal_twi_configure(0);
        return;
    }

    pIndex->cbRead     = &twiReadOD;
    pIndex->cbWrite    = &twiWriteOD;
    pIndex->cbPoll     = &twiPollOD;
    pIndex->sidx.Place = objTWI;        // TWI object
    pIndex->sidx.Type  = objArray;      // Variable Type -  Byte Array
    pIndex->sidx.Base  = 0;             // Device address
#endif  //  EXTPLC_USED
}

#ifdef EXTPLC_USED
static uint8_t twi_pnt = 0;

void twiControl(uint32_t ctrl)
{
    if(pTwi_exchange != NULL)
        return;
    
    pTwi_exchange = mqAlloc(sizeof(MQ_t));
    if(pTwi_exchange == NULL)
        return;
    
    twi_pnt = 0;
    
    memcpy((void *)&pTwi_exchange->frame, &ctrl, 4);

    pTwi_exchange->frame.access &= (TWI_WRITE | TWI_READ);

    if(pTwi_exchange->frame.read != 0)
        pTwi_exchange->frame.access |= TWI_READ;    

    if(pTwi_exchange->frame.write != 0)
        pTwi_exchange->frame.access |= TWI_WRITE;
    else
        hal_twi_start();
}

uint32_t twiStat(void)
{
    if(pTwi_exchange == NULL)
        return 0;
    
    uint8_t access = pTwi_exchange->frame.access;

    if((access & (TWI_RDY | TWI_WD | TWI_SLANACK | TWI_ERROR)) == 0)
    {
        static uint16_t twi_ms = 0;
        uint16_t act_ms = HAL_get_ms() & 0x0000FFFF;

        if((access & TWI_WD_ARMED) == 0)
        {
            pTwi_exchange->frame.access |= TWI_WD_ARMED;
            twi_ms = act_ms;
        }
        else if((act_ms - twi_ms) > TWIM_BUS_TIMEOUT)
        {
            if(pTwi_exchange->frame.access & TWI_BUSY)
                hal_twi_stop();
            
            pTwi_exchange->frame.access |= TWI_WD;
            access |= TWI_WD;
        }
    }

    uint32_t retval;
    memcpy(&retval, (void *)&pTwi_exchange->frame, 4);
    
    if(access & (TWI_WD | TWI_SLANACK | TWI_ERROR))     // Error state
    {
        mqFree((void *)pTwi_exchange);
        pTwi_exchange = NULL;
    }
    else if(access & TWI_RDY)
    {
        twi_pnt = 0;
        if(pTwi_exchange->frame.read == 0)              // Bus Free
        {
            mqFree((void *)pTwi_exchange);
            pTwi_exchange = NULL;
        }
    }
    
    return retval;
}

void twiWr(uint8_t data)
{
    if( (pTwi_exchange == NULL) || 
       ((pTwi_exchange->frame.access & (TWI_BUSY | TWI_RDY | TWI_WD | TWI_SLANACK | TWI_ERROR)) != 0))
        return;
    
    if(twi_pnt < pTwi_exchange->frame.write)
    {
        pTwi_exchange->frame.data[twi_pnt++] = data;
        if(twi_pnt == pTwi_exchange->frame.write)
            hal_twi_start();
    }
}

uint8_t twiRd(void)
{
    uint8_t retval = 0;
    
    if((pTwi_exchange != NULL) && (pTwi_exchange->frame.access & TWI_RDY))
    {
        if(twi_pnt < pTwi_exchange->frame.read)
            retval = pTwi_exchange->frame.data[twi_pnt++];
        
        if(twi_pnt == pTwi_exchange->frame.read)
        {
            mqFree((void *)pTwi_exchange);
            pTwi_exchange = NULL;
        }
    }

    return retval;
}

#endif  //  EXTPLC_USED


#endif    //  EXTTWI_USED
