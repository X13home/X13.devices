/*
Copyright (c) 2011-2012 <comparator@gmx.de>

This file is part of the X13.Home project.
https://github.com/X13home

BSD License
See LICENSE.txt file for license details.
*/

// TWI(I2C) Prototypes

#include <util/twi.h>

#define TW_SUCCESS      0xFF
#define TW_SEQUENTIAL   2                           // Not send Stop sequence.

// Start TWI HAL
#define twiSendStop()   TWCR = (1<<TWEN) | (1<<TWINT) | (1<<TWSTO)

// Local Variables
static uint8_t twi_trv_addr;
static uint8_t twi_trv_access;
static uint8_t twi_trv_len;
volatile uint8_t twi_trv_buf[4];

static uint8_t twi_bus_busy;

// Read/Write data from/to buffer
static uint8_t twiExch(uint8_t addr, uint8_t access, uint8_t len, uint8_t *pBuf)
{
    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);     //  Send START
    while(!(TWCR & (1<<TWINT)));                    //  Wait for TWI interrupt flag set
    if((TWSR != TW_START) &&                        //  If status other than START transmitted(0x08)
       (TWSR != TW_REP_START))                      //  or Repeated START transmitted(0x10)
    {
        twiSendStop();                              // Send stop
        return TWSR;                                //-> error
    }
    // Send Address & read/write
    TWDR = addr<<1 | (access & TW_READ);
    TWCR = (1<<TWINT) | (1<<TWEN);                  // Clear int flag to send byte
    while(!(TWCR & (1<<TWINT)));                    // Wait for TWI interrupt flag set
    
    if((TWSR != TW_MT_SLA_ACK) && (TWSR != TW_MR_SLA_ACK))
    {
        twiSendStop();                              // Send stop
        return TWSR;                                // If NACK received return TWSR
    }

    while(len--)
    {
        if(access & TW_READ)    // Read
        {
            // Wait for TWINT to receive one byte from the slave and send ACK. If this is the last byte
            //  the master will send NACK to tell the slave that it shall stop transmitting.
            TWCR = (1<<TWINT) | (1<<TWEN) | (len ? (1<<TWEA) : 0);
            while(!(TWCR & (1<<TWINT)));        //  Wait for TWI interrupt flag set
            *(pBuf++) = TWDR;
        }
        else        // Write
        {
            // Send one byte to the bus.
            TWDR = *(pBuf++);
            TWCR = (1<<TWINT) | (1<<TWEN);      //  Clear int flag to send byte
            while(!(TWCR & (1<<TWINT)));        //  Wait for TWI interrupt flag set
            if(TWSR != TW_MT_DATA_ACK)          //  If NACK received return TWSR
            {
                twiSendStop();                  // Send stop
                return TWSR;
            }
        }
    }

    if(!(access & TW_SEQUENTIAL))
        twiSendStop();                          // Send stop
    return TW_SUCCESS;
}

// Read/Write data with ISR & callback functions
static void twiExch_ISR(uint8_t addr, uint8_t access, uint8_t len)
{
    twi_trv_addr = addr;
    twi_trv_access = access;
    twi_trv_len = len;
    
    TWCR = (1<<TWEN) |                          // TWI Interface enabled.
           (1<<TWIE) | (1<<TWINT) |             // Enable TWI Interupt and clear the flag.
           (1<<TWSTA);                          // Initiate a START condition.
}

