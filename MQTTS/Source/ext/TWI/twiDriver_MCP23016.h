#ifndef _TWI_DRIVER_MCP23016_H
#define _TWI_DRIVER_MCP23016_H

#define MCP23016_START_ADDR         0x20
#define MCP23016_STOP_ADDR          0x27

#define MCP23016_MAX_DEV            4           // 8/4/2/1

uint8_t twi_MCP23016_Config(void);

#endif
