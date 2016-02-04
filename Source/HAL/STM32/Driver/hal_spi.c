#include "config.h"

#if (defined HAL_USE_SPI1) || \
    (defined HAL_USE_SPI2) || \
    (defined HAL_USE_SPI3)
    
/*
        Config 1                Config 2
SPIx    MOSI    MISO    SCK     MOSI    MISO    SCK     APB
SPI1    PA7     PA6     PA5     PB5     PB4     PB3     2
SPI2    PB15    PB14    PB13                            1
SPI3    PB5     PB4     PB3     PC12    PC11    PC10    1

STM32F0
APB1 clk = SystemCoreClock
APB2 clk = SystemCoreClock

STM32F1
APB1 clk = SystemCoreClock/2
APB2 clk = SystemCoreClock
*/

#define SPI1_PORT                   GPIOA
#define SPI1_SCK_PIN                GPIO_Pin_5
#define SPI1_MISO_PIN               GPIO_Pin_6
#define SPI1_MOSI_PIN               GPIO_Pin_7

#define SPI11_PORT                  GPIOB
#define SPI11_SCK_PIN               GPIO_Pin_3
#define SPI11_MISO_PIN              GPIO_Pin_4
#define SPI11_MOSI_PIN              GPIO_Pin_5

#define SPI2_PORT                   GPIOB
#define SPI2_SCK_PIN                GPIO_Pin_13
#define SPI2_MISO_PIN               GPIO_Pin_14
#define SPI2_MOSI_PIN               GPIO_Pin_15

#ifdef SPI3

#define SPI3_PORT                   GPIOB
#define SPI3_SCK_PIN                GPIO_Pin_3
#define SPI3_MISO_PIN               GPIO_Pin_4
#define SPI3_MOSI_PIN               GPIO_Pin_5

#define SPI13_PORT                  GPIOC
#define SPI13_SCK_PIN               GPIO_Pin_10
#define SPI13_MISO_PIN              GPIO_Pin_11
#define SPI13_MOSI_PIN              GPIO_Pin_12

#elif defined HAL_USE_SPI3
#error SPI3 not exist
#undef HAL_USE_SPI3
#endif  //  SPI3

#if (defined STM32F0)

// STM32F051
#define DIO_MODE_SPI1               DIO_MODE_AF_PP_HS
#define DIO_MODE_SPI2               DIO_MODE_AF_PP_HS
#define DIO_MODE_SPI11              DIO_MODE_AF_PP_HS

#elif (defined STM32F1)

#define DIO_MODE_SPI1               DIO_MODE_AF_PP_HS
#define DIO_MODE_SPI2               DIO_MODE_AF_PP_HS
#define DIO_MODE_SPI3               DIO_MODE_AF_PP_HS

#else
    #error hal_spi.c unknown uC Family
#endif

extern uint32_t SystemCoreClock;

static SPI_TypeDef * hal_spi_port2spi(uint8_t port)
{
    switch(port)
    {
#ifdef HAL_USE_SPI1
        case 1:
#ifdef DIO_MODE_SPI11
        case 11:
#endif  //  DIO_MODE_SPI11
            return SPI1;
#endif  //  SPI1
#ifdef HAL_USE_SPI2
        case 2:
            return SPI2;
#endif  //  SPI2
#ifdef HAL_USE_SPI3
        case 3:
#ifdef DIO_MODE_SPI13
        case 13:
#endif  //  DIO_MODE_SPI13
            return SPI3;
#endif  //  SPI3
        default:
            while(1);      // Unknown SPI port
    }
}

