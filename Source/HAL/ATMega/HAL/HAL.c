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

// System Tick subroutine, defined in main.c
void SystemTick(void);

void INIT_SYSTEM(void)
{
    cli();

    MCUSR &= ~(1<<WDRF);
    wdt_reset();
    wdt_disable();
}

void StartSheduler(void)
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
uint16_t hal_RNG(void)
{
    static uint16_t rand16 = 0xA15E;

    // Galois LFSRs
    if(rand16 & 1)
    {
        rand16 >>= 1;
        rand16 ^= 0xB400;
    }
    else
        rand16 >>= 1;
  
    return rand16;
}

static volatile uint16_t hal_ms_counter = 0;
static volatile uint32_t hal_sec_counter = 0;        // Max Uptime 136 Jr.

uint16_t hal_get_ms(void)
{
    return hal_ms_counter;
}

uint32_t hal_get_sec(void)
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
        ticks_counter++;
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

void hal_ASleep(uint16_t duration)
{
    duration >>= 3; // Minimal Sleep Time 8S

#ifdef EXTAIN_USED
    // Disable ADC
    if(ADCSRA & (1<<ADEN))
        while(ADCSRA & (1<<ADSC));
    uint8_t oldAD = ADCSRA;
    ADCSRA &= ~(1<<ADEN);
#endif  //  EXTAIN_USED

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
    ADCSRA = oldAD;
#endif  //  EXTAIN_USED
}
#endif  //  ASLEEP

void hal_reboot(void)
{
#ifdef LED1_On
    LED1_On();
#endif

#ifdef LED2_On
    LED2_On();
#endif

    cli();
    wdt_enable(WDTO_250MS);
    while(1);
    
}
