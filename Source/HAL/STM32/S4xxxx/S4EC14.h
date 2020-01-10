// This file is part of the https://github.com/X13home project.

#ifndef _S4EC14_H
#define _S4EC14_H

// Board: S4EC14
// uC: STM32F303RET
// PHY1: ENC28J60
// PHY2: CC1101

// == GPIOA
// 14	GPIOA.00		| AIN 00	| PWM 02.0	
// 15	GPIOA.01		| AIN 01	| PWM 02.1	
// 16	GPIOA.02		| AIN 02	| PWM 02.2	| UART_TX 0	
// 17	GPIOA.03		| AIN 03	| PWM 02.3	| UART_RX 0	
// 20	GPIOA.04		| AIN 04	| PWM 03.1	
// 21	GPIOA.05		| AIN 05	| PWM 02.0	
// 22	GPIOA.06		| AIN 06	| PWM 16.0	
// 23	GPIOA.07		| AIN 07	| PWM 17.0	
// 46	GPIOA.13		| sys_SWDAT
// 49	GPIOA.14		| sys_SWCLK
// == GPIOB
// 26	GPIOB.00		| AIN 08	| PWM 03.2	
// 27	GPIOB.01		| AIN 09	| PWM 03.3	
// 28	GPIOB.02		| sys_LED_P
// 55	GPIOB.03		| phy1_CLK
// 56	GPIOB.04		| phy1_SO
// 57	GPIOB.05		| phy1_SI
// 58	GPIOB.06		| phy1_NCS
// 59	GPIOB.07		| phy1_RST
// 61	GPIOB.08		| PWM 04.2	| TWI_SCL	
// 62	GPIOB.09		| PWM 04.3	| TWI_SDA	
// 29	GPIOB.10		| PWM 02.2	| UART_TX 1	
// 30	GPIOB.11		| PWM 02.3	| UART_RX 1	
// 33	GPIOB.12		| phy2_NCS
// 34	GPIOB.13		| phy2_CLK
// 35	GPIOB.14		| phy2_SO
// 36	GPIOB.15		| phy2_SI
// == GPIOC
// 24	GPIOC.04		| AIN 19	
// 25	GPIOC.05		| AIN 20	
// 37	GPIOC.06		| phy2_GPIO0
// 38	GPIOC.07		| phy2_GPIO2
// == GPIOD
// 54	GPIOD.02		| phy1_IRQ
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
#define HAL_PWM_PORT2CFG            {0x110, 0x111, 0x112, 0x113, 0x219, 0x110, 0x180, 0x188, 0xFF, 0xFF, 0x222, 0x223, 0xFF, 0xFF, 0x21A, 0x21B, 0x112, 0x113}
// End PWM Section

// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           21
#define HAL_AIN_BASE2APIN           {0xFF, 0xFF, 0xFF, 0, 1, 2, 3, 4, 5, 6, 7, 0xFF, 0xFF, 0xFF, 0xFF, 19, 20, 8, 9}
// End Analogue Inputs

// UART Section
#define HAL_USE_USART2              0
#define HAL_USE_USART3              1
#define EXTSER_USED                 2
#define HAL_UART_NUM_PORTS          2
// End UART Section

// TWI Section
#define HAL_TWI_BUS                 1
#define HAL_TWI_REMAP               1
#define EXTTWI_USED                 1
// End TWI Section

#define LED_Init()                  hal_dio_configure(18, DIO_MODE_OUT_PP)
#define LED_On()                    GPIOB->BSRR = (1 << 2)
#define LED_Off()                   GPIOB->BRR = (1 << 2)

//ENC28J60 PHY1 Section	 NCS: GPIOB.06, CLK: GPIOB.03, SO: GPIOB.04, SI: GPIOB.05, RST: GPIOB.07, IRQ: GPIOD.02
#define HAL_USE_SPI1                2
#define ENC_USE_SPI                  1
#define ENC_NSS_PIN                  22
#define ENC_SELECT()                 GPIOB->BRR = (1 << 6)
#define ENC_RELEASE()                GPIOB->BSRR = (1 << 6)
#define ENC_RST_PIN                  23
#define ENC_RST_SET()                GPIOB->BRR = (1 << 7)
#define ENC_RST_CLR()                GPIOB->BSRR = (1 << 7)
#define ENC_IRQ_PIN                  50
#define ENC_IRQ_STATE()              ((GPIOD->IDR & (1 << 2)) != 0)
#define ENC_PHY                      1
//#define OD_DEF_DEV_MAC               {0x00,0x04,0xA3,0x00,0x01,0x15}   // MAC MSB->LSB
//#define OD_DEF_IP_ADDR               inet_addr(192,168,10,202)
//#define OD_DEF_IP_MASK               inet_addr(255,255,255,0)
//#define OD_DEF_IP_ROUTER             inet_addr(192,168,10,1)
//#define OD_DEF_IP_BROKER             inet_addr(192,168,20,8)
#include "PHY/ENC28J60/enc28j60_phy.h"
//End ENC28J60 PHY1 Section

//CC1101 PHY2 Section	 NCS: GPIOB.12, CLK: GPIOB.13, SO: GPIOB.14, SI: GPIOB.15, GPIO0: GPIOC.06, GPIO2: GPIOC.07
#define HAL_USE_SPI2                1
#define CC11_USE_SPI                2
#define CC11_NSS_PIN                28
#define CC11_WAIT_LOW_MISO()        while(GPIOB->IDR & (1 << 14))
#define CC11_SELECT()               GPIOB->BRR = (1 << 12)
#define CC11_RELEASE()              GPIOB->BSRR = (1 << 12)
#define CC11_IRQ_PIN                38
#define CC11_IRQ_STATE()            ((GPIOC->IDR & (1 << 6)) != 0)
#define CC11_PHY                    2
#define CC11_DEFAULT_FREQ           868300000UL
#include "PHY/CC1101/cc11_phy.h"
//End CC1101 PHY2 Section

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           48
#define OD_DEV_UC_TYPE              'S'
#define OD_DEV_UC_SUBTYPE           '4'
#define OD_DEV_PHY1                 'E'
#define OD_DEV_PHY2                 'C'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '4'
// End Object's Dictionary Section

#ifdef __cplusplus
}
#endif

#endif //_S4EC14_H
