#include "../../config.h"

#if ((defined HAL_USE_USART1) || \
     (defined HAL_USE_ALT_USART1) || \
     (defined HAL_USE_USART2) || \
     (defined HAL_USE_USART3) || \
     (defined HAL_USE_UART4))

#define HAL_SIZEOF_UART_RX_FIFO         32      // Should be 2^n

#if (defined __STM32F0XX_H)
    #define HAL_USART_RX_DATA           RDR
    #define HAL_USART_TX_DATA           TDR
#else
    #define HAL_USART_RX_DATA           DR
    #define HAL_USART_TX_DATA           DR
#endif  //  STM32

#if (defined __STM32F0XX_H)
// STM32F051
#if (defined HAL_USE_ALT_USART1)
    #define HAL_USE_USART1          HAL_USE_ALT_USART1
    #define HAL_USART1_ALT_AF       DIO_MODE_AF_PP
#else
    #define HAL_USART1_AF           ((1<<DIO_AF_OFFS) | DIO_MODE_AF_PP)
#endif  //  HAL_USE_ALT_USART1
    #define HAL_USART2_AF           ((1<<DIO_AF_OFFS) | DIO_MODE_AF_PP)
#elif (defined __STM32F10x_H)
    #define HAL_USART1_AF           DIO_MODE_AF_PP
    #define HAL_USART2_AF           DIO_MODE_AF_PP
#elif (defined STM32F4XX)
    #define HAL_USART1_AF           ((7<<DIO_AF_OFFS) | DIO_MODE_AF_PP)
    #define HAL_USART2_AF           ((7<<DIO_AF_OFFS) | DIO_MODE_AF_PP)
    #define HAL_USART3_AF           ((7<<DIO_AF_OFFS) | DIO_MODE_AF_PP)
    #define HAL_UART4_AF            ((8<<DIO_AF_OFFS) | DIO_MODE_AF_PP)
#else
    #error unknown uC Family
#endif  // uC

/*
            Config 1        Config 2
U[S]ARTx    RX      TX      RX      TX      APB
USART1      PA10    PA9     PB7     PB6     2
USART2      PA3     PA2                     1
USART3      PB11    PB10    PC11    PC10    1
UART4       PA1     PA0     PC11    PC10    1
UART5       PD2     PC12                    1
USART6      PC7     PC6                     2

STM32F0
APB1 clk = SystemCoreClock
APB2 clk = SystemCoreClock

STM32F1
APB1 clk = SystemCoreClock/2
APB2 clk = SystemCoreClock

STM32F4
APB1 clk = SystemCoreClock/4
APB2 clk = SystemCoreClock/2
*/

typedef struct
{
    uint8_t             rx_fifo[HAL_SIZEOF_UART_RX_FIFO];
    volatile uint8_t    rx_head;
    uint8_t             rx_tail;

    uint8_t         *   pTxBuf;
    uint8_t             tx_len;
    uint8_t             tx_pos;
}HAL_UART_t;

static const uint16_t hal_baud_list[] = {2400, 4800, 9600, 19200, 38400};
static HAL_UART_t * hal_UARTv[HAL_UART_NUM_PORTS] = {NULL, };