void hal_spi_cfg(uint8_t port, uint8_t mode, uint32_t speed)
{
    SPI_TypeDef * SPIx;
    uint32_t spi_clock;

    switch(port)
    {
#if (defined HAL_USE_SPI1)
        case 1:
            RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
            hal_gpio_cfg(SPI1_PORT, (SPI1_SCK_PIN | SPI1_MISO_PIN | SPI1_MOSI_PIN), DIO_MODE_SPI1);
            SPIx = SPI1;
            break;
#if (defined DIO_MODE_SPI11)
        case 11:
#if (defined STM32F1)
            AFIO->MAPR |= AFIO_MAPR_SPI1_REMAP;     // Remap SPI1
#endif  //  STM32F1
            RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
            hal_gpio_cfg(SPI11_PORT, (SPI11_SCK_PIN | SPI11_MISO_PIN | SPI11_MOSI_PIN), DIO_MODE_SPI11);
            SPIx = SPI1;
            break;
#endif  //  DIO_MODE_SPI11
#endif  //  HAL_USE_SPI1
#if (defined HAL_USE_SPI2)
        case 2:
            RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
            hal_gpio_cfg(SPI2_PORT, (SPI2_SCK_PIN | SPI2_MISO_PIN | SPI2_MOSI_PIN), DIO_MODE_SPI2);
            SPIx = SPI2;
            break;
#endif  //  DIO_MODE_SPI2
#if (defined HAL_USE_SPI3)
        case 3:
            RCC->APB1ENR |= RCC_APB1ENR_SPI3EN;
            hal_gpio_cfg(SPI3_PORT, (SPI3_SCK_PIN | SPI3_MISO_PIN | SPI3_MOSI_PIN), DIO_MODE_SPI3);
            SPIx = SPI3;
            break;
#if (defined DIO_MODE_SPI13)
        case 13:
            RCC->APB1ENR |= RCC_APB1ENR_SPI3EN;
            hal_gpio_cfg(SPI13_PORT, (SPI13_SCK_PIN | SPI13_MISO_PIN | SPI13_MOSI_PIN), DIO_MODE_SPI13);
            SPIx = SPI3;
            break;
#endif  //  DIO_MODE_SPI13
#endif  //  HAL_USE_SPI3
        default:
            while(1);      // Unknown SPI port
    }
    
    // Calculate pre-scaler
#if (defined STM32F0)
    spi_clock = SystemCoreClock;
#elif (defined STM32F1)
    if(port == 1)
        spi_clock = SystemCoreClock;
    else
        spi_clock = SystemCoreClock/2;
#else
    #error hal_spi_cfg unknown uC Family
#endif  //CPU

    spi_clock /= 2;

    uint16_t div = 0;
    while((spi_clock > speed) && (div < 7))
    {
        div++;
        spi_clock /= 2;
    }

    // Configure SPI
    SPIx->CR1 = (uint16_t)(SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_MSTR) | (div<<3);
#if (defined STM32F0)
    if(mode & HAL_SPI_16B)
        SPIx->CR2 = (uint16_t)(SPI_CR2_DS_3 | SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0); // 16 bit
    else
        SPIx->CR2 = (uint16_t)(SPI_CR2_FRXTH | SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0); // 8 bit
#elif (defined STM32F1)
    if(mode & HAL_SPI_16B)
        SPIx->CR1 |= SPI_CR1_DFF;
#else
    #error hal_spi_cfg unknown uC Family
#endif  // CPU
    if(mode & HAL_SPI_LSB)
        SPIx->CR1 |= SPI_CR1_LSBFIRST;
    
    if(mode & 1)        // Mode 1/3
        SPIx->CR1 |= SPI_CR1_CPHA;
    if(mode & 2)        // Mode 2/3
        SPIx->CR1 |= SPI_CR1_CPOL;

    SPIx->CRCPR =  7;
    SPIx->I2SCFGR &= (uint16_t)~((uint16_t)SPI_I2SCFGR_I2SMOD);
    SPIx->CR1 |= SPI_CR1_SPE;       // SPI enable
}

uint8_t hal_spi_exch8(uint8_t port, uint8_t data)
{
    SPI_TypeDef * SPIx = hal_spi_port2spi(port);
#if (defined STM32F0)
    uint32_t spixbase = (uint32_t)SPIx + 0x0C;
    *(__IO uint8_t *)spixbase = data;
    while((SPIx->SR & SPI_SR_RXNE) == 0);
    data = *(__IO uint8_t *)spixbase;
    return data;
#elif (defined STM32F1)
    SPIx->DR = data;
    while((SPIx->SR & SPI_SR_RXNE) == 0);
    return (SPIx->DR & 0xFF);
#else
    #error hal_spi_exch8 unknown uC Family
#endif
}

uint16_t hal_spi_exch16(uint8_t port, uint16_t data)
{
    SPI_TypeDef * SPIx = hal_spi_port2spi(port);
    SPIx->DR = data;
    while((SPIx->SR & SPI_SR_RXNE) == 0);
    return SPIx->DR;
}

#endif  //  HAL_USE_SPIx
