#include "config.h"

#if (defined ENC28J60_PHY)

void hal_enc28j60_init_hw(void)
{
    // ENC_NSS_PIN
    hal_gpio_cfg(ENC_NSS_PORT, ENC_NSS_PIN, DIO_MODE_OUT_PP_HS);
    hal_gpio_set(ENC_NSS_PORT, ENC_NSS_PIN);

    // Configure SPI and SPI Pins
    hal_spi_cfg(ENC_USE_SPI, (HAL_SPI_MODE_0 | HAL_SPI_MSB | HAL_SPI_8B), 20000000UL);
}
#endif  //  ENC28J60_PHY
