// This file is part of the https://github.com/X13home project.

#ifndef _S4SM2P_H
#define _S4SM2P_H

// Board: S4SM2P
// uC: STM32F303KT
// PHY1: UART
// PHY2: RS485

// == GPIOA
// 06	GPIOA.00	ENA	
// 07	GPIOA.01	OUT4	
// 08	GPIOA.02	OUT3	
// 09	GPIOA.03	DIAG	
// 10	GPIOA.04	OUT2	| PWM 03.1	
// 11	GPIOA.05	OUT1	
// 12	GPIOA.06	AIN	| AIN 06	
// 13	GPIOA.07	SEL	
// 18	GPIOA.08	STAT	
// 19	GPIOA.09	TX1	| phy2_TX
// 20	GPIOA.10	RX1	| phy2_RX
// 21	GPIOA.11	IRQ	
// 22	GPIOA.12	DE1	| phy2_DE
// 23	GPIOA.13	SWDIO	| sys_SWDAT
// 24	GPIOA.14	SWCLK	| sys_SWCLK
// 25	GPIOA.15	NSS	
// == GPIOB
// 14	GPIOB.00	ENS2	
// 15	GPIOB.01	ENS1	
// 26	GPIOB.03	TX	| phy1_TX
// 27	GPIOB.04	RX	| phy1_RX
// 28	GPIOB.05	MOSI	
// 29	GPIOB.06	SCL	| TWI_SCL	
// 30	GPIOB.07	SDA	| TWI_SDA	
// == GPIOF
// 02	GPIOF.00		| sys_HSE_IN
// 03	GPIOF.01		| sys_HSE_OUT

#ifdef __cplusplus
extern "C" {
#endif

#define HAL_USE_RTC                 1
#define HAL_RTC_USE_HSE             1

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_MAXPORT_NR           2
#define HAL_DIO_MAPPING             {0, 1, 2, 3, 4, 5, 6, 7, 8, 0xFF, 0xFF, 11, 0xFF, 0xFF, 0xFF, 15, 16, 17, 0xFF, 0xFF, 21, 22, 23}
// End DIO Section

// PWM Section
#define EXTPWM_USED                 1
#define HAL_PWM_BASE_OFFSET         4
#define HAL_PWM_PORT2CFG            {0x219}
// End PWM Section

// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           7
#define HAL_AIN_BASE2APIN           {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 6}
// End Analogue Inputs

// UART Section
#define HAL_UART_NUM_PORTS          2
// End UART Section

// TWI Section
#define HAL_TWI_BUS                 1
#define EXTTWI_USED                 1
// End TWI Section

//UART PHY1 Section	 RX: GPIOB.04, TX: GPIOB.03
#define HAL_USE_USART2              1
#define HAL_USART2_REMAP           2
#define UART_PHY_PORT               1
#define UART_PHY                    1
#include "PHY/UART/uart_phy.h"
//End UART PHY1 Section

//RS485 PHY2 Section	 RX: GPIOA.10, TX: GPIOA.09, DE: GPIOA.12
#define HAL_USE_USART1              0
#define RS485_PHY_PORT              0
#define RS485_PHY                   2
#include "PHY/RS485/rs485_phy.h"
//End RS485 PHY2 Section

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           32
#define OD_DEV_UC_TYPE              'S'
#define OD_DEV_UC_SUBTYPE           '4'
#define OD_DEV_PHY1                 'S'
#define OD_DEV_PHY2                 'M'
#define OD_DEV_HW_TYP_H             '2'
#define OD_DEV_HW_TYP_L             'P'
// End Object's Dictionary Section

#ifdef __cplusplus
}
#endif

#endif //_S4SM2P_H
