/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.

Based on IPstack for AVR from Guido Socher and Pascal Stang
*/

typedef union
{
  uint32_t  l;
  uint16_t  i[2];
  uint8_t   c[4];
}uL2C;

typedef union
{
  uint16_t  i;
  uint8_t   c[2];
}uI2C;

extern uint8_t macaddr[6];
extern uint8_t ipaddr[4];

#ifdef ARP_MAC_resolver_client
// ARP
static uint8_t arpip_state = 0;
#define ARP_REQ_ARMED     0x80
#define ARP_REQ_READY     0x40
#define ARP_REQ_REQUEST   0x20

static uint8_t arpip[4];                // IP to find via arp

const char arpreqhdr[] PROGMEM ={0,1,8,0,6,4,0,1};

// This function will be called if we ever get a result back from the
// the arp request we sent out.
void (*client_arp_result_callback)(uint8_t*);
#endif    //  ARP_MAC_resolver_client

// IP

#ifdef ALL_clients
//static uint8_t ipnetmask[4]={255,255,255,255};
//static uint8_t iprouter[4]={0,0,0,0};   // Gateway IP
static uint8_t ipid = 0x2;

const char iphdr[] PROGMEM ={0x45, 0, 0, 0x82, 0, 0, 0x40, 0, 0x20};
// 0x45     - IP Header Version - 4 | Len = 5 * 32bit
// 0,       - Differentiated Services Code Point | Explicit Congestion Notification
// 0, 0x82  - Total Length - 130 
// 0, 0     - Identification
// 0x40, 0  - Flags:  Don't Fragment | Fragment Offset
// 0x20     - Time To Live(Hops) - 32 Sec.
#endif    //    ALL_clients

// DHCP
#ifdef  DHCP_client
#ifndef UDP_client
#error "ERROR: you need to enable UDP_client support in ip_config.h to use the DHCP client"
#endif

static volatile uint8_t dhcp_sec_cnt = 0;       // Second counts
static uint8_t dhcp_opt_server_id[4]={0,0,0,0}; // server ip
//static uint8_t dhcp_opt_message_type=0;
static uint8_t dhcp_tid = 0;                    // Transaction ID
static uint16_t dhcp_opt_renewtime_minutes = 0xFFFF;
const char dhcp_magic_cookies[] PROGMEM = {0x63, 0x82, 0x53, 0x63};
//const char dhcp_opt_req_lst[] PROGMEM = {53, 1, 1, 55, 2, 1, 3, 255};
const char dhcp_opt_req_lst[] PROGMEM = {53, 1, 1, 255};
const char dhcp_opt_53[] PROGMEM = {53, 1, 3};

static void send_dhcp_renew_request(uint8_t *buf, uint8_t *yiaddr);
static uint8_t is_dhcp_msg_for_me(uint8_t *buf,uint16_t plen);
static uint8_t dhcp_get_message_type(uint8_t *buf,uint16_t plen);
static uint8_t dhcp_is_renew_tid(uint8_t *buf);
static void dhcp_option_parser(uint8_t *buf,uint16_t plen);
#endif  //  DHCP_client

