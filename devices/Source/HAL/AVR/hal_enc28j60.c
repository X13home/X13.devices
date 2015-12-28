#include "../../config.h"

#ifdef ENC28J60_PHY

void hal_enc28j60_init_hw(void)
{
    ENC_PORT |= (1<<ENC_PIN_SS);
    ENC_DDR  &= ~(1<<ENC_PIN_MISO);
    ENC_DDR  |= (1<<ENC_PIN_SS) | (1<<ENC_PIN_MOSI) | (1<<ENC_PIN_SCK);
  
    SPCR = (1<<SPE) | (1<<MSTR);
    SPSR |= (1<<SPI2X);
}

uint8_t hal_enc28j60exchg(uint8_t data)
{
    SPDR = data;
    while(!(SPSR &(1<<SPIF)));
    return SPDR;
}

#endif  //  ENC28J60_PHY