/*
Copyright (c) 2011-2016 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#include "config.h"

#if ((defined HAL_USE_USART1) || (defined HAL_USE_USART2) || (defined HAL_USE_USART3))

/*
 HAL USART, STM32F0/F1/F3

            Config 1        Config 2        Config 3
U[S]ARTx    RX      TX      RX      TX      RX      TX      APB
USART1      PA10    PA9     PB7     PB6                     2
USART2      PA3     PA2     PB4     PB3     PA15    PA14    1
USART3      PB11    PB10    PC11    PC10                    1

STM32F05x
 CH2 - USART1_TX
 CH3 - USART1_RX
 CH4 - USART2_TX
 CH5 - USART2_RX
STM32F09x
 CH6 - USART3_RX
 CH7 - USART3_TX

STM32F103, STM32F334xx
 CH2 - USART3_TX
 CH3 - USART3_RX
 CH4 - USART1_TX
 CH5 - USART1_RX
 CH6 - USART2_RX
 CH7 - USART2_TX
*/

#define HAL_SIZEOF_UART_RX_FIFO     64

static const uint32_t hal_baud_list[] = {2400, 4800, 9600, 19200, 38400, 200000};

#if (defined HAL_USE_USART1)
static uint8_t  Rx1_FIFO[HAL_SIZEOF_UART_RX_FIFO];
static uint16_t Rx1_Tail = 0;
#endif  // HAL_USE_USART1

#if (defined HAL_USE_USART2)
static uint8_t  Rx2_FIFO[HAL_SIZEOF_UART_RX_FIFO];
static uint16_t Rx2_Tail = 0;
#endif  //  HAL_USE_USART2

#if (defined HAL_USE_USART3)
static uint8_t  Rx3_FIFO[HAL_SIZEOF_UART_RX_FIFO];
static uint16_t Rx3_Tail = 0;
#endif  //  HAL_USE_USART3

