#include "config.h"

#if (defined CC11_PHY)

void hal_cc11_init_hw(void)
{
    // CC11_NSS_PIN
    hal_gpio_cfg(CC11_NSS_PORT, CC11_NSS_PIN, DIO_MODE_OUT_PP_HS);
    hal_gpio_set(CC11_NSS_PORT, CC11_NSS_PIN);
    
    hal_spi_cfg(CC11_USE_SPI, (HAL_SPI_MODE_0 | HAL_SPI_MSB | HAL_SPI_8B), 6500000UL);
}

uint8_t hal_cc11_spiExch(uint8_t data)
{
    return hal_spi_exch8(CC11_USE_SPI, data);
}

#endif  //  CC11_PHY
