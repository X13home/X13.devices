// This file is part of the https://github.com/X13home project.

#ifndef _A1SR10_H
#define _A1SR10_H

// Board: A1SR10
// uC: ATMEGA328P
// PHY1: UART
// PHY2: RFM12

// == PORTB
// 12	PORTB.00	ISP.7	
// 13	PORTB.01	ISP.8	| PWM 01.0	
// 14	PORTB.02		| phy2_NCS
// 15	PORTB.03		| phy2_SI
// 16	PORTB.04		| phy2_SO
// 17	PORTB.05		| phy2_CLK
// 07	PORTB.06		| sys_HSE_IN
// 08	PORTB.07		| sys_HSE_OUT
// == PORTC
// 23	PORTC.00	P1.5	| AIN 00	
// 24	PORTC.01	P2.5	| AIN 01	
// 25	PORTC.02	P3.5	| AIN 02	
// 26	PORTC.03	P4.5	| AIN 03	
// 27	PORTC.04	S.5	| AIN 04	| TWI_SDA	
// 28	PORTC.05	S.6	| AIN 05	| TWI_SCL	
// == PORTD
// 30	PORTD.00		| phy1_RX
// 31	PORTD.01		| phy1_TX
// 32	PORTD.02		| phy2_IRQ
// 01	PORTD.03	Px.6	
// 02	PORTD.04	P1.2	
// 09	PORTD.05	P2.2	| PWM 00.1	
// 10	PORTD.06	P3.2	| PWM 00.0	
// 11	PORTD.07	P4.2	

#ifdef __cplusplus
extern "C" {
#endif

#define F_CPU                       16000000UL

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_PORT_OFFSET          1
#define EXTDIO_MAXPORT_NR           3
#define HAL_DIO_MAPPING             {16, 17, 18, 19, 28, 29, 30, 31, 27, 20, 21, 8, 9}
// End DIO Section

// PWM Section
#define EXTPWM_USED                 1
#define HAL_PWM_BASE_OFFSET         5
#define HAL_PWM_PORT2CFG            {0x01, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x08}
// End PWM Section

// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           6
#define HAL_AIN_BASE2APIN           {0, 1, 2, 3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 4, 5}
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

//RFM12 PHY2 Section	 NCS: PORTB.02, CLK: PORTB.05, SO: PORTB.04, SI: PORTB.03, IRQ: PORTD.02
#define HAL_USE_SPI1                1
#define HAL_USE_EXTI                1
#define RFM12_USE_SPI               1
#define RFM12_NSS_PIN               10
#define RFM12_SELECT()              PORTB &= ~(1 << 2)
#define RFM12_RELEASE()             PORTB |= (1 << 2)
#define RFM12_IRQ_PIN               26
#define RFM12_IRQ_STATE()           ((PIND & (1 << 2)) == 0)
#define RFM12_PHY                   2
#define RFM12_DEFAULT_FREQ          868300000UL
#include "PHY/RFM12/rfm12_phy.h"
//End RFM12 PHY2 Section

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           13
#define OD_DEV_UC_TYPE              'A'
#define OD_DEV_UC_SUBTYPE           '1'
#define OD_DEV_PHY1                 'S'
#define OD_DEV_PHY2                 'R'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '0'
// End Object's Dictionary Section

#ifdef __cplusplus
}
#endif

#endif //_A1SR10_H
