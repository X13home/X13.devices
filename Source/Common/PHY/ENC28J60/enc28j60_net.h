/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _NET_H
#define _NET_H

// Configuration section
#define NET_WITH_ICMP
#define NET_WITH_DHCP

#if ((defined NET_WITH_DHCP) && (MQTTSN_MSG_SIZE < 72))
    #define MAX_FRAME_BUF   (sizeof(eth_frame_t) + sizeof(ip_packet_t) + sizeof(udp_packet_t) + 72)
#else
    #define MAX_FRAME_BUF   (sizeof(eth_frame_t) + sizeof(ip_packet_t) + sizeof(udp_packet_t) + MQTTSN_MSG_SIZE)
#endif  //  DHCP

#define htons(x)   (((x)&0x00FF)<<8)+((x)>>8)
#define htonl(x)   ((((x)&0xFF000000)>>24)|(((x)&0x00FF0000)>>8)|(((x)&0x0000FF00)<<8)|(((x)&0x000000FF)<<24))
#define ntohs(x)    htons(x)
#define ntohl(x)    htonl(x)

#define IP_PACKET_TTL       64

#define MQTTSN_UDP_PORT     htons(1883)

// Ethernet Section
#define ETH_TYPE_ARP        htons(0x0806)
#define ETH_TYPE_IP         htons(0x0800)

typedef struct eth_frame
{
    uint8_t     target_mac[6];
    uint8_t     sender_mac[6];
    uint16_t    type;
    uint8_t     data[];
} eth_frame_t;

// ARP Section
#define ARP_HW_TYPE_ETH     htons(0x0001)
#define ARP_PROTO_TYPE_IP   htons(0x0800)

#define ARP_TYPE_REQUEST    htons(1)
#define ARP_TYPE_RESPONSE   htons(2)

typedef struct arp_message
{
    uint16_t    hw_type;
    uint16_t    proto_type;
    uint8_t     hw_addr_len;
    uint8_t     proto_addr_len;
    uint16_t    opcode;
    uint8_t     sender_mac[6];
    uint8_t     sender_ip[4];
    uint8_t     target_mac[6];
    uint8_t     target_ip[4];
} arp_message_t;

// IP Section
#define IP_PROTOCOL_ICMP    1
#define IP_PROTOCOL_TCP     6
#define IP_PROTOCOL_UDP     17

typedef struct ip_packet
{
    uint8_t     ver_head_len;
    uint8_t     tos;
    uint16_t    total_len;
    uint16_t    fragment_id;
    uint16_t    flags_framgent_offset;
    uint8_t     ttl;
    uint8_t     protocol;
    uint16_t    cksum;
    uint8_t     sender_ip[4];
    uint8_t     target_ip[4];
    uint8_t     data[];
} ip_packet_t;

// ICMP Section
#define ICMP_TYPE_ECHO_RQ   8
#define ICMP_TYPE_ECHO_RPLY 0

typedef struct icmp_echo_packet
{
    uint8_t     type;
    uint8_t     code;
    uint16_t    cksum;
    uint16_t    id;
    uint16_t    seq;
    uint8_t     data[];
} icmp_echo_packet_t;

// UDP Section
typedef struct udp_packet
{
    uint16_t    sender_port;
    uint16_t    target_port;
    uint16_t    len;
    uint16_t    cksum;
    uint8_t     data[];
} udp_packet_t;

// DHCP Section
#define DHCP_SERVER_PORT        htons(67)
#define DHCP_CLIENT_PORT        htons(68)

#define DHCP_OP_REQUEST         1
#define DHCP_OP_REPLY           2

#define DHCP_HW_ADDR_TYPE_ETH   1

#define DHCP_FLAG_BROADCAST     htons(0x8000)

#define DHCP_MAGIC_COOKIE       htonl(0x63825363)

#define SIZEOF_DHCP_MESSAGE     240
typedef struct dhcp_message
{
    uint8_t     operation;          //  0
    uint8_t     hw_addr_type;       //  1
    uint8_t     hw_addr_len;        //  2
    uint8_t     unused1;            //  3
    uint8_t     transaction_id[4];  //  4
    uint16_t    second_count;       //  8
    uint16_t    flags;              //  10
    uint8_t     client_addr[4];     //  12
    uint8_t     offered_addr[4];    //  16
    uint8_t     server_addr[4];     //  20
    uint32_t    unused2;            //  24
    uint8_t     hw_addr[16];        //  28
    //uint8_t     unused3[192];       //  44
    //uint8_t     magic_cookie[4];    //  236
    uint8_t     options[];          //  240
} dhcp_message_t;

#define DHCP_CODE_PAD           0
#define DHCP_CODE_END           255
#define DHCP_CODE_SUBNETMASK    1
#define DHCP_CODE_ROUTER        3
#define DHCP_CODE_REQUESTEDADDR 50
#define DHCP_CODE_LEASETIME     51
#define DHCP_CODE_MESSAGETYPE   53
#define DHCP_CODE_DHCPSERVER    54
#define DHCP_CODE_REQUESTLIST   55
#define DHCP_CODE_RENEWTIME     58
#define DHCP_CODE_REBINDTIME    59

typedef struct
{
    uint8_t     code;
    uint8_t     len;
    uint8_t     data[];
} dhcp_option_t;

#define DHCP_MESSAGE_DISCOVER   1
#define DHCP_MESSAGE_OFFER      2
#define DHCP_MESSAGE_REQUEST    3
#define DHCP_MESSAGE_DECLINE    4
#define DHCP_MESSAGE_ACK        5
#define DHCP_MESSAGE_NAK        6
#define DHCP_MESSAGE_RELEASE    7
#define DHCP_MESSAGE_INFORM     8


void dhcp_poll(eth_frame_t * pFrame);
void enc28j60_init_net(void);
void eth_filter(uint16_t len, eth_frame_t * pFrame);
void udp_send(uint16_t len, eth_frame_t *pFrame);

void phy_mqttsn_filter(uint16_t len, eth_frame_t * pFrame);     // defined in enc28j60_phy.c

#endif