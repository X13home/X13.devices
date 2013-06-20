#ifndef _EXTAI_H
#define _EXTAI_H

uint16_t aiApin2Mask(uint8_t apin);
uint8_t cvtBase2Apin(uint16_t base);
uint8_t checkAnalogBase(uint16_t base);
void aiClean(void);
uint8_t aiCheckIdx(subidx_t * pSubidx);
uint8_t aiRegisterOD(indextable_t *pIdx);
void aiDeleteOD(subidx_t * pSubidx);

#endif
