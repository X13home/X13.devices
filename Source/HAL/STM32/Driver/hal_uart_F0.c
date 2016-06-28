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

#if (defined STM32F0) && ((defined HAL_USE_USART1) | (defined HAL_USE_USART2))

// DMA STM32F05x
//CH2 - USART1_TX
//CH3 - USART1_RX
//CH4 - USART2_TX
//CH5 - USART2_RX

#define HAL_SIZEOF_UART_RX_FIFO     64

static const uint32_t hal_baud_list[] = {2400, 4800, 9600, 19200, 38400, 93750};

#if (defined HAL_USE_USART1)
static uint8_t  Rx1_FIFO[HAL_SIZEOF_UART_RX_FIFO];
static uint16_t Rx1_Tail = 0;
#endif  // HAL_USE_USART1

#if (defined HAL_USE_USART2)
static uint8_t  Rx2_FIFO[HAL_SIZEOF_UART_RX_FIFO];
static uint16_t Rx2_Tail = 0;
#endif  //  HAL_USE_USART2

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
                RCC->APB2ENR |= RCC_APB2ENR_USART1EN;           // Enable UART1 Clock
                RCC->AHBENR |= RCC_AHBENR_DMA1EN;               // Enable the peripheral clock DMA1

                USART1->BRR = ((hal_pclk2 + (baud/2))/baud);
                
                USART1->CR1 = 0;                                // Disable USART1
                USART1->CR2 = 0;                                // 8N1
                USART1->CR3 = 0;                                // Without flow control
            }
            else
            {
                USART1->CR1 &= ~USART_CR1_UE;                   // DIsable USART
            }

            if(enable & 1) // Enable Rx
            {
                // Configure DIO
                hal_dio_configure(HAL_USART1_PIN_RX, HAL_USART1_AF);
                // DMA1 Channel3 USART_RX config
                DMA1_Channel3->CPAR = (uint32_t)&(USART1->RDR); // Peripheral address
                DMA1_Channel3->CMAR = (uint32_t)Rx1_FIFO;       // Memory address
                DMA1_Channel3->CNDTR = HAL_SIZEOF_UART_RX_FIFO; // Data size
                DMA1_Channel3->CCR =                            // Priority - Low
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
                RCC->APB1ENR |= RCC_APB1ENR_USART2EN;           // Enable UART1 Clock
                RCC->AHBENR |= RCC_AHBENR_DMA1EN;               // Enable the peripheral clock DMA1

                USART2->BRR = ((hal_pclk2 + (baud/2))/baud);
                
                USART2->CR1 = 0;                                // Disable USART1
                USART2->CR2 = 0;                                // 8N1
                USART2->CR3 = 0;                                // Without flow control
            }
            else
            {
                USART2->CR1 &= ~USART_CR1_UE;                   // DIsable USART
            }

            if(enable & 1) // Enable Rx
            {
                // Configure DIO
                hal_dio_configure(HAL_USART2_PIN_RX, HAL_USART2_AF);
                // DMA1 Channel5 USART_RX config
                DMA1_Channel5->CPAR = (uint32_t)&(USART2->RDR); // Peripheral address
                DMA1_Channel5->CMAR = (uint32_t)Rx2_FIFO;       // Memory address
                DMA1_Channel5->CNDTR = HAL_SIZEOF_UART_RX_FIFO; // Data size
                DMA1_Channel5->CCR =                            // Priority - Low
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
            if(enable & 4)  //  RS485 Mode
            {
                hal_dio_configure(HAL_USART2_PIN_DE, HAL_USART2_AF);
                USART2->CR3 |= USART_CR3_DEM;
            }
#endif  //  HAL_USART2_PIN_DE

            USART2->CR1 |= USART_CR1_UE;                        // Enable USART
            break;
#endif  //  HAL_USE_USART2

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
            return (tmp != DMA1_Channel3->CNDTR);
#endif
#if (defined HAL_USE_USART2)
        case HAL_USE_USART2:
            tmp -= Rx2_Tail;
            return (tmp != DMA1_Channel5->CNDTR);
#endif
        default:
            return false;
    }
}

uint8_t hal_uart_get(uint8_t port)
{
    uint16_t retval;

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
#endif  //  HAL_USE_USART1

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
            if(DMA1_Channel2->CCR & DMA_CCR_EN)
            {
                if(DMA1->ISR & DMA_ISR_TCIF2)
                {
                    DMA1_Channel2->CCR &= ~DMA_CCR_EN;
                    DMA1->IFCR |= DMA_IFCR_CTCIF2;  // Clear TC flag
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
            if(DMA1_Channel4->CCR & DMA_CCR_EN)
            {
                if(DMA1->ISR & DMA_ISR_TCIF4)
                {
                    DMA1_Channel4->CCR &= ~DMA_CCR_EN;
                    DMA1->IFCR |= DMA_IFCR_CTCIF4;  // Clear TC flag
                }
                else
                {
                    return false;
                }
            }
            break;
#endif  //  HAL_USE_USART2
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
            // DMA1 Channel2 USART1_TX config
            DMA1_Channel2->CPAR = (uint32_t)&(USART1->TDR);     // Peripheral address
            DMA1_Channel2->CMAR = (uint32_t)pBuf;               // Memory address
            DMA1_Channel2->CNDTR = len;
            DMA1_Channel2->CCR =                                // Priority - Low
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
            // DMA1 Channel2 USART1_TX config
            DMA1_Channel4->CPAR = (uint32_t)&(USART2->TDR);     // Peripheral address
            DMA1_Channel4->CMAR = (uint32_t)pBuf;               // Memory address
            DMA1_Channel4->CNDTR = len;
            DMA1_Channel4->CCR =                                // Priority - Low
                                                                // Memory Size - 8 bit
                                                                // Peripheral size - 8 bit
                                    DMA_CCR_MINC |              // Memory increment
                                                                // Peripheral increment disabled
                                    DMA_CCR_DIR |               // Read from memory
                                    DMA_CCR_EN;                 // DMA Channel Enable
            break;
#endif  //  HAL_USE_USART1

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

            DMA1_Channel2->CCR = 0;
            DMA1_Channel3->CCR = 0;

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

            DMA1_Channel4->CCR = 0;
            DMA1_Channel5->CCR = 0;

            RCC->APB1RSTR |= RCC_APB1RSTR_USART2RST;    // Reset USART
            RCC->APB1RSTR &= ~RCC_APB1RSTR_USART2RST;

            RCC->APB1ENR   &= ~RCC_APB1ENR_USART2EN;    // Disable UART2 Clock
            hal_dio_configure(HAL_USART2_PIN_RX, DIO_MODE_IN_FLOAT);
            hal_dio_configure(HAL_USART2_PIN_TX, DIO_MODE_IN_FLOAT);
            }
            break;
#endif  //  USART2

        default:
            while(1);
    }
}

#endif  //  EXTSER_USED

#endif