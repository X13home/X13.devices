#ifndef _EXTAI_H
#define _EXTAI_H

void aiClean(void);
uint8_t aiCheckIdx(subidx_t * pSubidx);
uint8_t aiRegisterOD(indextable_t *pIdx);
void aiDeleteOD(subidx_t * pSubidx);

#endif
