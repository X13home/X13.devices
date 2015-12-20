#include "../../config.h"

#if (defined EXTDIO_USED) && (defined EXTPWM_USED)

#include <avr/pgmspace.h>

static const PROGMEM uint8_t hal_pwm_port2dio[] = EXTPWM_PORT2DIO;
static const PROGMEM uint8_t hal_pwm_port2cfg[] = EXTPWM_PORT2CFG;

uint8_t hal_pwm_base2dio(uint16_t base)
{
    if(base >= (const uint16_t)(sizeof(hal_pwm_port2dio)))
        return 0xFF;

    return pgm_read_byte(&hal_pwm_port2dio[base]);
}

void hal_pwm_configure(uint16_t base)
{
    // Configure port
    uint8_t port, mask;
    port = pgm_read_byte(&hal_pwm_port2dio[base]);
    mask = (1<<(port & 0x07));
    port >>= 3;

#ifdef EXTDIO_BASE_OFFSET
    port -= EXTDIO_BASE_OFFSET;
#endif  //  EXTDIO_BASE_OFFSET
    hal_dio_configure(port, mask, DIO_MODE_OUT_PP);
    
    // configure timer
    uint8_t Config = pgm_read_byte(&hal_pwm_port2cfg[base]);
    uint8_t channel = Config & 0x07;
    uint8_t tccra;

    volatile uint8_t * pTIM;
    
    switch(Config>>3)
    {
        case 0:     // Timer 0
            tccra = TCCR0A & ((1<<COM0A1) | (1<<COM0B1));
            if(channel == 0)
                tccra |= (1<<COM0A1);
            else
                tccra |= (1<<COM0B1);

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
    tccra = *pTIM & ((1<<COM0A1) | (1<<COM0B1));
#else           // ATM2560
    tccra = *pTIM & ((1<<COM1A1) | (1<<COM1B1) | (1<<COM1C1));
    if(channel == 2)
    {
        tccra |= (1<<COM1C1);
        *(pTIM + 0x0D) = 0;         // OCRnCH
        *(pTIM + 0x0C) = 0;         // OCRnCL
    }
    else
#endif  //  OCR1C
    if(channel == 0)        // Channel 0
    {
        tccra |= (1<<COM1A1);
        *(pTIM + 0x09) = 0;         // OCRnAH
        *(pTIM + 0x08) = 0;         // OCRnAL
    }
    else                    // Channel 1
    {
        tccra |= (1<<COM1B1);
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
    // configure timer
    uint8_t Config = pgm_read_byte(&hal_pwm_port2cfg[base]);
    uint8_t channel = Config & 0x07;

    volatile uint8_t * pTIM;
    
    switch(Config>>3)
    {
        case 0:     // Timer 0
            if(channel == 0)
                TCCR0A &= ~(1<<COM0A1);
            else
                TCCR0A &= ~(1<<COM0B1);

            if((TCCR0A & ((1<<COM0A1) | (1<<COM0B1))) == 0)     // stop timer
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
        tccra = *pTIM & ((1<<COM0A1) | (1<<COM0B1));
#else           // ATM2560
        tccra = *pTIM & ((1<<COM1A1) | (1<<COM1B1) | (1<<COM1C1));

        if(channel == 2)
        {
            tccra &= ~(1<<COM1C1);
        }
        else
#endif  //  OCR1C
        if(channel == 0)        // Channel 0
        {
            tccra &= ~(1<<COM1A1);
        }
        else                    // Channel 1
        {
            tccra &= ~(1<<COM1B1);
        }

        if(tccra == 0)
        {
            *(pTIM + 0) = 0;        // TCCRnA
            *(pTIM + 1) = 0;        // TCCRnB
        }
        else
        {
            *(pTIM + 0) = tccra | (2<<WGM10);       // FastPWM, top in ICR1
        }
    }

    // Configure port
    uint8_t port, mask;
    port = pgm_read_byte(&hal_pwm_port2dio[base]);
    mask = (1<<(port & 0x07));
    port >>= 3;

#ifdef EXTDIO_BASE_OFFSET
    port -= EXTDIO_BASE_OFFSET;
#endif  //  EXTDIO_BASE_OFFSET
    hal_dio_configure(port, mask, DIO_MODE_IN_FLOAT);
}

void hal_pwm_write(uint16_t base, uint16_t value)
{
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