#include "../../config.h"

#ifdef EXTDIO_USED

#include <avr/pgmspace.h>

static const PROGMEM uint16_t hal_dio_portnum2port[] = 
{
    #ifdef PORTA
        (uint16_t)&PORTA,
    #else
        0xFFFF,
    #endif  //  PORTA
    #ifdef PORTB
        (uint16_t)&PORTB,
    #else
        0xFFFF,
    #endif  //  PORTB
    #ifdef PORTC
        (uint16_t)&PORTC,
    #else
        0xFFFF,
    #endif  //  PORTC
    #ifdef PORTD
        (uint16_t)&PORTD,
    #else
        0xFFFF,
    #endif  //  PORTD
    #ifdef PORTE
        (uint16_t)&PORTE,
    #else
        0xFFFF,
    #endif  //  PORTE
    #ifdef PORTF
        (uint16_t)&PORTF,
    #else
        0xFFFF,
    #endif  //  PORTF
    #ifdef PORTG
        (uint16_t)&PORTG,
    #else
        0xFFFF,
    #endif  //  PORTG
    #ifdef PORTH
        (uint16_t)&PORTH,
    #else
        0xFFFF,
    #endif  //  PORTH
    #ifdef PORTI
        (uint16_t)&PORTI,
    #else
        0xFFFF,
    #endif  //  PORTI
    #ifdef PORTJ
        (uint16_t)&PORTJ,
    #else
        0xFFFF,
    #endif  //  PORTJ
    #ifdef PORTK
        (uint16_t)&PORTK,
    #else
        0xFFFF,
    #endif  //  PORTK
    #ifdef PORTL
        (uint16_t)&PORTL
    #else
        0xFFFF
    #endif  //  PORTL
};

static const PROGMEM uint8_t  hal_dio_sBase2Base[] = EXTDIO_MAPPING;

/*
void hal_dio_pin2hw(uint8_t pin uint8_t *pPort, uint8_t *pMask)
{
    uint8_t port = pin>>3;
#ifdef EXTDIO_BASE_OFFSET
    port -= EXTDIO_BASE_OFFSET;
#endif  //  EXTDIO_BASE_OFFSET
    *pPort = port;

    uint8_t mask = 1;
    pin &= 7;
    while(pin--)
        mask <<= 1;
    *pMask = mask;
}
*/

void hal_dio_base2hw(uint16_t base, uint8_t *pPort, uint8_t *pMask)
{
    uint8_t sbase;

    if((base >= sizeof(hal_dio_sBase2Base)) ||
       ((sbase = pgm_read_word(&hal_dio_sBase2Base[base])) == 0xFF))
    {
        *pPort = 0xFF;
        *pMask = 0;
        return;
    }

    uint8_t port = sbase>>3;
#ifdef EXTDIO_BASE_OFFSET
    port -= EXTDIO_BASE_OFFSET;
#endif  //  EXTDIO_BASE_OFFSET
    *pPort = port;

    uint8_t mask = 1;
    sbase &= 7;
    while(sbase--)
        mask <<= 1;
    *pMask = mask;
}

void hal_dio_configure(uint8_t PortNr, uint8_t Mask, uint16_t Mode)
{
#ifdef EXTDIO_BASE_OFFSET
    PortNr += EXTDIO_BASE_OFFSET;
#endif  //  EXTDIO_BASE_OFFSET

    uint16_t base = pgm_read_word(&hal_dio_portnum2port[PortNr]);

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
//        case DIO_MODE_PWM:
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
#ifdef EXTDIO_BASE_OFFSET
    PortNr += EXTDIO_BASE_OFFSET;
#endif  //  EXTDIO_BASE_OFFSET
    
    uint8_t * pPIN = (uint8_t *)(pgm_read_word(&hal_dio_portnum2port[PortNr]) - 2);
    return *pPIN;
}

void hal_dio_set(uint8_t PortNr, uint8_t Mask)
{
#ifdef EXTDIO_BASE_OFFSET
    PortNr += EXTDIO_BASE_OFFSET;
#endif  //  EXTDIO_BASE_OFFSET

    uint8_t * pPORT = (uint8_t *)(pgm_read_word(&hal_dio_portnum2port[PortNr]));
    *pPORT |= Mask;
}

void hal_dio_reset(uint8_t PortNr, uint8_t Mask)
{
#ifdef EXTDIO_BASE_OFFSET
    PortNr += EXTDIO_BASE_OFFSET;
#endif  //  EXTDIO_BASE_OFFSET
    
    uint8_t * pPORT = (uint8_t *)(pgm_read_word(&hal_dio_portnum2port[PortNr]));
    *pPORT &= ~Mask;
}

#endif  //  EXTDIO_USED