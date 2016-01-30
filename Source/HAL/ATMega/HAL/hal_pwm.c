#include "config.h"

#if (defined EXTDIO_USED) && (defined EXTPWM_USED)

#include <avr/pgmspace.h>

static const PROGMEM uint8_t hal_pwm_port2dio[] = EXTPWM_PORT2DIO;
static const PROGMEM uint8_t hal_pwm_port2cfg[] = EXTPWM_PORT2CFG;

uint8_t hal_pwm_base2dio(uint16_t base)
{
#ifdef EXTPWM_BASE_OFFSET
    if(base < EXTPWM_BASE_OFFSET)
        return 0xFF;
    base -= EXTPWM_BASE_OFFSET;
#endif

    if(base >= (const uint16_t)(sizeof(hal_pwm_port2dio)))
        return 0xFF;

    return pgm_read_byte(&hal_pwm_port2dio[base]);
}

void hal_pwm_configure(uint16_t base, bool inv)
{
#ifdef EXTPWM_BASE_OFFSET
    base -= EXTPWM_BASE_OFFSET;
#endif

    // Configure port
    uint8_t pin;
    pin = pgm_read_byte(&hal_pwm_port2dio[base]);
    hal_dio_configure(pin, DIO_MODE_OUT_PP);
    
    // configure timer
    uint8_t Config = pgm_read_byte(&hal_pwm_port2cfg[base]);
    uint8_t channel = Config & 0x07;
    uint8_t tccra;
    uint8_t mode;
    if(inv)
        mode = 2;
    else
        mode = 3;

    volatile uint8_t * pTIM;
    
    switch(Config>>3)
    {
        case 0:     // Timer 0
            tccra = TCCR0A & ((3<<COM0A0) | (3<<COM0B0));
            if(channel == 0)
                tccra |= (mode<<COM0A0);
            else
                tccra |= (mode<<COM0B0);

            TCCR0A = tccra | (1<<WGM00);    // PWM, Phase Correct, 0x00 - 0xFF
            TCCR0B = (4<<CS00);             // Clock = Fcpu/256
                                            // Fpwm = Clock/512
            return;
        case 1:         //  Timer 1
            pTIM = &TCCR1A;
            break;
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
#ifdef EXTPWM_BASE_OFFSET
    base -= EXTPWM_BASE_OFFSET;
#endif

    // configure timer
    uint8_t Config = pgm_read_byte(&hal_pwm_port2cfg[base]);
    uint8_t channel = Config & 0x07;

    volatile uint8_t * pTIM;
    
    switch(Config>>3)
    {
        case 0:     // Timer 0
            if(channel == 0)
                TCCR0A &= ~(3<<COM0A0);
            else
                TCCR0A &= ~(3<<COM0B0);

            if((TCCR0A & ((3<<COM0A0) | (3<<COM0B0))) == 0)     // stop timer
            {
                TCCR0A = 0;
                TCCR0B = 0;
            }
            pTIM = NULL;
            break;
        case 1:         //  Timer 1
            pTIM = &TCCR1A;
            break;
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
    uint8_t pin = pgm_read_byte(&hal_pwm_port2dio[base]);
    hal_dio_configure(pin, DIO_MODE_IN_FLOAT);
}

void hal_pwm_write(uint16_t base, uint16_t value)
{
#ifdef EXTPWM_BASE_OFFSET
    base -= EXTPWM_BASE_OFFSET;
#endif
    
    uint8_t Config = pgm_read_byte(&hal_pwm_port2cfg[base]);
    uint8_t channel = Config & 0x07;

    volatile uint8_t * pTIM;
    
    switch(Config>>3)
    {
        case 0:
            if(channel == 0)
                OCR0A = value>>8;
            else
                OCR0B = value>>8;
            return;
        case 1:         //  Timer 1
            pTIM = &TCCR1A;
            break;
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
