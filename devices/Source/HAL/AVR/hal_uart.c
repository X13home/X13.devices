#include "../../config.h"

#if ((defined HAL_USE_USART0) || (defined HAL_USE_USART1))

#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#define HAL_SIZEOF_UART_RX_FIFO         32      // Should be 2^n

#if (defined __AVR_ATmega328P__)
    #define UART0_PORT                  PORTD
    #define UART0_DDR                   DDRD
    #define UART0_RX_PIN                PD0
    #define UART0_TX_PIN                PD1
#elif (defined __AVR_ATmega1284P__) || (defined __AVR_ATmega164P__) || (defined __AVR_ATmega164PA__)
    #define UART0_PORT                  PORTD
    #define UART0_DDR                   DDRD
    #define UART0_RX_PIN                PD0
    #define UART0_TX_PIN                PD1

    #define UART1_PORT                  PORTD
    #define UART1_DDR                   DDRD
    #define UART1_RX_PIN                PD2
    #define UART1_TX_PIN                PD3
#elif defined (__AVR_ATmega2560__)
    #define UART0_PORT                  PORTE
    #define UART0_DDR                   DDRE
    #define UART0_RX_PIN                PE0
    #define UART0_TX_PIN                PE1

    #define UART1_PORT                  PORTD
    #define UART1_DDR                   DDRD
    #define UART1_RX_PIN                PD2
    #define UART1_TX_PIN                PD3

    #define UART2_PORT                  PORTH
    #define UART2_DDR                   DDRH
    #define UART2_RX_PIN                PH0
    #define UART2_TX_PIN                PH1

    #define UART3_PORT                  PORTJ
    #define UART3_DDR                   DDRJ
    #define UART3_RX_PIN                PJ0
    #define UART3_TX_PIN                PJ1
#else
    #error unknown uC
#endif  //  uC

typedef struct
{
    uint8_t             rx_fifo[HAL_SIZEOF_UART_RX_FIFO];
    volatile uint8_t    rx_head;
    uint8_t             rx_tail;
    
    uint8_t         *   pTxBuf;
    uint8_t             tx_len;
    uint8_t             tx_pos;
}HAL_UART_t;

static const PROGMEM uint16_t hal_baud_list[] = 
                                        {((F_CPU/16/2400) - 1),
                                         ((F_CPU/16/4800) - 1),
                                         ((F_CPU/16/9600) - 1),
                                         ((F_CPU/16/19200) - 1),
                                         ((F_CPU/16/38400) - 1)};

static HAL_UART_t * hal_UARTv[HAL_UART_NUM_PORTS] = {NULL,};

#if (defined HAL_USE_USART0)

#ifndef USART0_RX_vect
#define USART0_RX_vect              USART_RX_vect
#endif  //  USART_RX_vect

#ifndef USART0_UDRE_vect
#define USART0_UDRE_vect            USART_UDRE_vect
#endif  //  USART_UDRE_vect

ISR(USART0_RX_vect)
{
    uint8_t data = UDR0;
    HAL_UART_t * pVar = hal_UARTv[HAL_USE_USART0];

    uint8_t tmp_head = (pVar->rx_head + 1) & (uint8_t)(HAL_SIZEOF_UART_RX_FIFO - 1);
    if(tmp_head == pVar->rx_tail)        // Overflow
        return;

    pVar->rx_fifo[pVar->rx_head] = data;
    pVar->rx_head = tmp_head;
}

ISR(USART0_UDRE_vect)
{
    HAL_UART_t * pVar = hal_UARTv[HAL_USE_USART0];
    
    if(pVar->tx_len == pVar->tx_pos)
    {
        pVar->tx_len = 0;
        UCSR0B &= ~(1<<UDRIE0);
        return;
    }

    UDR0 = pVar->pTxBuf[pVar->tx_pos++];
}
#endif  //  HAL_USE_USART0

#if (defined HAL_USE_USART1)
ISR(USART1_RX_vect)
{
    uint8_t data = UDR1;
    HAL_UART_t * pVar = hal_UARTv[HAL_USE_USART1];

    uint8_t tmp_head = (pVar->rx_head + 1) & (uint8_t)(HAL_SIZEOF_UART_RX_FIFO - 1);
    if(tmp_head == pVar->rx_tail)        // Overflow
        return;

    pVar->rx_fifo[pVar->rx_head] = data;
    pVar->rx_head = tmp_head;
}

