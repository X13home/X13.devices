#include "config.h"

#ifdef RFM12_PHY

#include <avr/interrupt.h>

#ifndef PRR
#define PRR     PRR0
#endif  //  PRR

#define RFM12_SELECT()     RFM12_PORT &= ~(1<<RFM12_PIN_SS);
#define RFM12_RELEASE()    RFM12_PORT |= (1<<RFM12_PIN_SS);

// HAL section
void hal_rfm12_init_hw(void)
{
    // ToDo IRQ only for UNode.
    // Disable IRQ
#ifdef RFM12_INT0           // IRQ on INT0
    EIMSK = 0;              // INT0 disable
#else                       // IRQ On PCINT
    PCMSK0 = 0;
#endif  //  RFM12_INT0

    // HW Initialise
    PRR &= ~(1<<PRSPI);                                 // Enable SPI
    RFM12_PORT |= (1<<RFM12_PIN_SS);
    RFM12_DDR  |= (1<<RFM12_PIN_SCK) | (1<<RFM12_PIN_MOSI) | (1<<RFM12_PIN_SS);
    RFM12_DDR &= ~(1<<RFM12_PIN_MISO);

    RFM12_IRQ_PORT |= RFM12_IRQ_PIN;

#if (F_CPU <= 5000000UL)    // Prescaler = 2
    SPCR = (1<<SPE) | (1<<MSTR);
    SPSR = (1<<SPI2X);
#elif (F_CPU <= 10000000UL) // Prescaler = 4
    SPCR = (1<<SPE) | (1<<MSTR);
    SPSR = 0;
#else // FCPU <= 20M, Prescaler = 8
    SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR0);
    SPSR = (1<<SPI2X);
#endif
}

// low level SPI exchange
uint16_t hal_rfm12_spiExch(uint16_t data)
{
    uint16_t retval;
    RFM12_SELECT();
    SPDR = data>>8;
    while(!(SPSR &(1<<SPIF)));          // Wait until SPI operation is terminated
    retval = SPDR<<8;
    SPDR = data & 0xFF;
    while(!(SPSR &(1<<SPIF)));          // Wait until SPI operation is terminated
    retval |= SPDR;
    RFM12_RELEASE();
    return retval;
}

bool hal_rfm12_irq_stat(void)
{
    return ((RFM12_IRQ_PORT_PIN & (1<<RFM12_IRQ_PIN)) == 0);
}

void hal_rfm12_enable_irq(void)
{
#ifdef RFM12_INT0
    EICRA = (0<<ISC00);
    EIMSK = (1<<INT0);          // INT0 int enable
#else           // IRQ On PBx
    PCMSK0 = (1<<RFM12_IRQ_PIN);
    PCICR = (1<<PCIE0);         // PCINTx int enable
#endif  //  RFM12_INT0
}

// defined in rfm12_phy.c
void rfm12_irq(void);


#ifdef RFM12_INT0
ISR(INT0_vect)
{
    rfm12_irq();
}
#else           // IRQ On PBx
ISR(PCINT0_vect)
{
    if(RFM12_IRQ_PORT_PIN & (1<<RFM12_IRQ_PIN))
        return;

    rfm12_irq();
}
#endif  //  RFM12_INT0

#endif  //  RFM12_PHY
