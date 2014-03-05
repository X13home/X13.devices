/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

// Extensions digital inputs/outputs

#include "../config.h"

#ifdef EXTDIO_USED
#include "extdio.h"

#ifdef EXTAI_USED
#include "extai.h"
extern uint16_t aiApin2Mask(uint8_t apin);
extern uint8_t cvtBase2Apin(uint16_t base);
extern uint8_t checkAnalogBase(uint16_t base);

extern uint16_t ai_busy_mask;
#endif  //  EXTAI_USED

static uint8_t pin_busy_mask[EXTDIO_MAXPORT_NR];
static uint8_t pin_read_state[EXTDIO_MAXPORT_NR];
static uint8_t pin_read_flag[EXTDIO_MAXPORT_NR];

const PROGMEM uint16_t portnum2port[]  = PORTNUM_2_PORT;
const PROGMEM uint16_t portnum2ddr[]   = PORTNUM_2_DDR;
const PROGMEM uint16_t portnum2pin[]   = PORTNUM_2_PIN;
const PROGMEM uint8_t  portnum2mask[]  = PORTNUM_2_MASK;

// Convert Base to Mask
uint8_t base2Mask(uint16_t base)
{
    uint8_t retval = 1;
    base &= 0x07;
    while(base--)
        retval <<= 1;
    return retval;
}

// Start DIO HAL
static uint8_t cvtBase2Port(uint16_t base)     // Digital Ports
{
  uint8_t tmp = (base & 0xF8)>>3;
#if (defined EXTDIO_BASE_OFFSET) && (EXTDIO_BASE_OFFSET > 0)
  tmp -= EXTDIO_BASE_OFFSET;
#endif
  if(tmp >= EXTDIO_MAXPORT_NR)
    return EXTDIO_MAXPORT_NR;
  return tmp;
}

uint8_t checkDigBase(uint16_t base)
{
  uint8_t pinmask = base2Mask(base);
  uint8_t port = cvtBase2Port(base);
  uint8_t tmp;

  if(port == EXTDIO_MAXPORT_NR)
    return 2;

  tmp = pgm_read_byte(&portnum2mask[port]);
  if(tmp & pinmask)
    return 2;

  if(pin_busy_mask[port] & pinmask)
    return 1;

  return 0;
}

static void out2DDR(uint16_t base, uint8_t set)
{
  uint8_t pinmask = base2Mask(base);
  uint8_t *pDDR;
  pDDR = (uint8_t *)pgm_read_word(&portnum2ddr[cvtBase2Port(base)]);

  if(set)
    *pDDR |= pinmask;
  else
    *pDDR &= ~pinmask;
}

static void out2PORT(uint16_t base, uint8_t set)
{
  uint8_t pinmask = base2Mask(base);
  uint8_t *pPORT;

  pPORT = (uint8_t *)pgm_read_word(&portnum2port[cvtBase2Port(base)]);

  if(set)
    *pPORT |= pinmask;
  else
    *pPORT &= ~pinmask;
}

uint8_t inpPort(uint16_t base)
{
  uint8_t *pPIN;
  pPIN = (uint8_t *)pgm_read_word(&portnum2pin[cvtBase2Port(base)]);
  return *pPIN;
}
// End DIO HAL

void dioClean(void)
{
#ifdef EXTPWM_USED
  DISABLE_PWM0();
  DISABLE_PWM1();
  PWM_DISABLE();
#endif  //  EXTPWM_USED

  uint8_t i;
  for(i = 0; i < EXTDIO_MAXPORT_NR; i++)
  {
    pin_busy_mask[i] = 0;
    pin_read_state[i] = 0;
    pin_read_flag[i] = 0;
  }
}

// Check Index digital inp/out
uint8_t dioCheckIdx(subidx_t * pSubidx)
{
    uint16_t base = pSubidx->Base;
    if((checkDigBase(base) == 2) || 
       ((pSubidx->Type != objPinNPN) && (pSubidx->Type != objPinPNP)
#ifdef ASLEEP
        && (pSubidx->Type != objActPNP) && (pSubidx->Type != objActNPN)
#endif  //  ASLEEP
       ))
        return MQTTS_RET_REJ_NOT_SUPP;
#ifdef EXTPWM_USED
    if((pSubidx->Place == objPWM) && (base != PWM_PIN0) && (base != PWM_PIN1))
        return MQTTS_RET_REJ_NOT_SUPP;
#endif  //  EXTPWM_USED
    return MQTTS_RET_ACCEPTED;
}

// Read digital Inputs
static uint8_t dioReadOD(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
    uint16_t base = pSubidx->Base;
    uint8_t state = inpPort(base);
    if(pSubidx->Type == objPinNPN)
        state = ~state;
    *pLen = 1;
    *pBuf = ((state & base2Mask(base)) != 0) ? 1 : 0;
    return MQTTS_RET_ACCEPTED;
}

