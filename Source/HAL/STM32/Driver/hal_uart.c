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

#if ((defined HAL_USE_USART1)       || \
     (defined HAL_USE_USART2)       || \
     (defined HAL_USE_USART3))

#define HAL_SIZEOF_UART_RX_FIFO     32      // Should be 2^n

/*          Config 1        Config 2
U[S]ARTx    RX      TX      RX      TX      APB
USART1      PA10    PA9     PB7     PB6     2
USART2      PA3     PA2                     1
USART3      PB11    PB10    PC11    PC10    1
*/

typedef struct
{
    uint8_t         *   pTxBuf;
    uint8_t             tx_len;
    uint8_t             tx_pos;
    
    uint8_t             rx_fifo[HAL_SIZEOF_UART_RX_FIFO];
    volatile uint8_t    rx_head;
    uint8_t             rx_tail;
}HAL_UART_t;

static const uint16_t hal_baud_list[] = {2400, 4800, 9600, 19200, 38400};
static HAL_UART_t * hal_UARTv[HAL_UART_NUM_PORTS] = {NULL, };

// IRQ handlers
static void hal_uart_irq_handler(uint8_t port, USART_TypeDef * USARTx)
{
    HAL_UART_t * control = hal_UARTv[port];

    uint8_t data;
    uint32_t itstat;

#if ((defined STM32F0) || (defined STM32F3))
    itstat = USARTx->ISR;
    if(itstat & USART_ISR_ORE)
    {
        USARTx->ICR = USART_ICR_ORECF;
        return;
    }
#elif (defined STM32F1)
    itstat = USARTx->SR;
    if(itstat & USART_SR_ORE)
    {
        data = USARTx->RDR;
        return;
    }
#else
#error hal_uart_irq_handler Unknown uC Family
#endif

    itstat &= USARTx->CR1;

    // Received data is ready to be read
    if(itstat & USART_CR1_RXNEIE)
    {
        data = USARTx->RDR;
        uint8_t tmp_head = (control->rx_head + 1) & (uint8_t)(HAL_SIZEOF_UART_RX_FIFO - 1);
        if(tmp_head == control->rx_tail)        // Overflow
        {
            return;
        }

        control->rx_fifo[control->rx_head] = data;
        control->rx_head = tmp_head;
    }

    // Transmit data register empty
    if(itstat & USART_CR1_TXEIE)
    {
        if(control->tx_pos == control->tx_len)
        {
            control->tx_len = 0;
            USARTx->CR1 &= ~(uint32_t)USART_CR1_TXEIE;
            return;
        }

        USARTx->TDR = control->pTxBuf[control->tx_pos];
        control->tx_pos++;
    }
}

#if (defined HAL_USE_USART1)
void USART1_IRQHandler(void)
{
    hal_uart_irq_handler(HAL_USE_USART1, USART1);
}
#endif  // USART1

#if (defined HAL_USE_USART2)
void USART2_IRQHandler(void)
{
    hal_uart_irq_handler(HAL_USE_USART2, USART2);
}
#endif  //  USART2

#if (defined HAL_USE_USART3)
void USART3_IRQHandler(void)
{
    hal_uart_irq_handler(HAL_USE_USART3, USART3);
}
#endif  //  USART3

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
            NVIC_DisableIRQ(USART1_IRQn);               // Disable USART IRQ

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
            NVIC_DisableIRQ(USART2_IRQn);               // Disable USART IRQ

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
            NVIC_DisableIRQ(USART3_IRQn);               // Disable USART IRQ

            RCC->APB1RSTR |= RCC_APB1RSTR_USART3RST;    // Reset USART
            RCC->APB1RSTR &= ~RCC_APB1RSTR_USART3RST;

            RCC->APB1ENR   &= ~RCC_APB1ENR_USART3EN;    // Disable UART2 Clock
            hal_dio_configure(HAL_USART3_PIN_RX, DIO_MODE_IN_FLOAT);
            hal_dio_configure(HAL_USART3_PIN_TX, DIO_MODE_IN_FLOAT);
            }
            break;
#endif  //  USART3

        default:
            while(1);
    }

    if(hal_UARTv[port] != NULL)
    {
        mqFree(hal_UARTv[port]);
        hal_UARTv[port] = NULL;
    }
}

