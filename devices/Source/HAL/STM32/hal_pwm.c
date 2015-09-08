#include "../../config.h"

#ifdef EXTPWM_USED

static const uint8_t  hal_pwm_port2dio[] = EXTPWM_PORT2DIO;

uint8_t hal_pwm_base2dio(uint16_t base)
{
    if(base >= sizeof(hal_pwm_port2dio))
        return 0xFF;
    
    return hal_pwm_port2dio[base];
}

void hal_pwm_configure(uint16_t base)
{

}

void hal_pwm_delete(uint16_t base)
{

}

void hal_pwm_write(uint16_t base, uint16_t value)
{

}


#endif  //  EXTPWM_USED