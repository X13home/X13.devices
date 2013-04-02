#ifndef _PHY_H
#define _PHY_H

void LanPool(void);
void LAN_Init(void);
MQ_t * LAN_GetBuf(void);
void LAN_Send(MQ_t * pBuf);

#endif  //  _PHY_H