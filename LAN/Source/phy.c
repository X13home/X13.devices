#include "config.h"

#if (defined ENC28J60)
#include "Phy\ENC28J60\ip_arp_udp_tcp.h"
#include "Phy\ENC28J60\enc28j60.h"
#include "Phy\ENC28J60\net.h"

uint8_t macaddr[6] = {0x00,0x04,0xA3,0x00,0x00,0x01}; // Microchip

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
  plen = enc28j60PacketReceive(MAX_FRAMELEN, buf);
#ifdef  DHCP_client
  // DHCP renew IP:
  plen = packetloop_dhcp_renewhandler(buf, plen);
#endif  //  DHCP_client
  type = packetloop_arp_icmp(buf, plen);
  if(type == 1)       // Packet for us
  {
  }
  else if(type == 2)  // Broadcast
  {
  }


/*
  
  type = 


  {
   uint8_t * pTmp;
                    pTmp = (uint8_t *)mqAssert();
  }
*/

  return NULL;
}

#endif  //  (defined ENC28J60)