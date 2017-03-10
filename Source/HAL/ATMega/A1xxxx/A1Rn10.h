// This file is part of the https://github.com/X13home project.

#ifndef _A1Rn10_H
#define _A1Rn10_H

// Board: A1Rn10
// uC: ATMEGA328P
// PHY1: RFM12

// == PORTB
// 14	PORTB.02		| phy1_NCS
// 15	PORTB.03		| phy1_SI
// 16	PORTB.04		| phy1_SO
// 17	PORTB.05		| phy1_CLK
// 07	PORTB.06		| sys_HSE_IN
// 08	PORTB.07		| sys_HSE_OUT
// == PORTC
// 23	PORTC.00	P1.5	| AIN 00	
// 24	PORTC.01	P2.5	| AIN 01	
// 25	PORTC.02	P3.5	| AIN 02	
// 26	PORTC.03	P4.5	| AIN 03	
// == PORTD
// 32	PORTD.02		| phy1_IRQ
// 02	PORTD.04	P1.2	
// 09	PORTD.05	P2.2	| PWM 00.1	
// 10	PORTD.06	P3.2	| PWM 00.0	
// 11	PORTD.07	P4.2	

#ifdef __cplusplus
extern "C" {
#endif

#define F_CPU                       16000000UL

#define ASLEEP                      1
#define OD_DEFAULT_TASLEEP          0

// DIO Section
#define EXTDIO_USED                 1
#define EXTDIO_PORT_OFFSET          2
#define EXTDIO_MAXPORT_NR           2
#define HAL_DIO_MAPPING             {16, 17, 18, 19, 28, 29, 30, 31}
// End DIO Section

// PWM Section
#define EXTPWM_USED                 1
#define HAL_PWM_BASE_OFFSET         5
#define HAL_PWM_PORT2CFG            {0x01, 0x00}
// End PWM Section

// Analogue Inputs
#define EXTAIN_USED                 1
#define EXTAIN_MAXPORT_NR           4
#define HAL_AIN_BASE2APIN           {0, 1, 2, 3}
// End Analogue Inputs

// UART Section
// End UART Section

//RFM12 PHY1 Section	 NCS: PORTB.02, CLK: PORTB.05, SO: PORTB.04, SI: PORTB.03, IRQ: PORTD.02
#define HAL_USE_SPI1                1
#define HAL_USE_EXTI                1
#define RFM12_USE_SPI               1
#define RFM12_NSS_PIN               10
#define RFM12_SELECT()              PORTB &= ~(1 << 2)
#define RFM12_RELEASE()             PORTB |= (1 << 2)
#define RFM12_IRQ_PIN               26
#define RFM12_IRQ_STATE()           ((PIND & (1 << 2)) == 0)
#define RFM12_PHY                   1
#define RFM12_DEFAULT_FREQ          868300000UL
#include "PHY/RFM12/rfm12_phy.h"
//End RFM12 PHY1 Section

// Object's Dictionary Section
#define OD_MAX_INDEX_LIST           8
#define OD_DEV_UC_TYPE              'A'
#define OD_DEV_UC_SUBTYPE           '1'
#define OD_DEV_PHY1                 'R'
#define OD_DEV_PHY2                 'n'
#define OD_DEV_HW_TYP_H             '1'
#define OD_DEV_HW_TYP_L             '0'
// End Object's Dictionary Section

#ifdef __cplusplus
}
#endif

#endif //_A1Rn10_H
