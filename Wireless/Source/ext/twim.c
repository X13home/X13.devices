/*
Copyright (c) 2011-2012 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com/

BSD License
See LICENSE.txt file for license details.
*/

// TWI(I2C) Prototypes

#include <util/twi.h>

#define TW_SUCCESS      0xFF

#define TWIM_READ       1           // Read Data
#define TWIM_WRITE      2           // Write Data
#define TWIM_SEQ        4           // Sequential Read - Not send stop after write access.
#define TWIM_BUSY       8           // Bus Busy

// Start TWI HAL
#define twiSendStop()   TWCR = (1<<TWEN) | (1<<TWINT) | (1<<TWSTO)

// Local Variables
/*
static uint8_t twi_trv_addr;
static uint8_t twi_trv_access;
static uint8_t twi_trv_len;
volatile uint8_t twi_trv_buf[4];

static uint8_t twi_bus_busy;
*/
/// new version
static uint8_t twim_addr;                           // Device address
volatile static uint8_t twim_access;                // access mode & busy flag
static uint8_t twim_bytes2write;                    // bytes to write
static uint8_t twim_bytes2read;                     // bytes to read
volatile static uint8_t * twim_ptr;                 // pointer to data buffer
static uint8_t twim_buf[4];                         // temporary buffer

// Read/Write data from/to buffer
static uint8_t twimExch(uint8_t addr, uint8_t access, uint8_t write, uint8_t read, uint8_t *pBuf)
{
    twim_addr = addr;
    twim_access = access;
    twim_bytes2write = write; 
    twim_bytes2read = read;
    twim_ptr = pBuf;
    
    uint8_t pos;

    while(twim_access & (TWIM_WRITE | TWIM_READ))
    {
        TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN); // Send START
        while(!(TWCR & (1<<TWINT)));                // Wait for TWI interrupt flag set
        if((TWSR != TW_START) &&                    // If status other than START transmitted(0x08)
           (TWSR != TW_REP_START))                  // or Repeated START transmitted(0x10)
        {
            twiSendStop();                          // Send stop
            return TWSR;                            //-> error
        }

        if(twim_access & TWIM_WRITE)
            TWDR = (twim_addr<<1) | TW_WRITE;
        else
            TWDR = (twim_addr<<1) | TW_READ;

        TWCR = (1<<TWINT) | (1<<TWEN);              // Clear int flag to send byte
        while(!(TWCR & (1<<TWINT)));                // Wait for TWI interrupt flag set
        if((TWSR != TW_MT_SLA_ACK) &&
            (TWSR != TW_MR_SLA_ACK))
        {
            twiSendStop();                          // Send stop
            return TWSR;                            // If NACK received return TWSR
        }

        if(twim_access & TWIM_WRITE)
        {
            pos = 0;
            while(pos < twim_bytes2write)
            {
                // Send one byte to the bus.
                TWDR = twim_ptr[pos++];
                TWCR = (1<<TWINT) | (1<<TWEN);          //  Clear int flag to send byte
                while(!(TWCR & (1<<TWINT)));            //  Wait for TWI interrupt flag set
                if(TWSR != TW_MT_DATA_ACK)              //  If NACK received return TWSR
                {
                    twiSendStop();                      // Send stop
                    return TWSR;
                }
            }

            if(!(twim_access & TWIM_SEQ))
                twiSendStop();                          // Send stop

            twim_access &= ~(TWIM_WRITE | TWIM_SEQ);
        }
        else                    // Read
        {
            pos = 0;
            while(pos < twim_bytes2read)
            {
                // Wait for TWINT to receive one byte from the slave and send ACK. 
                // If this is the last byte the master will send NACK to tell the slave 
                //  that it shall stop transmitting.
                TWCR = (1<<TWINT) | (1<<TWEN) | ((pos + 1) < twim_bytes2read ? (1<<TWEA) : 0);
                while(!(TWCR & (1<<TWINT)));            //  Wait for TWI interrupt flag set
                twim_ptr[pos++] = TWDR;
            }

            twiSendStop();                              // Send stop

            twim_access &= ~TWIM_READ;
        }
    }

    twim_access = 0;
    return TW_SUCCESS;
}

// Read/Write data with ISR & callback functions
static void twimExch_ISR(uint8_t addr, uint8_t access, uint8_t write, uint8_t read, uint8_t *pBuf)
{
    twim_addr = addr;
    twim_access = access;
    twim_bytes2write = write; 
    twim_bytes2read = read;
    twim_ptr = pBuf;

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
            if(twim_access & TWIM_WRITE)
                TWDR = (twim_addr<<1) | TW_WRITE;
            else
                TWDR = (twim_addr<<1) | TW_READ;
            TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT);
            break;
        case TW_MT_SLA_ACK:                     // SLA+W has been tramsmitted and ACK received
        case TW_MT_DATA_ACK:                    // Data byte has been tramsmitted and ACK received
            if(twi_ptr < twim_bytes2write)
            {
                TWDR = twim_ptr[twi_ptr++];
                TWCR = (1<<TWEN) |              // TWI Interface enabled
                       (1<<TWIE) | (1<<TWINT);  // Enable TWI Interupt and clear the flag to send byte
            }
            else    // End transmittion
            {
                if(!(twim_access & TWIM_SEQ))
                    TWCR = (1<<TWEN) |          // TWI Interface enabled
                           (1<<TWINT) |         // Disable TWI Interrupt and clear the flag
                           (1<<TWSTO);          // Initiate a STOP condition.
                           
                twim_access &= ~(TWIM_WRITE | TWIM_SEQ);

                if(twim_access & TWIM_READ)
                    TWCR = (1<<TWEN) |          // TWI Interface enabled.
                      (1<<TWIE) | (1<<TWINT) |  // Enable TWI Interupt and clear the flag.
                      (1<<TWSTA);               // Initiate a START condition.
            }
            break;
        case TW_MR_DATA_ACK:                    // Data byte has been received and ACK tramsmitted
            twim_ptr[twi_ptr++] = TWDR;
        case TW_MR_SLA_ACK:                     // SLA+R has been tramsmitted and ACK received
            if(twi_ptr < (twim_bytes2read - 1)) // Detect the last byte to NACK it.
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
            twim_ptr[twi_ptr++] = TWDR;
            TWCR = (1<<TWEN) |                  // TWI Interface enabled
                   (1<<TWINT) |                 // Disable TWI Interrupt and clear the flag
                   (1<<TWSTO);                  // Initiate a STOP condition.
            twim_access &= ~TWIM_READ;
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
    twim_access = 0;
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
    
    twim_access = 0;

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
