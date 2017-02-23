// This file is part of the https://github.com/X13home project.

#ifndef _A1Mn10_H
#define _A1Mn10_H

// Board: A1Mn10
// uC: ATMEGA328P
// PHY1: RS485

// 19	AIN6		| AIN 06	
// 22	AIN7		| AIN 07	
// == PORTB
// 12	PORTB.00		
// 13	PORTB.01		
// 14	PORTB.02		
// 15	PORTB.03		
// 16	PORTB.04		
// 17	PORTB.05		
// 07	PORTB.06		| sys_HSE_IN
// 08	PORTB.07		| sys_HSE_OUT
// == PORTC
// 23	PORTC.00		| AIN 00	
// 24	PORTC.01		| AIN 01	
// 25	PORTC.02		| AIN 02	
// 26	PORTC.03		| AIN 03	
// 27	PORTC.04		| AIN 04	| TWI_SDA	
// 28	PORTC.05		| AIN 05	| TWI_SCL	
// == PORTD
// 30	PORTD.00		| phy1_RX
// 31	PORTD.01		| phy1_TX
// 32	PORTD.02		| phy1_DE
// 01	PORTD.03		
// 02	PORTD.04		
// 09	PORTD.05		| PWM 00.1	
// 10	PORTD.06		| PWM 00.0	
// 11	PORTD.07		

#ifdef __cplusplus
extern "C" {
#endif

#define F_CPU                       16000000UL

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_PORT_OFFSET          1
#define EXTDIO_MAXPORT_NR           3
#define HAL_DIO_MAPPING             {8, 9, 10, 11, 12, 13, 16, 17, 18, 19, 20, 21, 27, 28, 29, 30, 31}
// End DIO Section

// PWM Section
#define EXTPWM_USED                 1
#define HAL_PWM_BASE_OFFSET         14
#define HAL_PWM_PORT2CFG            {0x01, 0x00}
// End PWM Section

// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           15
#define HAL_AIN_BASE2APIN           {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0, 1, 2, 3, 4, 5, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 6, 7, 14}
// End Analogue Inputs

// UART Section
#define HAL_UART_NUM_PORTS          1
// End UART Section

// TWI Section
#define HAL_TWI_BUS                 1
#define EXTTWI_USED                 1
// End TWI Section

//RS485 PHY1 Section	 RX: PORTD.00, TX: PORTD.01, DE: PORTD.02
#define HAL_USE_USART0              0
#define RS485_PHY_PORT              0
#define RS485_PHY                   1
#include "PHY/RS485/rs485_phy.h"
//End RS485 PHY1 Section

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           20
#define OD_DEV_UC_TYPE              'A'
#define OD_DEV_UC_SUBTYPE           '1'
#define OD_DEV_PHY1                 'M'
#define OD_DEV_PHY2                 'n'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '0'
// End Object's Dictionary Section

#ifdef __cplusplus
}
#endif

#endif //_A1Mn10_H
