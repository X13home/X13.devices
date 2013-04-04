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

void send_udp_prepare(uint8_t *buf,uint16_t sport, const uint8_t *dip, uint16_t dport,const uint8_t *dstmac);
void send_udp_transmit(uint8_t *buf,uint16_t datalen);

uint8_t packetloop_arp_icmp(uint8_t *buf, uint16_t plen);

// Initial_tid can be a random number for every board. E.g the last digit
// of the mac address. It is not so important that the number is random.
// It is more important that it is unique and no other board on the same
// Lan has the same number. This is because there might be a power outage
// and all boards reboot afterwards at the same time. At that moment they
// must all have different TIDs otherwise there will be an IP address mess-up.
//
// The function returns 1 once we have a valid IP. 
// At this point you must not call the function again.
uint8_t packetloop_dhcp_initial_ip_assignment(uint8_t *buf,uint16_t plen,uint8_t initial_tid);
// Put the following function into your main packet loop.
// returns plen of original packet if buf is not touched.
// returns 0 if plen was originally zero. returns 0 if DHCP messages
// was processed.
uint16_t packetloop_dhcp_renewhandler(uint8_t *buf,uint16_t plen);

#endif /* IP_ARP_UDP_TCP_H */

void ip_arp_sec_tick(void);

//@}
