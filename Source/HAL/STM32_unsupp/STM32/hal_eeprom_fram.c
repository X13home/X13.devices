#include "../../config.h"

//Op-code Commands 
#define FRAM_CMD_WREN                   0x06    // Set Write Enable Latch
#define FRAM_CMD_WRDI                   0x04    // Write Disable
#define FRAM_CMD_RDSR                   0x05    // Read Status Register
#define FRAM_CMD_WRSR                   0x01    // Write Status Register
#define FRAM_CMD_READ                   0x03    // Read Memory Data
#define FRAM_CMD_WRITE                  0x02    // Write Memory Dat


/////////////////////////////
// API

// Initialise EEPROM hardware
void eeprom_init_hw(void)
{
    // M_NSS - Output, High Speed
    hal_dio_gpio_cfg(FRAM_NSS_PORT, FRAM_NSS_PIN, DIO_MODE_OUT_PP_HS);
    hal_gpio_set(FRAM_NSS_PORT, FRAM_NSS_PIN);       //  #CS = 1;

    hal_spi_cfg(FRAM_SPI_PORT, (HAL_SPI_MODE_0 | HAL_SPI_MSB | HAL_SPI_8B), 25000000UL);
}

// Read data from EEPROM
void eeprom_read(uint8_t *pBuf, uint32_t Addr, uint32_t Len)
{
    hal_gpio_reset(FRAM_NSS_PORT, FRAM_NSS_PIN);        //  #CS = 0;
    hal_spi_exch8(FRAM_SPI_PORT, FRAM_CMD_READ);
    hal_spi_exch8(FRAM_SPI_PORT, (Addr >> 8));
    hal_spi_exch8(FRAM_SPI_PORT, (Addr & 0xFF));

    while(Len)
    {
        Len--;
        *pBuf = hal_spi_exch8(FRAM_SPI_PORT, 0);
        pBuf++;
    }
    hal_gpio_set(FRAM_NSS_PORT, FRAM_NSS_PIN);          //  #CS = 1;
}

// Write data to EEPROM
void eeprom_write(uint8_t *pBuf, uint32_t Addr, uint32_t Len)
{
    // Write Enable Latch
    hal_gpio_reset(FRAM_NSS_PORT, FRAM_NSS_PIN);        //  #CS = 0;
    hal_spi_exch8(FRAM_SPI_PORT, FRAM_CMD_WREN);        // Write enable
    hal_gpio_set(FRAM_NSS_PORT, FRAM_NSS_PIN);          //  #CS = 1;
    // Write Data
    hal_gpio_reset(FRAM_NSS_PORT, FRAM_NSS_PIN);        //  #CS = 0;
    hal_spi_exch8(FRAM_SPI_PORT, FRAM_CMD_WRITE);
    hal_spi_exch8(FRAM_SPI_PORT, (Addr >> 8));
    hal_spi_exch8(FRAM_SPI_PORT, (Addr & 0xFF));
    while(Len)
    {
        hal_spi_exch8(FRAM_SPI_PORT, *pBuf);
        pBuf++;
        Len--;
    }
    hal_gpio_set(FRAM_NSS_PORT, FRAM_NSS_PIN);       //  #CS = 1;
}
