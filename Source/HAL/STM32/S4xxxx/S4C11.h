// This file is part of the https://github.com/X13home project.

#ifndef _S4C11_H
#define _S4C11_H

// Board: S4C11
// uC: STM32F303KT
// PHY1: CC1101

// == GPIOA
// 06	GPIOA.00	P1.3	| AIN 00	| PWM 02.0	
// 07	GPIOA.01	P1.4	| AIN 01	| PWM 02.1	
// 08	GPIOA.02	P1.5	| AIN 02	| PWM 02.2	| UART_TX 0	
// 09	GPIOA.03	P1.6	| AIN 03	| PWM 02.3	| UART_RX 0	
// 10	GPIOA.04	P1.7	| AIN 04	| PWM 03.1	
// 11	GPIOA.05	P1.8	| AIN 05	| PWM 02.0	
// 12	GPIOA.06	P1.9	| AIN 06	| PWM 03.0	
// 13	GPIOA.07	P1.10	| AIN 07	| PWM 17.0	
// 19	GPIOA.09	P2.9	| PWM 01.1	| UART_TX 1	
// 20	GPIOA.10	P2.8	| PWM 01.2	| UART_RX 1	
// 22	GPIOA.12	P2.7	| PWM 16.0	
// 23	GPIOA.13	P2.6	
// 24	GPIOA.14	P2.5	
// 25	GPIOA.15		| phy1_NCS
// == GPIOB
// 14	GPIOB.00		| sys_LED_P
// 15	GPIOB.01	LED2	| PWM 03.3	
// 26	GPIOB.03		| phy1_CLK
// 27	GPIOB.04		| phy1_SO
// 28	GPIOB.05		| phy1_SI
// 29	GPIOB.06	P2.3	| TWI_SCL	
// 30	GPIOB.07	P2.4	| PWM 03.3	| TWI_SDA	
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
#define HAL_DIO_MAPPING             {0, 1, 2, 3, 4, 5, 6, 7, 22, 23, 14, 13, 12, 10, 9, 17}
// End DIO Section

// PWM Section
#define EXTPWM_USED                 1
#define HAL_PWM_PORT2CFG            {0x110, 0x111, 0x112, 0x113, 0x219, 0x110, 0x218, 0x188, 0xFF, 0xA1B, 0xFF, 0xFF, 0x180, 0x60A, 0x609, 0x21B}
// End PWM Section

// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           8
#define HAL_AIN_BASE2APIN           {0, 1, 2, 3, 4, 5, 6, 7}
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

#define LED_Init()                  hal_dio_configure(16, DIO_MODE_OUT_PP)
#define LED_On()                    GPIOB->BSRR = (1 << 0)
#define LED_Off()                   GPIOB->BRR = (1 << 0)

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
#define OD_MAX_INDEX_LIST           32
#define OD_DEV_UC_TYPE              'S'
#define OD_DEV_UC_SUBTYPE           '4'
#define OD_DEV_PHY1                 'C'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '1'
// End Object's Dictionary Section

#ifdef __cplusplus
}
#endif

#endif //_S4C11_H
