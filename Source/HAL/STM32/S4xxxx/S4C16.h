// This file is part of the https://github.com/X13home project.

#ifndef _S4C16_H
#define _S4C16_H

// Board: S4C16
// uC: STM32F303KT
// PHY1: CC1101

// == GPIOA
// 06	GPIOA.00		| AIN 00	| PWM 02.0	
// 07	GPIOA.01		| AIN 01	| PWM 02.1	
// 08	GPIOA.02		| AIN 02	| PWM 02.2	| UART_TX 0	
// 09	GPIOA.03		| AIN 03	| PWM 02.3	| UART_RX 0	
// 10	GPIOA.04		| AIN 04	| PWM 03.1	
// 11	GPIOA.05		| AIN 05	| PWM 02.0	
// 12	GPIOA.06		| AIN 06	| PWM 16.0	
// 13	GPIOA.07		| AIN 07	| PWM 17.0	
// 18	GPIOA.08		| PWM 01.0	
// 19	GPIOA.09		| PWM 01.1	| UART_TX 1	
// 20	GPIOA.10		| PWM 01.2	| UART_RX 1	
// 21	GPIOA.11		| PWM 01.3	
// 22	GPIOA.12		
// 23	GPIOA.13		
// 24	GPIOA.14		
// 25	GPIOA.15		| phy1_NCS
// == GPIOB
// 14	GPIOB.00		| sys_LED_P
// 15	GPIOB.01		| AIN 09	| PWM 03.3	
// 26	GPIOB.03		| phy1_CLK
// 27	GPIOB.04		| phy1_SO
// 28	GPIOB.05		| phy1_SI
// 29	GPIOB.06		| TWI_SCL	
// 30	GPIOB.07		| TWI_SDA	
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
#define HAL_DIO_MAPPING             {0, 1, 2, 3, 4, 5, 6, 7, 17, 8, 11, 13, 14, 23, 22, 9, 10, 12}
// End DIO Section

// PWM Section
#define EXTPWM_USED                 1
#define HAL_PWM_PORT2CFG            {0x110, 0x111, 0x112, 0x113, 0x219, 0x110, 0x180, 0x188, 0x21B, 0x608, 0xB0B, 0xFF, 0xFF, 0xFF, 0xFF, 0x609, 0x60A}
// End PWM Section

// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           10
#define HAL_AIN_BASE2APIN           {0, 1, 2, 3, 4, 5, 6, 7, 9}
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
#define CC11_DEFAULT_FREQ           868300000UL
#include "PHY/CC1101/cc11_phy.h"
//End CC1101 PHY1 Section

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           34
#define OD_DEV_UC_TYPE              'S'
#define OD_DEV_UC_SUBTYPE           '4'
#define OD_DEV_PHY1                 'C'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '6'
// End Object's Dictionary Section

#ifdef __cplusplus
}
#endif

#endif //_S4C16_H
