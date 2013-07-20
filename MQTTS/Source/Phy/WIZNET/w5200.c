/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

#include "W5200.h"

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

/**
@brief  for copy the data form application buffer to Transmit buffer of the chip.

This function is being used for copy the data form application buffer to Transmit
buffer of the chip. It calculate the actual physical address where one has to write
the data in transmit buffer. Here also take care of the condition while it exceed
the Tx memory upper-bound of socket.
*/
/*
static void write_data(SOCKET s, volatile uint8_t * src, volatile uint8_t * dst, uint16_t len)
{
  uint16_t size;
  uint16_t dst_mask;
  uint8_t * dst_ptr;

//  dst_mask = (uint32)dst & getIINCHIP_TxMASK(s);
//  dst_ptr = (uint8 *)(getIINCHIP_TxBASE(s) + dst_mask);

	if (dst_mask + len > getIINCHIP_TxMAX(s)) 
	{
		size = getIINCHIP_TxMAX(s) - dst_mask;
		wiz_write_buf((uint32)dst_ptr, (uint8*)src, size);
		src += size;
		size = len - size;
		dst_ptr = (uint8 *)(getIINCHIP_TxBASE(s));
		wiz_write_buf((uint32)dst_ptr, (uint8*)src, size);
	} 
	else
	{
		wiz_write_buf((uint32)dst_ptr, (uint8*)src, len);
	}
}
*/
/**
@brief  This function is being called by send() and sendto() function also. 
  This function read the Tx write pointer register and after copy the data in buffer update the Tx write pointer
  register. User should read upper byte first and lower byte later to get proper value.
*/
/*
void send_data_processing(SOCKET s, uint8_t *data, uint16_t len)
{
  //cli();
  uint16_t ptr;
  ptr = wiz_read(Sn_TX_WR0(s));
  ptr = (ptr << 8) + wiz_read(Sn_TX_WR0(s) + 1);
	write_data(s, data, (uint8 *)(ptr), len);
	ptr += len;
//	sent_ptr = ptr;
	IINCHIP_WRITE(Sn_TX_WR0(s),(uint8)((ptr & 0xff00) >> 8));
	IINCHIP_WRITE((Sn_TX_WR0(s) + 1),(uint8)(ptr & 0x00ff));
//	sent_ptr = IINCHIP_READ(Sn_SMWRPTR0(s));
//	sent_ptr = (sent_ptr << 8) + IINCHIP_READ(Sn_SMWRPTR1(s));
//	if(ptr != sent_ptr)
//	{
//		printf("!!!!!!!err : ptr = %04x, sent_ptr = %04x\r\n",ptr,sent_ptr);
//		while(1);
//	}
	//sei();
}
*/
