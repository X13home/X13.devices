// This file is part of the https://github.com/X13home project.

#ifndef _S4SM14_H
#define _S4SM14_H

// Board: S4SM14
// uC: STM32F303RET
// PHY1: UART
// PHY2: RS485

// == GPIOA
// 14	GPIOA.00		| AIN 00	| PWM 02.0	
// 15	GPIOA.01		| AIN 01	| PWM 02.1	
// 16	GPIOA.02		| AIN 02	| PWM 15.0	
// 17	GPIOA.03		| AIN 03	| PWM 15.1	
// 20	GPIOA.04		| AIN 04	| PWM 03.1	
// 21	GPIOA.05		| AIN 05	| PWM 02.0	
// 22	GPIOA.06		| AIN 06	| PWM 16.0	
// 23	GPIOA.07		| AIN 07	| PWM 17.0	
// 42	GPIOA.09		| phy2_TX
// 43	GPIOA.10		| phy2_RX
// 45	GPIOA.12		| phy2_DE
// 46	GPIOA.13		| sys_SWDAT
// 49	GPIOA.14		| sys_SWCLK
// == GPIOB
// 26	GPIOB.00		| AIN 08	| PWM 03.2	
// 27	GPIOB.01		| AIN 09	| PWM 03.3	
// 28	GPIOB.02		| sys_LED_P
// 55	GPIOB.03		| phy1_TX
// 56	GPIOB.04		| phy1_RX
// 61	GPIOB.08		| PWM 04.2	| TWI_SCL	
// 62	GPIOB.09		| PWM 04.3	| TWI_SDA	
// 29	GPIOB.10		| PWM 02.2	| UART_TX 0	
// 30	GPIOB.11		| PWM 02.3	| UART_RX 0	
// == GPIOC
// 24	GPIOC.04		| AIN 19	
// 25	GPIOC.05		| AIN 20	
// == GPIOD
// == GPIOF
// 05	GPIOF.00		| sys_HSE_IN
// 06	GPIOF.01		| sys_HSE_OUT

#ifdef __cplusplus
extern "C" {
#endif

#define HAL_USE_RTC                1
#define HAL_RTC_USE_HSE            1

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_MAXPORT_NR           3
#define HAL_DIO_MAPPING             {0xFF, 0xFF, 0xFF, 0, 1, 2, 3, 4, 5, 6, 7, 0xFF, 0xFF, 24, 25, 36, 37, 16, 17, 26, 27}
// End DIO Section

// PWM Section
#define EXTPWM_USED                 1
#define HAL_PWM_BASE_OFFSET         3
#define HAL_PWM_PORT2CFG            {0x110, 0x111, 0x978, 0x979, 0x219, 0x110, 0x180, 0x188, 0xFF, 0xFF, 0x222, 0x223, 0xFF, 0xFF, 0x21A, 0x21B, 0x112, 0x113}
// End PWM Section

// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           21
#define HAL_AIN_BASE2APIN           {0xFF, 0xFF, 0xFF, 0, 1, 2, 3, 4, 5, 6, 7, 0xFF, 0xFF, 0xFF, 0xFF, 19, 20, 8, 9}
// End Analogue Inputs

// UART Section
#define HAL_USE_USART3              0
#define EXTSER_USED                 1
#define HAL_UART_NUM_PORTS          3
// End UART Section

// TWI Section
#define HAL_TWI_BUS                 1
#define HAL_TWI_REMAP               1
#define EXTTWI_USED                 1
// End TWI Section

#define LED_Init()                  hal_dio_configure(18, DIO_MODE_OUT_PP)
#define LED_On()                    GPIOB->BSRR = (1 << 2)
#define LED_Off()                   GPIOB->BRR = (1 << 2)

//UART PHY1 Section	 RX: GPIOB.04, TX: GPIOB.03
#define HAL_USE_USART2              2
#define HAL_USART2_REMAP           2
#define UART_PHY_PORT               2
#define UART_PHY                    1
#include "PHY/UART/uart_phy.h"
//End UART PHY1 Section

//RS485 PHY2 Section	 RX: GPIOA.10, TX: GPIOA.09, DE: GPIOA.12
#define HAL_USE_USART1              1
#define RS485_PHY_PORT              1
#define RS485_PHY                   2
#include "PHY/RS485/rs485_phy.h"
//End RS485 PHY2 Section

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           48
#define OD_DEV_UC_TYPE              'S'
#define OD_DEV_UC_SUBTYPE           '4'
#define OD_DEV_PHY1                 'S'
#define OD_DEV_PHY2                 'M'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '4'
// End Object's Dictionary Section

#ifdef __cplusplus
}
#endif

#endif //_S4SM14_H
