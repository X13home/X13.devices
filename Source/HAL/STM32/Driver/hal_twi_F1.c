#include "config.h"

#if (defined HAL_TWI_BUS) && (defined STM32F1)

#include "EXT/exttwi.h"

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
        // I2C Bus = 100 KHz, Standard Mode
        uint32_t freqrange = hal_pclk1 / 1000000UL;

        I2C_BUS->CR2 = freqrange;
        I2C_BUS->TRISE = freqrange + 1;

        uint32_t tmp = hal_pclk1 / 200000UL;    // i2c clock = 100kHz
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

        NVIC_SetPriority(I2C_IRQn, 3);
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
        return;
    }

#warning hal_twi_start

//    pTWI->frame.access |= TWI_BUSY;
}

void ISR_TWI_HANDLER()
{

#warning ISR_TWI_HANDLER

}

#endif  //  HAL_TWI_BUS
