/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// Extensions prototypes

#include "config.h"
#include "ext.h"
#include "util.h"

#ifdef EXTAIN_USED
#include "ext/extain.h"
#endif  //  EXTAIN_USED

#ifdef EXTDIO_USED
#include "ext/extdio.h"

#ifdef EXTPWM_USED
#include "ext/extpwm.h"
#endif  //  EXTPWM_USED

#ifdef TWI_USED
#include "ext/twim.h"
#endif  //  TWI_USED

#if (defined EXTSER_TX_USED) || (defined EXTSER_RX_USED)
#include "ext/extSer.h"
#endif  //  EXTSER_TX_USED || EXTSER_RX_USED
#endif  //  EXTDIO_USED

// Delete All objects, & init hardware
void extClean(void)
{
#ifdef EXTAIN_USED
  ainClean();
#endif  //  EXTAIN_USED
#ifdef EXTDIO_USED
  dioClean();
#ifdef EXTPWM_USED
  pwmClean();
#endif  //  EXTPWM_USED
#if (defined EXTSER_TX_USED) || (defined EXTSER_RX_USED)
  serClean();
#endif  //  EXTSER_TX_USED || EXTSER_RX_USED
#ifdef TWI_USED
  twiClean();
#endif  //  TWI_USED
#endif  //  EXTDIO_USED
}

// Configure & Register PnP devices
void extConfig(void)
{
#ifdef TWI_USED
  twiConfig();
#endif  //  TWI_USED
}

uint8_t extRegisterOD(indextable_t * pIdx)
{
  switch(pIdx->sidx.Place)
  {
#ifdef EXTAIN_USED
    case objAin:        // Analogue Input's(unsigned word)
      return ainRegisterOD(pIdx);
#endif  //  EXTAIN_USED
#ifdef EXTDIO_USED
    case objDin:        // Digital(bool) Input's
    case objDout:       // Digital(bool) Output's
      return dioRegisterOD(pIdx);
#ifdef EXTPWM_USED
    case objPWM:        // PWM
      return pwmRegisterOD(pIdx);
#endif  //  EXTPWM_USED
#if (defined EXTSER_TX_USED) || (defined EXTSER_RX_USED)
    case objSer:        // Serial IO
      return serRegisterOD(pIdx);
#endif  //  EXTSER_TX_USED || EXTSER_RX_USED
#endif  //  EXTDIO_USED
  }
  return MQTTS_RET_REJ_NOT_SUPP;
}

uint8_t extCheckIdx(subidx_t * pSubidx)
{
  switch(pSubidx->Place)
  {
#ifdef EXTAIN_USED
    case objAin:
      return ainCheckIdx(pSubidx);
#endif  //  EXTAIN_USED
#ifdef EXTDIO_USED
    case objDin:
    case objDout:
      return dioCheckIdx(pSubidx);
#ifdef EXTPWM_USED
    case objPWM:        // LED HW PWM
      return pwmCheckIdx(pSubidx);
#endif  //  EXTPWM_USED
#if (defined EXTSER_TX_USED) || (defined EXTSER_RX_USED)
    case objSer:        // Serial IO
      return serCheckIdx(pSubidx);
#endif  //  EXTSER_TX_USED || EXTSER_RX_USED
#endif  //  EXTDIO_USED
  }
  return MQTTS_RET_REJ_NOT_SUPP;
}

void extDeleteOD(subidx_t * pSubidx)
{
  // Delete Objects
  switch(pSubidx->Place)
  {
#ifdef EXTAIN_USED
    case objAin:
      ainDeleteOD(pSubidx);
      break;
#endif  //  EXTAIN_USED
#ifdef EXTDIO_USED
    case objDin:
    case objDout:
      dioDeleteOD(pSubidx);
      break;
#ifdef EXTPWM_USED
    case objPWM:        // LED HW PWM
      pwmDeleteOD(pSubidx);
      break;
#endif  //  EXTPWM_USED
#if (defined EXTSER_TX_USED) || (defined EXTSER_RX_USED)
    case objSer:        // Serial IO
      serDeleteOD(pSubidx);
      break;
#endif  //  EXTSER_TX_USED || EXTSER_RX_USED
#endif  //  EXTDIO_USED
  }
}

// Convert Index to Topic ID
uint8_t extCvtIdx2TopicId(subidx_t * pSubidx, uint8_t * pPnt)
{
  uint8_t size;
  *(uint8_t*)(pPnt++) = pSubidx->Place;
  *(uint8_t*)(pPnt++) = pSubidx->Type;
  uint16_t addr = pSubidx->Base;
#ifdef EXTAIN_USED
  if(pSubidx->Place == objAin)
    addr &= EXTAIN_CHN_MASK;
#endif  // EXTAIN_USED
  size = sprintdec((uint8_t*)pPnt, addr);
  return (size + 2);
}