// The Ip checksum is calculated over the ip header only starting
// with the header length field and a total length of 20 bytes
// unitl ip.dst
// You must set the IP checksum field to zero before you start
// the calculation.
// len for ip is 20.
//
// For UDP/TCP we do not make up the required pseudo header. Instead we 
// use the ip.src and ip.dst fields of the real packet:
// The udp checksum calculation starts with the ip.src field
// Ip.src=4bytes,Ip.dst=4 bytes,Udp header=8bytes + data length=16+len
// In other words the len here is 8 + length over which you actually
// want to calculate the checksum.
// You must set the checksum field to zero before you start
// the calculation.
// The same algorithm is also used for udp and tcp checksums.
// len for udp is: 8 + 8 + data length
// len for tcp is: 4+4 + 20 + option len + data length
//
// For more information on how this algorithm works see:
// http://www.netfor2.com/checksum.html
// http://www.msc.uky.edu/ken/cs471/notes/chap3.htm
// The RFC has also a C code example: http://www.faqs.org/rfcs/rfc1071.html
static uint16_t checksum(uint8_t *buf, uint16_t len, uint8_t type)
{
  // type 0=ip , icmp
  //      1=udp
  //      2=tcp
  uL2C  sum;
  uI2C  cvt;

  if(type == 1)
  {
    sum.l = (IP_PROTO_UDP_V - 8);  // protocol udp
                            // the length here is the length of udp (data+header len)
                            // =length given to this function - (IP.scr+IP.dst length)
    sum.l += len;           // = real udp len
  }
  else if(type == 2)
  {
    sum.l = IP_PROTO_TCP_V;
                            // the length here is the length of tcp (data+header len)
                            // =length given to this function - (IP.scr+IP.dst length)
    sum.l += len - 8;         // = real tcp len
  }
  else
    sum.l = 0;
  
  // build the sum of 16bit words
  while(len > 1)
  {
    cvt.c[1] = *buf;
    cvt.c[0] = *(buf + 1);
    sum.l += cvt.i;

    buf += 2;
    len -= 2;
  }
  
  // if there is a byte left then add it (padded with zero)
  if(len)
    sum.l += ((uint16_t)(*buf))<<8;

  // now calculate the sum over the bytes in the sum
  // until the result is only 16bit long
  while(sum.i[1])
    sum.l = sum.i[0] + sum.i[1];

  // build 1's complement:
  return ~sum.i[0];
}

#if defined (ALL_clients)
/*
// returns 1 if destip must be routed via the GW. Returns 0 if destip is on the local LAN
static uint8_t route_via_gw(uint8_t *destip)
{
  uint8_t i=0;
  // we have to start at the left side of the IP:
  while(i<4)
  {
    if((destip[i] & ipnetmask[i]) != (ipaddr[i] & ipnetmask[i]))
      return(1);
    i++;
  }
  return(0);
}
*/
#endif  //  (ALL_clients)

static uint8_t eth_type_is_arp_and_my_ip(uint8_t *buf,uint16_t len)
{
  if((len < 41) || (buf[ETH_TYPE_H_P] != ETHTYPE_ARP_H_V) || (buf[ETH_TYPE_L_P] != ETHTYPE_ARP_L_V))
    return 0;

  return memcmp((const void *)&buf[ETH_ARP_DST_IP_P], (const void *)ipaddr, 4) == 0 ? 1: 0;
}

static uint8_t eth_type_is_ip_and_my_ip(uint8_t *buf,uint16_t len)
{
  if((len < 42) || 
    (buf[ETH_TYPE_H_P] != ETHTYPE_IP_H_V) || 
    (buf[ETH_TYPE_L_P] != ETHTYPE_IP_L_V) || 
    (buf[IP_HEADER_LEN_VER_P] != 0x45))       // must be IP V4 and 20 byte header
    return 0;

    return memcmp((const void *)&buf[IP_DST_P], (const void *)ipaddr, 4) == 0 ? 1: 0;
}

// make a return eth header from a received eth packet
static void make_eth(uint8_t *buf)
{
  memcpy(&buf[ETH_DST_MAC], &buf[ETH_SRC_MAC], 6);
  memcpy(&buf[ETH_SRC_MAC], macaddr, 6);
}

static void fill_ip_hdr_checksum(uint8_t *buf)
{
  uint16_t ck;
  // clear the 2 byte checksum
  buf[IP_CHECKSUM_P] = 0;
  buf[IP_CHECKSUM_P+1] = 0;
  buf[IP_FLAGS_P] = 0x40; // don't fragment
  buf[IP_FLAGS_P+1] = 0;  // fragement offset
  buf[IP_TTL_P] = 64; // ttl
  // calculate the checksum:
  ck = checksum(&buf[IP_P], IP_HEADER_LEN, 0);
  buf[IP_CHECKSUM_P] = ck>>8;
  buf[IP_CHECKSUM_P+1] = ck & 0xff;
}

