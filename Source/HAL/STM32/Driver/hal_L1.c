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

#if (defined STM32L1)

// System Section
// Default HSE 8M

#define HSE_STARTUP_TIME            0x00005000UL

uint32_t    hal_hclk  = 2097000UL;
uint32_t    hal_pclk1 = 2097000UL;
uint32_t    hal_pclk2 = 2097000UL;

void hal_SetSysClock(void)
{
    uint32_t tmp;
    uint32_t StartUpCounter = 0;
    uint32_t pllMul;
    
#if (defined HAL_USE_HSE)
#if (defined HSE_CRYSTAL_BYPASS)
    // HSE crystal oscillator bypassed with external clock
    RCC->CR |= RCC_CR_HSEBYP;
#endif  //  HSE_CRYSTAL_BYPASS
    // Enable HSE
    RCC->CR |= RCC_CR_HSEON;
    
    // Wait till HSE is ready and if Time out is reached exit
    while(((RCC->CR & RCC_CR_HSERDY) == 0) && (StartUpCounter < HSE_STARTUP_TIME))
    {
        StartUpCounter++;
    }

    if(StartUpCounter < HSE_STARTUP_TIME)
    {
        RCC->CFGR |= RCC_CFGR_PLLSRC;               // PLL Source HSE
        pllMul = RCC_CFGR_PLLMUL8;
    }
    else
    {
        RCC->CR &= ~RCC_CR_HSEON;                   // Disble HSE
        // Clock Source HSI - 16M
        RCC->CR |= RCC_CR_HSION;
        // Wait till HSI is ready
        while((RCC->CR & RCC_CR_HSIRDY) != RCC_CR_HSIRDY);
        pllMul = RCC_CFGR_PLLMUL4;
    }
#else   // HSI
    // Clock Source HSI - 16M
    RCC->CR |= RCC_CR_HSION;
    // Wait till HSI is ready
    while((RCC->CR & RCC_CR_HSIRDY) != RCC_CR_HSIRDY);
    pllMul = RCC_CFGR_PLLMUL4;
#endif  // HAL_USE_HSE
    /* Enable 64-bit access */
    FLASH->ACR |= FLASH_ACR_ACC64;
    /* Enable Prefetch Buffer */
    FLASH->ACR |= FLASH_ACR_PRFTEN;
    /* Flash 1 wait state */
    FLASH->ACR |= FLASH_ACR_LATENCY;
    /* Power enable */
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;

    // Set Voltage Range 1, Vcore - 1.8V, Fcore - 32(1WS)/16(0WS)MHz
    while(PWR->CSR & PWR_CSR_VOSF);
    tmp = PWR->CR;
    tmp &= ~PWR_CR_VOS;
    tmp |= PWR_CR_VOS_0;
    PWR->CR = tmp;
    while(PWR->CSR & PWR_CSR_VOSF);

    // Prescalers
    // HCLK = SYSCLK
    // PCLK2 = HCLK
    // PCLK1 = HCLK

    // Configure PLL
    RCC->CFGR &= ~(RCC_CFGR_PLLMUL | RCC_CFGR_PLLDIV);
    RCC->CFGR |= RCC_CFGR_PLLDIV2;
    RCC->CFGR |= pllMul;
    
    /* Enable PLL */
    RCC->CR |= RCC_CR_PLLON;
    /* Wait till PLL is ready */
    while((RCC->CR & RCC_CR_PLLRDY) == 0) {};

    /* Select PLL as system clock source */
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL;

    /* Wait till PLL is used as system clock source */
    while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)RCC_CFGR_SWS_PLL) {};

    hal_hclk  = 32000000UL;
    hal_pclk1 = 32000000UL;
    hal_pclk2 = 32000000UL;
}

// GPIO Section

void hal_prepare_gpio(void)
{
    RCC->AHBENR |=   RCC_AHBENR_GPIOAEN
                   | RCC_AHBENR_GPIOBEN
                   | RCC_AHBENR_GPIOCEN
                   | RCC_AHBENR_GPIODEN
                   ;
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
            GPIOx->MODER  &= ~(GPIO_MODER_MODER0 << pos);
            GPIOx->MODER |= mod << pos;

            if((mod == 1) || (mod == 2))
            {
                // Low Speed
                GPIOx->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR0 << pos);
                GPIOx->OSPEEDR |= spd << pos;

                // Output type - push/pull
                GPIOx->OTYPER &= ~((GPIO_OTYPER_OT_0) << pinpos);
                GPIOx->OTYPER |= ppod << pinpos;
            }
            // without PullUp/Down
            GPIOx->PUPDR &= ~(GPIO_PUPDR_PUPDR0 << pos);
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
    uint32_t eep_addr = FLASH_EEPROM_BASE;
    eep_addr += Addr;
    if((eep_addr + Len) > FLASH_EEPROM_END)
    {
        return;
    }

    memcpy(pBuf, (const void *)eep_addr, Len);
}

#define FLASH_PRG_TIMEOUT   ((uint32_t)0x00008000)

// Write data to EEPROM
void eeprom_write(uint8_t *pBuf, uint32_t Addr, uint32_t Len)
{
    uint32_t eep_addr = FLASH_EEPROM_BASE;
    eep_addr += Addr;
    if((eep_addr + Len) > FLASH_EEPROM_END)      // Invalid Address or Length
    {
        return;
    }

    // Convert uint8 to uint32
    uint32_t e_addr = eep_addr & 0xFFFFFFFC;
    uint32_t val = *(uint32_t *)e_addr;
    uint8_t  pos = 8 * (eep_addr & 0x00000003);

    while(Len > 0)
    {
        if(FLASH->SR & (FLASH_SR_WRPERR |   // Write protected error
                        FLASH_SR_PGAERR |   // Programming Alignment Error
                        FLASH_SR_SIZERR))   // Size error
        {
            return;
        }

        uint32_t timeout = FLASH_PRG_TIMEOUT;
        while(FLASH->SR & FLASH_SR_BSY)
        {
            if(timeout > 0)
            {
                timeout--;
            }
            else
            {
                return;
            }
        }

        val &= ~(0x000000FF << pos);
        val |= (*pBuf) << pos;

        eep_addr++;
        pBuf++;
        Len--;

        if(Len == 0)
        {
            *(uint32_t *)e_addr = val;
        }
        else if((eep_addr & 0x00000003) == 0)
        {
            *(uint32_t *)e_addr = val;

            e_addr = eep_addr;
            val = *(uint32_t *)e_addr;
            pos = 0;
        }
        else
        {
            pos += 8;
        }
    }
}

#endif  //  STM32L0
