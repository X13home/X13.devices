#include "config.h"

#if (defined EXTDIO_USED) && (defined EXTPWM_USED)

#include <avr/pgmspace.h>

static const PROGMEM uint8_t hal_pwm_port2cfg[] = HAL_PWM_PORT2CFG;

bool hal_pwm_checkbase(uint16_t base)
{
#ifdef HAL_PWM_BASE_OFFSET
    if(base < HAL_PWM_BASE_OFFSET)
    {
        return false;
    }
    base -= HAL_PWM_BASE_OFFSET;
#endif
    if((base >= sizeof(hal_pwm_port2cfg)) ||
       (pgm_read_byte(&hal_pwm_port2cfg[base]) == 0xFF))
    {
        return false;
    }

    return true;
}

bool hal_pwm_busy(uint16_t base)
{
#ifdef HAL_PWM_BASE_OFFSET
    base -= HAL_PWM_BASE_OFFSET;
#endif

    // configure timer
    uint8_t Config = pgm_read_byte(&hal_pwm_port2cfg[base]);
    uint8_t channel = Config & 0x07;

    volatile uint8_t * pTIM;

    switch(Config>>3)
    {
        case 0:     // Timer 0
            if(channel == 0)
            {
                return ((TCCR0A & (3<<COM0A0)) != 0);
            }
            else
            {
                return ((TCCR0A & (3<<COM0B0)) != 0);
            }
#ifndef HAL_USE_SUBMSTICK
        case 1:         //  Timer 1
            pTIM = &TCCR1A;
            break;
#endif  //  HAL_USE_SUBMSTICK
#ifdef TCCR3A
        case 3:         //  Timer 3
            pTIM = &TCCR3A;
            break;
#endif  // TCCR3A
#ifdef TCCR4A
        case 4:         //  Timer 4
            pTIM = &TCCR4A;
            break;
#endif  // TCCR3A
#ifdef TCCR5A
        case 5:         //  Timer 5
            pTIM = &TCCR5A;
            break;
#endif  // TCCR3A
        default:
            return true;
    }
    
    switch(channel)
    {
        case 0:
            return ((*pTIM & (3<<COM1A0)) != 0);
        case 1:
            return ((*pTIM & (3<<COM1B0)) != 0);
#ifdef OCR1C
        case 2:
            return ((*pTIM & (3<<COM1C0)) != 0);
#endif
        default:
            break;
    }
    return true;
}

void hal_pwm_configure(uint16_t base, bool inv)
{
    hal_dio_configure(hal_dio_base2pin(base), DIO_MODE_OUT_PP);
#ifdef HAL_PWM_BASE_OFFSET
    base -= HAL_PWM_BASE_OFFSET;
#endif
    // configure timer
    uint8_t Config = pgm_read_byte(&hal_pwm_port2cfg[base]);
    uint8_t channel = Config & 0x07;
    uint8_t tccra;
    uint8_t mode;
    if(inv)
    {
        mode = 3;
    }
    else
    {
        mode = 2;
    }

    volatile uint8_t * pTIM;

    switch(Config>>3)
    {
        case 0:     // Timer 0
            tccra = TCCR0A & ((3<<COM0A0) | (3<<COM0B0));
            if(channel == 0)
            {
                tccra |= (mode<<COM0A0);
            }
            else
            {
                tccra |= (mode<<COM0B0);
            }

            TCCR0A = tccra | (1<<WGM00);    // PWM, Phase Correct, 0x00 - 0xFF
            TCCR0B = (4<<CS00);             // Clock = Fcpu/256
                                            // Fpwm = Clock/512
            return;
#ifndef HAL_USE_SUBMSTICK
        case 1:         //  Timer 1
            pTIM = &TCCR1A;
            break;
#endif  //  HAL_USE_SUBMSTICK
#ifdef TCCR3A
        case 3:         //  Timer 3
            pTIM = &TCCR3A;
            break;
#endif  // TCCR3A
#ifdef TCCR4A
        case 4:         //  Timer 4
            pTIM = &TCCR4A;
            break;
#endif  // TCCR3A
#ifdef TCCR5A
        case 5:         //  Timer 5
            pTIM = &TCCR5A;
            break;
#endif  // TCCR3A
        default:
            return;
    }

#ifndef OCR1C   // ATM328 + ATM1284
    tccra = *pTIM & ((3<<COM0A0) | (3<<COM0B0));
#else           // ATM2560
    tccra = *pTIM & ((3<<COM1A0) | (3<<COM1B0) | (3<<COM1C0));
    if(channel == 2)
    {
        tccra |= (mode<<COM1C0);
        *(pTIM + 0x0D) = 0;         // OCRnCH
        *(pTIM + 0x0C) = 0;         // OCRnCL
    }
    else
#endif  //  OCR1C
    if(channel == 0)        // Channel 0
    {
        tccra |= (mode<<COM1A0);
        *(pTIM + 0x09) = 0;         // OCRnAH
        *(pTIM + 0x08) = 0;         // OCRnAL
    }
    else                    // Channel 1
    {
        tccra |= (mode<<COM1B0);
        *(pTIM + 0x0B) = 0;         // OCRnBH
        *(pTIM + 0x0A) = 0;         // OCRnBL
    }

    *(pTIM + 0x07) = 0xFF;          // ICRnH
    *(pTIM + 0x06) = 0xFF;          // ICRnL

    *(pTIM + 0) = tccra | (1<<WGM11);       // PWM Phase Correct, top in ICR1
    *(pTIM + 1) = (1<<WGM13) | (1<<CS00);   // Clock = Fosc
                                            // Fpwm = Clock/131072
}

