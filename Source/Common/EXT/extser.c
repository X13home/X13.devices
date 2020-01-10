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
#define EXTSER_FLAG_EXT     4
#define EXTSER_FLAG_RXRDY   0x10

typedef struct
{
    uint8_t   * pRxBuf;
    uint8_t   * pTxBuf;

    uint8_t     RxHead;
    uint8_t     RxTail;

#ifdef EXTPLC_USED
    uint8_t     TxBusy;
    uint8_t     TxPnt;
#endif  //  EXTPLC_USED

    uint8_t     nBaud;
    uint8_t     flags;
}EXTSER_VAR_t;

static EXTSER_VAR_t extSerV[EXTSER_USED];

void serInit(void)
{
    uint8_t port;
    
    for(port = 0; port < EXTSER_USED; port++)
    {
        hal_uart_deinit(port);

        if(extSerV[port].pRxBuf != NULL)
        {
            mqFree(extSerV[port].pRxBuf);
            extSerV[port].pRxBuf = NULL;
        }

        if(extSerV[port].pTxBuf != NULL)
        {
            mqFree(extSerV[port].pTxBuf);
            extSerV[port].pTxBuf = NULL;
        }

        extSerV[port].flags = 0;
    }
}

// Check Index
bool serCheckSubidx(subidx_t * pSubidx)
{
    uint8_t type = pSubidx->Type;
    uint8_t port = pSubidx->Base & 0x0F;
    uint8_t nBaud = pSubidx->Base >> 4;

    if((port >= EXTSER_USED) ||
       (nBaud > UART_BAUD_MAX) ||
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
    uint8_t head = extSerV[port].RxHead;
    uint8_t tail = extSerV[port].RxTail;

    while((pos < size) && (tail != head))
    {
        *(pBuf++) = extSerV[port].pRxBuf[tail++];
        if(tail >= sizeof(MQ_t))
        {
            tail = 0;
        }
        pos++;
    }

    extSerV[port].RxTail = tail;
    *pLen = pos;
    return MQTTSN_RET_ACCEPTED;
}

// Write data
static e_MQTTSN_RETURNS_t serWriteOD(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf)
{
    uint8_t port = pSubidx->Base & 0x0F;

    if(Len > 0)
    {
        if((extSerV[port].pTxBuf == NULL) && hal_uart_free(port))
        {
            extSerV[port].pTxBuf = mqAlloc(sizeof(MQ_t));
            memcpy(extSerV[port].pTxBuf, pBuf, Len);
            hal_uart_send(port, Len, extSerV[port].pTxBuf);
        }
        else
        {
            return MQTTSN_RET_REJ_CONG;
        }
    }

    return MQTTSN_RET_ACCEPTED;
}

// Poll Procedure
static uint8_t serPollOD(subidx_t * pSubidx)
{
    uint8_t port = pSubidx->Base & 0x0F;

    if(extSerV[port].RxHead != extSerV[port].RxTail)
    {
        if(extSerV[port].flags & EXTSER_FLAG_RXRDY)
        {
            return 1;
        }

        extSerV[port].flags |= EXTSER_FLAG_RXRDY;
    }

    return 0;
}

static void serProc(void)
{
    uint8_t port;

    for(port = 0; port < EXTSER_USED; port++)
    {
        if(((extSerV[port].flags & EXTSER_FLAG_TXEN) != 0) &&
            (extSerV[port].pTxBuf != NULL) &&
            (hal_uart_free(port)))
        {
#ifdef EXTPLC_USED
            if(extSerV[port].TxPnt != 0)
            {
                hal_uart_send(port, extSerV[port].TxPnt, extSerV[port].pTxBuf);

                extSerV[port].TxBusy = 1;
                extSerV[port].TxPnt = 0;
            }
            else
            {
                extSerV[port].TxBusy = 0;
                mqFree(extSerV[port].pTxBuf);
                extSerV[port].pTxBuf = NULL;
            }
#else   // !PLC
            mqFree(extSerV[port].pTxBuf);
            extSerV[port].pTxBuf = NULL;
#endif  //  EXTPLC_USED
        }

        if((extSerV[port].flags & EXTSER_FLAG_RXEN) != 0)
        {
            if(hal_uart_datardy(port))
            {
                uint8_t RxHead = extSerV[port].RxHead;
                RxHead++;
                if(RxHead >= sizeof(MQ_t))
                {
                    RxHead = 0;
                }

                if(RxHead != extSerV[port].RxTail)
                {
                    extSerV[port].pRxBuf[extSerV[port].RxHead] = hal_uart_get(port);
                    extSerV[port].RxHead = RxHead;

                    uint8_t size;

                    if(RxHead > extSerV[port].RxTail)
                    {
                        size = RxHead - extSerV[port].RxTail;
                    }
                    else
                    {
                        size = sizeof(MQ_t) - extSerV[port].RxTail + RxHead;
                    }

                    if(size < (sizeof(MQ_t)/2))
                    {
                        extSerV[port].flags &= ~EXTSER_FLAG_RXRDY;
                    }
                    else
                    {
                        extSerV[port].flags |= EXTSER_FLAG_RXRDY;
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
        {
            return MQTTSN_RET_REJ_INV_ID;
        }
    }
    else
    {
        if(dioCheckBase(RxPin) == 2)
        {
            return MQTTSN_RET_REJ_INV_ID;
        }
    }

    if(extSerV[port].flags != 0)
    {
        if(extSerV[port].nBaud != nBaud)
        {
            return MQTTSN_RET_REJ_INV_ID;
        }

        if(type == ObjSerTx)
        {
            if(extSerV[port].flags & EXTSER_FLAG_TXEN)
            {
                return MQTTSN_RET_REJ_INV_ID;
            }
        }
        else
        {
            if(extSerV[port].flags & EXTSER_FLAG_RXEN)
            {
                return MQTTSN_RET_REJ_INV_ID;
            }
        }
    }
    else
    {
        extSerV[port].pRxBuf = NULL;
        extSerV[port].pTxBuf = NULL;
        extSerV[port].RxHead = 0;
        extSerV[port].RxTail = 0;
#ifdef EXTPLC_USED
        extSerV[port].TxBusy = 0;
        extSerV[port].TxPnt = 0;
#endif  //  EXTPLC_USED
        extSerV[port].nBaud = nBaud;
        extSerV[port].flags = EXTSER_FLAG_EXT;
    }

    if(type == ObjSerTx)
    {
        extSerV[port].flags |= EXTSER_FLAG_TXEN;

        pIdx->cbWrite = &serWriteOD;

        dioTake(TxPin);
        hal_uart_init_hw(port, nBaud, 2);
    }
    else // ObjSerRx
    {
        if(extSerV[port].pRxBuf == NULL)
        {
            extSerV[port].pRxBuf = mqAlloc(sizeof(MQ_t));
        }

        extSerV[port].flags |= EXTSER_FLAG_RXEN;
        
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

    if((extSerV[port].flags & EXTSER_FLAG_EXT) != 0)
    {
        uint8_t TxPin, RxPin;
        hal_uart_get_pins(port, &RxPin, &TxPin);

        if(pSubidx->Type == ObjSerTx)
        {
            extSerV[port].flags &= ~EXTSER_FLAG_TXEN;
            dioRelease(TxPin);
        }
        else
        {
            extSerV[port].flags &= ~EXTSER_FLAG_RXEN;
            dioRelease(RxPin);

            if(extSerV[port].pRxBuf != NULL)
            {
                mqFree(extSerV[port].pRxBuf);
                extSerV[port].pRxBuf = NULL;
            }
        }

        if((extSerV[port].flags & (EXTSER_FLAG_TXEN | EXTSER_FLAG_RXEN)) == 0)
        {
            hal_uart_deinit(port);
            extSerV[port].flags = 0;
        }
    }
}


#ifdef EXTPLC_USED
void serInitPlc(uint32_t port, uint32_t speed)
{
    if((port >= EXTSER_USED) ||     // Port Not Exist
       (speed > 5))                 // Speed Not Exist
    {
        plcException(PLC_ANSWER_ERROR_PARM);
        return;
    }

    if((extSerV[port].flags & EXTSER_FLAG_EXT) != 0)
    {
        plcException(PLC_ANSWER_ERROR_RSC_BUSY);
        return;
    }

    if(extSerV[port].flags != 0)
    {
        if(extSerV[port].nBaud != speed)
        {
            plcException(PLC_ANSWER_ERROR_PARM);
        }
        return;
    }

    uint8_t TxPin, RxPin;
    hal_uart_get_pins(port, &RxPin, &TxPin);
    if((dioCheckBase(TxPin) != 0) || dioCheckBase(RxPin) == 2)  // Pins Busy
    {
        plcException(PLC_ANSWER_ERROR_RSC_BUSY);
        return;
    }

    extSerV[port].pRxBuf = mqAlloc(sizeof(MQ_t));
    extSerV[port].pTxBuf = NULL;
    extSerV[port].RxHead = 0;
    extSerV[port].RxTail = 0;
    extSerV[port].TxBusy = 0;
    extSerV[port].TxPnt = 0;
    extSerV[port].nBaud = speed;
    extSerV[port].flags = EXTSER_FLAG_TXEN | EXTSER_FLAG_RXEN;

    hal_uart_init_hw(port, speed, 3);

    extRegProc(&serProc);
}

uint32_t serBytes2ReadPlc(uint32_t port)
{
    if(port >= EXTSER_USED)         // Port Not Exist
    {
        plcException(PLC_ANSWER_ERROR_PARM);
        return 0xFFFFFFF0;
    }

    if((extSerV[port].flags & (EXTSER_FLAG_EXT | EXTSER_FLAG_RXEN)) != EXTSER_FLAG_RXEN)
    {
        plcException(PLC_ANSWER_ERROR_RSC_BUSY);
        return 0xFFFFFFFE;
    }

    uint8_t RxHead = extSerV[port].RxHead;
    uint8_t RxTail = extSerV[port].RxTail;

    if(RxHead != RxTail)
    {
        if(extSerV[port].flags & EXTSER_FLAG_RXRDY)
        {
            if(RxHead > RxTail)
            {
                return (RxHead - RxTail);
            }
            else
            {
                return (sizeof(MQ_t) - RxTail + RxHead);
            }
        }
        extSerV[port].flags |= EXTSER_FLAG_RXRDY;
    }

    return 0;
}

uint32_t serGetBytePlc(uint32_t port)
{
    if(port >= EXTSER_USED)             // Port Not Exist
    {
        plcException(PLC_ANSWER_ERROR_PARM);
        return 0xFFFFFFF0;
    }

    if((extSerV[port].flags & (EXTSER_FLAG_EXT | EXTSER_FLAG_RXEN)) != EXTSER_FLAG_RXEN)
    {
        plcException(PLC_ANSWER_ERROR_RSC_BUSY);
        return 0xFFFFFFFE;
    }

    uint8_t RxTail = extSerV[port].RxTail;
    if(extSerV[port].RxHead == RxTail)
    {
        return 0xFFFFFFFF;
    }

    uint32_t retval = extSerV[port].pRxBuf[RxTail++];

    if(RxTail >= sizeof(MQ_t))
    {
        RxTail = 0;
    }

    extSerV[port].RxTail = RxTail;

    return retval;
}

uint32_t serPutBytePlc(uint32_t port, uint32_t data)
{
    if(port >= EXTSER_USED)                         // Port Not Exist
    {
        plcException(PLC_ANSWER_ERROR_PARM);
        return 0xFFFFFFF0;
    }

    if((extSerV[port].flags & (EXTSER_FLAG_EXT | EXTSER_FLAG_TXEN)) != EXTSER_FLAG_TXEN)
    {
        plcException(PLC_ANSWER_ERROR_RSC_BUSY);
        return 0xFFFFFFFE;
    }

    if(extSerV[port].pTxBuf == NULL)
    {
        extSerV[port].pTxBuf = (uint8_t *)mqAlloc(sizeof(MQ_t));
        extSerV[port].TxBusy = 0;
        extSerV[port].TxPnt  = 0;
    }

    if(extSerV[port].TxBusy == 0)
    {
        uint8_t TxPnt = extSerV[port].TxPnt;
        if(TxPnt < sizeof(MQ_t))
        {
            extSerV[port].pTxBuf[TxPnt++] = data;
            extSerV[port].TxPnt = TxPnt;
            return TxPnt;
        }
    }

    return 0xFFFFFFFF;
}
#endif  //  EXTPLC_USED

#endif  //  EXTSER_USED
