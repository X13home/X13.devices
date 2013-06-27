#ifndef _TWI_DRIVER_BLINKM_H
#define _TWI_DRIVER_BLINKM_H

#define BLINKM_START_ADDR           8
#define BLINKM_STOP_ADDR            31

#define BLINKM_MAX_DEV              4           // 8/4/2/1

uint8_t twi_BlinkM_Config(void);

#endif
