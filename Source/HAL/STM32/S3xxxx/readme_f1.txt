// STM32F0

// Config;  Board; uC
// S3xx10; Maple Mini; STM32F103CBT6

// SPI1, Config 1, PA5, PA6, PA7
// SPI1, Config 2, PB3, PB4, PB5
// SPI2, Config 1, PB13, PB14, PB15

// USART1, Config 0, PA9 - Tx, PA10 - Rx
// USART1, Config 1, PB6 - Tx, PB7 - Rx
//
// USART2, Config 0, PA2 - Tx, PA3 - Rx
// USART2, Config 1, PD5 - Tx, PD6 - Rx, LQFP100
//
// USART3, Config 0, PB10 - Tx, PB11 - Rx
// USART3, Config 1, PC10 - Tx, PC11 - Rx

// I2C1, Config 0, PB6 - SCL, PB7 - SDA
// I2C1, Config 1, PB8 - SCL, PB9 - SDA
// I2C2, Config 0, PB10 - SCL, PB11 - SDA

// PWM
((2<<3) | 0)}    /* PA0:  TIM2_CH1  */ \
((2<<3) | 1),    /* PA1:  TIM2_CH2  */ \
((2<<3) | 2),    /* PA2:  TIM2_CH3  */ \
((2<<3) | 3),    /* PA3:  TIM2_CH4  */ \
((3<<3) | 0),    /* PA6:  TIM3_CH1  */ \
((3<<3) | 1),    /* PA7:  TIM3_CH2  */ \
((1<<3) | 0),    /* PA8:  TIM1_CH1  */ \
((1<<3) | 1),    /* PA9:  TIM1_CH2  */ \
((1<<3) | 2),    /* PA10: TIM1_CH3  */ \
((1<<3) | 3),    /* PA11: TIM1_CH4  */ \

((3<<3) | 2),    /* PB0:  TIM3_CH3  */ \
((3<<3) | 3),    /* PB1:  TIM3_CH4  */ \
((2<<3) | 1),    /* PB3:  TIM2_CH2  */ \
((3<<3) | 0),    /* PB4:  TIM3_CH1  */ \
((3<<3) | 1),    /* PB5:  TIM3_CH2  */ \
((4<<3) | 0),    /* PB6:  TIM4_CH1  */ \
((4<<3) | 1),    /* PB7:  TIM4_CH2  */ \
((4<<3) | 2),    /* PB8:  TIM4_CH3  */ \
((4<<3) | 3),    /* PB9:  TIM4_CH4  */ \
