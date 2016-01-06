#ifndef __HAL_H
#define __HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <avr/io.h>
#include <avr/eeprom.h>

#define ENTER_CRITICAL_SECTION()    asm volatile ( "in      __tmp_reg__, __SREG__" :: );    \
                                    asm volatile ( "cli" :: );                              \
                                    asm volatile ( "push    __tmp_reg__" :: )

#define LEAVE_CRITICAL_SECTION()    asm volatile ( "pop     __tmp_reg__" :: );              \
                                    asm volatile ( "out     __SREG__, __tmp_reg__" :: )

#define eeprom_init_hw()
#define eeprom_read(pBuf, Addr, Len)  eeprom_read_block((void *)pBuf, (const void *)Addr, (size_t)Len)
#define eeprom_write(pBuf, Addr, Len) eeprom_write_block((const void *)pBuf, (void *)Addr, (size_t)Len)

// AVR Architecture specifics.
#define portBYTE_ALIGNMENT          1
#define portPOINTER_SIZE_TYPE       uintptr_t
#define configTOTAL_HEAP_SIZE       1024

//////////////////////////////////////////////////////////////
// DIO Section
#define DIO_PORT_POS                3
#define DIO_PORT_MASK               0x07
#define DIO_PORT_TYPE               uint8_t

// DIO Types
typedef enum
{
    DIO_MODE_IN_FLOAT   = 0,
    DIO_MODE_IN_PD      = 0,
    DIO_MODE_IN_PU      = 0x01,

    DIO_MODE_OUT_PP     = 0x08,
    
    DIO_MODE_AIN        = 0x18
}DIOmode_e;
// DIO Section
//////////////////////////////////////////////////////////////

void     INIT_SYSTEM(void);
void     StartSheduler(void);

uint16_t hal_get_ms(void);
uint32_t hal_get_sec(void);
uint16_t hal_RNG(void);
void     hal_reboot(void);

void     hal_ASleep(uint16_t duration);

#ifdef __cplusplus
}
#endif

#endif  //  __HAL_H