/*
Copyright (c) 2011-2012 <comparator@gmx.de>

This file is part of the X13.Home project.
https://github.com/X13home

BSD License
See LICENSE.txt file for license details.
*/

// Extensions digital inputs/outputs

static uint8_t pin_busy_mask[EXTDIO_MAXPORT_NR];
static uint8_t pin_read_state[EXTDIO_MAXPORT_NR];
static uint8_t pin_read_flag[EXTDIO_MAXPORT_NR];
#ifdef EXTPWM_USED
static uint8_t pwm_val[2];
#endif  //  EXTPWM_USED

// Convert Base to Mask
static uint8_t base2Mask(uint16_t base)
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
    uint8_t tmp = base & 0xF8;
    switch(tmp)
    {
#ifdef PORTNUM0
        case 0x00:      // PORT A
            return PORTNUM0;
#endif  //  PORTNUM1
#ifdef PORTNUM1
        case 0x08:      // PORT B
            return PORTNUM1;
#endif  //  PORTNUM1
#ifdef PORTNUM2
        case 0x10:      // PORT C
            return PORTNUM2;
#endif  //  PORTNUM2
#ifdef PORTNUM3
        case 0x18:      // PORT D
            return PORTNUM3;
#endif  //  PORTNUM3
    }
    return 0xFF;
}

static uint8_t checkDigBase(uint16_t base)
{
    uint8_t pinmask = base2Mask(base);
    uint8_t tmp = base & 0xF8;
    switch(tmp)
    {
#ifdef PORT0MASK
        case 0x00:      // PORT A
            if(PORT0MASK & pinmask)
                return 2;
            break;
#endif  //  PORT0MASK
#ifdef PORT1MASK
        case 0x08:      // PORT B
            if(PORT1MASK & pinmask)
                return 2;
            break;
#endif  //  PORT1MASK
#ifdef PORT2MASK
        case 0x10:      // PORT C
            if(PORT2MASK & pinmask)
                return 2;
            break;
#endif
#ifdef PORT3MASK
        case 0x18:      // PORT D
            if(PORT3MASK & pinmask)
                return 2;
            break;
#endif
        default:
            return 2;
    }
    
    if(pin_busy_mask[cvtBase2Port(base)] & pinmask)
        return 1;

    return 0;
}

static void out2DDR(uint16_t base, uint8_t set)
{
    uint8_t pinmask = base2Mask(base);
    uint8_t tmp = base & 0xF8;
    switch(tmp)
    {
#ifdef PORTDDR0
        case 0x00:
            if(set)
                PORTDDR0 |= pinmask;
            else
                PORTDDR0 &= ~pinmask;
            break;
#endif
#ifdef PORTDDR1
        case 0x08:
            if(set)
                PORTDDR1 |= pinmask;
            else
                PORTDDR1 &= ~pinmask;
            break;
#endif
#ifdef PORTDDR2
        case 0x10:
            if(set)
                PORTDDR2 |= pinmask;
            else
                PORTDDR2 &= ~pinmask;
            break;
#endif
#ifdef PORTDDR3
        case 0x18:
            if(set)
                PORTDDR3 |= pinmask;
            else
                PORTDDR3 &= ~pinmask;
            break;
#endif
    }
}

static void out2PORT(uint16_t base, uint8_t set)
{
    uint8_t pinmask = base2Mask(base);
    uint8_t tmp = base & 0xF8;
    switch(tmp)
    {
#ifdef PORTOUT0
        case 0x00:
            if(set)
                PORTOUT0 |= pinmask;
            else
                PORTOUT0 &= ~pinmask;
            break;
#endif
#ifdef PORTOUT1
        case 0x08:
            if(set)
                PORTOUT1 |= pinmask;
            else
                PORTOUT1 &= ~pinmask;
            break;
#endif
#ifdef PORTOUT2
        case 0x10:
            if(set)
                PORTOUT2 |= pinmask;
            else
                PORTOUT2 &= ~pinmask;
            break;
#endif
#ifdef PORTOUT3
        case 0x18:
            if(set)
                PORTOUT3 |= pinmask;
            else
                PORTOUT3 &= ~pinmask;
            break;
#endif
    }
}

