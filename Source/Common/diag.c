#include "config.h"

#ifdef DIAG_USED

#define DIAG_DEF_TIMEOUT    300

static uint32_t diag_timeout = 0;

void DIAG_Poll(void)
{
    if((hal_get_sec() - diag_timeout) < DIAG_DEF_TIMEOUT)
        return;

    diag_timeout = hal_get_sec();

    uint16_t min, max, act;
    mqGetHeapStat(&act, &max, &min);
    
    MQ_t * pMessage = mqAlloc(sizeof(MQ_t));
    
    pMessage->mq.publish.Data[0] = (min >> 8);
    pMessage->mq.publish.Data[1] = min & 0xFF;
    pMessage->mq.publish.Data[2] = (act >> 8);
    pMessage->mq.publish.Data[3] = act & 0xFF;
    pMessage->mq.publish.Data[4] = (max >> 8);
    pMessage->mq.publish.Data[5] = max & 0xFF;
    
    pMessage->Length = 6;
    
    mqttsn_trace_msg(lvlINFO, pMessage);
}

#endif  //  DIAG_USED
