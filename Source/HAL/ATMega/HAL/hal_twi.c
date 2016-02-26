#include "config.h"

#ifdef EXTTWI_USED

#include "EXT/exttwi.h"

#include <avr/interrupt.h>
#include <util/twi.h>

// Global variable defined in exttwi.c
extern volatile TWI_QUEUE_t * pTWI;

void hal_twi_get_pins(uint8_t * pSCL, uint8_t * pSDA)
{
    *pSCL = I2C_DIO_SCL;
    *pSDA = I2C_DIO_SDA;
}

// HAL
bool hal_twi_configure(uint8_t enable)
{
    TWCR = (1<<TWINT);

    if(enable)
    {
        if(TWIM_SCL_STAT() == 0)
            return false;

        // Set Speed
        TWBR = (((F_CPU/100000UL)-16)/2);   // 100kHz
        TWDR = 0xFF;                        // Clear data register
        TWCR = (1<<TWEN) | (1<<TWINT);      // Enable TWI and clear the interrupt flag.
        TWCR |= (1<<TWIE);                  // Enable TWI Interrupt

        return true;
    }

    return false;
}

void hal_twi_stop(void)
{
    TWCR &= ~((1<<TWEN) | (1<<TWSTO));
    TWCR |= (1<<TWINT) | (1<<TWEN);
}

void hal_twi_start(void)
{
    if(TWIM_SCL_STAT() != 0)    // Bus Free
    {
        pTWI->frame.access |= TWI_BUSY;

        TWCR = (1<<TWEN) |                      // TWI Interface enabled.
               (1<<TWIE) | (1<<TWINT) |         // Enable TWI Interrupt and clear the flag.
               (1<<TWSTA);                      // Initiate a START condition.
    }
}

ISR(TWI_vect)
{
    static uint8_t twi_ptr;

    switch(TW_STATUS)
    {
        // Master
        case TW_START:                              // start condition transmitted
        case TW_REP_START:                          // repeated start condition transmitted
            twi_ptr = 0;
            
            if(pTWI->frame.access & TWI_WRITE)
                TWDR = (pTWI->frame.address<<1);
            else
                TWDR = (pTWI->frame.address<<1) | TW_READ;
            TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
            break;
        // Master Send
        case TW_MT_SLA_NACK:                        // SLA+W transmitted, NACK received
        case TW_MR_SLA_NACK:                        // SLA+R transmitted, NACK received
            pTWI->frame.read = 0;
            pTWI->frame.access |= TWI_SLANACK;
            TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN) | (1<<TWIE); // Send Stop
            break;
        case TW_MT_SLA_ACK:                         // SLA+W transmitted, ACK received
        case TW_MT_DATA_ACK:                        // data transmitted, ACK received
            if(twi_ptr < pTWI->frame.write)
            {
                TWDR = pTWI->frame.data[twi_ptr];
                twi_ptr++;
                TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
                break;
            }
            // else, ACK received but should be NACK
            // No break, it's OK
        case TW_MT_DATA_NACK:                       // data transmitted, NACK received
            pTWI->frame.write = twi_ptr;
            pTWI->frame.access &= ~TWI_WRITE;
            if((pTWI->frame.access & TWI_READ) == 0)
            {
                pTWI->frame.read = 0;
                pTWI->frame.access = TWI_RDY;
                TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN) | (1<<TWIE); // Send Stop
            }
            else
                TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT) | (1<<TWSTA); // Send RepSTART
            break;
        // Master Receive
        case TW_MR_DATA_ACK:                        // Data byte has been received and ACK transmitted
            pTWI->frame.data[twi_ptr++] = TWDR;
            // No break, it's OK
        case TW_MR_SLA_ACK:                         // SLA+R has been transmitted and ACK received
            if((twi_ptr + 1) < pTWI->frame.read)
            {
                TWCR = (1<<TWEN) |                  // TWI Interface enabled
                       (1<<TWIE) | (1<<TWINT) |     // Enable TWI Interrupt and clear the flag to read next byte
                       (1<<TWEA);                   // Send ACK after reception
            }
            else                                    // Send NACK after next reception
            {
                TWCR = (1<<TWEN) |                  // TWI Interface enabled
                       (1<<TWIE) | (1<<TWINT);      // Enable TWI Interrupt and clear the flag to read last byte
            }
            break;
        case TW_MR_DATA_NACK:                       // Data byte has been received and NACK transmitted
            pTWI->frame.data[twi_ptr++] = TWDR;
            pTWI->frame.read = twi_ptr;
            pTWI->frame.access = TWI_RDY;
            TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN) | (1<<TWIE); // Send Stop
            break;
        default:                                    // Error
            TWCR &= ~((1<<TWEN) | (1<<TWSTO));
            TWCR |= (1<<TWINT) | (1<<TWEN);
            pTWI->frame.write = twi_ptr;
            pTWI->frame.read = 1;
            pTWI->frame.data[0] = TWSR;
            pTWI->frame.access |= TWI_ERROR;
            break;
    }
}
#endif  //  EXTTWI_USED
