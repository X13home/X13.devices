// This file is part of the https://github.com/X13home project.

#ifndef _S2SR10_H
#define _S2SR10_H

// Board: S2SR10
// uC: STM32F051RT
// PHY1: UART
// PHY2: RFM12

// == GPIOA
// 14	GPIOA.00	P1.15	| AIN 00	| PWM 02.0	
// 15	GPIOA.01	P1.16	| AIN 01	| PWM 02.1	
// 16	GPIOA.02	P1.17	| AIN 02	| PWM 02.2	| UART_TX 0	
// 17	GPIOA.03	P1.18	| AIN 03	| PWM 02.3	| UART_RX 0	
// 20	GPIOA.04	P1.21	| AIN 04	| PWM 14.0	
// 21	GPIOA.05	P1.22	| AIN 05	| PWM 02.0	
// 22	GPIOA.06	P1.23	| AIN 06	| PWM 16.0	
// 23	GPIOA.07	P1.24	| AIN 07	| PWM 17.0	
// 41	GPIOA.08	P1.25	| PWM 01.0	
// 42	GPIOA.09	P2.24	| PWM 01.1	
// 43	GPIOA.10	P2.23	| PWM 01.2	
// 44	GPIOA.11	P2.22	| PWM 01.3	
// 45	GPIOA.12	P2.21	
// 46	GPIOA.13		| sys_SWDAT
// 49	GPIOA.14		| sys_SWCLK
// 50	GPIOA.15	P2.16	| PWM 02.0	
// == GPIOB
// 26	GPIOB.00	P1.27	| AIN 08	| PWM 03.2	
// 27	GPIOB.01	P1.28	| AIN 09	| PWM 03.3	
// 28	GPIOB.02	P1.29	
// 55	GPIOB.03	P2.11	| PWM 02.1	
// 56	GPIOB.04	P2.10	| PWM 03.0	
// 57	GPIOB.05	P2.9	| PWM 03.1	
// 58	GPIOB.06		| phy1_TX
// 59	GPIOB.07		| phy1_RX
// 61	GPIOB.08	P2.4	| PWM 16.0	
// 62	GPIOB.09	P2.3	| PWM 17.0	
// 29	GPIOB.10	P1.30	| PWM 02.2	
// 30	GPIOB.11		| phy2_IRQ
// 33	GPIOB.12		| phy2_NCS
// 34	GPIOB.13		| phy2_CLK
// 35	GPIOB.14		| phy2_SO
// 36	GPIOB.15		| phy2_SI
// == GPIOC
// 08	GPIOC.00	P1.11	| AIN 10	
// 09	GPIOC.01	P1.12	| AIN 11	
// 10	GPIOC.02	P1.13	| AIN 12	
// 11	GPIOC.03	P1.14	| AIN 13	
// 24	GPIOC.04	P1.25	| AIN 14	
// 25	GPIOC.05	P1.26	| AIN 15	
// 37	GPIOC.06	P2.29	| PWM 03.0	
// 38	GPIOC.07	P2.28	| PWM 03.1	
// 39	GPIOC.08	P2.27 LED-B	| PWM 03.2	
// 40	GPIOC.09	P2.26 LED-G	| PWM 03.3	
// 51	GPIOC.10	P2.15	
// 52	GPIOC.11	P2.14	
// 53	GPIOC.12	P2.13	
// 02	GPIOC.13	P1.4	
// 03	GPIOC.14	P1.5	
// 04	GPIOC.15	P1.6	
// == GPIOD
// 54	GPIOD.02	P2.12	
// == GPIOF
// 05	GPIOF.00		| sys_EXT_CLOCK
// 18	GPIOF.04	P1.19	
// 19	GPIOF.05	P1.20	
// 47	GPIOF.06	P2.19	| TWI_SCL	
// 48	GPIOF.07		| TWI_SDA	

#ifdef __cplusplus
extern "C" {
#endif

#define HSE_CRYSTAL_BYPASS          1

#define HAL_USE_RTC                 1
#define HAL_RTC_USE_HSE             1

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_MAXPORT_NR           6
#define HAL_DIO_MAPPING             {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 0xFF, 0xFF, 15, 16, 17, 18, 19, 20, 21, 0xFF, 0xFF, 24, 25, 26, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 0xFF, 0xFF, 50, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 84, 85, 86, 87}
// End DIO Section

// PWM Section
#define EXTPWM_USED                 1
#define HAL_PWM_PORT2CFG            {0x210, 0x211, 0x212, 0x213, 0x470, 0x210, 0x580, 0x588, 0x208, 0x209, 0x20A, 0x20B, 0xFF, 0xFF, 0xFF, 0x210, 0x11A, 0x11B, 0xFF, 0x211, 0x118, 0x119, 0xFF, 0xFF, 0x280, 0x288, 0x212, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x18, 0x19, 0x1A, 0x1B}
// End PWM Section

// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           16
#define HAL_AIN_BASE2APIN           {0, 1, 2, 3, 4, 5, 6, 7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 8, 9, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 10, 11, 12, 13, 14, 15}
// End Analogue Inputs

// UART Section
#define HAL_USE_USART2              0
#define EXTSER_USED                 1
#define HAL_UART_NUM_PORTS          2
// End UART Section

// TWI Section
#define HAL_TWI_BUS                 2
#define HAL_TWI_REMAP               3
#define EXTTWI_USED                 1
// End TWI Section

//UART PHY1 Section	 RX: GPIOB.07, TX: GPIOB.06
#define HAL_USE_USART1              1
#define HAL_USART1_REMAP           1
#define UART_PHY_PORT               1
#define UART_PHY                    1
#include "PHY/UART/uart_phy.h"
//End UART PHY1 Section

//RFM12 PHY2 Section	 NCS: GPIOB.12, CLK: GPIOB.13, SO: GPIOB.14, SI: GPIOB.15, IRQ: GPIOB.11
#define HAL_USE_SPI2                1
#define HAL_USE_EXTI                1
#define RFM12_USE_SPI               2
#define RFM12_NSS_PIN               28
#define RFM12_SELECT()              GPIOB->BRR = (1 << 12)
#define RFM12_RELEASE()             GPIOB->BSRR = (1 << 12)
#define RFM12_IRQ_PIN               27
#define RFM12_IRQ_STATE()           ((GPIOB->IDR & (1 << 11)) == 0)
#define RFM12_PHY                   2
#define RFM12_DEFAULT_FREQ          868300000UL
#include "PHY/RFM12/rfm12_phy.h"
//End RFM12 PHY2 Section

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           60
#define OD_DEV_UC_TYPE              'S'
#define OD_DEV_UC_SUBTYPE           '2'
#define OD_DEV_PHY1                 'S'
#define OD_DEV_PHY2                 'R'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '0'
// End Object's Dictionary Section

#ifdef __cplusplus
}
#endif

#endif //_S2SR10_H