// enable bit 0 - Rx, 1 - Tx
void hal_uart_init_hw(uint8_t port, uint8_t nBaud, uint8_t enable)
{
    USART_TypeDef     * USARTx;
    IRQn_Type           UARTx_IRQn;

    uint32_t            uart_clock;
    uint16_t            baud = hal_baud_list[nBaud];

    switch(port)
    {
#if (defined HAL_USE_USART1)
        case HAL_USE_USART1:
            {
#if (defined STM32F3)
            uart_clock = hal_pclk1;
#else
            uart_clock = hal_pclk2;
#endif
#if ((defined STM32F1) && (defined HAL_USART1_REMAP))
            AFIO->MAPR |= AFIO_MAPR_USART1_REMAP;           // Remap USART1 from PA9/PA10 to PB6/PB7
#endif
            USARTx = USART1;
            UARTx_IRQn = USART1_IRQn;
            RCC->APB2ENR   |= RCC_APB2ENR_USART1EN;                         // Enable UART1 Clock
#if ((defined STM32F0) || (defined STM32F3))
            if(enable & 1) // Enable Rx
            {
                hal_dio_configure(HAL_USART1_PIN_RX, HAL_USART1_AF);
            }
#endif  // STM32F0
            if(enable & 2) // Enable Tx
            {
                hal_dio_configure(HAL_USART1_PIN_TX, HAL_USART1_AF);
            }
            }
            break;
#endif  //  USART1

#if (defined HAL_USE_USART2)
        case HAL_USE_USART2:
            {
            uart_clock = hal_pclk1;
            USARTx = USART2;
            UARTx_IRQn = USART2_IRQn;
            RCC->APB1ENR   |= RCC_APB1ENR_USART2EN;                         // Enable UART2 Clock
#if ((defined STM32F0) || (defined STM32F3))
            if(enable & 1) // Enable Rx
            {
                hal_dio_configure(HAL_USART2_PIN_RX, HAL_USART2_AF);
            }
#endif  // STM32F0
            if(enable & 2) // Enable Tx
            {
                hal_dio_configure(HAL_USART2_PIN_TX, HAL_USART2_AF);
            }
            }
            break;
#endif  //  USART2

#if (defined HAL_USE_USART3)
        case HAL_USE_USART3:
            {
            uart_clock = hal_pclk1;
#if ((defined STM32F1) && (defined HAL_USART3_REMAP))
            AFIO->MAPR |= AFIO_MAPR_USART3_REMAP;       // Remap USART1 from PB10/PB11 to PC10/PC11
#endif
            USARTx = USART3;
            UARTx_IRQn = USART3_IRQn;
            RCC->APB1ENR   |= RCC_APB1ENR_USART3EN;                         // Enable UART3 Clock
#if ((defined STM32F0) || (defined STM32F3))
            if(enable & 1) // Enable Rx
            {
                hal_dio_configure(HAL_USART3_PIN_RX, HAL_USART3_AF);
            }
#endif  // STM32F0
            if(enable & 2) // Enable Tx
            {
                hal_dio_configure(HAL_USART3_PIN_TX, HAL_USART3_AF);
            }
            }
            break;
#endif  //  USART3

        default:
            while(1);
    }

    if(hal_UARTv[port] == NULL)
    {
        hal_UARTv[port] = mqAlloc(sizeof(HAL_UART_t));
    }

    if(USARTx->CR1 & USART_CR1_UE)
    {
        USARTx->CR1 &= ~USART_CR1_UE;
    }
    else
    {
        hal_UARTv[port]->rx_head = 0;
        hal_UARTv[port]->rx_tail = 0;

        hal_UARTv[port]->pTxBuf = NULL;
        hal_UARTv[port]->tx_len = 0;
        hal_UARTv[port]->tx_pos = 0;

        USARTx->CR1 = 0;                                // Disable USART1
        USARTx->CR2 = 0;                                // 8N1
        USARTx->CR3 = 0;                                // Without flow control
        USARTx->BRR  = ((uart_clock + baud/2)/baud);    // Speed
    }

    if(enable & 1) // Enable Rx
    {
        USARTx->CR1 |= USART_CR1_RE |               // Enable RX
                       USART_CR1_RXNEIE;            // Enable RX Not Empty IRQ
    }

    if(enable & 2) // Enable Tx
    {
        USARTx->CR1 |= USART_CR1_TE;                // Enable TX
    }

    NVIC_EnableIRQ(UARTx_IRQn);                     // Enable UASRT IRQ
    USARTx->CR1 |= USART_CR1_UE;                    // Enable USART
}

bool hal_uart_datardy(uint8_t port)
{
    return (hal_UARTv[port]->rx_head != hal_UARTv[port]->rx_tail);
}

uint8_t hal_uart_get(uint8_t port)
{
    if(hal_UARTv[port]->rx_head == hal_UARTv[port]->rx_tail)
    {
        return 0;
    }

    uint8_t data = hal_UARTv[port]->rx_fifo[hal_UARTv[port]->rx_tail];
    hal_UARTv[port]->rx_tail++;
    hal_UARTv[port]->rx_tail &= (uint8_t)(HAL_SIZEOF_UART_RX_FIFO - 1);

    return data;
}

// Tx free
bool hal_uart_free(uint8_t port)
{
    return (hal_UARTv[port]->tx_len == 0);
}

void hal_uart_send(uint8_t port, uint8_t len, uint8_t * pBuf)
{
    hal_UARTv[port]->tx_len = len;
    hal_UARTv[port]->tx_pos = 1;
    hal_UARTv[port]->pTxBuf = pBuf;

    switch(port)
    {
#if (defined HAL_USE_USART1)
        case HAL_USE_USART1:
            USART1->TDR = *pBuf;
            USART1->CR1 |= USART_CR1_TXEIE;
            break;
#endif  //  USART1
#if (defined HAL_USE_USART2)
        case HAL_USE_USART2:
            USART2->TDR = *pBuf;
            USART2->CR1 |= USART_CR1_TXEIE;
            break;
#endif  //  USART2
#if (defined HAL_USE_USART3)
        case HAL_USE_USART3:
            USART3->TDR = *pBuf;
            USART3->CR1 |= USART_CR1_TXEIE;
            break;
#endif  //  USART3
    }
}

#endif  //  (defined HAL_USE_U[S]ARTx)
