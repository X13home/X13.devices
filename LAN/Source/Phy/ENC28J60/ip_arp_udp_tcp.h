/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.

Based on IPstack for AVR from Guido Socher and Pascal Stang
*/

#ifndef IP_ARP_UDP_TCP_H
#define IP_ARP_UDP_TCP_H 1

#include "ip_config.h"

void ip_arp_sec_tick(void);
uint8_t packetloop_arp_icmp(uint8_t *buf, uint16_t plen);

#ifdef UDP_client
void send_udp_prepare(uint8_t *buf,uint16_t sport, const uint8_t *dip, uint16_t dport,const uint8_t *dstmac);
void send_udp_transmit(uint8_t *buf,uint16_t datalen);
#endif  //  UDP_client

#ifdef DHCP_client
// The function returns 1 once we have a valid IP. 
// At this point you must not call the function again.
uint8_t packetloop_dhcp_initial_ip_assignment(uint8_t *buf,uint16_t plen);
#endif  //  DHCP_client

#endif /* IP_ARP_UDP_TCP_H */

//@}
