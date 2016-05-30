#include "config.h"

#if (defined HAL_TWI_BUS) && (defined STM32F1)

#include "EXT/exttwi.h"

#define DIO_MODE_TWI    DIO_MODE_AF_OD          // Alternative function, Open Drain
#define I2C_GPIO        GPIOB

#if (HAL_TWI_BUS == 1)
    
void I2C1_EV_IRQHandler(void);
void I2C1_ER_IRQHandler(void);

#define I2C_Event_Handler()     I2C1_EV_IRQHandler(void)
#define I2C_Error_Handler()     I2C1_ER_IRQHandler(void)

#define I2C_BUS         I2C1
#define I2C_EV_IRQ      I2C1_EV_IRQn
#define I2C_ER_IRQ      I2C1_ER_IRQn

#if (defined HAL_TWI1_REMAP)
// I2C1, PB8 - SCL, PB9 - SDA
#define I2C_PIN_SCL     GPIO_Pin_8
#define I2C_PIN_SDA     GPIO_Pin_9
#define I2C_DIO_SCL     24
#define I2C_DIO_SDA     25

#else
// I2C1, PB6 - SCL, PB7 - SDA
#define I2C_PIN_SCL     GPIO_Pin_6
#define I2C_PIN_SDA     GPIO_Pin_7
#define I2C_DIO_SCL     22
#define I2C_DIO_SDA     23

#endif  //  HAL_TWI1_REMAP

#else   //  BUS 2

void I2C2_EV_IRQHandler(void);
void I2C2_ER_IRQHandler(void);

#define I2C_Event_Handler()     I2C2_EV_IRQHandler(void)
#define I2C_Error_Handler()     I2C2_ER_IRQHandler(void)

#define I2C_BUS         I2C2
#define I2C_EV_IRQ      I2C2_EV_IRQn
#define I2C_ER_IRQ      I2C2_ER_IRQn

// I2C2, PB10 - SCL, PB11 - SDA
#define I2C_PIN_SCL     GPIO_Pin_10
#define I2C_PIN_SDA     GPIO_Pin_11
#define I2C_DIO_SCL     26
#define I2C_DIO_SDA     27

#endif  //  HAL_TWI_BUS

// Global variable defined in exttwi.c
extern volatile TWI_QUEUE_t * pTWI;
static volatile uint8_t twi_pnt = 0;

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
        hal_gpio_cfg(I2C_GPIO, (I2C_PIN_SCL | I2C_PIN_SDA), DIO_MODE_IN_PD);                // Configure GPIO as inputs with pull down

        uint16_t timeout = 0x8000;
        while(timeout != 0)
        {
            if((I2C_GPIO->IDR & (I2C_PIN_SCL | I2C_PIN_SDA)) != (I2C_PIN_SCL | I2C_PIN_SDA))
            {
                timeout++;
                if(timeout == 0)
                {
                    hal_gpio_cfg(I2C_GPIO, (I2C_PIN_SCL | I2C_PIN_SDA), DIO_MODE_IN_FLOAT);     // Release GPIO
                    return false;
                }
            }
            else
            {
                if(timeout > 0x8000)
                {
                    timeout = 0x8000;
                }
                else
                {
                    timeout--;
                }
            }
        }

        hal_gpio_cfg(I2C_GPIO, (I2C_PIN_SCL | I2C_PIN_SDA), DIO_MODE_TWI);                  // Configure GPIO

#if (HAL_TWI_BUS == 1)
#if (defined HAL_TWI1_REMAP)
        AFIO->MAPR |= AFIO_MAPR_I2C1_REMAP;
#endif  //  HAL_TWI1_REMAP
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
        // Set Timings
        // I2C Bus = 88 KHz, Standard Mode. Don't use 100 kHz, show uC errata.
        uint32_t freqrange = hal_pclk1 / 88000UL;

        I2C_BUS->CR2 = freqrange;
        I2C_BUS->TRISE = freqrange + 1;

        uint32_t tmp = freqrange / 2;
        tmp &= 0x0FFF;
        if(tmp < 4)
        {
            tmp = 4;
        }
        I2C_BUS->CCR = tmp;

        // Own Addr 0x00 - 7bit
        // Dual addressing disable

        // Enable I2C
        I2C_BUS->CR1 = I2C_CR1_PE;

        NVIC_SetPriority(I2C_EV_IRQ, 3);
        NVIC_SetPriority(I2C_ER_IRQ, 3);
        NVIC_EnableIRQ(I2C_EV_IRQ);
        NVIC_EnableIRQ(I2C_ER_IRQ);
    }
    else
    {
        NVIC_DisableIRQ(I2C_ER_IRQ);
        NVIC_DisableIRQ(I2C_EV_IRQ);

#if (HAL_TWI_BUS == 1)
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
        hal_gpio_cfg(I2C_GPIO, (I2C_PIN_SCL | I2C_PIN_SDA), DIO_MODE_IN_FLOAT);            // Release GPIO
    }

    return true;
}

void hal_twi_stop(void)
{
    I2C_BUS->CR1 &= ~I2C_CR1_PE;                                // Disable I2C
}

