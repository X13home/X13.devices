/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE file for license details.
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>

#include "../smart/smart.h"

// Local Variables
static uint8_t TWI_OwnAddr;

void InitTWI(uint8_t tAddr)
{
  TWI_OwnAddr = tAddr<<1;
  TWAR = TWI_OwnAddr;
  TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT) | (1<<TWEA);
}

uint8_t TWI_addr(void)
{
  return TWI_OwnAddr;
}

ISR(TWI_vect)
{
  switch (TWSR & TW_STATUS_MASK)
  {
    case TW_SR_SLA_ACK:           // 0x60, SLA+W received, ACK returned
      smart_reset_reg();
      break;
    case TW_SR_DATA_ACK:          // 0x80, Previously addressed with own SLA+W; data has been received; ACK has been returned
      if(smart_write_data(TWDR) != 0)
        TWCR = (1<<TWINT);
      break;
    case TW_ST_SLA_ACK:           // 0xA8,  SLA+R received, ACK returned
      smart_reset_offs();
    case TW_ST_DATA_ACK:          // 0xB8, data transmitted, ACK received
      TWDR = smart_read_data();
      break;
    case TW_ST_DATA_NACK:         // 0xC0, data transmitted, NACK received
      TWCR = (1<<TWINT);
      break;
    default:
      break;

//    case TW_SR_STOP:              // 0xA0, stop or repeated start condition received while selected 
//      TWCR = (1<<TWEA) | (1<<TWEN) | (1<<TWIE);
//      break;
/*
    case TW_SR_GCALL_DATA_NACK:
    case TW_SR_DATA_NACK:
    case TW_ST_LAST_DATA:
    case TW_BUS_ERROR:
      TWCR = (1<<TWSTO) | (1<<TWINT);
      break;
*/
  }

  TWCR = (1<<TWINT) | (1<<TWEA) | (1<<TWEN) | (1<<TWIE);
}