ISR(USART1_UDRE_vect)
{
    HAL_UART_t * pVar = hal_UARTv[HAL_USE_USART1];
    
    if(pVar->tx_len == pVar->tx_pos)
    {
        pVar->tx_len = 0;
        UCSR1B &= ~(1<<UDRIE1);
        return;
    }

    UDR1 = pVar->pTxBuf[pVar->tx_pos++];
}
#endif  //  HAL_USE_USART1

/*
#if (defined UCSR2A) && (defined UART2_PORT)
ISR(USART2_RX_vect)
{
    uint8_t data = UDR2;
    uint8_t tmp_head = (hal_UARTv[2]->rx_head + 1) & (uint8_t)(HAL_SIZEOF_UART_RX_FIFO - 1);
    if(tmp_head == hal_UARTv[2]->rx_tail)        // Overflow
        return;

    hal_UARTv[2]->rx_fifo[hal_UARTv[2]->rx_head] = data;
    hal_UARTv[2]->rx_head = tmp_head;
}

ISR(USART2_UDRE_vect)
{
    if(hal_UARTv[2]->tx_len == hal_UARTv[2]->tx_pos)
    {
        hal_UARTv[2]->tx_len = 0;
        UCSR2B &= ~(1<<UDRIE2);
        return;
    }

    UDR2 = hal_UARTv[2]->pTxBuf[hal_UARTv[2]->tx_pos++];
}
#endif  //  UCSR2A

#if (defined UCSR3A) && (defined UART3_PORT)
ISR(USART3_RX_vect)
{
    uint8_t data = UDR3;
    uint8_t tmp_head = (hal_UARTv[3]->rx_head + 1) & (uint8_t)(HAL_SIZEOF_UART_RX_FIFO - 1);
    if(tmp_head == hal_UARTv[3]->rx_tail)        // Overflow
        return;

    hal_UARTv[3]->rx_fifo[hal_UARTv[3]->rx_head] = data;
    hal_UARTv[3]->rx_head = tmp_head;
}

ISR(USART3_UDRE_vect)
{
    if(hal_UARTv[3]->tx_len == hal_UARTv[3]->tx_pos)
    {
        hal_UARTv[3]->tx_len = 0;
        UCSR3B &= ~(1<<UDRIE3);
        return;
    }

    UDR3 = hal_UARTv[3]->pTxBuf[hal_UARTv[3]->tx_pos++];
}
#endif  //  UCSR3A
*/

// HAL API
void hal_uart_get_pins(uint8_t port, uint8_t * pRx, uint8_t * pTx)
{
    switch(port)
    {
#ifdef HAL_USE_USART0
        case HAL_USE_USART0:
            *pRx = UART0_RX_PIN;
            *pTx = UART0_TX_PIN;
            break;
#endif  //  HAL_USE_USART0
#ifdef HAL_USE_USART1
        case HAL_USE_USART1:
            *pRx = UART1_RX_PIN;
            *pTx = UART1_TX_PIN;
            break;
#endif  //  HAL_USE_USART1
/*
#if (defined UART2_PORT)
        case 2:
            *pRx = UART2_RX_PIN;
            *pTx = UART2_TX_PIN;
            break;
#endif  //  UART2_PORT
#if (defined UART3_PORT)
        case 3:
            *pRx = UART3_RX_PIN;
            *pTx = UART3_TX_PIN;
            break;
#endif  //  UART0_PORT
*/
        default:
            *pRx = 0xFF;
            *pTx = 0xFF;
            break;
    }
}

void hal_uart_deinit(uint8_t port)
{
    switch(port)
    {
#ifdef HAL_USE_USART0
        case HAL_USE_USART0:
            {
            UCSR0B = 0;
            UART0_PORT &= ~((1<<UART0_RX_PIN) | (1<<UART0_TX_PIN));
            UART0_DDR &= ~(1<<UART0_TX_PIN);
            }
            break;
#endif  //  HAL_USE_USART0
#ifdef HAL_USE_USART1
        case HAL_USE_USART1:
            {
            UCSR1B = 0;
            UART1_PORT &= ~((1<<UART1_RX_PIN) | (1<<UART1_TX_PIN));
            UART1_DDR &= ~(1<<UART1_TX_PIN);
            }
            break;
#endif  //  HAL_USE_USART1
/*
#if (defined UCSR2A) && (defined UART2_PORT)
        case 2:
            {
            UCSR2B = 0;
            UART2_PORT &= ~((1<<UART2_RX_PIN) | (1<<UART2_TX_PIN));
            UART2_DDR &= ~(1<<UART2_TX_PIN);
            }
            break;
#endif  //  (defined UCSR2A) && (defined UART2_PORT)
#if (defined UCSR3A) && (defined UART3_PORT)
        case 3:
            {
            UCSR3B = 0;
            UART3_PORT &= ~((1<<UART3_RX_PIN) | (1<<UART3_TX_PIN));
            UART3_DDR &= ~(1<<UART3_TX_PIN);
            }
            break;
#endif  //  (defined UCSR3A) && (defined UART3_PORT)
*/
        default:
            assert(0);
    }

    if(hal_UARTv[port] != NULL)
    {
        mqFree(hal_UARTv[port]);
        hal_UARTv[port] = NULL;
    }
}

