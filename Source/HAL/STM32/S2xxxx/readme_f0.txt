// STM32F0

// Config;  Board; uC
// S2xx10; MB1034B - STM32F0 Discovery; STM32F051R8T
// S2xx11; MB11136 - STM32F0 Nucleo; STM32F091RC
// S2xx13; S2EC13/S2ER13; STM32F051C8T
// S2xx14; Proto GS; STM32F051K8T
// S2xx15; uNode V3.0; STM32F051K8T
// S2xx16; S2Mn10; STM32F051K8T


// SPI1, Config 1, PA5, PA6, PA7
// SPI1, Config 2, PB3, PB4, PB5
//
// SPI2, Config 1, PB13, PB14, PB15

// USART1, Config 0, PA9 - Tx, PA10 - Rx, PA12 - DE
// USART1, Config 1, PB6 - Tx, PB7 - Rx
//
// USART2, Config 0, PA2 - Tx, PA3 - Rx, PA1 - DE
// USART2, Config 1, PA14 - Tx, PA15 - Rx
//
// USART3, Config 0, PB10 - Tx, PB11 - Rx, PB1 - DE
// USART3, Config 1, PC10 - Tx, PC11 - Rx, PD2 - DE
// USART3, Config 2, PC4 - Tx, PC5 - Rx

// I2C1, Config 0, PB6 - SCL, PB7 - SDA
// I2C1, Config 1, PB8 - SCL, PB9 - SDA
// I2C1, Config 2, PA9 - SCL, PA10 - SDA, F091
// I2C1, Config 3, PF1 - SCL, PF0 - SDA, F091
//
// I2C2, Config 0, PB10 - SCL, PB11 - SDA
// I2C2, Config 1, PB13 - SCL, PB14 - SDA, F091
// I2C2, Config 2, PA11 - SCL, PA12 - SDA, F091
// I2C2, Config 3, PF6 - SCL, PF7 - SDA, F051

// PWM
((2<<8) |  (2<<3) | 0),    /* 0x0210 PA0:  AF2, TIM2_CH1  */ \
((2<<8) |  (2<<3) | 1),    /* 0x0211 PA1:  AF2, TIM2_CH2  */ \
((2<<8) |  (2<<3) | 2),    /* 0x0212 PA2:  AF2, TIM2_CH3  */ \
((0<<8) | (15<<3) | 0),    /* PA2:  AF0, TIM15_CH1 */ \
((0<<8) | (15<<3) | 1),    /* PA3:  AF0, TIM15_CH2 */ \
((2<<8) |  (2<<3) | 3),    /* 0x0213 PA3:  AF2, TIM2_CH4  */ \
((4<<8) | (14<<3) | 0),    /* PA4:  AF4, TIM14_CH1 */ \
((2<<8) |  (2<<3) | 0),    /* PA5:  AF2, TIM2_CH1  */ \
((1<<8) |  (3<<3) | 0),    /* 0x0118 PA6:  AF1, TIM3_CH1  */ \
((5<<8) | (16<<3) | 0),    /* PA6:  AF5, TIM16_CH1 */ \
((1<<8) |  (3<<3) | 1),    /* 0x0119 PA7:  AF1, TIM3_CH2  */ \
((4<<8) | (14<<3) | 0),    /* PA7:  AF4, TIM14_CH1 */ \
((5<<8) | (17<<3) | 0),    /* PA7:  AF5, TIM17_CH1 */ \
((2<<8) | ( 1<<3) | 0),    /* PA8:  AF2, TIM1_CH1  */ \
((2<<8) | ( 1<<3) | 1),    /* PA9:  AF2, TIM1_CH2  */ \
((2<<8) |  (1<<3) | 2),    /* PA10: AF2, TIM1_CH3  */ \
((2<<8) |  (1<<3) | 3),    /* PA11: AF2, TIM1_CH4  */ \
((2<<8) |  (2<<3) | 0)}    /* PA15: AF2, TIM2_CH1  */ \

((1<<8) |  (3<<3) | 2),    /* 0x011A PB0:  AF1, TIM3_CH3  */ \
((1<<8) |  (3<<3) | 3),    /* 0x011B PB1:  AF1, TIM3_CH4  */ \
((0<<8) | (14<<3) | 0),    /* PB1:  AF0, TIM14_CH1 */ \
((2<<8) |  (2<<3) | 1),    /* PB3:  AF2, TIM2_CH2  */ \
((1<<8) |  (3<<3) | 0),    /* PB4:  AF1, TIM3_CH1  */ \
((1<<8) |  (3<<3) | 1),    /* PB5:  AF1, TIM3_CH2  */ \
((2<<8) | (16<<3) | 0),    /* PB8:  AF2, TIM16_CH1 */ \
((2<<8) | (17<<3) | 0),    /* PB9:  AF2, TIM17_CH1 */ \
((2<<8) |  (2<<3) | 2),    /* PB10: AF2, TIM2_CH3  */ \
((2<<8) |  (2<<3) | 3),    /* PB11: AF2, TIM2_CH4  */ \
((1<<8) | (15<<3) | 0),    /* PB14: AF1, TIM15_CH1 */ \
((1<<8) | (15<<3) | 1),    /* PB15: AF1, TIM15_CH2 */ \

((0<<8) |  (3<<3) | 0),    /* PC6:  AF0, TIM3_CH1  */ \
((0<<8) |  (3<<3) | 1),    /* PC7:  AF0, TIM3_CH2  */ \
((0<<8) |  (3<<3) | 2),    /* PC8:  AF0, TIM3_CH3  */ \
((0<<8) |  (3<<3) | 3),    /* PC9:  AF0, TIM3_CH4  */ \
