// STM32L0

// Config;  Board; uC
// S5xx10; uNode V3.0; STM32L051K8
// S5xx11: CPUe; STM32L051K8

// SPI1, Config 1, PA5, PA6, PA7
// SPI1, Config 2, PB3, PB4, PB5
//
// SPI2, Config 1, PB13, PB14, PB15

// USART1, Config 0, PA9 - Tx, PA10 - Rx, PA12 - DE
// USART1, Config 1, PB6 - Tx, PB7 - Rx
//
// USART2, Config 0, PA2 - Tx, PA3 - Rx, PA1 - DE
// USART2, Config 1, PA14 - Tx, PA15 - Rx

// I2C1, Config 0, PB6 - SCL, PB7 - SDA
// I2C1, Config 1, PB8 - SCL, PB9 - SDA
//
// I2C2, Config 0, PB10 - SCL, PB11 - SDA
// I2C2, Config 1, PB13 - SCL, PB14 - SDA

// PWM
((2<<8) |  (2<<3) | 0)}    /* PA0:  AF2, TIM2_CH1  */ \
((2<<8) |  (2<<3) | 1),    /* PA1:  AF2, TIM2_CH2  */ \
((2<<8) |  (2<<3) | 2),    /* PA2:  AF2, TIM2_CH3  */ \
((0<<8) | (21<<3) | 0),    /* PA2:  AF0, TIM21_CH1 */ \
((2<<8) |  (2<<3) | 3),    /* PA3:  AF2, TIM2_CH4  */ \
((0<<8) | (21<<3) | 1),    /* PA3:  AF0, TIM21_CH2 */ \
((5<<8) |  (2<<3) | 0),    /* PA5:  AF5, TIM2_CH1  */ \
((5<<8) | (22<<3) | 0),    /* PA6:  AF5, TIM22_CH1 */ \
((5<<8) | (22<<3) | 1),    /* PA7:  AF5, TIM22_CH2 */ \
((5<<8) |  (2<<3) | 0)}    /* PA15: AF5, TIM2_CH1  */ \

((2<<8) |  (2<<3) | 1),    /* PB3:  AF2, TIM2_CH2  */ \
((4<<8) |  (22<<3) | 0),   /* PB4:  AF4, TIM22_CH1  */ \
((4<<8) |  (22<<3) | 1),   /* PB5:  AF4, TIM22_CH2  */ \
((2<<8) |  (2<<3) | 2),    /* PB10: AF2, TIM2_CH3  */ \
((2<<8) |  (2<<3) | 3),    /* PB11: AF2, TIM2_CH4  */ \
((6<<8) |  (21<<3) | 0),   /* PB13: AF6, TIM21_CH1  */ \
((6<<8) |  (21<<3) | 1),   /* PB14: AF6, TIM21_CH2  */ \

((0<<8) |  (22<<3) | 0),   /* PC6:  AF0, TIM22_CH1  */ \
((0<<8) |  (22<<3) | 1),   /* PC7:  AF0, TIM22_CH2  */ \