void hal_uart_init_hw(uint8_t port, uint8_t nBaud, uint8_t enable)
{
    volatile uint8_t * pPort;

    switch(port)
    {
#ifdef HAL_USE_USART0
        case HAL_USE_USART0:
            pPort = &UCSR0A;

            if(enable & 1)  // Rx
            {
                UART0_PORT |= (1<<UART0_RX_PIN);
                UART0_DDR &= ~(1<<UART0_RX_PIN);
            }
            
            if(enable & 2)  // Tx
            {
                UART0_PORT |= (1<<UART0_TX_PIN);
                UART0_DDR |= (1<<UART0_TX_PIN);
            }
            break;
#endif  //  HAL_USE_USART0
#if (defined HAL_USE_USART1)
        case HAL_USE_USART1:
            pPort = &UCSR1A;

            if(enable & 1)  // Rx
            {
                UART1_PORT |= (1<<UART1_RX_PIN);
                UART1_DDR &= ~(1<<UART1_RX_PIN);
            }
            
            if(enable & 2)  // Tx
            {
                UART1_PORT |= (1<<UART1_TX_PIN);
                UART1_DDR |= (1<<UART1_TX_PIN);
            }
            break;
#endif  //  HAL_USE_USART0
/*
#if (defined UCSR2A) && (defined UART2_PORT)
        case 2:
            UCSR2B = 0;
            if(enable & 1)  // Rx
            {
                UART2_PORT |= (1<<UART2_RX_PIN);
                UART2_DDR &= ~(1<<UART2_RX_PIN);
            }

            if(enable & 2)  // Tx
            {
                UART2_PORT |= (1<<UART2_TX_PIN);
                UART2_DDR |= (1<<UART2_TX_PIN);
            }
            break;
#endif  //  (defined UCSR2A) && (defined UART2_PORT)
#if (defined UCSR3A) && (defined UART3_PORT)
        case 3:
            UCSR3B = 0;
            if(enable & 1)  // Rx
            {
                UART3_PORT |= (1<<UART3_RX_PIN);
                UART3_DDR &= ~(1<<UART3_RX_PIN);
            }
            
            if(enable & 2)  // Tx
            {
                UART3_PORT |= (1<<UART3_TX_PIN);
                UART3_DDR |= (1<<UART3_TX_PIN);
            }
            break;
#endif  //  (defined UCSR3A) && (defined UART3_PORT)
*/
        default:
            assert(0);
    }

    if(hal_UARTv[port] == NULL)
    {
        hal_UARTv[port] = mqAlloc(sizeof(HAL_UART_t));

        hal_UARTv[port]->rx_head = 0;
        hal_UARTv[port]->rx_tail = 0;
        hal_UARTv[port]->tx_len = 0;
        hal_UARTv[port]->tx_pos = 0;

        uint16_t baud = pgm_read_word(&hal_baud_list[nBaud]);

        *(pPort + 5) = (baud >> 8);                     // UBRRH
        *(pPort + 4) = (baud & 0xFF);                   // UBRRL
        *(pPort + 1) = 0;                               // UCSRB
        *(pPort + 2) = (3<<UCSZ00);                     // UCSRC
    }
    
    if(enable & 1)  // Rx
        *(pPort + 1) |= ((1<<RXCIE0) | (1<<RXEN0));     // UCSRB
    if(enable & 2)  // Tx
        *(pPort + 1) |= (1<<TXEN0);                     // UCSRB
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
#ifdef HAL_USE_USART0
        case HAL_USE_USART0:
            UDR0 = *pBuf;
            UCSR0B |= (1<<UDRIE0);
            break;
#endif  //  HAL_USE_USART0
#ifdef HAL_USE_USART1
        case HAL_USE_USART1:
            UDR1 = *pBuf;
            UCSR1B |= (1<<UDRIE1);
            break;
#endif  //  HAL_USE_USART1
        default:
            assert(0);
    }
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

#endif  //  ((defined UART_PHY) || (defined EXTSER_USED))
