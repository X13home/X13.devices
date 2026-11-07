// This file is part of the https://github.com/X13home project.

#ifndef _S4S13_H
#define _S4S13_H

// Board: S4S13
// uC: STM32F303KT
// PHY1: UART

// == GPIOA
// 06	GPIOA.00	PA0	| AIN 00	| PWM 02.0	
// 07	GPIOA.01	PA1	| AIN 01	| PWM 02.1	
// 08	GPIOA.02	PA2	| phy1_TX
// 09	GPIOA.03	PA3	| phy1_RX
// 10	GPIOA.04	PA4	| AIN 04	| PWM 03.1	
// 11	GPIOA.05	PA5	| AIN 05	| PWM 02.0	
// 12	GPIOA.06	PA6	| AIN 06	| PWM 16.0	
// 13	GPIOA.07	PA7	| AIN 07	| PWM 17.0	
// 18	GPIOA.08	PA8	| PWM 01.0	
// 19	GPIOA.09	M_TX	| PWM 01.1	| UART_TX 0	
// 20	GPIOA.10	M_RX	| PWM 01.2	| UART_RX 0	
// 21	GPIOA.11	PA11	| PWM 01.3	
// 22	GPIOA.12	M_DE	| UART_DE 0	
// 23	GPIOA.13	SDIO	
// 24	GPIOA.14	SCLK	
// == GPIOB
// 14	GPIOB.00		| sys_LED_P
// 15	GPIOB.01	PB1	| AIN 09	| PWM 03.3	
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
#define HAL_DIO_MAPPING             {0, 1, 0xFF, 0xFF, 4, 5, 6, 7, 17, 8, 11, 13, 14, 23, 22, 9, 10, 12}
// End DIO Section

// PWM Section
#define EXTPWM_USED                 1
#define HAL_PWM_PORT2CFG            {0x110, 0x111, 0xFF, 0xFF, 0x219, 0x110, 0x180, 0x188, 0x21B, 0x608, 0xB0B, 0xFF, 0xFF, 0xFF, 0xFF, 0x609, 0x60A}
// End PWM Section

// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           10
#define HAL_AIN_BASE2APIN           {0, 1, 0xFF, 0xFF, 4, 5, 6, 7, 9}
// End Analogue Inputs

// UART Section
#define HAL_USE_USART1              0
#define EXTSER_USED                 1
#define HAL_UART_NUM_PORTS          2
// End UART Section

// TWI Section
#define HAL_TWI_BUS                 1
#define EXTTWI_USED                 1
// End TWI Section

#define LED_Init()                  hal_dio_configure(16, DIO_MODE_OUT_PP)
#define LED_On()                    GPIOB->BSRR = (1 << 0)
#define LED_Off()                   GPIOB->BRR = (1 << 0)

//UART PHY1 Section	 RX: GPIOA.03, TX: GPIOA.02
#define HAL_USE_USART2              1
#define UART_PHY_PORT               1
#define UART_PHY                    1
#include "PHY/UART/uart_phy.h"
//End UART PHY1 Section

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           32
#define OD_DEV_UC_TYPE              'S'
#define OD_DEV_UC_SUBTYPE           '4'
#define OD_DEV_PHY1                 'S'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '3'
// End Object's Dictionary Section

#ifdef __cplusplus
}
#endif

#endif //_S4S13_H
