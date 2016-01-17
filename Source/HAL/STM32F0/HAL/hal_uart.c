#include "config.h"

#if ((defined HAL_USE_USART1) || \
     (defined HAL_USE_ALT_USART1) || \
     (defined HAL_USE_USART2))

#define HAL_SIZEOF_UART_RX_FIFO     32      // Should be 2^n

#if (defined HAL_USE_ALT_USART1)
    #define HAL_USE_USART1          HAL_USE_ALT_USART1
    #define HAL_USART1_ALT_AF       DIO_MODE_AF_PP
#else
    #define HAL_USART1_AF           ((1<<DIO_AF_OFFS) | DIO_MODE_AF_PP)
#endif  //  HAL_USE_ALT_USART1
    #define HAL_USART2_AF           ((1<<DIO_AF_OFFS) | DIO_MODE_AF_PP)

/*          Config 1        Config 2
U[S]ARTx    RX      TX      RX      TX      APB
USART1      PA10    PA9     PB7     PB6     2
USART2      PA3     PA2                     1

APB1 clk = SystemCoreClock
APB2 clk = SystemCoreClock
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

extern uint32_t     SystemCoreClock;            // defined in system_stm32f0xx.c

// IRQ handlers
static void hal_uart_irq_handler(uint8_t port, USART_TypeDef * USARTx)
{
    HAL_UART_t * control = hal_UARTv[port];

    uint8_t data;
    uint32_t itstat;
    itstat = USARTx->ISR;
    if(itstat & USART_ISR_ORE)
    {
        USARTx->ICR = USART_ICR_ORECF;
        return;
    }

    itstat &= USARTx->CR1;

    // Received data is ready to be read
    if(itstat & USART_CR1_RXNEIE)
    {
        data = USARTx->RDR;
        uint8_t tmp_head = (control->rx_head + 1) & (uint8_t)(HAL_SIZEOF_UART_RX_FIFO - 1);
        if(tmp_head == control->rx_tail)        // Overflow
            return;
            
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

void hal_uart_get_pins(uint8_t port, uint8_t * pRx, uint8_t * pTx)
{
    switch(port)
    {
#if (defined HAL_USE_USART1)
        case HAL_USE_USART1:
#if (defined HAL_USE_ALT_USART1)
            *pRx = 23;      // GPIOB PIN7
            *pTx = 22;      // GPIOB PIN6
#else
            *pRx = 10;      // GPIOA PIN10
            *pTx = 9;       // GPIOA PIN9
#endif  //  HAL_USE_ALT_USART1
            break;
#endif  //  USART1
#if (defined HAL_USE_USART2)
        case HAL_USE_USART2:
            *pRx = 3;       // GPIOA PIN3
            *pTx = 2;       // GPIOA PIN2
            break;
#endif  //  USART2
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
#if (defined HAL_USE_ALT_USART1)
            hal_gpio_cfg(GPIOB, GPIO_Pin_6 | GPIO_Pin_7, DIO_MODE_IN_FLOAT);
#else
            hal_gpio_cfg(GPIOA, GPIO_Pin_9 | GPIO_Pin_10, DIO_MODE_IN_FLOAT);
#endif  //  HAL_USE_ALT_USART1
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
            
            hal_gpio_cfg(GPIOA, GPIO_Pin_2 | GPIO_Pin_3, DIO_MODE_IN_FLOAT);
            }
            break;
#endif  //  USART2
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

    uint32_t            uart_clock = SystemCoreClock;
    uint16_t            baud = hal_baud_list[nBaud];

    switch(port)
    {
#if (defined HAL_USE_USART1)
        case HAL_USE_USART1:
            {
            USARTx = USART1;
            UARTx_IRQn = USART1_IRQn;
            RCC->APB2ENR   |= RCC_APB2ENR_USART1EN;                             // Enable UART1 Clock

            if(enable & 1) // Enable Rx
#if (defined HAL_USE_ALT_USART1)
                hal_gpio_cfg(GPIOB, GPIO_Pin_7, HAL_USART1_ALT_AF);         // PB7(Rx)
#else
                hal_gpio_cfg(GPIOA, GPIO_Pin_10, HAL_USART1_AF);            // PA10(Rx)
#endif  //  HAL_USE_ALT_USART1

            if(enable & 2) // Enable Tx
#if (defined HAL_USE_ALT_USART1)
                hal_gpio_cfg(GPIOB, GPIO_Pin_6, HAL_USART1_ALT_AF);         // PB6(Tx)
#else
                hal_gpio_cfg(GPIOA, GPIO_Pin_9, HAL_USART1_AF);             // PA9(Tx)
#endif  //  HAL_USE_ALT_USART1
            }
            break;
#endif  //  USART1

#if (defined HAL_USE_USART2)
        case HAL_USE_USART2:
            {
            USARTx = USART2;
            UARTx_IRQn = USART2_IRQn;
            RCC->APB1ENR   |= RCC_APB1ENR_USART2EN;                             // Enable UART2 Clock
            
            if(enable & 1) // Enable Rx
                hal_gpio_cfg(GPIOA, GPIO_Pin_3, HAL_USART2_AF);             // PA3(Rx)

            if(enable & 2) // Enable Tx
                hal_gpio_cfg(GPIOA, GPIO_Pin_2, HAL_USART2_AF);             // PA2(Tx) - AF1
            }
            break;
#endif  //  USART2

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
        USARTx->CR1 |= USART_CR1_RE |               // Enable RX
                       USART_CR1_RXNEIE;            // Enable RX Not Empty IRQ

    if(enable & 2) // Enable Tx
        USARTx->CR1 |= USART_CR1_TE;                // Enable TX
                   
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
        return 0;
    
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
    }
}

#endif  //  (defined HAL_USE_U[S]ARTx)