// make a return ip header from a received ip packet
static void make_ip(uint8_t *buf)
{
  memcpy(&buf[IP_DST_P], &buf[IP_SRC_P], 4);
  memcpy(&buf[IP_SRC_P], ipaddr, 4);

  fill_ip_hdr_checksum(buf);
}

static void make_arp_answer_from_request(uint8_t *buf)
{
  make_eth(buf);
  buf[ETH_ARP_OPCODE_H_P] = ETH_ARP_OPCODE_REPLY_H_V;
  buf[ETH_ARP_OPCODE_L_P] = ETH_ARP_OPCODE_REPLY_L_V;
  // fill the mac addresses:
  memcpy(&buf[ETH_ARP_DST_MAC_P], &buf[ETH_ARP_SRC_MAC_P], 6);
  memcpy(&buf[ETH_ARP_SRC_MAC_P], macaddr, 6);
  memcpy(&buf[ETH_ARP_DST_IP_P], &buf[ETH_ARP_SRC_IP_P], 4);
  memcpy(&buf[ETH_ARP_SRC_IP_P], ipaddr, 4);

  enc28j60PacketSend(42, buf);  // eth+arp is 42 bytes:
}

static void make_echo_reply_from_request(uint8_t *buf,uint16_t len)
{
  make_eth(buf);
  make_ip(buf);
  buf[ICMP_TYPE_P] = ICMP_TYPE_ECHOREPLY_V;
  
  // we changed only the icmp.type field from request(=8) to reply(=0).
  // we can therefore easily correct the checksum:
  if(buf[ICMP_CHECKSUM_P] > (0xff-0x08))
    buf[ICMP_CHECKSUM_P+1]++;
  buf[ICMP_CHECKSUM_P]+=0x08;
  enc28j60PacketSend(len,buf);
}

#ifdef UDP_client
void send_udp_prepare(uint8_t *buf,uint16_t sport, const uint8_t *dip, uint16_t dport,const uint8_t *dstmac)
{
  memcpy(&buf[ETH_DST_MAC], dstmac, 6);
  memcpy(&buf[ETH_SRC_MAC], macaddr, 6);
  buf[ETH_TYPE_H_P] = ETHTYPE_IP_H_V;
  buf[ETH_TYPE_L_P] = ETHTYPE_IP_L_V;
  memcpy_P(&buf[IP_P], iphdr, 9);

  buf[IP_ID_L_P] = ipid; 
  ipid++;
  // total length field in the IP header must be set:
  buf[IP_TOTLEN_H_P] = 0;
  // done in transmit: buf[IP_TOTLEN_L_P]=IP_HEADER_LEN+UDP_HEADER_LEN+datalen;
  buf[IP_PROTO_P] = IP_PROTO_UDP_V;
  
  memcpy(&buf[IP_DST_P], dip, 4);
  memcpy(&buf[IP_SRC_P], ipaddr, 4);

  // done in transmit: fill_ip_hdr_checksum(buf);
  buf[UDP_DST_PORT_H_P] = (dport>>8);
  buf[UDP_DST_PORT_L_P] = 0xFF & dport; 
  buf[UDP_SRC_PORT_H_P] = (sport>>8);
  buf[UDP_SRC_PORT_L_P] = sport& 0xFF; 
  buf[UDP_LEN_H_P] = 0;
  // done in transmit: buf[UDP_LEN_L_P]=UDP_HEADER_LEN+datalen;
  // zero the checksum
  buf[UDP_CHECKSUM_H_P] = 0;
  buf[UDP_CHECKSUM_L_P] = 0;
}