// enable bit 0 - Rx, 1 - Tx, 2 - RS485 Mode
void hal_uart_init_hw(uint8_t port, uint8_t nBaud, uint8_t enable)
{
    uint32_t baud = hal_baud_list[nBaud];

    switch(port)
    {
#if (defined HAL_USE_USART1)
        case HAL_USE_USART1:
            if((RCC->APB2ENR & RCC_APB2ENR_USART1EN) == 0)      // USART Disabled
            {
                RCC->APB2ENR |= RCC_APB2ENR_USART1EN;           // Enable UART Clock
                RCC->AHBENR |= RCC_AHBENR_DMA1EN;               // Enable the peripheral clock DMA1
                
                USART1->BRR = ((hal_GetClock((uint32_t)USART1) + (baud/2))/baud);
/*
#if (defined STM32F3)
                USART1->BRR = ((hal_pclk1 + (baud/2))/baud);
#else   //  STM32F0
                USART1->BRR = ((hal_pclk2 + (baud/2))/baud);
#endif  //  STM32F3
*/
                USART1->CR1 = 0;                                // Disable USART
                USART1->CR2 = 0;                                // 8N1
                USART1->CR3 = 0;                                // Without flow control
            }
            else
            {
                USART1->CR1 &= ~USART_CR1_UE;                   // Disable USART
            }

            if(enable & 1) // Enable Rx
            {
#if (!defined STM32F1)
                // Configure DIO
                hal_dio_configure(HAL_USART1_PIN_RX, HAL_USART1_AF | DIO_MODE_IN_PU);
#endif  //  STM32F1
                // DMA1 USART1_RX config
                USART1_RX_DMA->CPAR = (uint32_t)&(USART1->RDR); // Peripheral address
                USART1_RX_DMA->CMAR = (uint32_t)Rx1_FIFO;       // Memory address
                USART1_RX_DMA->CNDTR = HAL_SIZEOF_UART_RX_FIFO; // Data size
                USART1_RX_DMA->CCR =                            // Priority - Low
                                                                // Memory Size - 8 bit
                                                                // Peripheral size - 8 bit
                                    DMA_CCR_MINC |              // Memory increment
                                                                // Peripheral increment disabled
                                    DMA_CCR_CIRC |              // Circular mode
                                    DMA_CCR_EN;                 // DMA Channel Enable
                // Configure UART
                USART1->CR1 |= USART_CR1_RE;                    // Enable RX
                USART1->CR3 |= USART_CR3_DMAR;                  // DMA enable receiver
                // Set Variables
                Rx1_Tail = 0;
            }

            if(enable & 2) // Enable Tx
            {
                hal_dio_configure(HAL_USART1_PIN_TX, HAL_USART1_AF);

                USART1->CR1 |= USART_CR1_TE;                    // Enable TX
                USART1->CR3 |= USART_CR3_DMAT;                  // DMA enable transmitter
            }

#if (defined HAL_USART1_PIN_DE)
            // Only STM32F0/F3
            if(enable & 4)  //  RS485 Mode
            {
                hal_dio_configure(HAL_USART1_PIN_DE, HAL_USART1_AF);
                USART1->CR3 |= USART_CR3_DEM;
            }
#endif  //  HAL_USART1_PIN_DE

            USART1->CR1 |= USART_CR1_UE;                        // Enable USART
            break;
#endif  //  HAL_USE_USART1

#if (defined HAL_USE_USART2)
        case HAL_USE_USART2:
            if((RCC->APB1ENR & RCC_APB1ENR_USART2EN) == 0)      // USART Disabled
            {
                RCC->APB1ENR |= RCC_APB1ENR_USART2EN;           // Enable UART Clock
                RCC->AHBENR |= RCC_AHBENR_DMA1EN;               // Enable the peripheral clock DMA1

                USART2->BRR = ((hal_GetClock((uint32_t)USART2) + (baud/2))/baud);

                USART2->CR1 = 0;                                // Disable USART
                USART2->CR2 = 0;                                // 8N1
                USART2->CR3 = 0;                                // Without flow control
            }
            else
            {
                USART2->CR1 &= ~USART_CR1_UE;                   // Disable USART
            }

            if(enable & 1) // Enable Rx
            {
#if (!defined STM32F1)
                // Configure DIO
                hal_dio_configure(HAL_USART2_PIN_RX, HAL_USART2_AF | DIO_MODE_IN_PU);
#endif  //  STM32F1
                // DMA1 USART2_RX config
                USART2_RX_DMA->CPAR = (uint32_t)&(USART2->RDR); // Peripheral address
                USART2_RX_DMA->CMAR = (uint32_t)Rx2_FIFO;       // Memory address
                USART2_RX_DMA->CNDTR = HAL_SIZEOF_UART_RX_FIFO; // Data size
                USART2_RX_DMA->CCR =                            // Priority - Low
                                                                // Memory Size - 8 bit
                                                                // Peripheral size - 8 bit
                                    DMA_CCR_MINC |              // Memory increment
                                                                // Peripheral increment disabled
                                    DMA_CCR_CIRC |              // Circular mode
                                    DMA_CCR_EN;                 // DMA Channel Enable
                // Configure UART
                USART2->CR1 |= USART_CR1_RE;                    // Enable RX
                USART2->CR3 |= USART_CR3_DMAR;                  // DMA enable receiver
                // Set Variables
                Rx2_Tail = 0;
            }

            if(enable & 2) // Enable Tx
            {
                hal_dio_configure(HAL_USART2_PIN_TX, HAL_USART2_AF);
                
                USART2->CR1 |= USART_CR1_TE;                    // Enable TX
                USART2->CR3 |= USART_CR3_DMAT;                  // DMA enable transmitter
            }

#if (defined HAL_USART2_PIN_DE)
            // Only STM32F0/F3
            if(enable & 4)  //  RS485 Mode
            {
                hal_dio_configure(HAL_USART2_PIN_DE, HAL_USART2_AF);
                USART2->CR3 |= USART_CR3_DEM;
            }
#endif  //  HAL_USART2_PIN_DE

            USART2->CR1 |= USART_CR1_UE;                        // Enable USART
            break;
#endif  //  HAL_USE_USART2

#if (defined HAL_USE_USART3)
        case HAL_USE_USART3:
            if((RCC->APB1ENR & RCC_APB1ENR_USART3EN) == 0)      // USART Disabled
            {
                RCC->APB1ENR |= RCC_APB1ENR_USART3EN;           // Enable UART Clock
                RCC->AHBENR |= RCC_AHBENR_DMA1EN;               // Enable the peripheral clock DMA1

                USART3->BRR = ((hal_GetClock((uint32_t)USART3) + (baud/2))/baud);

                USART3->CR1 = 0;                                // Disable USART
                USART3->CR2 = 0;                                // 8N1
                USART3->CR3 = 0;                                // Without flow control
            }
            else
            {
                USART3->CR1 &= ~USART_CR1_UE;                   // DIsable USART
            }

            if(enable & 1) // Enable Rx
            {
#if (!defined STM32F1)
                // Configure DIO
                hal_dio_configure(HAL_USART3_PIN_RX, HAL_USART3_AF | DIO_MODE_IN_PU);
#endif  //  STM32F1
                // DMA1 USART3_RX config
                USART3_RX_DMA->CPAR = (uint32_t)&(USART3->RDR); // Peripheral address
                USART3_RX_DMA->CMAR = (uint32_t)Rx3_FIFO;       // Memory address
                USART3_RX_DMA->CNDTR = HAL_SIZEOF_UART_RX_FIFO; // Data size
                USART3_RX_DMA->CCR =                            // Priority - Low
                                                                // Memory Size - 8 bit
                                                                // Peripheral size - 8 bit
                                    DMA_CCR_MINC |              // Memory increment
                                                                // Peripheral increment disabled
                                    DMA_CCR_CIRC |              // Circular mode
                                    DMA_CCR_EN;                 // DMA Channel Enable
                // Configure UART
                USART3->CR1 |= USART_CR1_RE;                    // Enable RX
                USART3->CR3 |= USART_CR3_DMAR;                  // DMA enable receiver
                // Set Variables
                Rx3_Tail = 0;
            }

            if(enable & 2) // Enable Tx
            {
                hal_dio_configure(HAL_USART3_PIN_TX, HAL_USART3_AF);
                
                USART3->CR1 |= USART_CR1_TE;                    // Enable TX
                USART3->CR3 |= USART_CR3_DMAT;                  // DMA enable transmitter
            }

#if (defined HAL_USART3_PIN_DE)
            // Only STM32F0/F3
            if(enable & 4)  //  RS485 Mode
            {
                hal_dio_configure(HAL_USART3_PIN_DE, HAL_USART3_AF);
                USART3->CR3 |= USART_CR3_DEM;
            }
#endif  //  HAL_USART3_PIN_DE

            USART3->CR1 |= USART_CR1_UE;                        // Enable USART
            break;
#endif  //  HAL_USE_USART3

        default:
            while(1);
    }
}

