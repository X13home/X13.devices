/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _EXTDIO_H
#define _EXTDIO_H

#ifdef __cplusplus
extern "C" {
#endif

// DIO HAL
uint8_t hal_dio_base2pin(uint16_t base);
void    hal_dio_configure(uint8_t PortNr, DIO_PORT_TYPE Mask, uint16_t Mode);
DIO_PORT_TYPE hal_dio_read(uint8_t PortNr);
void    hal_dio_set(uint8_t PortNr, DIO_PORT_TYPE Mask);
void    hal_dio_reset(uint8_t PortNr, DIO_PORT_TYPE Mask);

// DIO API
void    dioInit(void);
bool    dioCheckSubidx(subidx_t * pSubidx);
e_MQTTSN_RETURNS_t dioRegisterOD(indextable_t *pIdx);
void    dioDeleteOD(subidx_t * pSubidx);
void    dioProc(void);

// Shared DIO subroutines
uint8_t dioCheckBase(uint8_t base);
void    dioTake(uint8_t base);
void    dioRelease(uint8_t base);

#ifdef __cplusplus
}
#endif

#endif  //  _EXTDIO_H
