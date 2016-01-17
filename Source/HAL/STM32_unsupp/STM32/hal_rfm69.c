#include "../../config.h"

#if (defined RFM69_PHY)
    
void hal_rfm69_init_hw(void)
{
    // RFM69_NSS_PIN
    hal_dio_gpio_cfg(RFM69_NSS_PORT, RFM69_NSS_PIN, DIO_MODE_OUT_PP_HS);
    hal_gpio_set(RFM69_NSS_PORT, RFM69_NSS_PIN);
    // RFM69_IRQ_PIN
    hal_dio_gpio_cfg(RFM69_IRQ_PORT, RFM69_IRQ_PIN, DIO_MODE_IN_PU);

    hal_spi_cfg(RFM69_USE_SPI, (HAL_SPI_MODE_0 | HAL_SPI_MSB | HAL_SPI_8B), 10000000UL);
}

uint8_t hal_rfm69_spiExch(uint8_t data)
{
    return hal_spi_exch8(RFM69_USE_SPI, data);
}

#endif  //  RFM69_PHY