static uint8_t inpPort(uint16_t base)
{
    uint8_t tmp = base & 0xF8;
    switch(tmp)
    {
#ifdef PORTIN0
        case 0x00:
            return PORTIN0;
#endif
#ifdef PORTIN1
        case 0x08:
            return PORTIN1;
#endif
#ifdef PORTIN2
        case 0x10:
            return PORTIN2;
#endif
#ifdef PORTIN3
        case 0x18:
            return PORTIN3;
#endif
    }
    return 0;
}
// End DIO HAL

void dioClean(void)
{
    uint8_t i;
    for(i = 0; i < EXTDIO_MAXPORT_NR; i++)
    {
        pin_busy_mask[i] = 0;
        pin_read_state[i] = 0;
        pin_read_flag[i] = 0;
    }
}

// Check Index digital inp/out
static uint8_t dioCheckIdx(subidx_t * pSubidx)
{
    uint16_t base = pSubidx->Base;
    if((checkDigBase(base) == 2) || 
       ((pSubidx->Type != objPinNPN) && (pSubidx->Type != objPinPNP)))
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
    uint8_t pinmask = base2Mask(base);
    if(pSubidx->Type == objPinNPN)
        state = ~state;
    state &= pinmask;
    *pLen = 1;
    *pBuf = state ? 1 : 0;
    return MQTTS_RET_ACCEPTED;
}

// Write DIO Object's
static uint8_t dioWriteOD(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf)
{
    uint16_t base = pSubidx->Base;
    uint8_t state = *pBuf;
    uint8_t place = pSubidx->Place;
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
        if(pSubidx->Type == objPinNPN)
            state = ~state;
        state &= 1;
        out2PORT(base, state);
    }
#ifdef EXTPWM_USED
    else if(place == objPWM)        // LED HW PWM
    {
        if(pSubidx->Type == objPinNPN)
            state = ~state;
        pwm_val[base == PWM_PIN0 ? 0:1] = state;
    }
#endif  //  EXTPWM_USED
    return MQTTS_RET_ACCEPTED;
}

static uint8_t dioPoolOD(subidx_t * pSubidx)
{
    uint16_t base = pSubidx->Base;
    uint8_t place = pSubidx->Place;
    if(place == objDin)
    {
        uint8_t state = inpPort(base);
        uint8_t port = cvtBase2Port(base);
        uint8_t pinmask = base2Mask(base);

        if(pSubidx->Type == objPinNPN)
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
#ifdef EXTPWM_USED
    else if(place == objPWM)
    {
        uint8_t val, tmp;
        if(base == PWM_PIN0)            // Channel 0
        {
            val = PWM_OCR0;
            tmp = pwm_val[0];
            if(val == tmp)
                return 0;
            else if(tmp < val)
                val--;
            else
                val++;

            PWM_OCR0 = val;
            if(val == 0 || val == 255)
            {
                DISABLE_PWM0();
                out2PORT(base, val);
            }
            else
                ENABLE_PWM0();
        }
        else                            // Channel 1
        {
            val = PWM_OCR1;
            tmp = pwm_val[1];

            if(val == tmp)
                return 0;
            else if(tmp < val)
                val--;
            else
                val++;

            PWM_OCR1 = val;
            if(val == 0 || val == 255)
            {
                DISABLE_PWM1();
                out2PORT(base, val);
            }
            else
                ENABLE_PWM1();
        }
    }
#endif  //  EXTPWM_USED
    return 0;
}

// Register digital inp/out/pwm Object
static uint8_t dioRegisterOD(indextable_t *pIdx)
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
        else if(tmp == 0)       // Frei
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
    else if(pIdx->sidx.Place == objPWM)
    {
        out2DDR(base, 1);
        PWM_ENABLE();
    }
#endif  //  EXTPWM_USED
    else                        // Digital Input
        out2DDR(base, 0);

    if(pIdx->sidx.Type == objPinNPN)
        out2PORT(base, 1);
        
    pIdx->cbRead = &dioReadOD;
    pIdx->cbWrite = &dioWriteOD;
    pIdx->cbPool = &dioPoolOD;
    return MQTTS_RET_ACCEPTED;
}

static void dioDeleteOD(subidx_t * pSubidx)
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
