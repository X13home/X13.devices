#include "config.h"

#if (defined EXTDIO_USED) && (defined EXTPWM_USED) && (defined STM32F1)

// Mapping physical DIO channel to configuration
// bits 7-3 Timer, bits 2-0 Channel
static const uint8_t hal_pwm_port2cfg[] = {
                                            ((2<<3) | 0),   // Timer2, Channel 1
                                            ((2<<3) | 1),   // Timer2, Channel 2
                                            ((2<<3) | 2),   // Timer2, Channel 3
                                            ((2<<3) | 3)    // Timer2, Channel 4
                                            }
                                            
static const uint8_t hal_pwm_port2dio[] = EXTPWM_PORT2DIO;

uint8_t hal_pwm_base2dio(uint16_t base)
{
#ifdef EXTPWM_BASE_OFFSET
    if(base < EXTPWM_BASE_OFFSET)
        return 0xFF;
    base -= EXTPWM_BASE_OFFSET;
#endif

    if(base >= (const uint16_t)(sizeof(hal_pwm_port2dio)))
        return 0xFF;

    return hal_pwm_port2dio[base];
}

void hal_pwm_configure(uint16_t base, bool inv)
{
#ifdef EXTPWM_BASE_OFFSET
    base -= EXTPWM_BASE_OFFSET;
#endif
    // Configure port
    uint8_t pin = hal_pwm_port2dio[base];
    
}

void hal_pwm_delete(uint16_t base)
{
#ifdef EXTPWM_BASE_OFFSET
    base -= EXTPWM_BASE_OFFSET;
#endif

}

void hal_pwm_write(uint16_t base, uint16_t value)
{
#ifdef EXTPWM_BASE_OFFSET
    base -= EXTPWM_BASE_OFFSET;
#endif

}

#endif  //  (defined EXTDIO_USED) && (defined EXTPWM_USED)