bool hal_uart_datardy(uint8_t port)
{
    uint32_t tmp = HAL_SIZEOF_UART_RX_FIFO;

    switch(port)
    {
#if (defined HAL_USE_USART1)
        case HAL_USE_USART1:
            tmp -= Rx1_Tail;
            return (tmp != USART1_RX_DMA->CNDTR);
#endif  //  HAL_USE_USART1
#if (defined HAL_USE_USART2)
        case HAL_USE_USART2:
            tmp -= Rx2_Tail;
            return (tmp != USART2_RX_DMA->CNDTR);
#endif  //  HAL_USE_USART2
#if (defined HAL_USE_USART3)
        case HAL_USE_USART3:
            tmp -= Rx3_Tail;
            return (tmp != USART3_RX_DMA->CNDTR);
#endif  //  HAL_USE_USART3
        default:
            return false;
    }
}

uint8_t hal_uart_get(uint8_t port)
{
    uint8_t retval;

    switch(port)
    {
#if (defined HAL_USE_USART1)
        case HAL_USE_USART1:
            retval = Rx1_FIFO[Rx1_Tail++];
            if(Rx1_Tail >= HAL_SIZEOF_UART_RX_FIFO)
            {
                Rx1_Tail = 0;
            }
            return retval;
#endif  //  HAL_USE_USART1

#if (defined HAL_USE_USART2)
        case HAL_USE_USART2:
            retval = Rx2_FIFO[Rx2_Tail++];
            if(Rx2_Tail >= HAL_SIZEOF_UART_RX_FIFO)
            {
                Rx2_Tail = 0;
            }
            return retval;
#endif  //  HAL_USE_USART2

#if (defined HAL_USE_USART3)
        case HAL_USE_USART3:
            retval = Rx3_FIFO[Rx3_Tail++];
            if(Rx3_Tail >= HAL_SIZEOF_UART_RX_FIFO)
            {
                Rx3_Tail = 0;
            }
            return retval;
#endif  //  HAL_USE_USART3

        default:
            return 0;
    }
}

