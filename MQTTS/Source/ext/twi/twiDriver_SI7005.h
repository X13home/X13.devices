#ifndef _TWI_DRIVER_SI7005_H
#define _TWI_DRIVER_SI7005_H

#define SI7005_ADDR                 0x40

// Si7005 Registers
#define SI7005_REG_STATUS           0x00
#define SI7005_REG_DATA             0x01
#define SI7005_REG_CONFIG           0x03
#define SI7005_REG_ID               0x11

// Status Register
#define SI7005_STATUS_NOT_READY     0x01

// Config Register
#define SI7005_CONFIG_START         0x01
#define SI7005_CONFIG_HEAT          0x02
#define SI7005_CONFIG_HUMIDITY      0x00
#define SI7005_CONFIG_TEMPERATURE   0x10
#define SI7005_CONFIG_FAST          0x20

// ID Register
#define SI7005_ID_SI7005            0x50

uint8_t twi_SI7005_Config(void);

#endif

