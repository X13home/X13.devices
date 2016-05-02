#include "config.h"

#if (defined EXTTWI_USED) && (defined STM32F1)

#include "EXT/exttwi.h"

// Global variable defined in exttwi.c
extern volatile TWI_QUEUE_t * pTwi_exchange;
static volatile uint8_t twi_pnt = 0;

#if (EXTTWI_USED == 1)
// I2C1, PB6 - SCL, PB7 - SDA
#define I2C_BUS         I2C1
#define I2C_IRQn        I2C1_EV_IRQn
#define I2C_PIN_SCL     GPIO_Pin_6
#define I2C_PIN_SDA     GPIO_Pin_7

#define I2C_DIO_SCL     22
#define I2C_DIO_SDA     23

#else
// I2C2, PB10 - SCL, PB11 - SDA
#define I2C_BUS         I2C2
#define I2C_IRQn        I2C2_EV_IRQn
#define I2C_PIN_SCL     GPIO_Pin_10
#define I2C_PIN_SDA     GPIO_Pin_11

#define I2C_DIO_SCL     26
#define I2C_DIO_SDA     27

#endif

void hal_twi_get_pins(uint8_t * pSCL, uint8_t * pSDA)
{
    *pSCL = I2C_DIO_SCL;
    *pSDA = I2C_DIO_SDA;
}

bool hal_twi_configure(uint8_t enable)
{
    if(enable)
    {
        // Check GPIO
        hal_gpio_cfg(GPIOB, (I2C_PIN_SCL | I2C_PIN_SDA), DIO_MODE_IN_PD);               // Configure GPIO as inputs with pull down
        if((GPIOB->IDR & (I2C_PIN_SCL | I2C_PIN_SDA)) != (I2C_PIN_SCL | I2C_PIN_SDA))
            return false;

        hal_gpio_cfg(GPIOB, (I2C_PIN_SCL | I2C_PIN_SDA), DIO_MODE_TWI);                 // Configure GPIO

#if (EXTTWI_USED == 1)
        RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;     // Enable I2C clock
        // Reset I2C1
        RCC->APB1RSTR |= RCC_APB1RSTR_I2C1RST;
        RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C1RST;
#else
        RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;     // Enable I2C clock
        // Reset I2C2
        RCC->APB1RSTR |= RCC_APB1RSTR_I2C2RST;
        RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C2RST;
#endif

#warning I2C not configured

        NVIC_SetPriority(I2C_IRQn, 3);
        NVIC_EnableIRQ(I2C_IRQn);
    }
    else
    {
        NVIC_DisableIRQ(I2C_IRQn);
#if (EXTTWI_USED == 1)
        // Reset I2C1
        RCC->APB1RSTR |= RCC_APB1RSTR_I2C1RST;
        RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C1RST;
        // Disable clock
        RCC->APB1ENR &= ~RCC_APB1ENR_I2C1EN;
#else
        // Reset I2C2
        RCC->APB1RSTR |= RCC_APB1RSTR_I2C2RST;
        RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C2RST;
        // Disable clock
        RCC->APB1ENR &= ~RCC_APB1ENR_I2C2EN;    
#endif
        hal_gpio_cfg(GPIOB, (I2C_PIN_SCL | I2C_PIN_SDA), DIO_MODE_IN_FLOAT);            // Release GPIO
    }

    return true;
}

