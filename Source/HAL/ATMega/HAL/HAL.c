#include "config.h"

#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

// Hardware constants for timer 2.
#if (((F_CPU/8000) - 1) < 255)
#define halCLOCK_CONFIG     2
#define halCLOCK_COMPARE_VALUE ((F_CPU/8000)-1)
#elif (((F_CPU/32000) - 1) < 255)
#define halCLOCK_CONFIG     3
#define halCLOCK_COMPARE_VALUE ((F_CPU/32000)-1)
#elif (((F_CPU/64000) - 1) < 255)
#define halCLOCK_CONFIG     4
#define halCLOCK_COMPARE_VALUE ((F_CPU/64000)-1)
#elif (((F_CPU/128000) - 1) < 255)
#define halCLOCK_CONFIG     5
#define halCLOCK_COMPARE_VALUE ((F_CPU/128000)-1)
#elif (((F_CPU/256000) - 1) < 255)
#define halCLOCK_CONFIG     6
#define halCLOCK_COMPARE_VALUE ((F_CPU/256000)-1)
#elif (((F_CPU/1024000) - 1) < 255)
#define halCLOCK_CONFIG     7
#define halCLOCK_COMPARE_VALUE ((F_CPU/1024000)-1)
#else
#error Check F_CPU
#endif

void HAL_Init(void)
{
    cli();

    MCUSR &= ~(1<<WDRF);
    wdt_reset();
    wdt_disable();
#ifdef HAL_USE_SUBMSTICK
#if (F_CPU != 16000000UL)
#error F_CPU should be 16M
#endif
    TCCR1B = 0;
    TCCR1A = 0;
    TCNT1 = 0;
    TCCR1B = 5;     //  Prescaler = 1024, Normal Mode
#endif  //  HAL_USE_SUBMSTICK
}

void HAL_StartSystemTick(void)
{
    TCCR2A = (1<<WGM21);
    TCNT2 = 0;
    OCR2A = halCLOCK_COMPARE_VALUE;
    TIFR2 = (1<<OCF2A);
    TIMSK2 = (1<<OCIE2A);
    TCCR2B = halCLOCK_CONFIG;

    sei();
}

// Generate pseudo random uint16
uint16_t HAL_RNG(void)
{
    static uint16_t rand16 = 0xA15E;

    // Galois LFSRs
    if(rand16 & 1)
    {
        rand16 >>= 1;
        rand16 ^= 0xB400;
    }
    else
    {
        rand16 >>= 1;
    }
  
    return rand16;
}

#if (defined EXTPLC_USED)
/*
static uint32_t deadbeef_seed;
static uint32_t deadbeef_beef = 0xdeadbeef;

uint32_t deadbeef_rand()
{
    deadbeef_seed = (deadbeef_seed << 7) ^ ((deadbeef_seed >> 25) + deadbeef_beef);
    deadbeef_beef = (deadbeef_beef << 7) ^ ((deadbeef_beef >> 25) + 0xdeadbeef);
    return deadbeef_seed;
}

void deadbeef_srand(uint32_t x) {
    deadbeef_seed = x;
    deadbeef_beef = 0xdeadbeef;
}
*/
uint32_t HAL_RNG32(void)
{
    // LFSR32, tap: 1,5,6,31

    static uint32_t lfsr = 0xAD1AE5EF;
    uint8_t i;
    for(i = 0; i <= 8; i++)
    {
        uint8_t new_bit = 0;

        // xor the tap values together
        if(lfsr & 0x40000000){  new_bit  = 1;}
        if(lfsr & 0x04000000){  new_bit ^= 1;}
        if(lfsr & 0x02000000){  new_bit ^= 1;}
        if(lfsr & 0x00000002){  new_bit ^= 1;}

        lfsr <<= 1;
        lfsr |= new_bit;
    }

    return lfsr;
}
#endif  //  EXTPLC_USED

#ifdef HAL_USE_SUBMSTICK
uint16_t HAL_get_submstick(void)
{
    return TCNT1;
}
#endif  //  HAL_USE_SUBMSTICK

