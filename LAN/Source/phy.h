#ifndef _PHY_H
#define _PHY_H

void PHY_Pool(void);
void PHY_Init(void);
void PHY_LoadConfig(void);
void PHY_Start(void);
MQ_t * PHY_GetBuf(void);
void PHY_Send(MQ_t * pBuf);
uint8_t PHY_BuildName(uint8_t * pBuf);

#endif  //  _PHY_H