// IRQ handlers
static void hal_uart_irq_handler(uint8_t port, USART_TypeDef * USARTx)
{
    assert(hal_UARTv[port] != NULL);
    HAL_UART_t * control = hal_UARTv[port];

    uint8_t data;
    uint32_t itstat;
#if (defined __STM32F0XX_H)                      // STM32F0
    itstat = USARTx->ISR;
    if(itstat & USART_ISR_ORE)
    {
        USARTx->ICR = USART_ICR_ORECF;
        return;
    }
#else                                           // STM32F1xx / STM32F4xx
    itstat = USARTx->SR;
    if(itstat & USART_SR_ORE)
    {
        data = USARTx->HAL_USART_RX_DATA;
        return;
    }
#endif  //  STM32

    itstat &= USARTx->CR1;

    // Received data is ready to be read
    if(itstat & USART_CR1_RXNEIE)
    {
        data = USARTx->HAL_USART_RX_DATA;
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

        USARTx->HAL_USART_TX_DATA = control->pTxBuf[control->tx_pos];
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

#if (defined HAL_USE_UART4)
void UART4_IRQHandler(void)
{
    hal_uart_irq_handler(HAL_USE_UART4, UART4);
}
#endif  //  UART4

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
            break;
#endif  //  HAL_USE_ALT_USART1
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
#if (defined HAL_USE_UART4)
        case HAL_USE_UART4:
            *pRx = 1;       // GPIOA PIN1
            *pTx = 0;       // GPIOA PIN0
            break;
#endif  //  UART4
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
            hal_dio_gpio_cfg(GPIOB, GPIO_Pin_6 | GPIO_Pin_7, DIO_MODE_IN_FLOAT);
#else
            hal_dio_gpio_cfg(GPIOA, GPIO_Pin_9 | GPIO_Pin_10, DIO_MODE_IN_FLOAT);
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
            
            hal_dio_gpio_cfg(GPIOA, GPIO_Pin_2 | GPIO_Pin_3, DIO_MODE_IN_FLOAT);
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
            
            hal_dio_gpio_cfg(GPIOB, GPIO_Pin_10 | GPIO_Pin_11, DIO_MODE_IN_FLOAT);
            }
            break;
#endif  //  USART3
#if (defined HAL_USE_UART4)
        case HAL_USE_UART4:
            {
            UART4->CR1 &= ~USART_CR1_UE;                // Disable USART
            NVIC_DisableIRQ(UART4_IRQn);                // Disable USART IRQ

            RCC->APB1RSTR |= RCC_APB1RSTR_UART4RST;     // Reset USART
            RCC->APB1RSTR &= ~RCC_APB1RSTR_UART4RST;

            RCC->APB1ENR   &= ~RCC_APB1ENR_UART4EN;     // Disable UART2 Clock
            
            hal_dio_gpio_cfg(GPIOA, GPIO_Pin_0 | GPIO_Pin_1, DIO_MODE_IN_FLOAT);
            }
            break;
#endif  //  UART4
        default:
            assert(0);
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
    assert(nBaud < 5)

    USART_TypeDef     * USARTx;
    IRQn_Type           UARTx_IRQn;
    RCC_ClocksTypeDef   RCC_ClocksStatus;
    
    uint32_t            uart_clock;
    uint16_t            baud = hal_baud_list[nBaud];

    RCC_GetClocksFreq(&RCC_ClocksStatus);

#if (defined HAL_USE_USART1)
    if(port == HAL_USE_USART1)
    {
#if (defined __STM32F0XX_H)
        uart_clock = RCC_ClocksStatus.USART1CLK_Frequency;
#else
        uart_clock = RCC_ClocksStatus.PCLK2_Frequency;
#endif
    }
    else                            // USART2/3 UART4/5
#endif  //  HAL_USE_USART1
    {
#if (defined __STM32F0XX_H)
        uart_clock = RCC_ClocksStatus.PCLK_Frequency;
#else
        uart_clock = RCC_ClocksStatus.PCLK1_Frequency;
#endif
    }

    switch(port)
    {
#if (defined HAL_USE_USART1)
        case HAL_USE_USART1:
            {
            USARTx = USART1;
            UARTx_IRQn = USART1_IRQn;
            RCC->APB2ENR   |= RCC_APB2ENR_USART1EN;                             // Enable UART1 Clock
#ifndef __STM32F10x_H
            if(enable & 1) // Enable Rx
#if (defined HAL_USE_ALT_USART1)
                hal_dio_gpio_cfg(GPIOB, GPIO_Pin_7, HAL_USART1_ALT_AF);         // PB7(Rx)
#else
                hal_dio_gpio_cfg(GPIOA, GPIO_Pin_10, HAL_USART1_AF);            // PA10(Rx)
#endif  //  HAL_USE_ALT_USART1
#endif  //  __STM32F10x_H
            if(enable & 2) // Enable Tx
#if (defined HAL_USE_ALT_USART1)
                hal_dio_gpio_cfg(GPIOB, GPIO_Pin_6, HAL_USART1_ALT_AF);         // PB6(Tx)
#else
                hal_dio_gpio_cfg(GPIOA, GPIO_Pin_9, HAL_USART1_AF);             // PA9(Tx)
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
            
#ifndef __STM32F10x_H
            if(enable & 1) // Enable Rx
                hal_dio_gpio_cfg(GPIOA, GPIO_Pin_3, HAL_USART2_AF);             // PA3(Rx)
#endif  //  __STM32F10x_H
            if(enable & 2) // Enable Tx
                hal_dio_gpio_cfg(GPIOA, GPIO_Pin_2, HAL_USART2_AF);             // PA2(Tx) - AF1
            }
            break;
#endif  //  USART2

#if (defined HAL_USE_USART3)
        case HAL_USE_USART3:
            {
            USARTx = USART3;
            UARTx_IRQn = USART3_IRQn;
            RCC->APB1ENR   |= RCC_APB1ENR_USART3EN;                             // Enable UART3 Clock
            
#ifndef __STM32F10x_H
            if(enable & 1) // Enable Rx
                hal_dio_gpio_cfg(GPIOB, GPIO_Pin_11, HAL_USART3_AF);            // PB11(Rx)
#endif  //  __STM32F10x_H
            if(enable & 2) // Enable Tx
                hal_dio_gpio_cfg(GPIOB, GPIO_Pin_10, HAL_USART3_AF);            // PB10(Tx)
            }
            break;
#endif  //  USART3

#if (defined HAL_USE_UART4)
        case HAL_USE_UART4:
            {
            USARTx = UART4;
            UARTx_IRQn = UART4_IRQn;
            RCC->APB1ENR   |= RCC_APB1ENR_UART4EN;                              // Enable UART4 Clock

            if(enable & 1) // Enable Rx
                hal_dio_gpio_cfg(GPIOA, GPIO_Pin_1, HAL_UART4_AF);              // PA1(Rx)
            if(enable & 2) // Enable Tx
                hal_dio_gpio_cfg(GPIOA, GPIO_Pin_0, HAL_UART4_AF);              // PA0(Tx)
            }
            break;
#endif  //  UART4

        default:
            assert(0);
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
            USART1->HAL_USART_TX_DATA = *pBuf;
            USART1->CR1 |= USART_CR1_TXEIE;
            break;
#endif  //  USART1
#if (defined HAL_USE_USART2)
        case HAL_USE_USART2:
            USART2->HAL_USART_TX_DATA = *pBuf;
            USART2->CR1 |= USART_CR1_TXEIE;
            break;
#endif  //  USART2
#if (defined HAL_USE_USART3)
        case HAL_USE_USART3:
            USART3->HAL_USART_TX_DATA = *pBuf;
            USART3->CR1 |= USART_CR1_TXEIE;
            break;
#endif  //  USART3
#if (defined HAL_USE_UART4)
        case HAL_USE_UART4:
            UART4->HAL_USART_TX_DATA = *pBuf;
            UART4->CR1 |= USART_CR1_TXEIE;
            break;
#endif  //  UART4
    }
}

#endif  //  (defined HAL_USE_U[S]ARTx)
