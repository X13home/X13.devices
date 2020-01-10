// This file is part of the https://github.com/X13home project.

#ifndef _A1SC10_H
#define _A1SC10_H

// Board:   Arduino Nano/Uno
// uC: ATMEGA328P
// PHY1: UART
// PHY2: CC1101

// 19	AIN6	A6	| AIN 06	
// 22	AIN7	A7	| AIN 07	
// == PORTB
// 12	PORTB.00	D8	
// 13	PORTB.01	D9	| PWM 01.0	
// 14	PORTB.02	D10	| phy2_NCS
// 15	PORTB.03	D11	| phy2_SI
// 16	PORTB.04	D12	| phy2_SO
// 17	PORTB.05	D13	| phy2_CLK
// 07	PORTB.06		| sys_HSE_IN
// 08	PORTB.07		| sys_HSE_OUT
// == PORTC
// 23	PORTC.00	A0	| AIN 00	
// 24	PORTC.01	A1	| AIN 01	
// 25	PORTC.02	A2	| AIN 02	
// 26	PORTC.03	A3	| AIN 03	
// 27	PORTC.04	A4	| AIN 04	| TWI_SDA	
// 28	PORTC.05	A5	| AIN 05	| TWI_SCL	
// == PORTD
// 30	PORTD.00	D0	| phy1_RX
// 31	PORTD.01	D1	| phy1_TX
// 32	PORTD.02	D2	
// 01	PORTD.03	D3	
// 02	PORTD.04	D4	
// 09	PORTD.05	D5	| PWM 00.1	
// 10	PORTD.06	D6	| PWM 00.0	
// 11	PORTD.07	D7	

#ifdef __cplusplus
extern "C" {
#endif

#define F_CPU                       16000000UL

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_PORT_OFFSET          1
#define EXTDIO_MAXPORT_NR           3
#define HAL_DIO_MAPPING             {16, 17, 18, 19, 20, 21, 0xFF, 0xFF, 0xFF, 0xFF, 26, 27, 28, 29, 30, 31, 8, 9}
// End DIO Section

// PWM Section
#define EXTPWM_USED                 1
#define HAL_PWM_BASE_OFFSET         13
#define HAL_PWM_PORT2CFG            {0x01, 0x00, 0xFF, 0xFF, 0x08}
// End PWM Section

// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           8
#define HAL_AIN_BASE2APIN           {0, 1, 2, 3, 4, 5, 6, 7}
// End Analogue Inputs

// UART Section
#define HAL_UART_NUM_PORTS          1
// End UART Section

// TWI Section
#define HAL_TWI_BUS                 1
#define EXTTWI_USED                 1
// End TWI Section

//UART PHY1 Section	 RX: PORTD.00, TX: PORTD.01
#define HAL_USE_USART0              0
#define UART_PHY_PORT               0
#define UART_PHY                    1
#include "PHY/UART/uart_phy.h"
//End UART PHY1 Section

//CC1101 PHY2 Section	 NCS: PORTB.02, CLK: PORTB.05, SO: PORTB.04, SI: PORTB.03
#define HAL_USE_SPI1                1
#define CC11_USE_SPI                1
#define CC11_NSS_PIN                10
#define CC11_WAIT_LOW_MISO()        while(PINB & (1 << 4))
#define CC11_SELECT()               PORTB &= ~(1 << 2)
#define CC11_RELEASE()              PORTB |= (1 << 2)
#define CC11_PHY                    2
#define CC11_DEFAULT_FREQ           868300000UL
#include "PHY/CC1101/cc11_phy.h"
//End CC1101 PHY2 Section

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           16
#define OD_DEV_UC_TYPE              'A'
#define OD_DEV_UC_SUBTYPE           '1'
#define OD_DEV_PHY1                 'S'
#define OD_DEV_PHY2                 'C'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '0'
// End Object's Dictionary Section

#ifdef __cplusplus
}
#endif

#endif //_A1SC10_H