static volatile uint32_t hal_ms_counter = 0;
static volatile uint32_t hal_sec_counter = 0;        // Max Uptime 136 Jr.

uint32_t HAL_get_ms(void)
{
    return hal_ms_counter;
}

uint32_t HAL_get_sec(void)
{
    return hal_sec_counter;
}

ISR(TIMER2_COMPA_vect)
{
    hal_ms_counter++;

    static uint16_t ms_counter = 0;
    if(++ms_counter >= 999)
    {
        ms_counter = 0;
        hal_sec_counter++;
    }

    static uint16_t ticks_counter = 0;
    if(ticks_counter < (const uint16_t)(1000/POLL_TMR_FREQ))
    {
        ticks_counter++;
    }
    else
    {
        SystemTick();
        ticks_counter = 1;
    }
}

// ASLeep Section
#ifdef ASLEEP
ISR(WDT_vect)
{
    hal_sec_counter += 8;
    hal_ms_counter += 8000;
}

void HAL_ASleep(uint16_t duration)
{
    duration >>= 3; // Minimal Sleep Time 8S

#ifdef EXTAIN_USED
    // Disable ADC
    uint8_t oldAD = ADCSRA;
    if(ADCSRA & (1<<ADEN))
    {
        while(ADCSRA & (1<<ADSC));
        ADCSRA &= ~(1<<ADEN);
    }
#endif  //  EXTAIN_USED

    // ToDo disable TWI

    MCUSR &= ~(1<<WDRF);
    wdt_reset();

    // start timed sequence
    WDTCSR |= (1<<WDCE) | (1<<WDE);
    // set new watchdog timeout value
    WDTCSR = (1<<WDCE) | (1<<WDP3) | (1<<WDP0); // 8S
    WDTCSR |= (1<<WDIE);                        // Enable WatchDog Interrupt

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();

    while(duration > 0)
    {
        sleep_mode();
        duration--;
    }

    wdt_disable();
    sleep_disable();

#ifdef EXTAIN_USED
    // Restore ADC State
    if(oldAD & (1<<ADEN))
    {
        ADCSRA = oldAD;
    }
#endif  //  EXTAIN_USED
}
#endif  //  ASLEEP

void HAL_Reboot(void)
{
#ifdef LED_On
    LED_On();
#endif

    cli();
    wdt_enable(WDTO_250MS);
    while(1);
}

// SPI Section
#ifdef HAL_USE_SPI1
void hal_spi_cfg(uint8_t port __attribute__ ((unused)), uint8_t mode, uint32_t speed)
{
    PRR &= ~(1<<PRSPI);                                          // Enable clock on SPI
    // Configure DIO

    // for Master SPI_PIN_SS MUST be set as out
    SPI_DDR |= (1<<SPI_PIN_SCK) | (1<<SPI_PIN_MOSI) | (1<<SPI_PIN_SS);
    SPI_DDR &= ~(1<<SPI_PIN_MISO);

    uint32_t spiclk = (F_CPU/2);
    uint8_t  div = 0;
    uint8_t  tmp = (1<<SPE) | (1<<MSTR);

    SPCR = 0;       // Disable SPI

    // Calculate divider
    while((spiclk > speed) && (div < 5))
    {
        div++;
        spiclk >>= 1;
    }

    if((div & 1) == 0)
    {
        SPSR = (1<<SPI2X);
    }

    if(mode & HAL_SPI_LSB)      // MSB/LSB
    {
        tmp |= (1<<DORD);
    }

    tmp |= (mode & 3) << 2;     // CPOL, CPHA
    tmp |= div >> 1;

    SPCR = tmp;
}

uint8_t hal_spi_exch8(uint8_t port __attribute__ ((unused)), uint8_t data)
{
    SPDR = data;
    while(!(SPSR &(1<<SPIF)));          // Wait until SPI operation is terminated
    return SPDR;
}
#endif  //  HAL_USE_SPI1
// End SPI Section
