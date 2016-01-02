#include "config.h"

void INIT_SYSTEM(void)
{}

void StartSheduler(void)
{}

void hal_enc28j60_init_hw(void)
{}

// ToDo Dummy section
uint8_t hal_enc28j60exchg(uint8_t data)
{
    return 0;
}


void _delay_ms(uint16_t ms)
{}

void _delay_us(uint16_t us)
{}

void eeprom_init_hw(void)
{}

void eeprom_read(uint8_t *pBuf, uint32_t Addr, uint32_t Len)
{}


void eeprom_write(uint8_t *pBuf, uint32_t Addr, uint32_t Len)
{}

uint16_t hal_get_ms(void)
{
    return 0;
}

uint32_t hal_get_sec(void)
{
    return 0;
}

void hal_reboot(void)
{}


uint16_t halRNG(void)
{
    return 42;
}