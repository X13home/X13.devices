// This file is part of the https://github.com/X13home project.

#ifndef _A1S12_H
#define _A1S12_H

// Board: A1S12
// uC: ATMEGA328P
// PHY1: UART

// 19	AIN6	CN20	| AIN 06	
// 22	AIN7	CN19	| AIN 07	
// == PORTB
// 12	PORTB.00		| sys_LED_N
// 13	PORTB.01		| PWM 01.0	
// 14	PORTB.02		| PWM 01.1	
// 15	PORTB.03		
// 16	PORTB.04		
// 17	PORTB.05		
// 07	PORTB.06		| sys_HSE_IN
// 08	PORTB.07		| sys_HSE_OUT
// == PORTC
// 23	PORTC.00	CN18	| AIN 00	
// 24	PORTC.01	CN17	| AIN 01	
// 25	PORTC.02	CN16	| AIN 02	
// 26	PORTC.03	CN15	| AIN 03	
// 27	PORTC.04	CN14	| AIN 04	| TWI_SDA	
// 28	PORTC.05	CN13	| AIN 05	| TWI_SCL	
// == PORTD
// 30	PORTD.00		| phy1_RX
// 31	PORTD.01		| phy1_TX
// 32	PORTD.02	CN8	
// 01	PORTD.03	CN7	
// 02	PORTD.04	CN6	
// 09	PORTD.05	CN5	| PWM 00.1	
// 10	PORTD.06	CN4	| PWM 00.0	
// 11	PORTD.07	CN3	

#ifdef __cplusplus
extern "C" {
#endif

#define F_CPU                       8000000UL

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_PORT_OFFSET          1
#define EXTDIO_MAXPORT_NR           3
#define HAL_DIO_MAPPING             {0xFF, 0xFF, 16, 17, 18, 19, 20, 21, 0xFF, 0xFF, 26, 27, 28, 29, 30, 31, 9, 10, 11, 12, 13}
// End DIO Section

// PWM Section
#define EXTPWM_USED                 1
#define HAL_PWM_BASE_OFFSET         13
#define HAL_PWM_PORT2CFG            {0x01, 0x00, 0xFF, 0x08, 0x09}
// End PWM Section

// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           15
#define HAL_AIN_BASE2APIN           {6, 7, 0, 1, 2, 3, 4, 5, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 14}
// End Analogue Inputs

// UART Section
#define HAL_UART_NUM_PORTS          1
// End UART Section

// TWI Section
#define HAL_TWI_BUS                 1
#define EXTTWI_USED                 1
// End TWI Section

#define LED_Init()                  hal_dio_configure(8, DIO_MODE_OUT_PP)
#define LED_On()                    PORTB &= ~(1 << 0)
#define LED_Off()                   PORTB |= (1 << 0)

//UART PHY1 Section	 RX: PORTD.00, TX: PORTD.01
#define HAL_USE_USART0              0
#define UART_PHY_PORT               0
#define UART_PHY                    1
#include "PHY/UART/uart_phy.h"
//End UART PHY1 Section

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           20
#define OD_DEV_UC_TYPE              'A'
#define OD_DEV_UC_SUBTYPE           '1'
#define OD_DEV_PHY1                 'S'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '2'
// End Object's Dictionary Section

#ifdef __cplusplus
}
#endif

#endif //_A1S12_H
