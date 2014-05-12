/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// OWM Outputs for ATMega xx8P

#include "../config.h"

#if (defined EXTDIO_USED) && (defined EXTPWM_USED)
#include "extdio.h"
#include "extpwm.h"

extern void out2PORT(uint16_t base, uint8_t set);

void pwmClean(void)
{
  DISABLE_PWM0();
  DISABLE_PWM1();
  PWM_DISABLE();
}

uint8_t pwmCheckIdx(subidx_t * pSubidx)
{
  uint16_t base = pSubidx->Base;
  if((dioCheckIdx(pSubidx) != MQTTS_RET_ACCEPTED) || // Not Exist
#ifdef ASLEEP
     ((pSubidx->Type != objPinNPN) && (pSubidx->Type != objPinPNP)) ||
#endif  //  ASLEEP
     ((base != PWM_PIN0) && (base != PWM_PIN1)))
    return MQTTS_RET_REJ_NOT_SUPP;
  return MQTTS_RET_ACCEPTED;    
}

static uint8_t pwmWriteOD(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf)
{
  uint8_t type = pSubidx->Type;
  uint8_t state = *pBuf;
  uint16_t base = pSubidx->Base;
  
  if(type == objPinNPN)
    state = ~state;

  if(base == PWM_PIN0)            // Channel 0
  {
    if(state != 0)
    {
      ENABLE_PWM0();
      PWM_OCR0 = state;
    }
    else
    {
      DISABLE_PWM0();
      out2PORT(PWM_PIN0, 0);
    }
  }
  else
  {
    if(state != 0)
    {
      ENABLE_PWM1();
      PWM_OCR1 = state;
    }
    else
    {
      DISABLE_PWM1();
      out2PORT(PWM_PIN1, 0);
    }
  }
  return MQTTS_RET_ACCEPTED;
}

uint8_t pwmRegisterOD(indextable_t *pIdx)
{
  pIdx->sidx.Place = objDout;
  
  uint8_t retval = dioRegisterOD(pIdx);
  if(retval != MQTTS_RET_ACCEPTED)
    return retval;

  PWM_ENABLE();

  pIdx->sidx.Place = objPWM;
  pIdx->cbRead = NULL;
  pIdx->cbWrite = &pwmWriteOD;
  pIdx->cbPoll = NULL;
  return MQTTS_RET_ACCEPTED;
}

void pwmDeleteOD(subidx_t * pSubidx)
{
  if(pSubidx->Base == PWM_PIN0)   // Channel 0
    DISABLE_PWM0();
  else                            // Channel 1
    DISABLE_PWM1();

  PWM_DISABLE();
  dioDeleteOD(pSubidx);
}

#endif  //  (defined EXTDIO_USED) && (defined EXTPWM_USED)