/*
#define DIO_MODE_TWI    ((1<<DIO_AF_OFFS) | DIO_MODE_AF_OD)       // Alternative function, AF = 1, Open Drain



bool hal_twi_configure(uint8_t enable)
{
#if (EXTTWI_USED == 1)
        RCC->CFGR3 |= RCC_CFGR3_I2C1SW;         // Use SysClk for I2C CLK
#else
#endif
        // Set Timings, RM0091, page 542
        // I2C Clock = 48MHz
        // I2C Bus = 100 KHz
        I2C_BUS->TIMINGR |= 0xB0420F13;

        // Enable I2C
        I2C_BUS->CR1 = I2C_CR1_PE;


}

void hal_twi_stop(void)
{
    I2C_BUS->CR1 &= ~I2C_CR1_PE;       // Disable I2C
}

void hal_twi_start(void)
{
    if((I2C_BUS->CR1 & I2C_CR1_PE) == 0)
    {
        I2C_BUS->CR1 = I2C_CR1_PE;                              // Enable I2C
        return;
    }

    uint32_t isr = I2C_BUS->ISR;
    if(isr & I2C_ISR_BUSY)                                      // Bus Busy
    {
        return;
    }

    if(isr & (I2C_ISR_OVR | I2C_ISR_ARLO | I2C_ISR_BERR))       // Bus Error
    {
        I2C_BUS->CR1 &= ~I2C_CR1_PE;                            // Disable I2C
        pTwi_exchange->frame.access |= TWI_ERROR;
        return;
    }

    I2C_BUS->ICR |= I2C_ICR_STOPCF | I2C_ICR_NACKCF;            // Clear STOP & NACK flags.
    I2C_BUS->CR1 |= (I2C_CR1_ERRIE |                            // Enable Interrupts on: Erros,
                  I2C_CR1_TCIE |                                //      Transfer complete,
                  I2C_CR1_STOPIE |                              //      Stop Detection,
                  I2C_CR1_NACKIE);                              //      NACK received

    twi_pnt = 0;
    
    uint8_t access = pTwi_exchange->frame.access;

    if(access & TWI_WRITE)
    {
        I2C_BUS->CR2 = (uint32_t)(pTwi_exchange->frame.address << 1) |  // Slave address
                    ((uint32_t)(pTwi_exchange->frame.write) << 16);     // Bytes to send

        if((access & TWI_READ) == 0)
            I2C_BUS->CR2 |= I2C_CR2_AUTOEND;

        if(pTwi_exchange->frame.write > 0)
            I2C_BUS->TXDR = pTwi_exchange->frame.data[twi_pnt++];

        I2C_BUS->CR1 |= I2C_CR1_TXIE;                                   // Interrupt on Tx Buffer empty
    }
    else
    {
        I2C_BUS->CR2 = ((uint32_t)(pTwi_exchange->frame.address << 1) | // Slave address
                    ((uint32_t)(pTwi_exchange->frame.read) << 16) |     // Bytes to read
                    I2C_CR2_RD_WRN | I2C_CR2_AUTOEND);                  // Read request with stop

        I2C_BUS->CR1 |= I2C_CR1_RXIE;
    }

    // Send Start & Address
    I2C_BUS->CR2 |= I2C_CR2_START;

    pTwi_exchange->frame.access |= TWI_BUSY;
}

#if (EXTTWI_USED == 1)
void I2C1_IRQHandler(void)
#else   // 
void I2C2_IRQHandler(void)
#endif
{
    uint32_t isr = I2C_BUS->ISR;

    if(isr & (I2C_ISR_OVR | I2C_ISR_ARLO | I2C_ISR_BERR))   // Bus Error
    {
        I2C_BUS->CR1 &= ~I2C_CR1_PE;       // Disable I2C
        pTwi_exchange->frame.access |= TWI_ERROR;

    }
    else if(isr & I2C_ISR_TC)                                           // Transfer complete
    {
        I2C_BUS->CR1 &= ~I2C_CR1_TXIE;                                     // Disable Tx Interrupt
        I2C_BUS->CR2 = ((uint32_t)(pTwi_exchange->frame.address << 1) |    // Slave address
                     ((uint32_t)(pTwi_exchange->frame.read) << 16) |    // Bytes to read
                      I2C_CR2_RD_WRN | I2C_CR2_AUTOEND);                // Read request with stop

        I2C_BUS->CR1 |= I2C_CR1_RXIE;                                      // Enable Rx interrupt

        // Send Repeat Start & Address
        I2C_BUS->CR2 |= I2C_CR2_START;
    }
    else if(isr & I2C_ISR_STOPF)                                    // Stop received
    {
        I2C_BUS->ICR |= I2C_ICR_STOPCF;                                // Clear Stop Flag

        if(pTwi_exchange->frame.access & TWI_WRITE)
        {
            pTwi_exchange->frame.access &= ~TWI_WRITE;
            
            if(I2C_BUS->CR1 & I2C_CR1_TXIE)                            // Last Stop on Tx
            {
                I2C_BUS->CR1 = I2C_CR1_PE;                             // Disable Interrupts
                pTwi_exchange->frame.access &= ~TWI_BUSY;
                pTwi_exchange->frame.access |= TWI_RDY;             // Transaction complete
            }
            pTwi_exchange->frame.write = twi_pnt;
            twi_pnt = 0;
        }
        else                                                    // Stop on Rx
        {
            I2C_BUS->CR1 = I2C_CR1_PE;                             // Disable Interrupts
            pTwi_exchange->frame.data[twi_pnt] = I2C_BUS->RXDR;
            pTwi_exchange->frame.access &= ~(TWI_READ | TWI_BUSY);
            pTwi_exchange->frame.access |= TWI_RDY;             // Transaction complete
            pTwi_exchange->frame.read = twi_pnt;
        }
    }
    else if(isr & I2C_ISR_NACKF)                            // NACK received
    {
        I2C_BUS->ICR |= I2C_ICR_NACKCF;                        // Clear NACK Flag
        I2C_BUS->CR1 = I2C_CR1_PE;                             // Disable Interrupts

        pTwi_exchange->frame.access &= ~(TWI_WRITE | TWI_READ);
        pTwi_exchange->frame.write = 0;
        pTwi_exchange->frame.read = 0;

        pTwi_exchange->frame.access |= TWI_SLANACK;
    }
    else if((I2C_BUS->CR1 & I2C_CR1_TXIE) && (isr & I2C_ISR_TXIS)) // Transmit buffer empty
    {
        I2C_BUS->TXDR = pTwi_exchange->frame.data[twi_pnt++];
    }
    else if((I2C_BUS->CR1 & I2C_CR1_RXIE) && (isr & I2C_ISR_RXNE))     // Data received
    {
         pTwi_exchange->frame.data[twi_pnt++] = I2C_BUS->RXDR;
    }
    else                                                            // Unknown state
    {
        I2C_BUS->CR1 &= ~I2C_CR1_PE;                                   // Disable I2C
    }
}
*/
#endif  //  EXTTWI_USED
