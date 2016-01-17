/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// SiLabs Si114x Sensor, present control & ambient light

void si114x_irq(void);

void hal_si1143x_init_hw(void)
{
    hal_exti_config(SI114X_IRQ_PORT, SI114X_IRQ_PIN, HAL_EXTI_TRIGGER_FALLING, &si114x_irq);
    
    // Enable and set interrupt
    NVIC_SetPriority(SI114X_IRQ, 0);
    NVIC_EnableIRQ(SI114X_IRQ);
}