void send_udp_transmit(uint8_t *buf,uint16_t datalen)
{
  uint16_t tmp16;
  tmp16 = IP_HEADER_LEN + UDP_HEADER_LEN + datalen;
  buf[IP_TOTLEN_L_P] = tmp16 & 0xFF;
  buf[IP_TOTLEN_H_P] = tmp16 >> 8;
  fill_ip_hdr_checksum(buf);
  tmp16 = UDP_HEADER_LEN + datalen;
  buf[UDP_LEN_L_P] = tmp16 & 0xFF;
  buf[UDP_LEN_H_P] = tmp16 >> 8;
  //
  tmp16 = checksum(&buf[IP_SRC_P], 16 + datalen, 1);
  buf[UDP_CHECKSUM_L_P] = tmp16 & 0xFF;
  buf[UDP_CHECKSUM_H_P] = tmp16 >> 8;
  enc28j60PacketSend(UDP_HEADER_LEN + IP_HEADER_LEN + ETH_HEADER_LEN + datalen, buf);
}
#endif // UDP_client

#if ARP_MAC_resolver_client
// make a arp request
// Note: you must have initialized the stack with 
// init_udp_or_www_server or client_ifconfig 
// before you can use this function
static void client_arp_whohas(uint8_t *buf, uint8_t *ip_we_search)
{
  uint8_t i = 6;
  while(i--)
  {
    buf[ETH_DST_MAC + i] = 0xFF;
    buf[ETH_ARP_DST_MAC_P + i] = 0x00;
  }

  memcpy(&buf[ETH_SRC_MAC], macaddr, 6);
  buf[ETH_TYPE_H_P] = ETHTYPE_ARP_H_V;
  buf[ETH_TYPE_L_P] = ETHTYPE_ARP_L_V;
  memcpy_P(&buf[ETH_ARP_P], arpreqhdr, 8);
        
  memcpy(&buf[ETH_ARP_SRC_MAC_P], macaddr, 6);

  memcpy(&buf[ETH_ARP_DST_IP_P], ip_we_search, 4);
  memcpy(&buf[ETH_ARP_SRC_IP_P], ipaddr, 4);

  enc28j60PacketSend(42, buf);
}

// return zero when current transaction is finished
static uint8_t get_mac_with_arp_wait(void)
{
  if(arpip_state == ARP_REQ_READY)
    return 0;
  return 1;
}

// reference_number is something that is just returned in the callback
// to make matching and waiting for a given ip/mac address pair easier
// Note: you must have initialized the stack with 
// init_udp_or_www_server or client_ifconfig 
// before you can use this function
void get_mac_with_arp(uint8_t *ip, void (*arp_result_callback)(uint8_t *mac))
{
  client_arp_result_callback = arp_result_callback;
  arpip_state = (ARP_REQ_ARMED | ARP_REQ_REQUEST);
  memcpy(arpip, ip, 4);
}
#endif  // ARP_MAC_resolver_client

