#include "../../config.h"

#ifdef EXTOPS_USED

#ifdef EXTTWI_USED
#error conflict, TWI not exclusive use.
#endif  //  EXTTWI_USED

#define DIO_MODE_TWI    ((1<<DIO_AF_OFFS) | DIO_MODE_AF_OD)       // Alternative function, AF = 1, Open Drain

// I2C1, PB6 - SCL, PB7 - SDA
#define I2C_BUS         I2C1
#define I2C_IRQn        I2C1_IRQn
#define I2C_PIN_SCL     GPIO_Pin_6
#define I2C_PIN_SDA     GPIO_Pin_7

// Sensor Interrupt on PA8

#define SI1143_ADDR     0x5A

typedef enum
{
    HAL_OPS_NONE            = 0,
    HAL_OPS_PREINIT,
    HAL_OPS_BUS_ERR         = 0x81,
    HAL_OPS_NACK_ERR,
    HAL_OPS_UNKNOWN_ERR
}e_HAL_OPS;

static uint8_t hal_ops_state;

static uint8_t * hal_ops_wr_pnt;
static uint8_t * hal_ops_rd_pnt;
static uint8_t 

void hal_ops_init(void)
{
    hal_dio_gpio_cfg(GPIOB, (I2C_PIN_SCL | I2C_PIN_SDA), DIO_MODE_TWI);             // Configure GPIO
    
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;     // Enable I2C clock
    RCC->CFGR3 |= RCC_CFGR3_I2C1SW;         // Use SysClk for I2C CLK
    // Reset I2C1
    RCC->APB1RSTR |= RCC_APB1RSTR_I2C1RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C1RST;

    // Set Timings, RM0091, page 542
    // I2C Clock = 48MHz
    // I2C Bus = 100 KHz
    //I2C_BUS->TIMINGR |= 0xB0420F13;

    // I2C Clock = 48MHz
    // I2C Bus = 400 KHz - fast Mode
    // rise time = 140ns, fall time = 40ns
    I2C_BUS->TIMINGR = (uint32_t)0x00B01A4B;

    // Enable I2C
    I2C_BUS->CR1 = I2C_CR1_PE;

    hal_ops_state = HAL_OPS_NONE;

    NVIC_SetPriority(I2C_IRQn, 3);
    NVIC_EnableIRQ(I2C_IRQn);



    // EXTI Section

    // Enable SYSCFG clock
    //RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
}

void I2C1_IRQHandler(void)
{
    uint32_t isr = I2C_BUS->ISR;

    if(isr & (I2C_ISR_OVR | I2C_ISR_ARLO | I2C_ISR_BERR))               // Bus Error
    {
        I2C_BUS->CR1 &= ~I2C_CR1_PE;                                    // Disable I2C
        hal_ops_state = HAL_OPS_BUS_ERR;
    }
    //
    //
    else                                                                // Unknown state
    {
        I2C_BUS->CR1 &= ~I2C_CR1_PE;                                    // Disable I2C
        hal_ops_state = HAL_OPS_UNKNOWN_ERR;
    }
}

// PA8, Sensor Int.
void EXTI4_15_IRQHandler(void)
{

}


/*
// TWI Access Flags
#define TWI_WRITE   0x01    // Write access
#define TWI_READ    0x02    // Read access
#define TWI_BUSY    0x08    // Bus busy
#define TWI_RDY     0x10    // Access complete
#define TWI_WD      0x20    // Timeout
#define TWI_SLANACK 0x40    // Slave Addr NACK received
#define TWI_ERROR   0x80    // Unknown error

typedef struct
{
    uint8_t     address;
    uint8_t     access;
    uint8_t     write;
    uint8_t     read;
    uint8_t     data[];
} TWI_FRAME_t;

static TWI_FRAME_t * pFrame;
*/

#endis  //  EXTOPS_USED