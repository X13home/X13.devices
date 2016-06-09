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

#if (defined STM32L0)

// System Section

uint32_t    hal_hclk  = 2097000UL;
uint32_t    hal_pclk1 = 2097000UL;
uint32_t    hal_pclk2 = 2097000UL;

void hal_SetSysClock(void)
{
    uint32_t tmp;

    RCC->APB1ENR |= RCC_APB1ENR_PWREN;

    // Set Voltage Range 1, Vcore - 1.8V, Fcore - 32(1WS)/16(0WS)MHz
    while(PWR->CSR & PWR_CSR_VOSF);
    tmp = PWR->CR;
    tmp &= ~PWR_CR_VOS_1;
    tmp |= PWR_CR_VOS_0;
    PWR->CR = tmp;
    while(PWR->CSR & PWR_CSR_VOSF);

    FLASH->ACR |= FLASH_ACR_LATENCY;        // Set FLASH Latensy 1WS
    while((FLASH->ACR & FLASH_ACR_LATENCY) == 0);

    // Clock Source HSI - 16M, prediv = 1
    RCC->CR |= RCC_CR_HSION;
    RCC->CR &= ~RCC_CR_HSIDIVEN;

    while((RCC->CR & (RCC_CR_HSIDIVF | RCC_CR_HSIRDY)) != RCC_CR_HSIRDY);

    // Configure PLL

    // Stop PLL
    RCC->CR &= ~RCC_CR_PLLON;
    while(RCC->CR & RCC_CR_PLLON);

    // PLL Source HSI - 16M
    // PLLDIV = 2
    // PLLMUL = 4
    RCC->CFGR &= ~(RCC_CFGR_PLLDIV | RCC_CFGR_PLLMUL |  RCC_CFGR_PLLSRC);
    RCC->CFGR |= RCC_CFGR_PLLDIV2 | RCC_CFGR_PLLMUL4;

    // PLL Enable
    RCC->CR |= RCC_CR_PLLON;

    // Wait till PLL is ready
    while((RCC->CR & RCC_CR_PLLRDY) == 0);

    // Prescaler
    // AHB = 1
    // PCLK1 = 1
    // PCLK2 = 1
    RCC->CFGR &= ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2);

    // System Clock Source - PLL
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;

    while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

    // Disable MSI
    RCC->CR &= ~RCC_CR_MSION;

    hal_hclk  = 32000000UL;
    hal_pclk1 = 32000000UL;
    hal_pclk2 = 32000000UL;
}

// GPIO Section

void hal_prepare_gpio(void)
{
    RCC->IOPENR |= RCC_IOPENR_IOPAEN | RCC_IOPENR_IOPBEN | RCC_IOPENR_IOPCEN |
                                       RCC_IOPENR_IOPDEN | RCC_IOPENR_IOPHEN;
}

// HAL GPIO Functions
void hal_gpio_cfg(GPIO_TypeDef * GPIOx, uint16_t Mask, uint16_t Mode)
{
    uint16_t pinpos;
    uint32_t pos;
    uint32_t pupd = Mode & 0x03;            // Float / Pull-Up / Pull-Down
    uint32_t ppod = (Mode & 0x04) >> 2;     // Push-Pull / Open Drain
    uint32_t mod  = (Mode & 0x18) >> 3;     // Input / Output / AF / Analog
    uint32_t spd  = (Mode & 0x60) >> 5;     // Low / Medim / Fast / High Speed
    uint32_t afr  = (Mode & 0x0F00) >> 8;   // Alternative function

    for(pinpos = 0; pinpos < 0x10; pinpos++)
    {
        pos = ((uint32_t)0x01) << pinpos;
        if(Mask & pos)
        {
            pos = pinpos << 1;

            // default state input
            GPIOx->MODER  &= ~(GPIO_MODER_MODE0 << pos);
            GPIOx->MODER |= mod << pos;

            if((mod == 1) || (mod == 2))
            {
                // Low Speed
                GPIOx->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEED0 << pos);
                GPIOx->OSPEEDR |= spd << pos;

                // Output type - push/pull
                GPIOx->OTYPER &= ~((GPIO_OTYPER_OT_0) << pinpos);
                GPIOx->OTYPER |= ppod << pinpos;
            }
            // without PullUp/Down
            GPIOx->PUPDR &= ~(GPIO_PUPDR_PUPD0 << pos);
            GPIOx->PUPDR |= pupd << pos;

            if(mod == 2)
            {
                // Set Alternative function
                if(pinpos < 8)              // AFR0
                {
                    pos = (pinpos << 2);
                    GPIOx->AFR[0] &= ~((uint32_t)0x0000000F << pos);
                    GPIOx->AFR[0] |= ((uint32_t)afr << pos);
                }
                else                        // AFR1
                {
                    pos = ((pinpos - 8) << 2);
                    GPIOx->AFR[1] &= ~((uint32_t)0x0000000F << pos);
                    GPIOx->AFR[1] |= ((uint32_t)afr << pos);
                }
            }
        }
    }
}

// EEPROM Section

// Initialise EEPROM hardware
void eeprom_init_hw(void)
{
    // Wait till no operation is on going
    while (FLASH->SR & FLASH_SR_BSY);

    // Unlocking FLASH_PECR register access
    if(FLASH->PECR & FLASH_PECR_PELOCK)         // Check if the PELOCK is unlocked
    {
        FLASH->PEKEYR = 0x89ABCDEF;             // Perform unlock sequence
        FLASH->PEKEYR = 0x02030405;
    }
}

// Read data from EEPROM
void eeprom_read(uint8_t *pBuf, uint32_t Addr, uint32_t Len)
{
    uint32_t eep_addr = DATA_EEPROM_BASE;
    eep_addr += Addr;
    if((eep_addr + Len) > DATA_EEPROM_END)
    {
        return;
    }

    memcpy(pBuf, (const void *)eep_addr, Len);
}

// Write data to EEPROM
void eeprom_write(uint8_t *pBuf, uint32_t Addr, uint32_t Len)
{
    uint32_t eep_addr = DATA_EEPROM_BASE;
    eep_addr += Addr;
    if((eep_addr + Len) > DATA_EEPROM_END)      // Invalid Address or Length
    {
        return;
    }

    //if(FLASH->PECR & FLASH_PECR_ERASE)
    //{
    //    FLASH->PECR &= ~FLASH_PECR_ERASE;
    //}

    while(Len > 0)
    {
        if(FLASH->SR & FLASH_SR_WRPERR)         //  Write Error
        {
            return;
        }

        *(uint8_t *)eep_addr = *pBuf;
        eep_addr++;
        pBuf++;
        Len--;
    }
}
#endif  //  STM32L0
