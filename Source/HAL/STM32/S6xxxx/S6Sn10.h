// This file is part of the https://github.com/X13home project.

#ifndef _S6Sn10_H
#define _S6Sn10_H

// Board: S6Sn10
// uC: STM32L151R
// PHY1: UART

// == GPIOA
// 14	GPIOA.00		| PWM 02.0	
// 15	GPIOA.01		| PWM 02.1	
// 16	GPIOA.02		| phy1_TX
// 17	GPIOA.03		| phy1_RX
// 20	GPIOA.04		
// 21	GPIOA.05		| PWM 02.0	
// 41	GPIOA.08		
// 42	GPIOA.09		| UART_TX 0	
// 43	GPIOA.10		| UART_RX 0	
// 44	GPIOA.11		
// 45	GPIOA.12		
// 46	GPIOA.13		| sys_SWDIO
// 49	GPIOA.14		| sys_SWCLK
// 50	GPIOA.15		| PWM 02.0	
// == GPIOB
// 28	GPIOB.02		
// 55	GPIOB.03		| PWM 02.1	
// 56	GPIOB.04		| PWM 03.0	
// 57	GPIOB.05		| PWM 03.1	
// 61	GPIOB.08		| PWM 04.2	
// 62	GPIOB.09		| PWM 04.3	
// 29	GPIOB.10		| PWM 02.2	| UART_TX 1	
// 30	GPIOB.11		| PWM 02.3	| UART_RX 1	
// 33	GPIOB.12		| PWM 10.0	
// 34	GPIOB.13		| PWM 09.0	
// 35	GPIOB.14		| PWM 09.1	
// 36	GPIOB.15		| PWM 11.0	
// == GPIOC
// 08	GPIOC.00		
// 09	GPIOC.01		
// 10	GPIOC.02		
// 11	GPIOC.03		
// 37	GPIOC.06		| PWM 03.0	
// 38	GPIOC.07		| PWM 03.1	
// 39	GPIOC.08		| PWM 03.2	
// 40	GPIOC.09		| PWM 03.3	
// 51	GPIOC.10		
// 52	GPIOC.11		
// 53	GPIOC.12		
// == GPIOD
// 54	GPIOD.02		
// == GPIOH
// 05	GPIOH.00		| sys_EXT_CLOCK

#ifdef __cplusplus
extern "C" {
#endif

#define HAL_USE_HSE                 1
#define HSE_CRYSTAL_BYPASS          1

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_MAXPORT_NR           4
#define HAL_DIO_MAPPING             {32, 33, 34, 35, 0, 1, 0xFF, 0xFF, 4, 5, 18, 26, 27, 28, 29, 30, 31, 38, 39, 40, 41, 8, 9, 10, 11, 12, 0xFF, 0xFF, 15, 42, 43, 44, 50, 19, 20, 21, 0xFF, 0xFF, 24, 25}
// End DIO Section

// PWM Section
#define EXTPWM_USED                 1
#define HAL_PWM_BASE_OFFSET         4
#define HAL_PWM_PORT2CFG            {0x110, 0x111, 0xFF, 0xFF, 0xFF, 0x110, 0xFF, 0x112, 0x113, 0x350, 0x348, 0x349, 0x358, 0x218, 0x219, 0x21A, 0x21B, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x110, 0xFF, 0xFF, 0xFF, 0xFF, 0x111, 0x218, 0x219, 0xFF, 0xFF, 0x222, 0x223}
// End PWM Section

// UART Section
#define HAL_USE_USART1              0
#define HAL_USE_USART3              1
#define EXTSER_USED                 2
#define HAL_UART_NUM_PORTS          3
// End UART Section

//UART PHY1 Section	 RX: GPIOA.03, TX: GPIOA.02
#define HAL_USE_USART2              2
#define UART_PHY_PORT               2
#define UART_PHY                    1
#include "PHY/UART/uart_phy.h"
//End UART PHY1 Section

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           34
#define OD_DEV_UC_TYPE              'S'
#define OD_DEV_UC_SUBTYPE           '6'
#define OD_DEV_PHY1                 'S'
#define OD_DEV_PHY2                 'n'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '0'
// End Object's Dictionary Section

#ifdef __cplusplus
}
#endif

#endif //_S6Sn10_H