uint8_t packetloop_lan(uint8_t *buf, uint16_t plen)
{
#ifdef  DHCP_client
  // we let it run a bit faster than once every minute because it is better this expires too early than too late
  if(dhcp_sec_cnt > 68)
  {
    dhcp_sec_cnt = 0;
    // count down unless the lease was infinite
    if ((dhcp_opt_renewtime_minutes != 0xFFFF) && (dhcp_opt_renewtime_minutes != 0))
      dhcp_opt_renewtime_minutes--;
  }
#endif  //  DHCP_client

  //plen will be unequal to zero if there is a valid 
  // packet (without crc error):
  if(plen == 0)
  {
#ifdef ARP_MAC_resolver_client
    if((arpip_state & (ARP_REQ_ARMED | ARP_REQ_REQUEST)) == (ARP_REQ_ARMED | ARP_REQ_REQUEST))
    {
      if(enc28j60linkup())
      {
        arpip_state &= ~ARP_REQ_REQUEST;
        client_arp_whohas(buf, arpip);
      }
      else
        arpip_state = 0;
      return 0;
    }
#endif // ARP_MAC_resolver_client
#ifdef  DHCP_client
    if((dhcp_opt_renewtime_minutes < 3) && enc28j60linkup())
    {
      dhcp_tid++;
      send_dhcp_renew_request(buf, ipaddr);
      dhcp_opt_renewtime_minutes = 5; // repeat in two minutes if no answer
      return 0;
    }
#endif  //    DHCP_client
    return 0;
  }
  else
  // arp is broadcast if unknown but a host may also
  // verify the mac address by sending it to 
  // a unicast address.
  if(eth_type_is_arp_and_my_ip(buf, plen))
  {
    if(buf[ETH_ARP_OPCODE_L_P] == ETH_ARP_OPCODE_REQ_L_V)
    {
      // is it an arp request 
      make_arp_answer_from_request(buf);
    }
#ifdef ARP_MAC_resolver_client
    else 
    if((arpip_state & ARP_REQ_ARMED) && 
       (buf[ETH_ARP_OPCODE_L_P] == ETH_ARP_OPCODE_REPLY_L_V) && // is it an arp reply
       (memcmp(&buf[ETH_ARP_SRC_IP_P], arpip, 4) == 0))         // arp reply for the IP we were searching
    {
      arpip_state = ARP_REQ_READY;
      (*client_arp_result_callback)(buf+ETH_ARP_SRC_MAC_P);
    }
#endif // ARP_MAC_resolver_client
    return(0);
  }
#ifdef  DHCP_client
  if(is_dhcp_msg_for_me(buf, plen))  // we check the dhcp_renew_tid because if 
  {
    if((dhcp_get_message_type(buf,plen) == 5) &&    // DHCPACK = 5, success, DHCPACK, we have the IP
       (dhcp_is_renew_tid(buf)))
    { 
      dhcp_option_parser(buf,plen); // get new lease time, it will as well GW and netmask but those should not change
    }
    return(0);
  }
#endif
  // check if ip packets are for us:
  if(eth_type_is_ip_and_my_ip(buf,plen))
  {
    if((buf[IP_PROTO_P] == IP_PROTO_ICMP_V) && (buf[ICMP_TYPE_P] == ICMP_TYPE_ECHOREQUEST_V))
    {
      // a ping packet, let's send pong
      make_echo_reply_from_request(buf,plen);
      return 0;
    }
    return 1; // Packet are for us
  }
  return 2; // maybe Broadcast
}

#ifdef  DHCP_client
// 
// The relevant RFCs are 
// DHCP protocol: http://tools.ietf.org/html/rfc1541
// newer version:
// DHCP protocol: http://tools.ietf.org/html/rfc2131
// message encoding: http://tools.ietf.org/html/rfc1533
//
// The normal message flow to get an IP address is:
// Client -> Server DHCPDISCOVER
// Server -> Client DHCPOFFER
// Client -> Server DHCPREQUEST
// Server -> Client DHCPACK
// There might be serveral seconds delay between the DHCPDISCOVER and
// the DHCPOFFER as the server does some test (e.g ping or arp) to see
// if the IP which is offered is really free.
//
// At lease renewal the message flow is:
// Client -> Server DHCPREQUEST
// Server -> Client DHCPACK
// The DHCPREQUEST for renewal is different from the one at inital assignment.
//
//

// This function writes a basic message template into buf
// It processes all fields excluding the options section.
// Most fields are initialized with zero.
static void make_dhcp_message_template(uint8_t *buf)
{
  uint8_t i;
  uint8_t allxff[6]={0xff,0xff,0xff,0xff,0xff,0xff}; // all of it can be used as mac, the first 4 can be used as IP

  send_udp_prepare(buf, DHCP_SRV_DST_PORT, allxff, DHCP_SRV_SRC_PORT, allxff);
  // source IP is 0.0.0.0, a while loop produces smaller code than memset
  i = 0;
  while(i < 4)
  {
    buf[IP_SRC_P + i]=0;
    i++;
  }
  // now fill the bootstrap protocol layer starting at UDP_DATA_P
  buf[UDP_DATA_P]   = 1;  // message type = boot request
  buf[UDP_DATA_P+1] = 1;  // hw type - Ethernet
  buf[UDP_DATA_P+2] = 6;  // mac len
  buf[UDP_DATA_P+3] = 0;  // hops
  // we use only one byte TIDs, we fill the first byte with 1 and
  // the rest with transactionID. The first byte is used to
  // distinguish inital requests from renew requests.
  buf[UDP_DATA_P + 4] = 1;
  i=0;
  while(i<3)
  {
    buf[UDP_DATA_P + i + 5] = dhcp_tid ^ macaddr[i + 3];
    i++;
  }
  // 2 Number of Seconds
  // 2 Flags
  // 4 Client IP Addr
  // 4 Your IP Addr
  // 4 Server IP Addr
  // 4 Gateway IP Addr
  i=8; // start at 8
  while(i<28)
  {
    buf[UDP_DATA_P + i]=0;
    i++;
  }
  // own mac
  memcpy(&buf[UDP_DATA_P + 28], &buf[ETH_SRC_MAC], 6);
  
  // 10  bytes: Rest of 'Client hardware address'
  // 64  bytes: Server Host Name
  // 128 bytes: Boot filename
  i = 34;
  while(i < 236)
  {
    buf[UDP_DATA_P + i] = 0;
    i++;
  }
  // the magic cookie has to be:
  // 99, 130, 83, 99
  memcpy_P(&buf[UDP_DATA_P + MAGIC_COOKIE_P], dhcp_magic_cookies, 4);
}

