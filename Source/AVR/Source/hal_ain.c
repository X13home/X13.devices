#include "config.h"

#ifdef EXTAIN_USED

#include <avr/pgmspace.h>

#ifndef PRR
#define PRR PRR0
#endif  //  PRR

static const PROGMEM uint8_t hal_ainBase2Apin[] = EXTAIN_BASE_2_APIN;

#if (defined __AVR_ATmega328P__)
static const PROGMEM uint8_t hal_ainBase2Dio[] = {16,   17,   18,   19,   20,   21,   0xFE, 0xFE,   // PC0 - PC5, Ain6, Ain7
                                                  0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFE};  // TempSens, Vbg, GND
#elif (defined __AVR_ATmega1284P__)
static const PROGMEM uint8_t hal_ainBase2Dio[] = {0,    1,    2,    3,    4,    5,    6,    7,      // PA0 - PA7
                                                  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // Diff AIn not used
                                                  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                                  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF};  // Vbg
#elif defined (__AVR_ATmega2560__)
static const PROGMEM uint8_t hal_ainBase2Dio[] = {40,   41,   42,   43,   44,   45,   46,   47,     // PF0 - PF7
                                                  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF,   // Vbg
                                                  72,   73,   74,   75,   76,   77,   78,   79,     // PK0 - PK7
                                                  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // Diff AIn not used
                                                  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                                  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                                  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                                  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
#else
    #error unknown uC
#endif  // uC

uint8_t hal_ain_apin2dio(uint8_t apin)
{
    if(apin > sizeof(hal_ainBase2Dio))
        return 0xFF;

    uint8_t base = pgm_read_byte(&hal_ainBase2Apin[apin]);

    // Paranoid check
    if(base < sizeof(hal_ainBase2Dio))
        return pgm_read_byte(&hal_ainBase2Dio[base]);
    else
        return 0xFF;
}

void hal_ain_configure(uint8_t apin, uint8_t aref)
{
    if(aref == 0xFF)
        return;
    
    if((ADCSRA & (1<<ADEN)) == 0)   // ADC disabled
    {
        PRR &= ~(1<<PRADC);
        ADCSRA = (1<<ADEN) | (7<<ADPS0);
    }
}

void hal_ain_select(uint8_t apin, uint8_t aref)
{
    uint8_t mux = pgm_read_byte(&hal_ainBase2Apin[apin]);
    mux |= aref<<6;

    ADMUX = mux | (1<<ADLAR);
#ifdef  MUX5
    ADCSRB = ((mux & 0x20)>>2);
#endif  //  MUX5

    // Start Conversion
    //ADCSRA |= (1<<ADSC);
}

int16_t hal_ain_get(void)
{
    int16_t retval = ADC>>1;
    ADCSRA |= (1<<ADSC);
    return retval;
}
#endif  //EXTAIN_USED
