// This file is part of the https://github.com/X13home project.

#ifndef _S2S14_H
#define _S2S14_H

// Board: S2S14
// uC: STM32F051KT
// PHY1: UART

// == GPIOA
// 06	GPIOA.00	SV1-3	| AIN 00	| PWM 02.0	
// 07	GPIOA.01	SV1-4	| AIN 01	| PWM 02.1	
// 08	GPIOA.02	SV1-5	| AIN 02	| PWM 02.2	| UART_TX 0	
// 09	GPIOA.03	SV1-6	| AIN 03	| PWM 02.3	| UART_RX 0	
// 10	GPIOA.04	SV1-15	| AIN 04	| PWM 14.0	
// 11	GPIOA.05	SV1-16	| AIN 05	
// 12	GPIOA.06	SV1-17	| AIN 06	| PWM 03.0	
// 13	GPIOA.07	SV1-18	| AIN 07	| PWM 03.1	
// 18	GPIOA.08	SINT	
// 19	GPIOA.09	SV1-9	| phy1_TX
// 20	GPIOA.10	SV1-10	| phy1_RX
// 21	GPIOA.11	SV1-7	| PWM 01.3	
// 22	GPIOA.12	SV1-8/LED	
// 23	GPIOA.13		| sys_SWDAT
// 24	GPIOA.14		| sys_SWCLK
// == GPIOB
// 14	GPIOB.00	SV1-19	| AIN 08	| PWM 03.2	
// 15	GPIOB.01	SV1-20	| AIN 09	| PWM 03.3	
// 29	GPIOB.06	SV1-13	| TWI_SCL	
// 30	GPIOB.07	SV1-14	| TWI_SDA	
// == GPIOF
// 02	GPIOF.00		| sys_HSE_IN
// 03	GPIOF.01		| sys_HSE_OUT

#ifdef __cplusplus
extern "C" {
#endif

#define                            HAL_USE_RTC 1
#define                            HAL_RTC_USE_HSE

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_MAXPORT_NR           2
#define HAL_DIO_MAPPING             {0, 1, 2, 3, 11, 12, 0xFF, 0xFF, 22, 23, 4, 5, 6, 7, 16, 17, 8}
// End DIO Section

// PWM Section
#define EXTPWM_USED                 1
#define HAL_PWM_PORT2CFG            {0x210, 0x211, 0x212, 0x213, 0x20B, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x470, 0xFF, 0x118, 0x119, 0x11A, 0x11B}
// End PWM Section

// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           10
#define HAL_AIN_BASE2APIN           {0, 1, 2, 3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 4, 5, 6, 7, 8, 9}
// End Analogue Inputs

// UART Section
#define HAL_USE_USART2              0
#define EXTSER_USED                 1
#define HAL_UART_NUM_PORTS          2
// End UART Section

// TWI Section
#define HAL_TWI_BUS                 1
#define EXTTWI_USED                 1
// End TWI Section

//UART PHY1 Section	 RX: GPIOA.10, TX: GPIOA.09
#define HAL_USE_USART1              1
#define UART_PHY_PORT               1
#define UART_PHY                    1
#include "PHY/UART/uart_phy.h"
//End UART PHY1 Section

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           23
#define OD_DEV_UC_TYPE              'S'
#define OD_DEV_UC_SUBTYPE           '2'
#define OD_DEV_PHY1                 'S'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '4'
// End Object's Dictionary Section

#ifdef __cplusplus
}
#endif

#endif //_S2S14_H