// the answer to this message will come as a broadcast
static void send_dhcp_discover(uint8_t *buf)
{
  make_dhcp_message_template(buf);
  // option dhcp message type:
  memcpy_P(&buf[UDP_DATA_P + DHCP_OPTION_OFFSET], dhcp_opt_req_lst, sizeof(dhcp_opt_req_lst));
  // no padding
  send_udp_transmit(buf, (DHCP_OPTION_OFFSET + sizeof(dhcp_opt_req_lst)));
}

// scan the options field for the message type field
// and return its value.
//
// Value   Message Type
// -----   ------------
//   1     DHCPDISCOVER
//   2     DHCPOFFER  (server to client)
//   3     DHCPREQUEST
//   4     DHCPDECLINE
//   5     DHCPACK  (server to client)
//   6     DHCPNAK  (server to client)
//   7     DHCPRELEASE
// return 0 on message type not found otherwise the numeric
// value for the message type as shown in the table above.
static uint8_t dhcp_get_message_type(uint8_t *buf,uint16_t plen)
{
  uint16_t option_idx;
  uint8_t option_len;
  // options are coded in the form: option_type,option_len,option_val
  option_idx = (UDP_DATA_P + DHCP_OPTION_OFFSET);
  while((option_idx + 2) < plen)
  {
    option_len = buf[option_idx + 1];
    if((option_len < 1) || ((option_idx + option_len + 1) > plen))
      break;
    if(buf[option_idx] == 53)
    {
      // found message type, return it:
      return(buf[option_idx + 2]);
    }
    option_idx += 2 + option_len;
  }
  return(0);
}

// this will as well update ipaddr
static uint8_t is_dhcp_msg_for_me(uint8_t *buf,uint16_t plen)
{
  if((plen >= (UDP_DATA_P + DHCP_OPTION_OFFSET + 3)) &&   // the smallest option is 3 bytes
     (buf[UDP_SRC_PORT_L_P] == DHCP_SRV_SRC_PORT) &&
     (buf[UDP_DATA_P] == 2) && 
     (buf[UDP_DATA_P+5] == (dhcp_tid ^ macaddr[3])) &&
     (buf[UDP_DATA_P+6] == (dhcp_tid ^ macaddr[4])))
        return(1);

  return(0);
}

// check if this message was part of a renew or 
static uint8_t dhcp_is_renew_tid(uint8_t *buf)
{
  if(buf[UDP_DATA_P + 4] == 2)
    return(1); // we did set first byte in transaction ID to 2 to indicate renew request. This trick makes the processing of the DHCPACK message easier.
  return(0);
}

