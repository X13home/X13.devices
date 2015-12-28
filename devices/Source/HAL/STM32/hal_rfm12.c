#include "../../config.h"

#if (defined RFM12_PHY)
    
// external procedure, defined in rfm12_phy.c
void rfm12_irq(void);

void hal_rfm12_init_hw(void)
{
    NVIC_DisableIRQ(RFM12_IRQ);
    
    // RFM12_NSS_PIN
    hal_dio_gpio_cfg(RFM12_NSS_PORT, RFM12_NSS_PIN, DIO_MODE_OUT_PP_HS);
    hal_gpio_set(RFM12_NSS_PORT, RFM12_NSS_PIN);
    // RFM12_IRQ_PIN
    hal_dio_gpio_cfg(RFM12_IRQ_PORT, RFM12_IRQ_PIN, DIO_MODE_IN_PU);

    hal_spi_cfg(RFM12_USE_SPI, (HAL_SPI_MODE_0 | HAL_SPI_MSB | HAL_SPI_16B), 2500000UL);
}

uint16_t hal_rfm12_spiExch(uint16_t data)
{
    hal_gpio_reset(RFM12_NSS_PORT, RFM12_NSS_PIN);
    data = hal_spi_exch16(RFM12_USE_SPI, data);
    hal_gpio_set(RFM12_NSS_PORT, RFM12_NSS_PIN);
    return data;
}

bool hal_rfm12_irq_stat(void)
{
    if(RFM12_IRQ_PORT->IDR & RFM12_IRQ_PIN)
        return false;
    
    return true;
}

void hal_rfm12_enable_irq(void)
{
    hal_exti_config(RFM12_IRQ_PORT, RFM12_IRQ_PIN, HAL_EXTI_TRIGGER_FALLING, &rfm12_irq);

    // Enable and set interrupt
    NVIC_SetPriority(RFM12_IRQ, 0);
    NVIC_EnableIRQ(RFM12_IRQ);
}
#endif  //  RFM12_PHY

