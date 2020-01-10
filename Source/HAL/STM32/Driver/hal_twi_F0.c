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

#if (defined HAL_TWI_BUS) && ((defined STM32F0) || (defined STM32L0))

#include "EXT/exttwi.h"

#if (HAL_TWI_BUS == 1)

void I2C1_IRQHandler(void);

#define I2C_IRQ_Handler()   I2C1_IRQHandler(void)

#define I2C_TIMING      0x2000090E
#define I2C_BUS         I2C1
#define I2C_IRQn        I2C1_IRQn

#elif (HAL_TWI_BUS == 2)

void I2C2_IRQHandler(void);

#define I2C_IRQ_Handler()   I2C2_IRQHandler(void)

#define I2C_TIMING      0x20303E5D
#define I2C_BUS         I2C2
#define I2C_IRQn        I2C2_IRQn

#else
#error HAL_TWI_F0 Unknown HAL_TWI_BUS
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

        // Configure GPIO as inputs with pull down
        hal_gpio_cfg(I2C_GPIO, (I2C_PIN_SCL | I2C_PIN_SDA), DIO_MODE_IN_PD);

        uint16_t timeout = 0x8000;
        while(timeout != 0)
        {
            if((I2C_GPIO->IDR & (I2C_PIN_SCL | I2C_PIN_SDA)) != (I2C_PIN_SCL | I2C_PIN_SDA))
            {
                timeout++;
                if(timeout == 0)
                {
                    // Release GPIO
                    hal_gpio_cfg(I2C_GPIO, (I2C_PIN_SCL | I2C_PIN_SDA), DIO_MODE_IN_FLOAT);
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

        // Configure GPIO
        hal_gpio_cfg(I2C_GPIO, (I2C_PIN_SCL | I2C_PIN_SDA), DIO_MODE_TWI);

#if (HAL_TWI_BUS == 1)
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
        // I2C Bus = 100 KHz
        I2C_BUS->TIMINGR = I2C_TIMING;

        // PEC Disable
        // SMBus Disable
        // General Call Disable
        // Wakeup Disable
        // Clock stretching in slave mode enabled
        // DMA disabled
        // Analog Filter On
        // Digital Filter Off

        // Enable I2C
        I2C_BUS->CR1 = I2C_CR1_PE;

        NVIC_SetPriority(I2C_IRQn, 0);
        NVIC_EnableIRQ(I2C_IRQn);
    }
    else
    {
        NVIC_DisableIRQ(I2C_IRQn);
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
        // Release GPIO
        hal_gpio_cfg(I2C_GPIO, (I2C_PIN_SCL | I2C_PIN_SDA), DIO_MODE_IN_FLOAT);
    }

    return true;
}

void hal_twi_stop(void)
{
    I2C_BUS->CR1 &= ~I2C_CR1_PE;                                // Disable I2C
}

void hal_twi_start(void)
{
    uint32_t isr = I2C_BUS->ISR;
    if(isr & (I2C_ISR_OVR | I2C_ISR_ARLO | I2C_ISR_BERR))       // Bus Error
    {
        I2C_BUS->CR1 &= ~I2C_CR1_PE;                            // Disable I2C
        pTWI->frame.access |= TWI_ERROR;
        return;
    }

    if(isr & I2C_ISR_BUSY)                                      // Bus Busy
    {
        return;
    }

    I2C_BUS->CR1 &= ~I2C_CR1_PE;                                // Reset I2C

    twi_pnt = 0;
    pTWI->frame.access |= TWI_BUSY;
    uint8_t access = pTWI->frame.access;

    I2C_BUS->CR1 =  ( I2C_CR1_PE |                              // Enable I2C
                                                                // Enable Interrupts on:
                      I2C_CR1_ERRIE |                           //  Errors
                      I2C_CR1_NACKIE);                          //  NACK received

    I2C_BUS->ICR = I2C_ICR_STOPCF | I2C_ICR_NACKCF;             // Clear STOP & NACK flags.

    if(access & TWI_WRITE)
    {
        I2C_BUS->CR2 = (uint32_t)(pTWI->frame.address << 1) |   // Slave address
                      ((uint32_t)(pTWI->frame.write) << 16);    // Bytes to send

        if(pTWI->frame.write > 0)
        {
            I2C_BUS->CR1 |= I2C_CR1_TXIE;                       // Interrupt on Tx Buffer empty
        }

        if((access & TWI_READ) == 0)
        {
            I2C_BUS->CR2 |= I2C_CR2_AUTOEND;
            I2C_BUS->CR1 |= I2C_CR1_STOPIE;
        }
        else
        {
            I2C_BUS->CR1 |= I2C_CR1_TCIE;                       // Enable TC IRQ
        }
    }
    else    // Only Read Access
    {
        uint32_t read = pTWI->frame.read;
        if(read == 0)
        {
            read = 1;
        }

        I2C_BUS->CR2 = ((uint32_t)(pTWI->frame.address << 1) |  // Slave address
                                                (read << 16) |  // Bytes to read
                    I2C_CR2_RD_WRN | I2C_CR2_AUTOEND);          // Read request with stop

        I2C_BUS->CR1 |= I2C_CR1_RXIE;
    }

    I2C_BUS->CR2 |= I2C_CR2_START;                              // Send Start & Address
}

void I2C_IRQ_Handler()
{
    uint32_t isr = I2C_BUS->ISR;

    if(isr & (I2C_ISR_BERR |                                        // Bus error
              I2C_ISR_ARLO |                                        // Arbitration lost
              I2C_ISR_OVR))                                         // Over-Run/Under-Run
    {
        I2C_BUS->CR1 &= ~I2C_CR1_PE;                                // Disable I2C
        pTWI->frame.access |= TWI_ERROR;
    }
    else if(isr & I2C_ISR_NACKF)                                    // NACK received
    {
        I2C_BUS->ICR = I2C_ICR_NACKCF;                              // Clear NACK Flag

        if(twi_pnt == 0)
        {
            I2C_BUS->CR1 = I2C_CR1_PE;                              // Disable Interrupts
            pTWI->frame.access |= TWI_SLANACK;
            pTWI->frame.read = 0;
        }
        else if(pTWI->frame.access & TWI_WRITE)
        {
            pTWI->frame.write = twi_pnt;

            if(pTWI->frame.access & TWI_READ)
            {
                uint32_t read = pTWI->frame.read;
                if(read == 0)
                {
                    read = 1;
                }

                I2C_BUS->CR2 = ((uint32_t)(pTWI->frame.address << 1) |  // Slave address
                                                        (read << 16) |  // Bytes to read
                            I2C_CR2_RD_WRN | I2C_CR2_AUTOEND);          // Read request with stop
                I2C_BUS->CR2 |= I2C_CR2_START;                      // Send Repeat Start & Address

                I2C_BUS->CR1 |= I2C_CR1_RXIE;
                I2C_BUS->CR1 &= ~(                                      // Disable Interrupts
                                  I2C_CR1_TCIE |                        // Transfer complete
                                  I2C_CR1_TXIE);                        // Tx
                twi_pnt = 0;
            }
            else
            {
                I2C_BUS->CR1 = I2C_CR1_PE;                      // Disable Interrupts

                pTWI->frame.access &= TWI_EXT;
                pTWI->frame.access |= TWI_RDY;                  // Transaction complete
            }
        }
        // else WTF ?
    }
    // Read Data
    else if((I2C_BUS->CR1 & I2C_CR1_RXIE) &&                    // Data received
                     (isr & I2C_ISR_RXNE))
    {
        pTWI->frame.data[twi_pnt++] = I2C_BUS->RXDR;

        if(twi_pnt >= pTWI->frame.read)
        {
            I2C_BUS->CR1 = I2C_CR1_PE;                          // Disable Interrupts
            pTWI->frame.access &= TWI_EXT;
            pTWI->frame.access |= TWI_RDY;                      // Transaction complete
        }
    }
    // Write Data
    else if((I2C_BUS->CR1 & I2C_CR1_TXIE) &&
                     (isr & I2C_ISR_TXIS))                      // Transmit buffer empty
    {
        I2C_BUS->TXDR = pTWI->frame.data[twi_pnt++];
    }
    // Write Last Byte and Start Read
    else if(isr & I2C_ISR_TC)                                   // Transfer complete
    {
        uint32_t read = pTWI->frame.read;
        if(read == 0)
        {
            read = 1;
        }

        I2C_BUS->CR2 = ((uint32_t)(pTWI->frame.address << 1) |  // Slave address
                                                (read << 16) |  // Bytes to read
                    I2C_CR2_RD_WRN | I2C_CR2_AUTOEND);          // Read request with stop
        I2C_BUS->CR2 |= I2C_CR2_START;                          // Send Repeat Start & Address

        I2C_BUS->CR1 |= I2C_CR1_RXIE;
        I2C_BUS->CR1 &= ~(                                      // Disable Interrupts
                          I2C_CR1_TCIE |                        // Transfer complete
                          I2C_CR1_TXIE);                        // Tx
        twi_pnt = 0;
    }
    // Write Only Access
    else if(isr & I2C_ISR_STOPF)                                // Stop received
    {
        I2C_BUS->ICR = I2C_ICR_STOPCF;                          // Clear Stop Flag
        I2C_BUS->CR1 = I2C_CR1_PE;                              // Disable Interrupts
        pTWI->frame.access &= TWI_EXT;
        pTWI->frame.access |= TWI_RDY;                          // Transaction complete
    }
    // else WTF ?
}

#endif  //  HAL_TWI_BUS
