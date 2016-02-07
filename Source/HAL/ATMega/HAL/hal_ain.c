#include "config.h"

#ifdef EXTAIN_USED

#include <avr/pgmspace.h>
static const PROGMEM uint8_t hal_ainBase2Apin[] = EXTAIN_BASE_2_APIN;
static const PROGMEM uint8_t hal_ainBase2Dio[] = HAL_AIN_BASE2DIO;

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

void hal_ain_configure(uint8_t unused, uint8_t aref)
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
