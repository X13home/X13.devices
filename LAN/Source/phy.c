#include "config.h"

#if (defined ENC28J60)
#include "Phy\ENC28J60\ip_arp_udp_tcp.h"
#include "Phy\ENC28J60\enc28j60.h"
#include "Phy\ENC28J60\net.h"

uint8_t macaddr[] = {0x00,0x04,0xA3,0x00,0x00,0x01}; // Microchip

static uint8_t gwip[] = {0xFF,0xFF,0xFF,0xFF};
static uint8_t gwmac[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

// eth/ip buffer:
static uint8_t buf[MAX_FRAMELEN];

void LanPool(void)
{
  static uint8_t PoolCnt = POOL_TMR_FREQ;

  if(PoolCnt)
    PoolCnt--;
  else
  {
    PoolCnt = POOL_TMR_FREQ;
    ip_arp_sec_tick();
  }
}

void LAN_Init(void)
{
  //initialize enc28j60
  enc28j60Init(macaddr);

#ifdef  DHCP_client
  // DHCP handling. Get the initial IP
  uint8_t i;
  uint16_t plen;

  i = 0;
  while(i == 0)
  {
    plen = enc28j60PacketReceive(MAX_FRAMELEN, buf);
    i = packetloop_dhcp_initial_ip_assignment(buf, plen, macaddr[5]);
  }
#else
  while (enc28j60linkup()==0)
    _delay_ms(250);
#endif  //  DHCP_client
}

MQ_t * LAN_GetBuf(void)
{
  uint16_t plen;
  uint8_t type;
  MQ_t * pTmp;
  
  plen = enc28j60PacketReceive(MAX_FRAMELEN, buf);
#ifdef  DHCP_client
  // DHCP renew IP:
  plen = packetloop_dhcp_renewhandler(buf, plen);
#endif  //  DHCP_client
  if((type = packetloop_arp_icmp(buf, plen)) != 0)    // 1 - Unicast, 2 - Broadcast
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

  return NULL;
}

void LAN_Send(MQ_t * pBuf)
{
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
}

#endif  //  (defined ENC28J60)