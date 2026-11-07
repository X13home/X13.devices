// This file is part of the https://github.com/X13home project.

#ifndef _A1E12_H
#define _A1E12_H

// Board: A1E12
// uC: ATMEGA328P
// PHY1: ENC28J60

// 19	AIN6	CN20	| AIN 06	
// 22	AIN7	CN19	| AIN 07	
// == PORTB
// 12	PORTB.00		| sys_LED_N
// 14	PORTB.02		| phy1_NCS
// 15	PORTB.03		| phy1_SI
// 16	PORTB.04		| phy1_SO
// 17	PORTB.05		| phy1_CLK
// 07	PORTB.06		| sys_HSE_IN
// == PORTC
// 23	PORTC.00	CN18	| AIN 00	
// 24	PORTC.01	CN17	| AIN 01	
// 25	PORTC.02	CN16	| AIN 02	
// 26	PORTC.03	CN15	| AIN 03	
// 27	PORTC.04	CN14	| AIN 04	| TWI_SDA	
// 28	PORTC.05	CN13	| AIN 05	| TWI_SCL	
// == PORTD
// 30	PORTD.00	CN10	| UART_RX 0	
// 31	PORTD.01	CN9	| UART_TX 0	
// 32	PORTD.02	CN8	
// 01	PORTD.03	CN7	
// 02	PORTD.04	CN6	
// 09	PORTD.05	CN5	| PWM 00.1	
// 10	PORTD.06	CN4	| PWM 00.0	
// 11	PORTD.07	CN3	

#ifdef __cplusplus
extern "C" {
#endif

#define F_CPU                       12500000UL

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_PORT_OFFSET          2
#define EXTDIO_MAXPORT_NR           2
#define HAL_DIO_MAPPING             {0xFF, 0xFF, 16, 17, 18, 19, 20, 21, 24, 25, 26, 27, 28, 29, 30, 31}
// End DIO Section

// PWM Section
#define EXTPWM_USED                 1
#define HAL_PWM_BASE_OFFSET         13
#define HAL_PWM_PORT2CFG            {0x01, 0x00}
// End PWM Section

// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           15
#define HAL_AIN_BASE2APIN           {6, 7, 0, 1, 2, 3, 4, 5, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 14}
// End Analogue Inputs

// UART Section
#define HAL_USE_USART0              0
#define EXTSER_USED                 1
#define HAL_UART_NUM_PORTS          1
// End UART Section

// TWI Section
#define HAL_TWI_BUS                 1
#define EXTTWI_USED                 1
// End TWI Section

#define LED_Init()                  hal_dio_configure(8, DIO_MODE_OUT_PP)
#define LED_On()                    PORTB &= ~(1 << 0)
#define LED_Off()                   PORTB |= (1 << 0)

//ENC28J60 PHY1 Section	 NCS: PORTB.02, CLK: PORTB.05, SO: PORTB.04, SI: PORTB.03
#define HAL_USE_SPI1                1
#define ENC_USE_SPI                  1
#define ENC_NSS_PIN                  10
#define ENC_SELECT()                 PORTB &= ~(1 << 2)
#define ENC_RELEASE()                PORTB |= (1 << 2)
#define ENC_PHY                      1
#define OD_DEF_DEV_MAC               {0x00,0x04,0xA3,0x00,0x00,0x05}   // MAC MSB->LSB
//#define OD_DEF_IP_ADDR               inet_addr(192,168,10,202)
//#define OD_DEF_IP_MASK               inet_addr(255,255,255,0)
//#define OD_DEF_IP_ROUTER             inet_addr(192,168,10,1)
//#define OD_DEF_IP_BROKER             inet_addr(192,168,20,8)
#include "PHY/ENC28J60/enc28j60_phy.h"
//End ENC28J60 PHY1 Section

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           17
#define OD_DEV_UC_TYPE              'A'
#define OD_DEV_UC_SUBTYPE           '1'
#define OD_DEV_PHY1                 'E'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '2'
// End Object's Dictionary Section

#ifdef __cplusplus
}
#endif

#endif //_A1E12_H