bool hal_uart_free(uint8_t port)
{
    switch(port)
    {
#if (defined HAL_USE_USART1)
        case HAL_USE_USART1:
            if(USART1_TX_DMA->CCR & DMA_CCR_EN)
            {
                if(USART1_TX_DMA->CNDTR == 0)
                {
                    USART1_TX_DMA->CCR &= ~DMA_CCR_EN;
                }
                else
                {
                    return false;
                }
            }
            break;
#endif  //  HAL_USE_USART1

#if (defined HAL_USE_USART2)
        case HAL_USE_USART2:
            if(USART2_TX_DMA->CCR & DMA_CCR_EN)
            {
                if(USART2_TX_DMA->CNDTR == 0)
                {
                    USART2_TX_DMA->CCR &= ~DMA_CCR_EN;
                }
                else
                {
                    return false;
                }
            }
            break;
#endif  //  HAL_USE_USART2

#if (defined HAL_USE_USART3)
        case HAL_USE_USART3:
            if(USART3_TX_DMA->CCR & DMA_CCR_EN)
            {
                if(USART3_TX_DMA->CNDTR == 0)
                {
                    USART3_TX_DMA->CCR &= ~DMA_CCR_EN;
                }
                else
                {
                    return false;
                }
            }
            break;
#endif  //  HAL_USE_USART3

        default:
            break;
    }
    return true;
}

void hal_uart_send(uint8_t port, uint8_t len, uint8_t * pBuf)
{
    switch(port)
    {
#if (defined HAL_USE_USART1)
        case HAL_USE_USART1:
            // DMA1 USART1_TX config
            USART1_TX_DMA->CPAR = (uint32_t)&(USART1->TDR);     // Peripheral address
            USART1_TX_DMA->CMAR = (uint32_t)pBuf;               // Memory address
            USART1_TX_DMA->CNDTR = len;
            USART1_TX_DMA->CCR =                                // Priority - Low
                                                                // Memory Size - 8 bit
                                                                // Peripheral size - 8 bit
                                    DMA_CCR_MINC |              // Memory increment
                                                                // Peripheral increment disabled
                                    DMA_CCR_DIR |               // Read from memory
                                    DMA_CCR_EN;                 // DMA Channel Enable
            break;
#endif  //  HAL_USE_USART1

#if (defined HAL_USE_USART2)
        case HAL_USE_USART2:
            // DMA1 USART2_TX config
            USART2_TX_DMA->CPAR = (uint32_t)&(USART2->TDR);     // Peripheral address
            USART2_TX_DMA->CMAR = (uint32_t)pBuf;               // Memory address
            USART2_TX_DMA->CNDTR = len;
            USART2_TX_DMA->CCR =                                // Priority - Low
                                                                // Memory Size - 8 bit
                                                                // Peripheral size - 8 bit
                                    DMA_CCR_MINC |              // Memory increment
                                                                // Peripheral increment disabled
                                    DMA_CCR_DIR |               // Read from memory
                                    DMA_CCR_EN;                 // DMA Channel Enable
            break;
#endif  //  HAL_USE_USART1

#if (defined HAL_USE_USART3)
        case HAL_USE_USART3:
            // DMA1 USART3_TX config
            USART3_TX_DMA->CPAR = (uint32_t)&(USART3->TDR);     // Peripheral address
            USART3_TX_DMA->CMAR = (uint32_t)pBuf;               // Memory address
            USART3_TX_DMA->CNDTR = len;
            USART3_TX_DMA->CCR =                                // Priority - Low
                                                                // Memory Size - 8 bit
                                                                // Peripheral size - 8 bit
                                    DMA_CCR_MINC |              // Memory increment
                                                                // Peripheral increment disabled
                                    DMA_CCR_DIR |               // Read from memory
                                    DMA_CCR_EN;                 // DMA Channel Enable
            break;
#endif  //  HAL_USE_USART3

        default:
            break;
    }
}

