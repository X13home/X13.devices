// API
// Load/Change configuration parameters

static uint8_t dummy_NodeID;


void PHY_LoadConfig(void)
{
  uint8_t Len = sizeof(dummy_NodeID);
  ReadOD(objRFNodeId, MQTTS_FL_TOPICID_PREDEF,  &Len, &dummy_NodeID);
}

void PHY_Init(void)
{}

#ifdef ASLEEP
// Change state
void rf_SetState(uint8_t state)
{}
#endif

uint8_t rf_GetNodeID(void)
{
    return dummy_NodeID;
}

MQ_t * PHY_GetBuf(void)
{
  return NULL;
}

// Can Send ? Then swich to TX mode
uint8_t PHY_CanSend(void)
{
  return 1;
}

void PHY_Send(MQ_t * pBuf)
{
  mqRelease(pBuf);
}

void PHY_Pool(void)
{}
