/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

// TWI(I2C) Prototypes

#include <util/twi.h>

#define TW_SUCCESS      0xFF

#define TWIM_READ       1               // Read Data
#define TWIM_WRITE      2               // Write Data
#define TWIM_BUSY       8               // Bus Busy
#define TWIM_ERROR      0x10            // Bus Error
#define TWIM_RELEASE    0x20            // Request to relase Bus

typedef void (*cbTWI)(void);            // TWI ISR ready, Callback function

// Local Variables
static uint8_t twim_addr;               // Device address
volatile static uint8_t twim_access;    // access mode & busy flag
static uint8_t twim_bytes2write;        // bytes to write
static uint8_t twim_bytes2read;         // bytes to read
volatile static uint8_t * twim_ptr;     // pointer to data buffer
static uint8_t twim_buf[4];             // temporary buffer
static cbTWI twim_callback;             // callback function

// Diag & WD variables
static uint8_t twim_addr_old;           // WatchDog addr
static uint8_t twim_busy_cnt;           // Busy counter

// Read/Write data from/to buffer
static uint8_t twimExch(uint8_t addr, uint8_t access, uint8_t write, uint8_t read, uint8_t *pBuf)
{
    twim_addr = (addr<<1);
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
            TWCR = (1<<TWEN) | (1<<TWINT) | (1<<TWSTO); // Send stop
            return TWSR;                            //-> error
        }

        if(twim_access & TWIM_WRITE)
            TWDR = twim_addr | TW_WRITE;
        else
            TWDR = twim_addr | TW_READ;

        TWCR = (1<<TWINT) | (1<<TWEN);              // Clear int flag to send byte
        while(!(TWCR & (1<<TWINT)));                // Wait for TWI interrupt flag set
        if((TWSR != TW_MT_SLA_ACK) &&
            (TWSR != TW_MR_SLA_ACK))
        {
            TWCR = (1<<TWEN) | (1<<TWINT) | (1<<TWSTO); // Send stop
            return TWSR;                            // If NACK received return TWSR
        }

        if(twim_access & TWIM_WRITE)
        {
            pos = 0;
            while(pos < twim_bytes2write)
            {
                // Send one byte to the bus.
                TWDR = twim_ptr[pos++];
                TWCR = (1<<TWINT) | (1<<TWEN);          // Clear int flag to send byte
                while(!(TWCR & (1<<TWINT)));            // Wait for TWI interrupt flag set

                if((pos < twim_bytes2write) &&          // Not Last Byte
                    (TWSR != TW_MT_DATA_ACK))           // If NACK received return TWSR
                {
                    TWCR = (1<<TWEN) | (1<<TWINT) | (1<<TWSTO); // Send stop
                    return TWSR;
                }
            }
            twim_access &= ~TWIM_WRITE;
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
            twim_access &= ~TWIM_READ;
        }
    }

    TWCR = (1<<TWEN) | (1<<TWINT) | (1<<TWSTO); // Send Stop

    twim_access = 0;
    twim_addr = 0;
    return TW_SUCCESS;
}

// Read/Write data with ISR
static void twimExch_ISR(uint8_t addr, uint8_t access, uint8_t write, uint8_t read, uint8_t *pBuf, cbTWI pCallback)
{
    if(TWIM_ERROR & twim_access)
        return;

    twim_addr = (addr<<1);
    twim_access = access;
    twim_bytes2write = write; 
    twim_bytes2read = read;
    twim_ptr = pBuf;
    twim_callback = pCallback;

    TWCR = (1<<TWEN) |                          // TWI Interface enabled.
           (1<<TWIE) | (1<<TWINT) |             // Enable TWI Interupt and clear the flag.
           (1<<TWSTA);                          // Initiate a START condition.
}