ISR(TWI_vect)
{
    static uint8_t twi_ptr;
    switch(TWSR)
    {
        case TW_START:                          // START has been transmitted  
        case TW_REP_START:                      // Repeated START has been transmitted
            twi_ptr = 0;
            TWDR = (twi_trv_addr<<1) | (twi_trv_access & TW_READ);
            TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT);
            break;
        case TW_MT_SLA_ACK:                     // SLA+W has been tramsmitted and ACK received
        case TW_MT_DATA_ACK:                    // Data byte has been tramsmitted and ACK received
            if(twi_ptr < twi_trv_len)
            {
                TWDR = twi_trv_buf[twi_ptr++];
                TWCR = (1<<TWEN) |              // TWI Interface enabled
                       (1<<TWIE) | (1<<TWINT);  // Enable TWI Interupt and clear the flag to send byte
            }
            else    // End transmittion
            {
                if(!(twi_trv_access & TW_SEQUENTIAL))
                    TWCR = (1<<TWEN) |          // TWI Interface enabled
                           (1<<TWINT) |         // Disable TWI Interrupt and clear the flag
                           (1<<TWSTO);          // Initiate a STOP condition.
            }
            break;
        case TW_MR_DATA_ACK:                    // Data byte has been received and ACK tramsmitted
            twi_trv_buf[twi_ptr++] = TWDR;
        case TW_MR_SLA_ACK:                     // SLA+R has been tramsmitted and ACK received
            if(twi_ptr < (twi_trv_len - 1))     // Detect the last byte to NACK it.
            {
                TWCR = (1<<TWEN) |              // TWI Interface enabled
                       (1<<TWIE) | (1<<TWINT) | // Enable TWI Interupt and clear the flag to read next byte
                       (1<<TWEA);               // Send ACK after reception
            }
            else                                // Send NACK after next reception
            {
                TWCR = (1<<TWEN) |              // TWI Interface enabled
                       (1<<TWIE) | (1<<TWINT);  // Enable TWI Interupt and clear the flag to read next byte
            }    
            break; 
        case TW_MR_DATA_NACK:                   // Data byte has been received and NACK tramsmitted
            twi_trv_buf[twi_ptr++] = TWDR;
            TWCR = (1<<TWEN) |                  // TWI Interface enabled
                   (1<<TWINT) |                 // Disable TWI Interrupt and clear the flag
                   (1<<TWSTO);                  // Initiate a STOP condition.
            break;
        case TW_MR_ARB_LOST:                    // Arbitration lost
            TWCR = (1<<TWEN) |                  // TWI Interface enabled
                   (1<<TWIE) | (1<<TWINT) |     // Enable TWI Interupt and clear the flag
                   (1<<TWSTA);                  // Initiate a (RE)START condition.
            break;
        default:                                // Bus error
            TWCR = (1<<TWEN);                   // Enable TWI-interface and release TWI pins
                                                // Disable Interupt
    }
}
// End TWI HAL

#ifdef TWI_USE_BMP085
#include "twi/twiDriver_BMP085.c"
#endif  //  TWI_USE_BMP085

#ifdef TWI_USE_HIH61XX
#include "twi/twiDriver_HIH61XX.c"
#endif  //  TWI_USE_HIH61XX

#ifdef TWI_USE_SI7005
#include "twi/twiDriver_SI7005.c"
#endif  //  TWI_USE_SI7005

static void twiClean()
{
    twi_bus_busy = 0;
}

// Check & configure TWI devices
static void twiConfig(void)
{
    TWI_DISABLE();

    if((checkDigBase(TWI_PIN_SDA) != 0) || (checkDigBase(TWI_PIN_SCL) != 0))
        return;

    uint8_t mask = base2Mask(TWI_PIN_SDA) | base2Mask(TWI_PIN_SCL);

    if((inpPort(TWI_PIN_SDA) & mask) != mask)
        return;

    TWI_ENABLE();
    
    twi_bus_busy = 0;

    uint8_t cnt = 0;

#ifdef TWI_USE_BMP085
    cnt += twi_BMP085_Config();
#endif
#ifdef TWI_USE_HIH61XX
    cnt += twi_HIH61xx_Config();
#endif
#ifdef TWI_USE_SI7005
    cnt += twi_SI7005_Config();
#endif

    if(cnt == 0)
    {
        TWI_DISABLE();
        return;
    }

    indextable_t idx;
    idx.sidx.Place = objDin;
    idx.sidx.Type = objPinPNP;
    idx.sidx.Base = TWI_PIN_SDA;
    dioRegisterOD(&idx);
    idx.sidx.Base = TWI_PIN_SCL;
    dioRegisterOD(&idx);
}