void hal_twi_start(void)
{
    if((I2C_BUS->CR1 & I2C_CR1_PE) == 0)
    {
        I2C_BUS->CR1 = I2C_CR1_PE;                              // Enable I2C
    }

    if(I2C_BUS->SR2 & I2C_SR2_BUSY)                             // Bus Busy
    {
        return;
    }

    // Bus Error ?
    if(I2C_BUS->SR1 & (I2C_SR1_TIMEOUT | I2C_SR1_OVR | I2C_SR1_ARLO | I2C_SR1_BERR))
    {
        I2C_BUS->CR1 &= ~I2C_CR1_PE;                            // Disable I2C
        pTWI->frame.access |= TWI_ERROR;
        return;
    }

    twi_pnt = 0;

    I2C_BUS->CR1 |= I2C_CR1_START;                              // Start Generation
    I2C_BUS->CR2 |= (I2C_CR2_ITERREN | I2C_CR2_ITEVTEN);        // Event & Error Interrupt Enable

    pTWI->frame.access |= TWI_BUSY;
}

void I2C_Event_Handler()
{
    uint32_t SR1 = I2C_BUS->SR1;
    uint32_t SR2 = I2C_BUS->SR2;
    uint8_t access = pTWI->frame.access;

    if(access & TWI_WRITE)                                      // Write Request
    {
        if(SR1 & I2C_SR1_SB)                                    // Start Bit Sent in Master Mode
        {
            I2C_BUS->DR = pTWI->frame.address << 1;
        }
        else if(SR1 & I2C_SR1_ADDR)                             // Address Sent
        {
            if(pTWI->frame.write != 0)
            {
                I2C_BUS->DR = pTWI->frame.data[0];
                twi_pnt = 1;
            }
//            else if(access & TWI_READ)
//            {
//                I2C_BUS->CR1 |= I2C_CR1_START;                  // Start Generation
//                pTWI->frame.access &= ~TWI_WRITE;
//            }
            else
            {
                // Event & Error Interrupt Disable
                I2C_BUS->CR2 &= ~(I2C_CR2_ITERREN | I2C_CR2_ITEVTEN | I2C_CR2_ITBUFEN);
                I2C_BUS->CR1 |= I2C_CR1_STOP;                   // Send Stop
                pTWI->frame.access = TWI_RDY;
            }
        }
        else if(SR1 & I2C_SR1_BTF)
        {
            if(twi_pnt < pTWI->frame.write)
            {
                I2C_BUS->DR = pTWI->frame.data[twi_pnt];
                twi_pnt++;
            }
//            else if(access & TWI_READ)
//            {
//                I2C_BUS->CR1 |= I2C_CR1_START;              // Start Generation
//                twi_pnt = 0;
//                pTWI->frame.access &= ~TWI_WRITE;
//            }
            else
            {
                // Event & Error Interrupt Disable
                I2C_BUS->CR2 &= ~(I2C_CR2_ITERREN | I2C_CR2_ITEVTEN | I2C_CR2_ITBUFEN);
                I2C_BUS->CR1 |= I2C_CR1_STOP;                               // Send Stop
                pTWI->frame.access = TWI_RDY;
            }
        }
        //else    // WTF ?
    }
/*
    else if(access & TWI_READ)                                  // Read Request
    {
        if(SR1 & I2C_SR1_SB)                                    // Start Bit Sent in Master Mode
        {
            I2C_BUS->DR = (pTWI->frame.address << 1) | 1;
        }
        else if(SR1 & I2C_SR1_ADDR)                             // Address Sent
        {
            if(pTWI->frame.read == 0)
            {
                // Event & Error Interrupt Disable
                I2C_BUS->CR2 &= ~(I2C_CR2_ITERREN | I2C_CR2_ITEVTEN);
                I2C_BUS->CR1 |= I2C_CR1_STOP;
                pTWI->frame.access = TWI_RDY;
            }
        }
        else if(I2C_BUS->SR1 & I2C_SR1_BTF)
        {
            // Event & Error Interrupt Disable
            I2C_BUS->CR2 &= ~(I2C_CR2_ITERREN | I2C_CR2_ITEVTEN);
            I2C_BUS->CR1 |= I2C_CR1_STOP;
            pTWI->frame.access = TWI_RDY;
        }
        // else WTF ?
    }
    // else WTF ?
*/
}

void I2C_Error_Handler()
{
    if(I2C_BUS->SR1 & I2C_SR1_AF)                                   // NACK received
    {
        I2C_BUS->SR1 &= ~I2C_SR1_AF;                                // Clear Flag AF
        if(twi_pnt == 0)
        {
            // Event & Error Interrupt Enable
            I2C_BUS->CR2 &= ~(I2C_CR2_ITERREN | I2C_CR2_ITEVTEN | I2C_CR2_ITBUFEN);
            I2C_BUS->CR1 |= I2C_CR1_STOP;                           // Send Stop

            pTWI->frame.access |= TWI_SLANACK;
            return;
        }
    }

    I2C_BUS->CR2 &= ~(I2C_CR2_ITERREN | I2C_CR2_ITEVTEN | I2C_CR2_ITBUFEN); // Event & Error Interrupt Enable
    I2C_BUS->CR1 |= I2C_CR1_STOP;

    pTWI->frame.access |= TWI_ERROR;

    #warning ISR_TWI_ER_HANDLER
}


#endif  //  HAL_TWI_BUS
