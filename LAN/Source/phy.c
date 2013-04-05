#include "config.h"
#include "util.h"

#ifdef RF_NODE
uint8_t rf_dst_addr;
uint8_t rf_src_addr;

#endif  //  RF_NODE

#ifdef LAN_NODE
uint8_t macaddr[6];
uint8_t ipaddr[4];

static uint8_t gwip[] =  {0xFF,0xFF,0xFF,0xFF};
static uint8_t gwmac[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

// eth/ip buffer:
#define MAX_FRAME_BUF 350
static uint8_t buf[MAX_FRAME_BUF];
#endif  //  LAN_NODE

void PHY_Init(void)
{
#ifdef ENC28J60
  //initialize enc28j60
  enc28j60Init(macaddr);
#endif
#ifdef RF_NODE
  rf_Initialize();
#endif  //  RF_NODE
}

void PHY_LoadConfig(void)
{
  uint8_t Len;

#ifdef RF_NODE
  // (Re)Load RF Configuration
  uint16_t uiTmp;
  uint8_t channel, NodeID;
  Len = sizeof(uint16_t);
  // Load config data
  ReadOD(objRFGroup, MQTTS_FL_TOPICID_PREDEF,   &Len, (uint8_t *)&uiTmp);
  ReadOD(objRFNodeId, MQTTS_FL_TOPICID_PREDEF,  &Len, &NodeID);
  ReadOD(objRFChannel, MQTTS_FL_TOPICID_PREDEF, &Len, &channel);
  rf_LoadCfg(channel, uiTmp, NodeID);
#endif  //  RF_NODE

#ifdef LAN_NODE
  Len = 6;
  ReadOD(objMACAddr, MQTTS_FL_TOPICID_PREDEF, &Len, (uint8_t *)macaddr);
  Len = 4;
  ReadOD(objIPAddr, MQTTS_FL_TOPICID_PREDEF, &Len, (uint8_t *)ipaddr);
//  ReadOD(objIPBroker, MQTTS_FL_TOPICID_PREDEF, &Len, (uint8_t *)gwip);
#endif  //  LAN_NODE
}

void PHY_Pool(void)
{
#ifdef RF_NODE
  rf_Pool();
#endif  // RF_NODE
#ifdef LAN_NODE
  static uint8_t PoolCnt = POOL_TMR_FREQ - 1;

  if(PoolCnt)
    PoolCnt--;
  else
  {
    PoolCnt = POOL_TMR_FREQ - 1;
    ip_arp_sec_tick();
  }
#endif  //  LAN_NODE
}

void PHY_Start(void)
{
#ifdef LAN_NODE
#ifdef  DHCP_client
  if(ipaddr[0] == 0xFF)
  {
    // DHCP handling. Get the initial IP
    uint8_t i;
    uint16_t plen;

    i = 0;
    while(i == 0)
    {
      plen = enc28j60PacketReceive(MAX_FRAME_BUF, buf);
      i = packetloop_dhcp_initial_ip_assignment(buf, plen);
    }
  }
#else
  while (enc28j60linkup()==0)
    _delay_ms(250);
#endif  //  DHCP_client
#endif  //  LAN_NODE
}

MQ_t * PHY_GetBuf(void)
{
#ifdef LAN_NODE
  uint16_t plen;
  MQ_t * pTmp;
  
  plen = enc28j60PacketReceive(MAX_FRAME_BUF, buf);
  if(packetloop_arp_icmp(buf, plen))    // 1 - Unicast, 2 - Broadcast
  {
    if((buf[UDP_DST_PORT_H_P] == (MQTTS_UDP_PORT >> 8)) &&
       (buf[UDP_DST_PORT_L_P] == (MQTTS_UDP_PORT & 0xFF)))
    {
      if(gwip[0] == 0xFF)
      {
        if(buf[UDP_DATA_P + 1] == MQTTS_MSGTYP_GWINFO)
        {
          memcpy(gwmac, &buf[ETH_SRC_MAC], 6);
          memcpy(gwip,  &buf[IP_SRC_P], 4);
        }
        else
          return NULL;
      }
      
      plen -= UDP_DATA_P;
      if((plen > sizeof(MQ_t)) || ((pTmp = mqAssert()) == NULL))
        return NULL;
      memcpy(pTmp, &buf[UDP_DATA_P], plen);
      return pTmp;
    }
  }
#endif  //  LAN_NODE
#ifdef RF_NODE
  MQ_t * pTmp;
  pTmp = (MQ_t *)rf_GetBuf(&rf_src_addr);
  if(pTmp->MsgType == MQTTS_MSGTYP_GWINFO)
    rf_dst_addr = rf_src_addr;

  return pTmp;
#endif  //  RF_NODE
}

void PHY_Send(MQ_t * pBuf)
{
#ifdef LAN_NODE
  if(pBuf->MsgType == MQTTS_MSGTYP_SEARCHGW)
  {
    uint8_t i = 0;
    while(i < 4)
    {
      gwip[i] = 0xFF;
      gwmac[i] = 0xFF;
      i++;
    }
    gwmac[4] = 0xFF;
    gwmac[5] = 0xFF;
  }
  send_udp_prepare(buf, MQTTS_UDP_PORT, gwip, MQTTS_UDP_PORT, gwmac);
  
  uint16_t datalen = pBuf->Length;

  memcpy(&buf[UDP_DATA_P], pBuf, datalen);
  mqRelease(pBuf);
  
  send_udp_transmit(buf, datalen);
#endif  //  LAN_NODE
#ifdef RF_NODE
  if(pBuf->MsgType == MQTTS_MSGTYP_SEARCHGW)
    rf_dst_addr = 0;

  rf_Send((uint8_t *)pBuf, &rf_dst_addr);
#endif  //  RF_NODE
}

uint8_t PHY_BuildName(uint8_t * pBuf)
{
#ifdef LAN_NODE
  sprinthex(&pBuf[0], ipaddr[0]);
  sprinthex(&pBuf[2], ipaddr[1]);
  sprinthex(&pBuf[4], ipaddr[2]);
  sprinthex(&pBuf[6], ipaddr[3]);

  return 8;
#endif  //  LAN_NODE
#ifdef RF_NODE
  sprinthex(&pBuf[0], rf_GetNodeID());
  return 2;
#endif  //  RF_NODE
}
