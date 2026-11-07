// This file is part of the https://github.com/X13home project.

#ifndef _A4S10_H
#define _A4S10_H

// Board: A4S10
// uC: ATMEGA2560
// PHY1: UART

// == PORTA
// 78	PORTA.00	22	
// 77	PORTA.01	23	
// 76	PORTA.02	24	
// 75	PORTA.03	25	
// 74	PORTA.04	26	
// 73	PORTA.05	27	
// 72	PORTA.06	28	
// 71	PORTA.07	29	
// == PORTB
// 19	PORTB.00	53	
// 20	PORTB.01	52	
// 21	PORTB.02	51	
// 22	PORTB.03	50	
// 23	PORTB.04	10	
// 24	PORTB.05	11	| PWM 01.0	
// 25	PORTB.06	12	| PWM 01.1	
// 26	PORTB.07	13	| PWM 01.2	
// == PORTC
// 53	PORTC.00	37	
// 54	PORTC.01	36	
// 55	PORTC.02	35	
// 56	PORTC.03	34	
// 57	PORTC.04	33	
// 58	PORTC.05	32	
// 59	PORTC.06	31	
// 60	PORTC.07	30	
// == PORTD
// 43	PORTD.00	21	| TWI_SCL	
// 44	PORTD.01	20	| TWI_SDA	
// 45	PORTD.02	19	| UART_RX 0	
// 46	PORTD.03	18	| UART_TX 0	
// 50	PORTD.07	38	
// == PORTE
// 02	PORTE.00	0	| phy1_RX
// 03	PORTE.01	1	| phy1_TX
// 05	PORTE.03	5	| PWM 03.0	
// 06	PORTE.04	2	| PWM 03.1	
// 07	PORTE.05	3	| PWM 03.2	
// == PORTF
// 97	PORTF.00	A0	| AIN 00	
// 96	PORTF.01	A1	| AIN 01	
// 95	PORTF.02	A2	| AIN 02	
// 94	PORTF.03	A3	| AIN 03	
// 93	PORTF.04	A4	| AIN 04	
// 92	PORTF.05	A5	| AIN 05	
// 91	PORTF.06	A6	| AIN 06	
// 90	PORTF.07	A7	| AIN 07	
// == PORTG
// 51	PORTG.00	41	
// 52	PORTG.01	40	
// 70	PORTG.02	39	
// 01	PORTG.05	4	| PWM 00.1	
// == PORTH
// 12	PORTH.00	17	| UART_RX 1	
// 13	PORTH.01	16	| UART_TX 1	
// 15	PORTH.03	6	| PWM 04.0	
// 16	PORTH.04	7	| PWM 04.1	
// 17	PORTH.05	8	| PWM 04.2	
// 18	PORTH.06	9	
// == PORTJ
// 63	PORTJ.00	15	| UART_RX 2	
// 64	PORTJ.01	14	| UART_TX 2	
// == PORTK
// 89	PORTK.00	A8	| AIN 32	
// 88	PORTK.01	A9	| AIN 33	
// 87	PORTK.02	A10	| AIN 34	
// 86	PORTK.03	A11	| AIN 35	
// 85	PORTK.04	A12	| AIN 36	
// 84	PORTK.05	A13	| AIN 37	
// 83	PORTK.06	A14	| AIN 38	
// 82	PORTK.07	A15	| AIN 39	
// == PORTL
// 35	PORTL.00	49	
// 36	PORTL.01	48	
// 37	PORTL.02	47	
// 38	PORTL.03	46	| PWM 05.0	
// 39	PORTL.04	45	| PWM 05.1	
// 40	PORTL.05	44	| PWM 05.2	
// 41	PORTL.06	43	
// 42	PORTL.07	42	

#ifdef __cplusplus
extern "C" {
#endif

#define F_CPU                       16000000UL

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_MAXPORT_NR           11
#define HAL_DIO_MAPPING             {40, 41, 42, 43, 44, 45, 46, 47, 72, 73, 74, 75, 76, 77, 78, 79, 0xFF, 0xFF, 36, 37, 53, 35, 59, 60, 61, 62, 12, 13, 14, 15, 65, 64, 57, 56, 27, 26, 25, 24, 0, 1, 2, 3, 4, 5, 6, 7, 23, 22, 21, 20, 19, 18, 17, 16, 31, 50, 49, 48, 87, 86, 85, 84, 83, 82, 81, 80, 11, 10, 9, 8}
// End DIO Section

// PWM Section
#define EXTPWM_USED                 1
#define HAL_PWM_BASE_OFFSET         18
#define HAL_PWM_PORT2CFG            {0x19, 0x1A, 0x01, 0x18, 0x20, 0x21, 0x22, 0xFF, 0xFF, 0x08, 0x09, 0x0A, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x2A, 0x29, 0x28}
// End PWM Section

// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           40
#define HAL_AIN_BASE2APIN           {0, 1, 2, 3, 4, 5, 6, 7, 32, 33, 34, 35, 36, 37, 38, 39}
// End Analogue Inputs

// UART Section
#define HAL_USE_USART1              0
#define HAL_USE_USART2              1
#define HAL_USE_USART3              2
#define EXTSER_USED                 3
#define HAL_UART_NUM_PORTS          4
// End UART Section

// TWI Section
#define HAL_TWI_BUS                 1
#define EXTTWI_USED                 1
// End TWI Section

//UART PHY1 Section	 RX: PORTE.00, TX: PORTE.01
#define HAL_USE_USART0              3
#define UART_PHY_PORT               3
#define UART_PHY                    1
#include "PHY/UART/uart_phy.h"
//End UART PHY1 Section

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           68
#define OD_DEV_UC_TYPE              'A'
#define OD_DEV_UC_SUBTYPE           '4'
#define OD_DEV_PHY1                 'S'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '0'
// End Object's Dictionary Section

#ifdef __cplusplus
}
#endif

#endif //_A4S10_H
