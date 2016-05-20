/*
Copyright (c) 2011-2016 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// Extensions, serial port

#include "../config.h"

#ifdef EXTSER_USED

#define EXTSER_FLAG_TXEN    1
#define EXTSER_FLAG_RXEN    2
#define EXTSER_FLAG_RXRDY   0x10

typedef struct
{
    uint8_t   * pRxBuf;
    MQ_t      * pTxBuf;

    uint8_t     RxHead;
    uint8_t     RxTail;

    uint8_t     nBaud;
    uint8_t     flags;
}EXTSER_VAR_t;

static EXTSER_VAR_t * extSerV[EXTSER_USED] = {NULL,};

void serInit(void)
{
    uint8_t port;
    
    for(port = 0; port < EXTSER_USED; port++)
    {
        hal_uart_deinit(port);
        if(extSerV[port] != NULL)
        {
            if(extSerV[port]->pTxBuf != NULL)
                mqFree(extSerV[port]->pTxBuf);

            mqFree(extSerV[port]);
            extSerV[port] = NULL;
        }
    }
}

// Check Index
bool serCheckSubidx(subidx_t * pSubidx)
{
    uint8_t type = pSubidx->Type;
    uint8_t port = pSubidx->Base & 0x0F;
    uint8_t nBaud = pSubidx->Base >> 4;

    if((port >= EXTSER_USED) ||
       (nBaud > 4) ||
       ((type != ObjSerRx) && (type != ObjSerTx)))
        return false;

    return true;
}

// Read data
static e_MQTTSN_RETURNS_t serReadOD(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
    uint8_t port = pSubidx->Base & 0x0F;
    
    uint8_t size = *pLen;
    uint8_t pos = 0;
    uint8_t head = extSerV[port]->RxHead;
    uint8_t tail = extSerV[port]->RxTail;

    while((pos < size) && (tail != head))
    {
        *(pBuf++) = extSerV[port]->pRxBuf[tail++];
        if(tail >= sizeof(MQ_t))
            tail = 0;
        pos++;
    }

    extSerV[port]->RxTail = tail;
    *pLen = pos;
    return MQTTSN_RET_ACCEPTED;
}

// Write data
static e_MQTTSN_RETURNS_t serWriteOD(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf)
{
    uint8_t port = pSubidx->Base & 0x0F;

    // Paranoid test
    if(extSerV[port] == NULL)
        return MQTTSN_RET_REJ_CONG;

    if(Len > 0)
    {
        if((extSerV[port]->pTxBuf == NULL) && hal_uart_free(port))
        {
            MQ_t * pTxBuf = mqAlloc(sizeof(MQ_t));
            memcpy(pTxBuf->m.raw, pBuf, Len);
            hal_uart_send(port, Len, pTxBuf->m.raw);

            extSerV[port]->pTxBuf = pTxBuf;
        }
        else
            return MQTTSN_RET_REJ_CONG;
    }

    return MQTTSN_RET_ACCEPTED;
}

// Poll Procedure
static uint8_t serPollOD(subidx_t * pSubidx)
{
    uint8_t port = pSubidx->Base & 0x0F;
    
    // Paranoid test
    if(extSerV[port] == NULL)
        return 0;

    if(extSerV[port]->RxHead != extSerV[port]->RxTail)
    {
        if(extSerV[port]->flags & EXTSER_FLAG_RXRDY)
            return 1;
            
        extSerV[port]->flags |= EXTSER_FLAG_RXRDY;
    }
    
    return 0;
}

static void serProc(void)
{
    uint8_t port;

    for(port = 0; port < EXTSER_USED; port++)
    {
        if(extSerV[port] != NULL)
        {
            if(extSerV[port]->flags & EXTSER_FLAG_TXEN)
            {
                if((extSerV[port]->pTxBuf != NULL) && (hal_uart_free(port)))
                {
                    mqFree(extSerV[port]->pTxBuf);
                    extSerV[port]->pTxBuf = NULL;
                }
            }
            
            if(extSerV[port]->flags & EXTSER_FLAG_RXEN)
            {
                if(hal_uart_datardy(port))
                {
                    uint8_t tmphead = extSerV[port]->RxHead + 1;
                    if(tmphead >= sizeof(MQ_t))
                        tmphead = 0;

                    if(tmphead != extSerV[port]->RxTail)
                    {
                        extSerV[port]->pRxBuf[extSerV[port]->RxHead] = hal_uart_get(port);
                        extSerV[port]->RxHead = tmphead;
                        
                        uint8_t size;
                        
                        if(tmphead > extSerV[port]->RxTail)
                            size = tmphead - extSerV[port]->RxTail;
                        else
                            size = sizeof(MQ_t) - extSerV[port]->RxTail + tmphead;
                            
                        if(size < (sizeof(MQ_t)/2))
                            extSerV[port]->flags &= ~EXTSER_FLAG_RXRDY;
                        else
                            extSerV[port]->flags |= EXTSER_FLAG_RXRDY;
                    }
                }
            }
        }
    }
}

// Register Object
e_MQTTSN_RETURNS_t serRegisterOD(indextable_t *pIdx)
{
    uint8_t port = pIdx->sidx.Base & 0x0F;
    uint8_t nBaud = pIdx->sidx.Base >> 4;
    eObjTyp_t type = pIdx->sidx.Type;

    uint8_t TxPin, RxPin;
    hal_uart_get_pins(port, &RxPin, &TxPin);

    if(type == ObjSerTx)
    {
        if(dioCheckBase(TxPin) != 0)
            return MQTTSN_RET_REJ_INV_ID;
    }
    else
    {
        if(dioCheckBase(RxPin) == 2)
            return MQTTSN_RET_REJ_INV_ID;
    }

    if(extSerV[port] != NULL)
    {
        if(extSerV[port]->nBaud != nBaud)
            return MQTTSN_RET_REJ_INV_ID;
        
        if(type == ObjSerTx)
        {
            if(extSerV[port]->flags & EXTSER_FLAG_TXEN)
                return MQTTSN_RET_REJ_INV_ID;
        }
        else
        {
            if(extSerV[port]->flags & EXTSER_FLAG_RXEN)
                return MQTTSN_RET_REJ_INV_ID;
        }
    }
    else
    {
        extSerV[port] = mqAlloc(sizeof(EXTSER_VAR_t));
        extSerV[port]->nBaud = nBaud;
        extSerV[port]->flags = 0;
        extSerV[port]->pRxBuf = NULL;
        extSerV[port]->RxHead = 0;
        extSerV[port]->RxTail = 0;
        extSerV[port]->pTxBuf = NULL;
    }

    if(type == ObjSerTx)
    {
        extSerV[port]->flags |= EXTSER_FLAG_TXEN;

        pIdx->cbWrite = &serWriteOD;

        dioTake(TxPin);
        hal_uart_init_hw(port, nBaud, 2);
    }
    else // ObjSerRx
    {
        if(extSerV[port]->pRxBuf == NULL)
        {
            extSerV[port]->pRxBuf = mqAlloc(sizeof(MQ_t));
        }

        extSerV[port]->flags |= EXTSER_FLAG_RXEN;
        
        pIdx->cbRead = &serReadOD;
        pIdx->cbPoll = &serPollOD;
        
        dioTake(RxPin);
        hal_uart_init_hw(port, nBaud, 1);
    }
    
    extRegProc(&serProc);
    return MQTTSN_RET_ACCEPTED;
}

void serDeleteOD(subidx_t * pSubidx)
{
    uint8_t port = pSubidx->Base & 0x0F;
    
    uint8_t TxPin, RxPin;
    hal_uart_get_pins(port, &RxPin, &TxPin);

    if(extSerV[port] != NULL)
    {
        if(pSubidx->Type == ObjSerTx)
        {
            extSerV[port]->flags &= ~EXTSER_FLAG_TXEN;
        }
        else
        {
            extSerV[port]->flags &= ~EXTSER_FLAG_RXEN;

            if(extSerV[port]->pRxBuf != NULL)
            {
                mqFree(extSerV[port]->pRxBuf);
                extSerV[port]->pRxBuf = NULL;
            }
        }

        if((extSerV[port]->flags & (EXTSER_FLAG_TXEN | EXTSER_FLAG_RXEN)) == 0)
        {
            hal_uart_deinit(port);
            mqFree(extSerV[port]);
            extSerV[port] = NULL;
            
            dioRelease(TxPin);
            dioRelease(RxPin);
        }
    }
}
#endif    //  EXTSER_USED
