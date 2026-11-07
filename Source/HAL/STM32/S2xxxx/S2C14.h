// This file is part of the https://github.com/X13home project.

#ifndef _S2C14_H
#define _S2C14_H

// Board: S2C14
// uC: STM32F051KT
// PHY1: CC1101

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
// 19	GPIOA.09	SV1-9	| PWM 01.1	| UART_TX 1	
// 20	GPIOA.10	SV1-10	| PWM 01.2	| UART_RX 1	
// 21	GPIOA.11	SV1-7	| PWM 01.3	
// 22	GPIOA.12	SV1-8/LED	
// 23	GPIOA.13		| sys_SWDAT
// 24	GPIOA.14		| sys_SWCLK
// 25	GPIOA.15		| phy1_NCS
// == GPIOB
// 14	GPIOB.00	SV1-19	| AIN 08	| PWM 03.2	
// 15	GPIOB.01	SV1-20	| AIN 09	| PWM 03.3	
// 26	GPIOB.03		| phy1_CLK
// 27	GPIOB.04		| phy1_SO
// 28	GPIOB.05		| phy1_SI
// 29	GPIOB.06	SV1-13	| TWI_SCL	
// 30	GPIOB.07	SV1-14	| TWI_SDA	
// == GPIOF
// 02	GPIOF.00		| sys_HSE_IN
// 03	GPIOF.01		| sys_HSE_OUT

#ifdef __cplusplus
extern "C" {
#endif

#define                            HAL_USE_RTC 1

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_MAXPORT_NR           2
#define HAL_DIO_MAPPING             {0, 1, 2, 3, 11, 12, 9, 10, 22, 23, 4, 5, 6, 7, 16, 17, 8}
// End DIO Section

// PWM Section
#define EXTPWM_USED                 1
#define HAL_PWM_PORT2CFG            {0x210, 0x211, 0x212, 0x213, 0x20B, 0xFF, 0x209, 0x20A, 0xFF, 0xFF, 0x470, 0xFF, 0x118, 0x119, 0x11A, 0x11B}
// End PWM Section

// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           10
#define HAL_AIN_BASE2APIN           {0, 1, 2, 3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 4, 5, 6, 7, 8, 9}
// End Analogue Inputs

// UART Section
#define HAL_USE_USART2              0
#define HAL_USE_USART1              1
#define EXTSER_USED                 2
#define HAL_UART_NUM_PORTS          2
// End UART Section

// TWI Section
#define HAL_TWI_BUS                 1
#define EXTTWI_USED                 1
// End TWI Section

//CC1101 PHY1 Section	 NCS: GPIOA.15, CLK: GPIOB.03, SO: GPIOB.04, SI: GPIOB.05
#define HAL_USE_SPI1                2
#define CC11_USE_SPI                1
#define CC11_NSS_PIN                15
#define CC11_WAIT_LOW_MISO()        while(GPIOB->IDR & (1 << 4))
#define CC11_SELECT()               GPIOA->BRR = (1 << 15)
#define CC11_RELEASE()              GPIOA->BSRR = (1 << 15)
#define CC11_PHY                    1
#define CC11_DEFAULT_FREQ           868650000UL
#include "PHY/CC1101/cc11_phy.h"
//End CC1101 PHY1 Section

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           25
#define OD_DEV_UC_TYPE              'S'
#define OD_DEV_UC_SUBTYPE           '2'
#define OD_DEV_PHY1                 'C'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '4'
// End Object's Dictionary Section

#ifdef __cplusplus
}
#endif

#endif //_S2C14_H
