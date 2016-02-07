#include "config.h"

#ifdef EXTDIO_USED

#include <avr/pgmspace.h>

static const PROGMEM uint16_t hal_dio_portnum2port[] = HAL_DIO_PORTNUM2PORT;
static const PROGMEM uint8_t  hal_dio_sBase2Base[]   = HAL_DIO_MAPPING;

uint8_t hal_dio_base2pin(uint16_t base)
{
    if(base >= sizeof(hal_dio_sBase2Base))
        return 0xFF;
    
    return pgm_read_byte(&hal_dio_sBase2Base[base]);
}

// Input: Hardware Pin
void hal_dio_configure(uint8_t Pin, uint8_t Mode)
{
    uint8_t PortNr = Pin;
    PortNr >>= 3;
    uint16_t base = pgm_read_word(&hal_dio_portnum2port[PortNr]);
    
    uint8_t Mask = 1;
    uint8_t pos = Pin & 7;
    while(pos > 0)
    {
        pos--;
        Mask <<= 1;
    }

    uint8_t *pPORT;
    pPORT = (uint8_t *)base;
    uint8_t *pDDR;
    pDDR = (uint8_t *)(base - 1);

    switch(Mode)
    {
        case DIO_MODE_IN_PU:
            *pPORT |= Mask;
            *pDDR  &= ~Mask;
            break;
        case DIO_MODE_OUT_PP:
            *pDDR  |= Mask;
            break;
        default:
            *pPORT &= ~Mask;
            *pDDR  &= ~Mask;
        break;
    }
}

uint8_t hal_dio_read(uint8_t PortNr)
{
    uint8_t * pPIN = (uint8_t *)(pgm_read_word(&hal_dio_portnum2port[PortNr]) - 2);
    return *pPIN;
}

void hal_dio_set(uint8_t PortNr, uint8_t Mask)
{
    uint8_t * pPORT = (uint8_t *)(pgm_read_word(&hal_dio_portnum2port[PortNr]));
    *pPORT |= Mask;
}

void hal_dio_reset(uint8_t PortNr, uint8_t Mask)
{
    uint8_t * pPORT = (uint8_t *)(pgm_read_word(&hal_dio_portnum2port[PortNr]));
    *pPORT &= ~Mask;
}

#endif  //  EXTDIO_USED
