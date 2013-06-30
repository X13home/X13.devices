

// node MAC & IP addresse
uint8_t macaddr[6];
uint8_t ipaddr[4];


void PHY_LoadConfig(void)
{
  uint8_t Len;

  Len = 6;
  ReadOD(objMACAddr, MQTTS_FL_TOPICID_PREDEF, &Len, (uint8_t *)macaddr);
  Len = 4;
  ReadOD(objIPAddr, MQTTS_FL_TOPICID_PREDEF, &Len, (uint8_t *)ipaddr);
//  ReadOD(objIPBroker, MQTTS_FL_TOPICID_PREDEF, &Len, (uint8_t *)gwip);
}

void PHY_Init(void)
{
  // Initialize PHY
}

void PHY_Start(void)
{
}

MQ_t * PHY_GetBuf(void)
{
  return NULL;
}

void PHY_Send(MQ_t * pBuf)
{
  mqRelease(pBuf);
}

void PHY_Pool(void)
{
}
