/*
Copyright (c) 2011-2012 <comparator@gmx.de>

This file is part of the X13.Home project.
https://github.com/X13home

BSD License
See LICENSE.txt file for license details.
*/

// Serial input/output

// Local Variables
static uint8_t serCurrSpeed;
#ifdef EXTSER_TX_USED
// TX Section
static uint8_t serTxBuf[MQTTS_MSG_SIZE];        // Tx Buffer
volatile static uint8_t serTxPos;               // Current Tx position
volatile static uint8_t serTxLen;               // Bytes to send
#endif  //  EXTSER_TX_USED
#ifdef  EXTSER_RX_USED
// RX Section
static uint8_t serRxPosOld;                     // Old Rx Position
volatile static uint8_t serRxPos;               // Current Rx position
volatile static uint8_t serRxBuf[MQTTS_MSG_SIZE];        // Rx Buffer
#endif  //  EXTSER_RX_USED

#define SER_MAX_BASE            4
const PROGMEM uint16_t ser_Speed_List[] = {((F_CPU/16/2400) - 1), ((F_CPU/16/4800) - 1), 
                                           ((F_CPU/16/9600) - 1), ((F_CPU/16/19200) - 1),
                                           ((F_CPU/16/38400) - 1)};
#ifdef EXTSER_TX_USED
// Start HAL
ISR(USART_UDRE_vect)
{
    // Check if all data is transmitted
    if(serTxPos < serTxLen)
        USART_DATA = serTxBuf[serTxPos++];
    else
    {
        serTxLen = 0;
        USART_DISABLE_DREINT();                 // Disable UDRE interrupt
    }
}
#endif  //  EXTSER_TX_USED

#ifdef EXTSER_RX_USED
ISR(USART_RX_vect)
{
    uint8_t data = USART_DATA;
    if(serRxPos < (MQTTS_MSG_SIZE -1))
        serRxBuf[++serRxPos] = data;
}
#endif  //  EXTSER_RX_USED
// End HAL

static void serClean(void)
{
    SER_DISABLE_RX();
    SER_DISABLE_TX();

    serCurrSpeed = 0xFF;
#ifdef EXTSER_TX_USED
    serTxPos = 0;
    serTxLen = 0;
#endif  //  EXTSER_TX_USED
#ifdef EXTSER_RX_USED
    serRxPosOld = 0;
    serRxPos = 0;
#endif  //  EXTSER_RX_USED
}

static uint8_t serCheckIdx(subidx_t * pSubidx)
{
    uint8_t type = pSubidx->Type;
    if((pSubidx->Base > SER_MAX_BASE) || ((type != ObjSerRx) && (type != ObjSerTx)))
        return MQTTS_RET_REJ_NOT_SUPP;
    return MQTTS_RET_ACCEPTED;
}

#ifdef EXTSER_RX_USED
static uint8_t serReadOD(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
    *pLen = ++serRxPos;
    memcpy(pBuf, (uint8_t *)serRxBuf, serRxPos);
    serRxPos = 0;
    serRxPosOld = 0;

    return MQTTS_RET_ACCEPTED;
}
#endif  //  EXTSER_RX_USED

#ifdef EXTSER_TX_USED
static uint8_t serWriteOD(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf)
{
    if(serTxLen != 0)       //  Busy
        return MQTTS_RET_REJ_CONG;
    
    memcpy(serTxBuf, pBuf, Len);
    serTxPos = 0;
    serTxLen = Len;
    USART_ENABLE_DREINT();

    return MQTTS_RET_ACCEPTED;
}
#endif  //  EXTSER_TX_USED

#ifdef EXTSER_RX_USED
static uint8_t serPoolOD(subidx_t * pSubidx)
{
    if(serRxPos > 0)
    {
        if(((MQTTS_MSG_SIZE - (MQTTS_MSG_SIZE>>2)) < serRxPos) || (serRxPos == serRxPosOld))
            return 1;
        serRxPosOld = serRxPos;
    }

    return 0;
}
#endif  //  EXTSER_RX_USED

// Register Object
static uint8_t serRegisterOD(indextable_t *pIdx)
{
    indextable_t idx;
#ifdef EXTSER_TX_USED
    if(pIdx->sidx.Type == ObjSerTx)
    {
        if(checkDigBase(SER_PIN_TX) != 0)
            return MQTTS_RET_REJ_INV_ID;
            
        if(serCurrSpeed == 0xFF)    // Serial not configured
        {
            SER_ENABLE();
            serCurrSpeed = pIdx->sidx.Base;
            uint16_t baud = pgm_read_word(&ser_Speed_List[serCurrSpeed]); 
            USART_SET_BAUD(baud);
        }
        else if(serCurrSpeed != pIdx->sidx.Base)
            return MQTTS_RET_REJ_INV_ID;
            
        SER_ENABLE_TX();

        idx.sidx.Place = objDout;
        idx.sidx.Type = objPinNPN;
        idx.sidx.Base = SER_PIN_TX;
        
        pIdx->cbRead = NULL;
        pIdx->cbWrite = &serWriteOD;
        pIdx->cbPool = NULL;
    }
    else 
#endif  //  EXTSER_TX_USED
#ifdef  EXTSER_RX_USED
    if(pIdx->sidx.Type == ObjSerRx)
    {
        if(checkDigBase(SER_PIN_RX) != 0)
            return MQTTS_RET_REJ_INV_ID;
            
        if(serCurrSpeed == 0xFF)    // Serial not configured
        {
            SER_ENABLE();
            serCurrSpeed = pIdx->sidx.Base;
            uint16_t baud = pgm_read_word(&ser_Speed_List[serCurrSpeed]); 
            USART_SET_BAUD(baud);
        }
        else if(serCurrSpeed != pIdx->sidx.Base)
            return MQTTS_RET_REJ_INV_ID;
            
        SER_ENABLE_RX();

        idx.sidx.Place = objDin;
        idx.sidx.Type = objPinNPN;
        idx.sidx.Base = SER_PIN_RX;
        
        pIdx->cbRead = &serReadOD;
        pIdx->cbWrite = NULL;
        pIdx->cbPool = &serPoolOD;
    }
    else
#endif  //  EXTSER_RX_USED
        return MQTTS_RET_REJ_INV_ID;

    dioRegisterOD(&idx);
    return MQTTS_RET_ACCEPTED;
}

// Delete Object
static void serDeleteOD(subidx_t * pSubidx)
{
}

