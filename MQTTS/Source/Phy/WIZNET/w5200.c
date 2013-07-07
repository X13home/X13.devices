/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

/**
@brief  This function writes the data into W5200 registers.
*/
void wiz_write(uint16_t addr, uint8_t data)
{
  WIZ_ISR_DISABLE();
  WIZ_SELECT();
  
  WIZ_SPI_DATA = addr >> 8;
  while(WIZ_SPI_BISY);
  WIZ_SPI_DATA = addr & 0xFF;
  while(WIZ_SPI_BISY);
  WIZ_SPI_DATA = 0x80;
  while(WIZ_SPI_BISY);
  WIZ_SPI_DATA = 0x01;
  while(WIZ_SPI_BISY);
  WIZ_SPI_DATA = data;
  while(WIZ_SPI_BISY);

  WIZ_RELEASE();
  WIZ_ISR_ENABLE();
}

/**
@brief  This function reads the value from W5100 registers.
*/
uint8_t wiz_read(uint16_t addr)
{
  uint8_t data;

  WIZ_ISR_DISABLE();
  WIZ_SELECT();
  
  WIZ_SPI_DATA = addr >> 8;
  while(WIZ_SPI_BISY);
  WIZ_SPI_DATA = addr & 0xFF;
  while(WIZ_SPI_BISY);
  WIZ_SPI_DATA = 0x00;
  while(WIZ_SPI_BISY);
  WIZ_SPI_DATA = 0x01;
  while(WIZ_SPI_BISY);
  WIZ_SPI_DATA = 0;
  while(WIZ_SPI_BISY);
  data = WIZ_SPI_DATA;
  
  WIZ_RELEASE();
  WIZ_ISR_ENABLE();
  
  return data;
}

/**
@brief  This function writes into W5100 memory(Buffer)
*/ 
void wiz_write_buf(uint16_t addr, uint8_t *pBuf, uint16_t len)
{
  WIZ_ISR_DISABLE();
  WIZ_SELECT();
  
  WIZ_SPI_DATA = addr >> 8;
  while(WIZ_SPI_BISY);
  WIZ_SPI_DATA = addr & 0xFF;
  while(WIZ_SPI_BISY);
  WIZ_SPI_DATA = 0x80 | (len>>8);
  while(WIZ_SPI_BISY);
  WIZ_SPI_DATA = len & 0xFF;
  while(WIZ_SPI_BISY);

  while(len)
  {
    len--;
    WIZ_SPI_DATA = *pBuf;
    pBuf++;
    while(WIZ_SPI_BISY);
  }

  WIZ_RELEASE();
  WIZ_ISR_ENABLE();
}

/**
@brief  This function reads into W5100 memory(Buffer)
*/ 
void wiz_read_buf(uint16_t addr, uint8_t * pBuf, uint16_t len)
{
  WIZ_ISR_DISABLE();
  WIZ_SELECT();
  
  WIZ_SPI_DATA = addr >> 8;
  while(WIZ_SPI_BISY);
  WIZ_SPI_DATA = addr & 0xFF;
  while(WIZ_SPI_BISY);
  WIZ_SPI_DATA = (len>>8) & 0x7F;
  while(WIZ_SPI_BISY);
  WIZ_SPI_DATA = len & 0xFF;
  while(WIZ_SPI_BISY);
  
  while(len)
  {
    len--;
    WIZ_SPI_DATA = 0;
    while(WIZ_SPI_BISY);
    *pBuf = WIZ_SPI_DATA;
    pBuf++;
  }

  WIZ_RELEASE();
  WIZ_ISR_ENABLE();
}