static void dhcp_option_parser(uint8_t *buf,uint16_t plen)
{
  uint16_t option_idx;
  uint8_t option_len;
  uL2C ltime;

  // the smallest option is 3 bytes
  if(plen < (UDP_DATA_P + DHCP_OPTION_OFFSET + 3))
    return;
  // options are coded in the form: option_type,option_len,option_val
  option_idx = UDP_DATA_P + DHCP_OPTION_OFFSET;
  while((option_idx + 2) < plen)
  {
    option_len = buf[option_idx+1];
    if((option_len < 1) || ((option_idx + option_len + 1) > plen))
      break;
    
    switch(buf[option_idx])
    {
      case 0:
        option_idx = plen; // stop loop, we are reading some padding bytes here (should not happen)
        break;
/*      case 1:
        if(option_len == 4)
          memcpy(ipnetmask, &buf[option_idx + 2], 4);
        break;
      case 3:
        if(option_len == 4)
          memcpy(iprouter, &buf[option_idx + 2], 4);
        break;*/
      // Lease time: throughout the protocol, times are to 
      // be represented in units of seconds.  The time value 
      // of 0xffffffff is reserved to represent "infinity". 
      // The max lease time size is therefore 32 bit. 
      // The code for this option is 51, and its length is 4
      // as per RFC 1533.
      case 51:
        if(option_len == 4)
        {
          ltime.c[0] = buf[option_idx + 5];
          ltime.c[1] = buf[option_idx + 4];
          ltime.c[2] = buf[option_idx + 3];
          ltime.c[3] = buf[option_idx + 2];

          if(ltime.l == 0xFFFFFFFF)
          {
            dhcp_opt_renewtime_minutes = 0xFFFF;
            break; // end of switch
          }
          
          ltime.l >>= 7; // Renew Time is usualy half of Lease Time
          if(ltime.l > 0xFFFE)
            ltime.l = 0xFFFE;
        }
        else
          ltime.l = 1440; // 24 * 60 min

        dhcp_opt_renewtime_minutes = ltime.i[0];
        if(dhcp_opt_renewtime_minutes < 5)
          dhcp_opt_renewtime_minutes = 5;
        break;
//    case 53: dhcp_opt_message_type=buf[option_idx+2];
//      break;
                        // rfc 2131: A DHCP server always returns its 
                        // own address in the 'server identifier' option
      case 54:
        if(option_len == 4)
          memcpy(dhcp_opt_server_id, &buf[option_idx + 2], 4);
        break;
    }
    option_idx += 2 + option_len;
  }
}

// the answer to this message will come as a broadcast
static void send_dhcp_request(uint8_t *buf)
{
  make_dhcp_message_template(buf);
  // option dhcp message type:
  memcpy_P(&buf[UDP_DATA_P+DHCP_OPTION_OFFSET], dhcp_opt_53, sizeof(dhcp_opt_53));
  uint8_t i = sizeof(dhcp_opt_53);

  if(dhcp_opt_server_id[0] != 0)
  {
    buf[UDP_DATA_P+DHCP_OPTION_OFFSET+i]=0x36; // 54=server identifier
    buf[UDP_DATA_P+DHCP_OPTION_OFFSET+i+1]=4; // len
    memcpy(buf+UDP_DATA_P+DHCP_OPTION_OFFSET+i+2,dhcp_opt_server_id, 4);
    i += 6;
  }
  
  if(ipaddr[0] != 0xFF)
  {
    buf[UDP_DATA_P+DHCP_OPTION_OFFSET+i]=0x32; // 50=requested IP address
    buf[UDP_DATA_P+DHCP_OPTION_OFFSET+i+1]=4; // len
    memcpy(buf+UDP_DATA_P+DHCP_OPTION_OFFSET+i+2,ipaddr, 4);
    i += 6;
  }

  memcpy_P(&buf[UDP_DATA_P + DHCP_OPTION_OFFSET + i], &dhcp_opt_req_lst[3],
                                                              sizeof(dhcp_opt_req_lst) - 3);
  i += (sizeof(dhcp_opt_req_lst) - 3);

  send_udp_transmit(buf, DHCP_OPTION_OFFSET + i);
}

