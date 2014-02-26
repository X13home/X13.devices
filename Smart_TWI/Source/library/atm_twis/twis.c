/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org

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
  switch (TWSR)
  {
    case TW_ST_SLA_ACK:
    case TW_ST_DATA_ACK:
      TWDR = smart_read_data();
      TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT) | (1<<TWEA);
      break;
    case TW_SR_SLA_ACK:
      smart_reset_reg();
      TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT) | (1<<TWEA);
      break;
    case TW_SR_DATA_ACK:
      smart_write_data(TWDR);
    default:
      TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT) |(1<<TWEA);
  }
}