ISR(TWI_vect)
{
    static uint8_t twi_ptr;
    switch(TW_STATUS)
    {
        case TW_START:                          // START has been transmitted  
        case TW_REP_START:                      // Repeated START has been transmitted
            twi_ptr = 0;
            if(twim_access & TWIM_WRITE)
                TWDR = twim_addr | TW_WRITE;
            else
                TWDR = twim_addr | TW_READ;
            TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT);
            break;
        case TW_MT_SLA_ACK:                     // SLA+W has been tramsmitted and ACK received
        case TW_MT_DATA_ACK:                    // Data byte has been tramsmitted and ACK received
            if(twi_ptr < twim_bytes2write)
            {
                TWDR = twim_ptr[twi_ptr++];
                TWCR = (1<<TWEN) |              // TWI Interface enabled
                       (1<<TWIE) | (1<<TWINT);  // Enable TWI Interupt and clear the flag to send byte
                break;
            }
        case TW_MT_DATA_NACK:                   // End transmittion
            twim_access &= ~TWIM_WRITE;

            if(twim_access & TWIM_READ)
                TWCR = (1<<TWEN) |              // TWI Interface enabled.
                      (1<<TWIE) | (1<<TWINT) |  // Enable TWI Interupt and clear the flag.
                      (1<<TWSTA);               // Initiate a START condition.
            else
            {
                TWCR = (1<<TWEN) | (1<<TWINT) | (1<<TWSTO); // Send Stop
                if(twim_callback != NULL)
                    (twim_callback)();
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
                       (1<<TWIE) | (1<<TWINT);  // Enable TWI Interupt and clear the flag to read last byte
            }
            break;
        case TW_MR_DATA_NACK:                   // Data byte has been received and NACK tramsmitted
            twim_ptr[twi_ptr++] = TWDR;
            TWCR = (1<<TWEN) | (1<<TWINT) | (1<<TWSTO); // Send Stop
            twim_access &= ~TWIM_READ;
            if(twim_callback != NULL)
                (twim_callback)();
            break;
        default:                                // Error
            TWCR = (1<<TWEN) | (1<<TWINT) | (1<<TWSTO); // Send Stop, Disable Interupt
            break;
    }
}
// End TWI HAL

#ifdef TWI_USE_BMP180
#include "twi/twiDriver_BMP180.c"
#endif  //  TWI_USE_BMP180

#ifdef TWI_USE_HIH61XX
#include "twi/twiDriver_HIH61XX.c"
#endif  //  TWI_USE_HIH61XX

#ifdef TWI_USE_SI7005
#include "twi/twiDriver_SI7005.c"
#endif  //  TWI_USE_SI7005

#ifdef TWI_USE_LM75
#include "twi/twiDriver_LM75.c"
#endif  //  TWI_USE_LM75

static void twiClean()
{
    twim_access = 0;
    twim_addr = 0xFF;
    twim_addr_old = 0;
    twim_callback = NULL;
}

static uint8_t twim_read(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
    *pLen = 1;
    *pBuf = (twim_addr>>1);
    return MQTTS_RET_ACCEPTED;
}

static uint8_t twim_write(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf)
{
    twim_addr_old = *pBuf;
    return MQTTS_RET_ACCEPTED;
}

static uint8_t twim_pool(subidx_t * pSubidx)
{
    if(twim_access == 0)
    {
        twim_addr_old = 0xFF;
        return 0;
    }

    if(twim_addr_old != twim_addr)
    {
        twim_addr_old = twim_addr;
        twim_busy_cnt = 0;
        return 0;
    }

    if((twim_access & (TWIM_ERROR | TWIM_WRITE | TWIM_READ)) > TWIM_ERROR)
    {
        twim_busy_cnt = 0xC0;
        twim_access = TWIM_ERROR;
        return 1;
    }

    twim_busy_cnt++;
    if(twim_busy_cnt == 0xC0)   // bus busy too long
    {
        TWCR = (1<<TWEN) | (1<<TWINT) | (1<<TWSTO); // Send Stop
        twim_access = TWIM_ERROR;
        return 1;
    }
    else if(twim_busy_cnt == 0)
    {
        twim_access = 0;
        twim_addr_old = 0xFF;
    }

    return 0;
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

    uint8_t cnt = 0;
    indextable_t * pIndex;
    
    pIndex = getFreeIdxOD();
    if(pIndex == NULL)
        return;

#ifdef TWI_USE_BMP180
    cnt += twi_BMP180_Config();
#endif
#ifdef TWI_USE_HIH61XX
    cnt += twi_HIH61xx_Config();
#endif
#ifdef TWI_USE_SI7005
    cnt += twi_SI7005_Config();
#endif
#ifdef TWI_USE_LM75
    cnt += twi_LM75_Config();
#endif

    if(cnt == 0)
    {
        pIndex->Index = 0xFFFF;
        TWI_DISABLE();
        return;
    }

    twiClean();

    pIndex->sidx.Place = objDin;
    pIndex->sidx.Type = objPinPNP;
    pIndex->sidx.Base = TWI_PIN_SDA;
    dioRegisterOD(pIndex);
    pIndex->sidx.Base = TWI_PIN_SCL;
    dioRegisterOD(pIndex);

    // Status Register
    pIndex->cbRead  =  &twim_read;
    pIndex->cbWrite =  &twim_write;
    pIndex->cbPool  =  &twim_pool;
    
    pIndex->sidx.Place = objTWI;
    pIndex->sidx.Type =  objUInt8;
    pIndex->sidx.Base = 0;
}