// The renew procedure is described in rfc2131. 
// We send DHCPREQUEST and 'server identifier' MUST NOT be filled 
// in, 'requested IP address' option MUST NOT be filled in, 'ciaddr' 
// MUST be filled. 
// The rfc suggest that I can send the DHCPREQUEST in this case as
// a unicast message and not as a broadcast message but test with
// various DHCP servers show that not all of them listen to
// unicast. We send therefor a broadcast message but we expect
// a unicast answer directly to our mac and IP.
static void send_dhcp_renew_request(uint8_t *buf, uint8_t *yiaddr)
{
  make_dhcp_message_template(buf);
  buf[UDP_DATA_P+4] = 2; // set first byte in transaction ID to 2 to indicate renew_request. This trick makes the processing of the DHCPACK message easier.
  // source IP must be my IP since we renew
  memcpy(buf+IP_SRC_P, yiaddr, 4); // ip level source IP
  memcpy(buf+UDP_DATA_P+12,yiaddr, 4); // ciaddr
  // option dhcp message type:
  memcpy_P(&buf[UDP_DATA_P + DHCP_OPTION_OFFSET], dhcp_opt_53, sizeof(dhcp_opt_53));
  buf[UDP_DATA_P + DHCP_OPTION_OFFSET + sizeof(dhcp_opt_53)] = 0xFF;
  // no option paramter request list is needed at renew
  send_udp_transmit(buf,(DHCP_OPTION_OFFSET + sizeof(dhcp_opt_53) + 1));
  // we will get a unicast answer, reception of broadcast packets is turned off
}

// Initial_tid can be a random number for every board. E.g the last digit
// of the mac address. It is not so important that the number is random.
// It is more important that it is unique and no other board on the same
// Lan has the same number. This is because there might be a power outage
// and all boards reboot afterwards at the same time. At that moment they
// must all have different TIDs otherwise there will be an IP address mess-up.
//
// The function returns 1 once we have a valid IP. 
// At this point you must not call the function again.
uint8_t packetloop_dhcp_initial_ip_assignment(uint8_t *buf, uint16_t plen)
{
  if(!enc28j60linkup()) // do nothing if the link is down
  {
    dhcp_sec_cnt = 0;
    return(0);
  }

  if(plen == 0)
  {
    if(dhcp_sec_cnt < 2)
    {
      dhcp_sec_cnt = 25;
      uint8_t i = 0;
      while(i < 5)
        dhcp_tid += macaddr[i++];

      enc28j60EnableBroadcast();
    }
    else if(dhcp_sec_cnt > 31)
    {
      dhcp_sec_cnt = 2;
      dhcp_tid++;
      send_dhcp_discover(buf);
    }
    return 0;
  }
  else if(is_dhcp_msg_for_me(buf, plen))
  {
    // It's really a borderline case that we the the dhcp_is_renew_tid
    // function call for. It could only happen if the board is power cyceled 
    // during operation.
    if(dhcp_is_renew_tid(buf) == 1) // should have been initial tid, just return
      return(0);
   
    uint8_t cmd;
    cmd = dhcp_get_message_type(buf, plen);
    if(cmd==2) // DHCPOFFER =2
    {
      dhcp_sec_cnt = 2;

      if(buf[UDP_DATA_P+16] != 0) // we have a yiaddr
        memcpy(ipaddr, buf+UDP_DATA_P+16, 4);

      dhcp_option_parser(buf, plen);

      // answer offer with a request:
      send_dhcp_request(buf);
    }
    else if(cmd==5) // DHCPACK = 5, success, we have the IP
    {
      dhcp_sec_cnt = 2;
      enc28j60DisableBroadcast();
      return(1);
    }
  }
  return(0);
}
#endif  //  DHCP_client
// === end of DHCP client

void sec_tick_lan(void)
{
#ifdef ARP_MAC_resolver_client
  if((arpip_state & ARP_REQ_ARMED) && (arpip_state < (ARP_REQ_ARMED | ARP_REQ_REQUEST)))
    arpip_state++;
#endif  //  ARP_MAC_resolver_client

#ifdef  DHCP_client
  dhcp_sec_cnt++;
#endif  //  DHCP_client
}
/* end of ip_arp_udp.c */
