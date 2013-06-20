#ifndef _EXTSER_H
#define _EXTSER_H

void serClean(void);
uint8_t serCheckIdx(subidx_t * pSubidx);
uint8_t serRegisterOD(indextable_t *pIdx);
void serDeleteOD(subidx_t * pSubidx);

#endif