#if (defined EXTSER_USED)
void hal_uart_get_pins(uint8_t port, uint8_t * pRx, uint8_t * pTx)
{
    switch(port)
    {
#if (defined HAL_USE_USART1)
        case HAL_USE_USART1:
            *pRx = HAL_USART1_PIN_RX;
            *pTx = HAL_USART1_PIN_TX;
            break;
#endif  //  HAL_USE_USART1
#if (defined HAL_USE_USART2)
        case HAL_USE_USART2:
            *pRx = HAL_USART2_PIN_RX;
            *pTx = HAL_USART2_PIN_TX;
            break;
#endif  //  USART2
#if (defined HAL_USE_USART3)
        case HAL_USE_USART3:
            *pRx = HAL_USART3_PIN_RX;
            *pTx = HAL_USART3_PIN_TX;
            break;
#endif  //  USART3
        default:
            *pRx = 0xFF;    // Not Exist
            *pTx = 0xFF;
            break;
    }
}

void hal_uart_deinit(uint8_t port)
{
    switch(port)
    {
#if (defined HAL_USE_USART1)
        case HAL_USE_USART1:
            {
            USART1->CR1 &= ~USART_CR1_UE;               // Disable USART

            USART1_TX_DMA->CCR = 0;
            USART1_RX_DMA->CCR = 0;

            RCC->APB2RSTR |= RCC_APB2RSTR_USART1RST;    // Reset USART
            RCC->APB2RSTR &= ~RCC_APB2RSTR_USART1RST;

            RCC->APB2ENR  &= ~RCC_APB2ENR_USART1EN;     // Disable UART1 Clock

            hal_dio_configure(HAL_USART1_PIN_RX, DIO_MODE_IN_FLOAT);
            hal_dio_configure(HAL_USART1_PIN_TX, DIO_MODE_IN_FLOAT);
            }
            break;
#endif  //  USART1

#if (defined HAL_USE_USART2)
        case HAL_USE_USART2:
            {
            USART2->CR1 &= ~USART_CR1_UE;               // Disable USART

            USART2_TX_DMA->CCR = 0;
            USART2_RX_DMA->CCR = 0;

            RCC->APB1RSTR |= RCC_APB1RSTR_USART2RST;    // Reset USART
            RCC->APB1RSTR &= ~RCC_APB1RSTR_USART2RST;

            RCC->APB1ENR   &= ~RCC_APB1ENR_USART2EN;    // Disable UART2 Clock
            hal_dio_configure(HAL_USART2_PIN_RX, DIO_MODE_IN_FLOAT);
            hal_dio_configure(HAL_USART2_PIN_TX, DIO_MODE_IN_FLOAT);
            }
            break;
#endif  //  USART2

#if (defined HAL_USE_USART3)
        case HAL_USE_USART3:
            {
            USART3->CR1 &= ~USART_CR1_UE;               // Disable USART

            USART3_TX_DMA->CCR = 0;
            USART3_RX_DMA->CCR = 0;

            RCC->APB1RSTR |= RCC_APB1RSTR_USART3RST;    // Reset USART
            RCC->APB1RSTR &= ~RCC_APB1RSTR_USART3RST;

            RCC->APB1ENR   &= ~RCC_APB1ENR_USART3EN;    // Disable UART2 Clock
            hal_dio_configure(HAL_USART3_PIN_RX, DIO_MODE_IN_FLOAT);
            hal_dio_configure(HAL_USART3_PIN_TX, DIO_MODE_IN_FLOAT);
            }
            break;
#endif  //  USART2

        default:
            while(1);
    }
}

#endif  //  EXTSER_USED

#endif