void hal_pwm_delete(uint16_t base)
{
#ifdef HAL_PWM_BASE_OFFSET
    base -= HAL_PWM_BASE_OFFSET;
#endif

    // configure timer
    uint8_t Config = pgm_read_byte(&hal_pwm_port2cfg[base]);
    uint8_t channel = Config & 0x07;

    volatile uint8_t * pTIM;

    switch(Config>>3)
    {
        case 0:     // Timer 0
            if(channel == 0)
            {
                TCCR0A &= ~(3<<COM0A0);
            }
            else
            {
                TCCR0A &= ~(3<<COM0B0);
            }

            if((TCCR0A & ((3<<COM0A0) | (3<<COM0B0))) == 0)     // stop timer
            {
                TCCR0A = 0;
                TCCR0B = 0;
            }
            pTIM = NULL;
            break;
#ifndef HAL_USE_SUBMSTICK
        case 1:         //  Timer 1
            pTIM = &TCCR1A;
            break;
#endif  //  HAL_USE_SUBMSTICK
#ifdef TCCR3A
        case 3:         //  Timer 3
            pTIM = &TCCR3A;
            break;
#endif  // TCCR3A
#ifdef TCCR4A
        case 4:         //  Timer 4
            pTIM = &TCCR4A;
            break;
#endif  // TCCR3A
#ifdef TCCR5A
        case 5:         //  Timer 5
            pTIM = &TCCR5A;
            break;
#endif  // TCCR3A
        default:
            return;
    }

    if(pTIM != NULL)
    {
        uint8_t tccra;
#ifndef OCR1C   // ATM328 + ATM1284
        tccra = *pTIM & ((3<<COM0A0) | (3<<COM0B0));
#else           // ATM2560
        tccra = *pTIM & ((3<<COM1A0) | (3<<COM1B0) | (3<<COM1C0));

        if(channel == 2)
        {
            tccra &= ~(3<<COM1C0);
        }
        else
#endif  //  OCR1C
        if(channel == 0)        // Channel 0
        {
            tccra &= ~(3<<COM1A0);
        }
        else                    // Channel 1
        {
            tccra &= ~(3<<COM1B0);
        }

        if(tccra == 0)
        {
            *(pTIM + 0) = 0;        // TCCRnA
            *(pTIM + 1) = 0;        // TCCRnB
        }
        else
        {
            *(pTIM + 0) = tccra | (1<<WGM11);       // PWM Phase Correct, top in ICR1
        }
    }
    
    // Configure port
    hal_dio_configure(hal_dio_base2pin(base), DIO_MODE_IN_FLOAT);
}

void hal_pwm_write(uint16_t base, uint16_t value)
{
#ifdef HAL_PWM_BASE_OFFSET
    base -= HAL_PWM_BASE_OFFSET;
#endif

    if(base > sizeof(hal_pwm_port2cfg))
    {
        return;
    }

    uint8_t Config = pgm_read_byte(&hal_pwm_port2cfg[base]);
    uint8_t channel = Config & 0x07;

    volatile uint8_t * pTIM;
    
    switch(Config>>3)
    {
        case 0:
            if(channel == 0)
            {
                OCR0A = value>>8;
            }
            else
            {
                OCR0B = value>>8;
            }
            return;
#ifndef HAL_USE_SUBMSTICK
        case 1:         //  Timer 1
            pTIM = &TCCR1A;
            break;
#endif  //  HAL_USE_SUBMSTICK
#ifdef TCCR3A
        case 3:         //  Timer 3
            pTIM = &TCCR3A;
            break;
#endif  // TCCR3A
#ifdef TCCR4A
        case 4:         //  Timer 4
            pTIM = &TCCR4A;
            break;
#endif  // TCCR3A
#ifdef TCCR5A
        case 5:         //  Timer 5
            pTIM = &TCCR5A;
            break;
#endif  // TCCR3A
        default:
            return;
    }

#ifdef OCR1C
    if(channel == 2)
    {
        *(pTIM + 0x0D) = value >> 8;            // OCRnCH
        *(pTIM + 0x0C) = value & 0xFF;          // OCRnCL
    }
    else
#endif  //  OCR1C
    if(channel == 0)        // Channel 0
    {
        *(pTIM + 0x09) = value >> 8;            // OCRnAH
        *(pTIM + 0x08) = value & 0xFF;          // OCRnAL
    }
    else                    // Channel 1
    {
        *(pTIM + 0x0B) = value >> 8;            // OCRnBH
        *(pTIM + 0x0A) = value & 0xFF;          // OCRnBL
    }
}

#endif  //  (defined EXTDIO_USED) && (defined EXTPWM_USED)
