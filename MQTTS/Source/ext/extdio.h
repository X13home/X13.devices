#ifndef _EXTDIO_H
#define _EXTDIO_H

void dioClean(void);
uint8_t dioCheckIdx(subidx_t * pSubidx);
uint8_t dioRegisterOD(indextable_t *pIdx);
void dioDeleteOD(subidx_t * pSubidx);

#endif
