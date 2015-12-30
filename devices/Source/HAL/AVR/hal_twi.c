#include "../../config.h"

#ifdef EXTTWI_USED

#include "../../EXT/exttwi.h"

#include <avr/interrupt.h>
#include <util/twi.h>

#if (defined __AVR_ATmega328P__)
#define TWIM_SCL_STAT()             (PINC & (1<<PC5))
#define I2C_DIO_SDA                 20
#define I2C_DIO_SCL                 21
#elif (defined __AVR_ATmega1284P__) || (defined __AVR_ATmega164P__) || (defined __AVR_ATmega164PA__)
#define TWIM_SCL_STAT()             (PINC & (1<<PC0))
#define I2C_DIO_SCL                 16
#define I2C_DIO_SDA                 17
#elif defined (__AVR_ATmega2560__)
#define TWIM_SCL_STAT()             (PIND & (1<<PD0))
#define I2C_DIO_SCL                 24
#define I2C_DIO_SDA                 25
#endif  // uC

// Global variable defined in exttwi.c
extern volatile TWI_QUEUE_t * pTwi_exchange;

void hal_twi_get_pins(uint8_t * pSCL, uint8_t * pSDA)
{
    *pSCL = I2C_DIO_SCL;
    *pSDA = I2C_DIO_SDA;
}

// HAL
bool hal_twi_configure(uint8_t enable)
{
    // Disable TWI
    TWCR = (1<<TWINT);

    if(enable)
    {
        if(TWIM_SCL_STAT() == 0)
            return false;

        // Set Speed
        TWBR = (((F_CPU/100000UL)-16)/2);   // 100kHz
        // Clear data register
        TWDR = 0xFF;
        // Enable TWI & Interrupt
        TWCR = (1<<TWEN) | (1<<TWIE);
    }

    return true;
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
        pTwi_exchange->frame.access |= TWI_BUSY;

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
            
            if(pTwi_exchange->frame.access & TWI_WRITE)
                TWDR = (pTwi_exchange->frame.address<<1);
            else
                TWDR = (pTwi_exchange->frame.address<<1) | TW_READ;
            TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
            break;
        // Master Send
        case TW_MT_SLA_NACK:                        // SLA+W transmitted, NACK received
        case TW_MR_SLA_NACK:                        // SLA+R transmitted, NACK received
            pTwi_exchange->frame.read = 0;
            pTwi_exchange->frame.access |= TWI_SLANACK;
            TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN) | (1<<TWIE); // Send Stop
            break;
        case TW_MT_SLA_ACK:                         // SLA+W transmitted, ACK received
        case TW_MT_DATA_ACK:                        // data transmitted, ACK received
            if(twi_ptr < pTwi_exchange->frame.write)
            {
                TWDR = pTwi_exchange->frame.data[twi_ptr];
                twi_ptr++;
                TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
                break;
            }
            // else, ACK received but should be NACK
            // No break, it's OK
        case TW_MT_DATA_NACK:                       // data transmitted, NACK received
            pTwi_exchange->frame.write = twi_ptr;
            pTwi_exchange->frame.access &= ~TWI_WRITE;
            if((pTwi_exchange->frame.access & TWI_READ) == 0)
            {
                pTwi_exchange->frame.read = 0;
                pTwi_exchange->frame.access = TWI_RDY;
                TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN) | (1<<TWIE); // Send Stop
            }
            else
                TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT) | (1<<TWSTA); // Send RepSTART
            break;
        // Master Receive
        case TW_MR_DATA_ACK:                        // Data byte has been received and ACK transmitted
            pTwi_exchange->frame.data[twi_ptr++] = TWDR;
            // No break, it's OK
        case TW_MR_SLA_ACK:                         // SLA+R has been transmitted and ACK received
            if((twi_ptr + 1) < pTwi_exchange->frame.read)
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
            pTwi_exchange->frame.data[twi_ptr++] = TWDR;
            pTwi_exchange->frame.read = twi_ptr;
            pTwi_exchange->frame.access = TWI_RDY;
            TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN) | (1<<TWIE); // Send Stop
            break;
        default:                                    // Error
            TWCR &= ~((1<<TWEN) | (1<<TWSTO));
            TWCR |= (1<<TWINT) | (1<<TWEN);
            pTwi_exchange->frame.write = twi_ptr;
            pTwi_exchange->frame.read = 1;
            pTwi_exchange->frame.data[0] = TWSR;
            pTwi_exchange->frame.access |= TWI_ERROR;
            break;
    }
}
#endif  //  EXTTWI_USED
