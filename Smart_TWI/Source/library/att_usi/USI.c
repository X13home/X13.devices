/*
 * USI.c, TWI/I2C Slave
 *
 Copyright (c) 2013-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org

BSD New License
See LICENSE.txt file for license details.
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>

#include "USI.h"
#include "../smart/smart.h"

typedef enum{
  USI_SLAVE_UNDEFINED = 0,
  USI_SLAVE_CHECK_ADDRESS,
  USI_SLAVE_SEND_DATA,
  USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA,
  USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA,
  USI_SLAVE_REQUEST_DATA,
  USI_SLAVE_GET_DATA_AND_SEND_ACK,
  USI_SLAVE_DATA_READY,
}uUSI_STAT;

// Local Variables
static uint8_t TWI_State;
static uint8_t TWI_OwnAddr;
 
// Initialise USI driver, Address 8 - 31
void InitUSI(uint8_t tAddr)
{
  TWI_OwnAddr = tAddr<<1;
  TWI_State = USI_SLAVE_UNDEFINED;

  USI_PORT |= (1<<SCL) | (1<<SDA);    // SDA & SCL PullUp On 
  USI_DDR |= (1<<SCL);                // SCL Output
  USI_DDR &= ~(1<<SDA);               // SDA Input

  USISR = (1<<USISIF) |   // Clear All interrupt flags & reset counter
          (1<<USIOIF) |
          (1<<USIPF)  |
          (1<<USIDC);
  USICR = (1<<USISIE) |   // Start Condition Interrupt Enable
          (1<<USIWM1) |   // Two-wire mode
          (1<<USICS1);    // External Clock, Sample on positive edge
}

// Return Own TWI address
uint8_t TWI_addr(void)
{
  return (TWI_OwnAddr>>1);
}

ISR(USI_START_vect)
{
  TWI_State = USI_SLAVE_CHECK_ADDRESS;    // set default starting conditions for new TWI package
  USI_DDR  &= ~(1<<SDA);                  // set SDA as input
  
  while ((USI_PIN & (1<<SCL)) &&          // wait for SCL to go low to ensure the Start Condition has completed
        !(USI_PIN & (1<<SDA)));

  if(!(USI_PIN & (1<<SDA)))   // a Stop Condition did not occur
    USICR = (1<<USISIE) | (1<<USIOIE) | (1<<USIWM1) | (1<<USIWM0) | (1<<USICS1);
  else  // a Stop Condition did occur
    USICR = (1<<USISIE) | (1<<USIWM1) | (1<<USICS1);

  // clear interrupt flags - resetting the Start Condition Flag will release SCL
  USISR = (1<<USISIF) | (1<<USIOIF) | (1<<USIPF) | (1<<USIDC);
} // ISR(USI_START_vect)

ISR(USI_OVF_vect)
{
  uint8_t data = USIDR;
  switch(TWI_State)
  {
    case USI_SLAVE_CHECK_ADDRESS:
      if(((data & 0xFE) == TWI_OwnAddr) ||      // Address match
         ((data & 0xFE) == 0))                  // Broadcast
      {
        if(data & 0x01)                         // Read
        {
          TWI_State = USI_SLAVE_SEND_DATA;
          smart_reset_offs();
        }
        else
        {
          TWI_State = USI_SLAVE_REQUEST_DATA;
          smart_reset_reg();
        }

        // Set USI to send ACK
        USIDR = 0;                                                        // Prepare ACK
        USI_DDR |= (1<<SDA);                                              // Set SDA as output
        USISR = (1<<USIOIF) | (1<<USIPF) | (1<<USIDC) | (0x0E<<USICNT0);  // Clear all flags, except Start Cond, set USI counter to shift 1 bit.
      }
      else
      {
        // Set TWI to TWI START CONDITION MODE
        USICR = (1<<USISIE) | (1<<USIWM1) | (1<<USICS1);
        USISR = (1<<USIOIF) | (1<<USIPF) | (1<<USIDC);
      }
      break;
    // ----- Master write data mode ------
    // Check reply and goto USI_SLAVE_SEND_DATA if OK, else reset USI.
    case USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA:
      if(data)  // if NACK, the master does not want more data
      {
        // Set TWI to TWI START CONDITION MODE
        USICR = (1<<USISIE) | (1<<USIWM1) | (1<<USICS1);
        USISR = (1<<USIOIF) | (1<<USIPF) | (1<<USIDC);
        return;
      }
    case USI_SLAVE_SEND_DATA:
      USIDR = smart_read_data();
      USI_DDR |= (1<<SDA);
      USISR = (1<<USIOIF) | (1<<USIPF) | (1<<USIDC);
      TWI_State = USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA;
      break;
    case USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA:
      // Set USI to read  ACK
      USI_DDR &= ~(1<<SDA);
      USIDR = 0;
      USISR = (1<<USIOIF) | (1<<USIPF) | (1<<USIDC) | (0x0E<<USICNT0);
      TWI_State = USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA;
      break;

    // Master read data mode: set USI to sample data from master
    case USI_SLAVE_REQUEST_DATA:
      TWI_State = USI_SLAVE_GET_DATA_AND_SEND_ACK;
    case USI_SLAVE_DATA_READY:
      USI_DDR &= ~(1<<SDA);
      USISR = (1<<USIOIF) | (1<<USIPF) | (1<<USIDC);
      break;
    case USI_SLAVE_GET_DATA_AND_SEND_ACK:
      if(smart_write_data(data))
      {
        USIDR = 0xFF;   //  Send NACK
        TWI_State = USI_SLAVE_DATA_READY;
      }
      else
      {
        USIDR = 0;      //  Send ACK
        TWI_State = USI_SLAVE_REQUEST_DATA;
      }

      USI_DDR |= (1<<SDA);
      USISR = (1<<USIOIF) | (1<<USIPF) | (1<<USIDC) | (0x0E<<USICNT0);
      break;
  }
} // ISR(USI_OVF_vect)