// Write DIO Object's
static uint8_t dioWriteOD(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf)
{
    uint16_t base = pSubidx->Base;
    uint8_t state = *pBuf;
    uint8_t place = pSubidx->Place;
    uint8_t type = pSubidx->Type;
    if(place == objDin)         // Digital Inputs, check data
    {
        uint8_t port = cvtBase2Port(base);
        uint8_t pinmask = base2Mask(base);
        if(state)
            pin_read_state[port] |= pinmask;
        else
            pin_read_state[port] &= ~pinmask;
    }
    else if(place == objDout)       // Digital outputs
    {
        if(type == objPinNPN)
            state = ~state;
#ifdef ASLEEP
        else if(type != objPinPNP)        // Write to Asleep state output
          return MQTTS_RET_REJ_NOT_SUPP;
#endif
        out2PORT(base, state & 1);
    }
#ifdef EXTPWM_USED
    else if(place == objPWM)        // LED HW PWM
    {
      if(type == objPinNPN)
        state = ~state;
      
      if((state == 0) || (state == 255))
      {
        if(base == PWM_PIN0)            // Channel 0
          DISABLE_PWM0();
        else
          DISABLE_PWM1();
        out2PORT(base, state);
      }
      else
      {
        if(base == PWM_PIN0)            // Channel 0
        {
          ENABLE_PWM0();
          PWM_OCR0 = state;
        }
        else
        {
          ENABLE_PWM1();
          PWM_OCR1 = state;
        }
      }
    }
#endif  //  EXTPWM_USED
    return MQTTS_RET_ACCEPTED;
}

static uint8_t dioPoolOD(subidx_t * pSubidx, uint8_t sleep)
{
  uint16_t base = pSubidx->Base;
  uint8_t place = pSubidx->Place;
  uint8_t type = pSubidx->Type;
  
  uint8_t state, port, pinmask;

  if(place == objDin)
  {
#ifdef ASLEEP
    if(sleep != 0)
      return 0;
#endif  //  ASLEEP
    state = inpPort(base);
    port = cvtBase2Port(base);
    pinmask = base2Mask(base);

    if(type == objPinNPN)
      state = ~state;
    state &= pinmask;    
    
    if(state != (pin_read_flag[port] & pinmask))
    {
      pin_read_flag[port] ^= pinmask;
    }
    else if(state != (pin_read_state[port] & pinmask))
    {
      pin_read_state[port] ^= pinmask;
      return 1;
    }
  }
#ifdef ASLEEP
  else if((place == objDout) && ((type == objActNPN) || (type == objActPNP)))
  {
    out2PORT(base, (sleep == 0) ^ (type == objActNPN));
  }
#endif  //  ASLEEP
  return 0;
}

// Register digital inp/out/pwm Object
uint8_t dioRegisterOD(indextable_t *pIdx)
{
    uint16_t base = pIdx->sidx.Base;
    if(checkDigBase(base) != 0)
        return MQTTS_RET_REJ_INV_ID;
    uint8_t port = cvtBase2Port(base);
#ifdef EXTAI_USED
    if(port == EXTAI_PORT_NUM)
    {
        uint8_t tmp = checkAnalogBase(base);
        if(tmp == 1)            // Busy
            return MQTTS_RET_REJ_INV_ID;
        else if(tmp == 0)       // Free
            ai_busy_mask |= aiApin2Mask(cvtBase2Apin(base));
    }
#endif  //  EXTAI_USED
    uint8_t mask = base2Mask(base);
    pin_busy_mask[port] |= mask;
    pin_read_state[port] &= ~mask;
    pin_read_flag[port] &= ~mask;
    if(pIdx->sidx.Place == objDout)      // Digital output
    {
        out2DDR(base, 1);
    }
#ifdef EXTPWM_USED
    else if (pIdx->sidx.Place == objPWM)
    {
      out2DDR(base, 1);
      PWM_ENABLE();
    }
#endif  //  EXTPWM_USED
    else                                  // Digital Input
        out2DDR(base, 0);

    if((pIdx->sidx.Type == objPinNPN)
#ifdef ASLEEP
    || (pIdx->sidx.Type == objActPNP)
#endif  // ASLEEP
    )
        out2PORT(base, 1);
        
    pIdx->cbRead = &dioReadOD;
    pIdx->cbWrite = &dioWriteOD;
    pIdx->cbPool = &dioPoolOD;
    return MQTTS_RET_ACCEPTED;
}

void dioDeleteOD(subidx_t * pSubidx)
{
    uint16_t base = pSubidx->Base;
    uint8_t port = cvtBase2Port(base);
    uint8_t mask = base2Mask(base);

    pin_busy_mask[port] &= ~mask;
    pin_read_state[port] &= ~mask;
    pin_read_flag[port] &= ~mask;
    
#ifdef EXTAI_USED
    if((port == EXTAI_PORT_NUM) && (checkAnalogBase(base) == 1))
        ai_busy_mask &= ~aiApin2Mask(cvtBase2Apin(base));
#endif  //  EXTAI_USED
#ifdef EXTPWM_USED
    if(pSubidx->Place == objPWM)
    {
        if(pSubidx->Base == PWM_PIN0)   // Channel 0
            DISABLE_PWM0();
        else                            // Channel 1
            DISABLE_PWM1();
        PWM_DISABLE();
    }
#endif  //  EXTPWM_USED

    out2PORT(base, 0);
    out2DDR(base, 0);
}

#endif    //  EXTDIO_USED
