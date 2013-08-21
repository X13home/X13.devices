/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

// Extensions - Analog Inputs
#include "../config.h"

#ifdef EXTAI_USED

static uint8_t aiBase[EXTAI_MAXPORT_NR];
static uint8_t aiTimeout[EXTAI_MAXPORT_NR];
static uint16_t aiOldVal[EXTAI_MAXPORT_NR];
static uint16_t aiActVal[EXTAI_MAXPORT_NR];

uint16_t ai_busy_mask;
static uint8_t ai_isPos, ai_isCnt;
static uint16_t ai_Summ;

uint16_t aiApin2Mask(uint8_t apin)
{
    uint8_t tmp =  apin & EXTAI_CHN_MASK;
    uint16_t retval = 1;
    while(tmp--)
        retval <<= 1;
    return retval;
}

// Start HAL
const uint8_t aiBase2Apin[] PROGMEM = EXTAI_BASE_2_APIN;

uint8_t cvtBase2Apin(uint16_t base)
{
  return pgm_read_byte(&aiBase2Apin[base & EXTAI_CHN_MASK]);
}

uint8_t checkAnalogBase(uint16_t base)
{
    uint8_t apin = cvtBase2Apin(base);
    if(apin == 0xFF)
        return 2;
    if(ai_busy_mask & aiApin2Mask(apin))
        return 1;
    return 0;
}

ISR(ADC_vect)
{
  uint16_t val = ADC;

  if(ai_isCnt < 0x10)
  {
    ai_Summ += val;
    if(ai_isCnt == 0)
    {
      if((ADMUX != 0x0F) && (ai_isPos < EXTAI_MAXPORT_NR))
        aiActVal[ai_isPos] = ai_Summ;

      while((++ai_isPos < EXTAI_MAXPORT_NR) && (aiBase[ai_isPos] == 0x0F));
      if(ai_isPos >= EXTAI_MAXPORT_NR)
      {
        DISABLE_ADC();
        return;
      }
    }
  }
  else if(ai_isCnt == 0xF0)
  {
    ai_Summ = 0;
    if(ai_isPos < EXTAI_MAXPORT_NR)
      ADMUX = aiBase[ai_isPos];
    else
      ADMUX = 0x0F;
  }

  ai_isCnt--;
  ADCSRA |= (1<<ADSC) | (1<<ADIF);
}
// End HAL

// Clear internal variables
void aiClean(void)
{
    uint8_t i;
    DISABLE_ADC();
    ai_busy_mask = 0;
    ai_isPos = 0;
    ai_isCnt = 0xFF;

    for(i = 0; i < EXTAI_MAXPORT_NR; i++)
        aiBase[i] = 0x0F;
}

// Check Index
uint8_t aiCheckIdx(subidx_t * pSubidx)
{
    uint16_t base = pSubidx->Base;
    if((base > EXTAI_CHN_MASK) || (checkAnalogBase(base) == 2))
        return MQTTS_RET_REJ_NOT_SUPP;

    switch(pSubidx->Type)
    {
        case objArefInt:
            base |= (1<<REFS0) | (1<<REFS1);
            break;
        case objArefVcc:
            base |= (1<<REFS0);
            break;
        case objArefExt:
            break;
        default:
            return MQTTS_RET_REJ_NOT_SUPP;
    }

    pSubidx->Base = base;
    return MQTTS_RET_ACCEPTED;
}

// Read Analogue Inputs
uint8_t aiReadOD(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
    uint8_t apin = cvtBase2Apin(pSubidx->Base);
    *pLen = 2;
    *(uint16_t *)pBuf = aiOldVal[apin];
    return MQTTS_RET_ACCEPTED;
}

// Renew data
uint8_t aiWriteOD(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf)
{
    uint8_t apin = cvtBase2Apin(pSubidx->Base);
    aiOldVal[apin] = *(uint16_t *)pBuf;
    aiTimeout[apin] = 1;
    return MQTTS_RET_ACCEPTED;
}

uint8_t aiPoolOD(subidx_t * pSubidx, uint8_t sleep)
{
  uint8_t apin;
  uint16_t ActVal;

  apin = cvtBase2Apin(pSubidx->Base);
#ifdef ASLEEP
  if(sleep != 0)
  {
    aiTimeout[apin] = POOL_TMR_FREQ;

    if(~PRR & (1<<PRADC))
      DISABLE_ADC();
    return 0;
  }
#endif  //  ASLEEP

  if(PRR & (1<<PRADC))    //  ADC Disabled
  {
    ai_isPos = 0xFF;
    ai_isCnt = 0xFF;
    ENABLE_ADC();
  }

  if(--aiTimeout[apin] != 0)
    return 0;

  ActVal = (aiActVal[apin] + 8)>>4;
  if(aiOldVal[apin] != ActVal)
  {
    aiOldVal[apin] = ActVal;
    return 1;
  }
  return 0;
}

// Register Ainp Object
uint8_t aiRegisterOD(indextable_t *pIdx)
{
    uint16_t base = pIdx->sidx.Base;

    if(checkAnalogBase(base) != 0)
        return MQTTS_RET_REJ_INV_ID;
        
    uint8_t apin = cvtBase2Apin(base);
    
    pIdx->cbRead = &aiReadOD;
    pIdx->cbWrite = &aiWriteOD;
    pIdx->cbPool = &aiPoolOD;

    aiTimeout[apin] = 0;
    aiOldVal[apin] = 0;
    aiActVal[apin] = 0;
    aiBase[apin] = base & 0xFF;

    ai_busy_mask |= aiApin2Mask(apin);

    return MQTTS_RET_ACCEPTED;
}

// Delete Ainp Object
void aiDeleteOD(subidx_t * pSubidx)
{
    uint16_t base = pSubidx->Base;
    if(checkAnalogBase(base) != 1)
        return;
    uint8_t apin = cvtBase2Apin(base);
    ai_busy_mask &= ~aiApin2Mask(apin);
    if(ai_busy_mask == 0)   // Disable ADC
        DISABLE_ADC();
    aiBase[apin] = 0x0F;
}

#endif  //  EXTAI_USED