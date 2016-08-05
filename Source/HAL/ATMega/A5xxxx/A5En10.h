// This file is part of the https://github.com/X13home project.

#ifndef _A5En10_H
#define _A5En10_H

// Board: A5En10
// uC: ATMEGA2561
// PHY1: ENC28J60

// == PORTA
// 51	PORTA.00	XP2.21	
// 50	PORTA.01	XP2.23	
// 49	PORTA.02	XP2.25	
// 48	PORTA.03	XP2.27	
// 47	PORTA.04	XP2.29	
// 46	PORTA.05	XP2.31	
// 45	PORTA.06	XP2.33	
// 44	PORTA.07		
// == PORTB
// 10	PORTB.00		| phy1_NSS 1.1
// 11	PORTB.01		| phy1_SCK 1.1
// 12	PORTB.02		| phy1_MOSI 1.1
// 13	PORTB.03		| phy1_MISO 1.1
// 14	PORTB.04	XP1.15	
// 15	PORTB.05	XP1.19	| PWM 01.0	
// 16	PORTB.06	XP1.17	| PWM 01.1	
// 17	PORTB.07	XP1.13	| PWM 01.2	
// == PORTC
// 35	PORTC.00	XP4.8	
// 36	PORTC.01	XP4.6	
// 37	PORTC.02	XP4.5	
// 38	PORTC.03	XP4.12	
// 39	PORTC.04	XP4.14	
// 40	PORTC.05	XP4.9	
// 41	PORTC.06	XP4.10	
// 42	PORTC.07	Xp4.13	
// == PORTD
// 25	PORTD.00	XT2.4	| TWI_SCL	
// 26	PORTD.01	XT2.5	| TWI_SDA	
// 27	PORTD.02	XT2.7	| UART_RX 0	
// 28	PORTD.03	XT2.8	| UART_TX 0	
// 29	PORTD.04	XT2.10	
// 30	PORTD.05	XT2.11	
// 31	PORTD.06	XP1.9	
// 32	PORTD.07	XP1.7	
// == PORTE
// 02	PORTE.00	XP1.11	| UART_RX 1	
// 03	PORTE.01	XP2.15	| UART_TX 1	
// 04	PORTE.02	XP2.11	
// 05	PORTE.03	XP2.19	| PWM 03.0	
// 06	PORTE.04	XP2.17	| PWM 03.1	
// 07	PORTE.05	XP2.13	| PWM 03.2	
// 08	PORTE.06	XP2.9	
// 09	PORTE.07	XP2.7	
// == PORTF
// 61	PORTF.00	XP1.23	| AIN 00	
// 60	PORTF.01	XP1.25	| AIN 01	
// 59	PORTF.02	XP1.27	| AIN 02	
// 58	PORTF.03	XP1.29	| AIN 03	
// 57	PORTF.04	XP1.31	| AIN 04	
// 56	PORTF.05	XP1.33	| AIN 05	
// 55	PORTF.06	XP5.1	| AIN 06	
// 54	PORTF.07	XP6.1	| AIN 07	
// == PORTG
// 33	PORTG.00		| sys_LED
// 34	PORTG.01	XP4.7	
// 43	PORTG.02	XP4.11	
// 18	PORTG.03	XP2.5	
// 19	PORTG.04	XP1.5	
// 01	PORTG.05	XP1.21	| PWM 00.1	

#ifdef __cplusplus
extern "C" {
#endif

#define F_CPU                       12500000UL

// DIO Section
#define EXTDIO_USED		1
#define EXTDIO_MAXPORT_NR	7
#define HAL_DIO_MAPPING		{52, 31, 30, 32, 15, 12, 14, 13, 53, 40, 41, 42, 43, 44, 45, 51, 39, 38, 34, 37, 33, 36, 35, 0, 1, 2, 3, 4, 5, 6, 24, 25, 26, 27, 28, 29, 46, 47, 18, 17, 49, 16, 21, 22, 50, 19, 23, 20, 7}
// End DIO Section

// PWM Section
#define EXTPWM_USED		1
#define HAL_PWM_BASE_OFFSET	4
#define HAL_PWM_PORT2CFG	{0x0A, 0xFF, 0x09, 0x08, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x1A, 0xFF, 0x19, 0x18}
// End PWM Section

// Analogue Inputs
#define EXTAIN_USED		1
#define EXTAIN_MAXPORT_NR	8
#define HAL_AIN_BASE2APIN	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0, 1, 2, 3, 4, 5, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 6, 7}
// End Analogue Inputs

// UART Section
#define HAL_USE_USART1		0
#define HAL_USE_USART0		1
#define EXTSER_USED		2
#define HAL_UART_NUM_PORTS	2
// End UART Section

// TWI Section
#define EXTTWI_USED		1
#define HAL_TWI_BUS		1
// End TWI Section

//ENC PHY1 Section	MOSI: PORTB.02, MISO: PORTB.03, SCK: PORTB.01, NSS: PORTB.00
#define HAL_USE_SPI1              1
#define ENC_USE_SPI                 1
#define ENC_NSS_PIN                 8
#define ENC_SELECT()                PORTB &= ~(1 << 0)
#define ENC_RELEASE()               PORTB |= (1 << 0)
#define ENC_PHY                     1
#include "PHY/ENC28J60/enc28j60_phy.h"
//End ENC PHY1 Section


// Object's Dictionary Section
#define OD_MAX_INDEX_LIST	49
#define OD_DEV_UC_TYPE		'A'
#define OD_DEV_UC_SUBTYPE	'5'
#define OD_DEV_PHY1		'E'
#define OD_DEV_PHY2		'n'
#define OD_DEV_HW_TYP_H		'1'
#define OD_DEV_HW_TYP_L		'0'
// End Object's Dictionary Section

#ifdef __cplusplus
}
#endif

#endif //_A5En10_H
