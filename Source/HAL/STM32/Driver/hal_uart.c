#include "config.h"

#if ((defined HAL_USE_USART1)       || \
     (defined HAL_USE_ALT_USART1)   || \
     (defined HAL_USE_USART2)       || \
     (defined HAL_USE_USART3))

#define HAL_SIZEOF_UART_RX_FIFO     32      // Should be 2^n

#if (defined STM32F0)
// STM32F0
#if (defined HAL_USE_ALT_USART1)
    #define HAL_USE_USART1          HAL_USE_ALT_USART1
    #define HAL_USART1_ALT_AF       DIO_MODE_AF_PP
#else
    #define HAL_USART1_AF           ((1<<DIO_AF_OFFS) | DIO_MODE_AF_PP)
#endif  //  HAL_USE_ALT_USART1
    #define HAL_USART2_AF           ((1<<DIO_AF_OFFS) | DIO_MODE_AF_PP)

#elif (defined STM32F1)
// STM32F1
#define RDR     DR
#define TDR     DR

#define HAL_USART1_AF           DIO_MODE_AF_PP
#define HAL_USART2_AF           DIO_MODE_AF_PP
#define HAL_USART3_AF           DIO_MODE_AF_PP

#elif (defined STM32F3)
// STM32F303/STM32F334
#define HAL_USART1_AF           ((7<<DIO_AF_OFFS) | DIO_MODE_AF_PP)
#define HAL_USART2_AF           ((7<<DIO_AF_OFFS) | DIO_MODE_AF_PP)

#else
#error hal_uart.c  Unknown uC Family
#endif

/*          Config 1        Config 2
U[S]ARTx    RX      TX      RX      TX      APB
USART1      PA10    PA9     PB7     PB6     2
USART2      PA3     PA2                     1
USART3      PB11    PB10    PC11    PC10    1

// STM32F0
APB1 clk = SystemCoreClock
APB2 clk = SystemCoreClock

// STM32F1
APB1 clk = SystemCoreClock/2
APB2 clk = SystemCoreClock

// STM32F303
USART1, USART2 clk = SystemCoreClock/2
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
#if (defined HAL_USE_USART3)
        case HAL_USE_USART3:
            *pRx = 27;      // GPIOB PIN11
            *pTx = 26;      // GPIOB PIN10
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
#if (defined HAL_USE_USART3)
        case HAL_USE_USART3:
            {
            USART3->CR1 &= ~USART_CR1_UE;               // Disable USART
            NVIC_DisableIRQ(USART3_IRQn);               // Disable USART IRQ

            RCC->APB1RSTR |= RCC_APB1RSTR_USART3RST;    // Reset USART
            RCC->APB1RSTR &= ~RCC_APB1RSTR_USART3RST;

            RCC->APB1ENR   &= ~RCC_APB1ENR_USART3EN;    // Disable UART2 Clock
            
            hal_gpio_cfg(GPIOB, GPIO_Pin_10 | GPIO_Pin_11, DIO_MODE_IN_FLOAT);
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

    uint32_t            uart_clock = SystemCoreClock;
    uint16_t            baud = hal_baud_list[nBaud];

    switch(port)
    {
#if (defined HAL_USE_USART1)
        case HAL_USE_USART1:
            {
#if (defined STM32F3)
            uart_clock  /= 2;
#endif
#if ((defined STM32F1) && (defined HAL_USE_ALT_USART1))
            AFIO->MAPR |= AFIO_MAPR_USART1_REMAP;                           // Remap USART1 from PA9/PA10 to PB6/PB7
#endif
            USARTx = USART1;
            UARTx_IRQn = USART1_IRQn;
            RCC->APB2ENR   |= RCC_APB2ENR_USART1EN;                         // Enable UART1 Clock
#if ((defined STM32F0) || (defined STM32F3))
            if(enable & 1) // Enable Rx
            {
#if (defined HAL_USE_ALT_USART1)
                hal_gpio_cfg(GPIOB, GPIO_Pin_7, HAL_USART1_ALT_AF);         // PB7(Rx)
#else
                hal_gpio_cfg(GPIOA, GPIO_Pin_10, HAL_USART1_AF);            // PA10(Rx)
#endif  //  HAL_USE_ALT_USART1
            }
#endif  // STM32F0
            if(enable & 2) // Enable Tx
            {
#if (defined HAL_USE_ALT_USART1)
                hal_gpio_cfg(GPIOB, GPIO_Pin_6, HAL_USART1_ALT_AF);         // PB6(Tx)
#else
                hal_gpio_cfg(GPIOA, GPIO_Pin_9, HAL_USART1_AF);             // PA9(Tx)
#endif  //  HAL_USE_ALT_USART1
            }
            }
            break;
#endif  //  USART1

#if (defined HAL_USE_USART2)
        case HAL_USE_USART2:
            {
#if ((defined STM32F1) || (defined STM32F3))
            uart_clock  /= 2;
#endif  //  STM32F1
            USARTx = USART2;
            UARTx_IRQn = USART2_IRQn;
            RCC->APB1ENR   |= RCC_APB1ENR_USART2EN;                         // Enable UART2 Clock
#if ((defined STM32F0) || (defined STM32F3))
            if(enable & 1) // Enable Rx
            {
                hal_gpio_cfg(GPIOA, GPIO_Pin_3, HAL_USART2_AF);             // PA3(Rx)
            }
#endif  // STM32F0
            if(enable & 2) // Enable Tx
            {
                hal_gpio_cfg(GPIOA, GPIO_Pin_2, HAL_USART2_AF);             // PA2(Tx) - AF1
            }
            }
            break;
#endif  //  USART2

#if (defined HAL_USE_USART3)
        case HAL_USE_USART3:
            {
#if ((defined STM32F1) || (defined STM32F3))
            uart_clock  /= 2;
#endif  //  STM32F1
            USARTx = USART3;
            UARTx_IRQn = USART3_IRQn;
            RCC->APB1ENR   |= RCC_APB1ENR_USART3EN;                         // Enable UART3 Clock
            
#if ((defined STM32F0) || (defined STM32F3))
            if(enable & 1) // Enable Rx
            {
                hal_gpio_cfg(GPIOB, GPIO_Pin_11, HAL_USART3_AF);            // PB11(Rx)
            }
#endif  // STM32F0
            if(enable & 2) // Enable Tx
            {
                hal_gpio_cfg(GPIOB, GPIO_Pin_10, HAL_USART3_AF);            // PB10(Tx)
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
#if (defined HAL_USE_USART3)
        case HAL_USE_USART3:
            USART3->TDR = *pBuf;
            USART3->CR1 |= USART_CR1_TXEIE;
            break;
#endif  //  USART3
    }
}

#endif  //  (defined HAL_USE_U[S]ARTx)
