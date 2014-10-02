/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// Extensions - Analogue Inputs
#include "../config.h"

#ifdef EXTAIN_USED

#if (EXTAIN_MAXPORT_NR <= 8)
#define tAIN_MASK_SIZE  uint8_t
#elif (EXTAIN_MAXPORT_NR <= 16)
#define tAIN_MASK_SIZE  uint16_t
#else
#if (EXTAIN_MAXPORT_NR > 32)
  #error EXTAIN_MAXPORT_NR is too big
#endif
#define tAIN_MASK_SIZE  uint32_t
#endif

#define AIN_ISCNT   (uint8_t)(F_CPU/179583UL)     // 10 ms pro channel.

const PROGMEM uint8_t ainBase2Apin_[] = EXTAIN_BASE_2_APIN;

static uint8_t          ainBase[EXTAIN_MAXPORT_NR];
static uint8_t          ainTimeout[EXTAIN_MAXPORT_NR];
static uint16_t         ainOldVal[EXTAIN_MAXPORT_NR];
static uint16_t         ainActVal[EXTAIN_MAXPORT_NR];
static tAIN_MASK_SIZE   ain_busy_mask;
static uint8_t          ain_isPos;

// Start HAL
static tAIN_MASK_SIZE ainApin2Mask(uint8_t apin)
{
  uint8_t tmp =  apin & EXTAIN_CHN_MASK;
  tAIN_MASK_SIZE retval = 1;
  while(tmp--)
    retval <<= 1;
  return retval;
}

static uint8_t ainBase2Apin(uint16_t base)
{
  return pgm_read_byte(&ainBase2Apin_[base & EXTAIN_CHN_MASK]);
}

static uint8_t ainCheckAnalogBase(uint16_t base)
{
  uint8_t apin = ainBase2Apin(base);
  if(apin == 0xFF)
    return 2;
  if(ain_busy_mask & ainApin2Mask(apin))
    return 1;
  return 0;
}

ISR(ADC_vect)
{
  static uint16_t ain_Summ;
  static uint8_t ain_isCnt;
  uint16_t val = ADC;
  
  if(ain_isPos >= EXTAIN_MAXPORT_NR)
  {
    ADCSRA = (1<<ADIF);                   // Disable ADC, Stop Conversion
    EXTAIN_SELECT(EXTAIN_MUX_GND);
    ain_isPos = 0;
    ain_isCnt = 0xFF;
    return;
  }

  if(ain_isCnt < 0x10)
  {
    ain_Summ += val;
    if(ain_isCnt == 0)
    {
      ainActVal[ain_isPos] = ain_Summ;
      ain_isPos++;
      ain_isCnt = (AIN_ISCNT + 1);
    }
  }
  else if(ain_isCnt == AIN_ISCNT)
  {
    EXTAIN_SELECT(ainBase[ain_isPos]);
    ain_Summ = 0;
  }

  ain_isCnt--;
  ADCSRA |= (1<<ADSC) | (1<<ADIF);
}
// End HAL

// Start API
// Clear internal variables
void ainClean(void)
{
  uint8_t i;
  DISABLE_ADC();
  ain_busy_mask = 0;
  ain_isPos = 0xFF;

  for(i = 0; i < EXTAIN_MAXPORT_NR; i++)
    ainBase[i] = EXTAIN_MUX_GND;
}

// Check Index
uint8_t ainCheckIdx(subidx_t * pSubidx)
{
  uint16_t base = pSubidx->Base;
  if((base > EXTAIN_CHN_MASK) || (ainCheckAnalogBase(base) == 2))
    return MQTTS_RET_REJ_NOT_SUPP;

  switch(pSubidx->Type)
  {
    case objArefExt:
      break;
    case objArefVcc:
      base |= (1<<REFS0);
      break;
    case objArefInt1:
      base |= (1<<REFS1);
      break;
    case objArefInt2:
      base |= (1<<REFS0) | (1<<REFS1);
      break;
    default:
      return MQTTS_RET_REJ_NOT_SUPP;
  }

  pSubidx->Base = base;
  return MQTTS_RET_ACCEPTED;
}

// Read Analogue Inputs
uint8_t ainReadOD(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
  uint8_t apin = ainBase2Apin(pSubidx->Base);
  *pLen = 2;
  *(uint16_t *)pBuf = (ainOldVal[apin] + 8)>>4;;
  return MQTTS_RET_ACCEPTED;
}

uint8_t ainPollOD(subidx_t * pSubidx, uint8_t sleep)
{
  uint8_t apin;
  uint16_t ActVal;

  apin = ainBase2Apin(pSubidx->Base);
#ifdef ASLEEP
  if(sleep != 0)
  {
    ainTimeout[apin] = POLL_TMR_FREQ;
    ain_isPos = 0xFF;
    return 0;
  }
#endif  //  ASLEEP

  if(ainTimeout[apin] > 0)
  {
    ainTimeout[apin]--;
    return 0;
  }
  ainTimeout[apin] = POLL_TMR_FREQ;

  if((ADCSRA & (1<<ADSC)) == 0) // ADC Disabled
  {
    ain_isPos = 0;
    // Start Conversion
    ADCSRA = (1<<ADEN) | (1<<ADIF) | (1<<ADIE) | (7<<ADPS0);
    ADCSRA |= (1<<ADSC);
  }

  ActVal = ainActVal[apin];
  if(ainOldVal[apin] != ActVal)
  {
    ainOldVal[apin] = ActVal;
    return 1;
  }
  return 0;
}

// Register Ainp Object
uint8_t ainRegisterOD(indextable_t *pIdx)
{
    uint16_t base = pIdx->sidx.Base;

    if(ainCheckAnalogBase(base) != 0)
        return MQTTS_RET_REJ_INV_ID;
        
    uint8_t apin = ainBase2Apin(base);
    
    pIdx->cbRead = &ainReadOD;
    pIdx->cbWrite = NULL;
    pIdx->cbPoll = &ainPollOD;

    ainTimeout[apin] = POLL_TMR_FREQ;
    ainOldVal[apin] = 0;
    ainActVal[apin] = 0;
    ainBase[apin] = base;

    if(ain_busy_mask == 0)   // Enable ADC
        ENABLE_ADC();

    ain_busy_mask |= ainApin2Mask(apin);

    return MQTTS_RET_ACCEPTED;
}

// Delete Ainp Object
void ainDeleteOD(subidx_t * pSubidx)
{
    uint16_t base = pSubidx->Base;
    if(ainCheckAnalogBase(base) != 1)
        return;
    uint8_t apin = ainBase2Apin(base);
    ain_busy_mask &= ~ainApin2Mask(apin);
    if(ain_busy_mask == 0)   // Disable ADC
        DISABLE_ADC();
    ainBase[apin] = EXTAIN_MUX_GND;
}
#endif  //  EXTAIN_USED