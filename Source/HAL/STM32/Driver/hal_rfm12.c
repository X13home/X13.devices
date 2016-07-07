/*
Copyright (c) 2011-2016 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#include "config.h"

#ifdef RFM12_PHY

#define RFM12_RELEASE()     {RFM12_NSS_GPIO->BSRR = RFM12_NSS_PIN;}
#define RFM12_SELECT()      {RFM12_NSS_GPIO->BRR  = RFM12_NSS_PIN;}

// defined in rfm12_phy.c
void rfm12_irq(void);

// HAL section
void hal_rfm12_init_hw(void)
{
    // Configure NSS Pin
    hal_gpio_cfg(RFM12_NSS_GPIO, RFM12_NSS_PIN, DIO_MODE_OUT_PP_HS);
    RFM12_RELEASE();

    // Configure IRQ Pin
    hal_dio_configure(RFM12_IRQ_PIN, DIO_MODE_IN_PU);

    // Configure SPI
    hal_spi_cfg(RFM12_USE_SPI, (HAL_SPI_MODE_0 | HAL_SPI_MSB | HAL_SPI_16B), 2500000UL);
}

uint16_t hal_rfm12_spiExch(uint16_t data)
{
    RFM12_SELECT();
    uint16_t retval = hal_spi_exch16(RFM12_USE_SPI, data);
    RFM12_RELEASE();
    return retval;
}

bool hal_rfm12_irq_stat(void)
{
    return ((hal_dio_read(RFM12_IRQ_PIN>>4) & (1<<(RFM12_IRQ_PIN & 0x0F))) == 0);
}

void hal_rfm12_enable_irq(void)
{
    hal_exti_config(RFM12_IRQ_PIN, HAL_EXTI_TRIGGER_FALLING, &rfm12_irq);
}

#endif  //  RFM12_PHY
