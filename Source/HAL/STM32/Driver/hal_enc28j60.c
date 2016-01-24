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

uint8_t hal_enc28j60exchg(uint8_t data)
{
    return hal_spi_exch8(ENC_USE_SPI, data);
}

#endif  //  ENC28J60_PHY
