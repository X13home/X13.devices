#ifndef _TWI_DRIVER_LM75_H
#define _TWI_DRIVER_LM75_H

#define LM75_START_ADDR             0x48
#define LM75_STOP_ADDR              0x4F

#define LM75_MAX_DEV                8           // 8/4/2/1

// LM75 Registers
#define LM75_REG_TEMP               0x00        // Temperature
#define LM75_REG_CONF               0x01        // Configuration
#define LM75_REG_THYST              0x02        // Hysteresis
#define LM75_REG_TOS                0x03        // Over temperature

// Config Register
#define LM75_CONFIG_PD              0x01        // shutdown, 0 - Normal operation, 1 - shutdown
#define LM75_CONFIG_OS_MODE_INT     0x02        // OS mode 0 -  Comparator, 1 - interrupt
#define LM75_CONFIG_OS_POL_HI       0x04        // OS active level; 0 - Low, 1 - High
#define LM75_CONFIG_QS_QUE_1        0x00        // OS fault queue = 1
#define LM75_CONFIG_QS_QUE_2        0x08        // OS fault queue = 2
#define LM75_CONFIG_QS_QUE_4        0x10        // OS fault queue = 4
#define LM75_CONFIG_QS_QUE_6        0x18        // OS fault queue = 6

uint8_t twi_LM75_Config(void);

#endif
