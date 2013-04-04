/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.

Based on IPstack for AVR from Guido Socher and Pascal Stang
*/

// notation: _P = position of a field
//           _V = value of a field

#ifndef NET_H
#define NET_H

// ******* ETH *******
#define ETH_HEADER_LEN              14
// values of certain bytes:
#define ETHTYPE_ARP_H_V             0x08
#define ETHTYPE_ARP_L_V             0x06
#define ETHTYPE_IP_H_V              0x08
#define ETHTYPE_IP_L_V              0x00
// byte positions in the ethernet frame:
//
// Ethernet type field (2bytes):
#define ETH_TYPE_H_P                12
#define ETH_TYPE_L_P                13
//
#define ETH_DST_MAC                 0x00
#define ETH_SRC_MAC                 0x06


// ******* ARP *******
#define ETH_ARP_OPCODE_REPLY_H_V    0x00
#define ETH_ARP_OPCODE_REPLY_L_V    0x02
#define ETH_ARP_OPCODE_REQ_H_V      0x00
#define ETH_ARP_OPCODE_REQ_L_V      0x01
// start of arp header:
#define ETH_ARP_P                   0x0E
//
#define ETHTYPE_ARP_L_V             0x06
// arp.dst.ip
#define ETH_ARP_DST_IP_P            0x26
// arp.opcode
#define ETH_ARP_OPCODE_H_P          0x14
#define ETH_ARP_OPCODE_L_P          0x15
// arp.src.mac
#define ETH_ARP_SRC_MAC_P           0x16
#define ETH_ARP_SRC_IP_P            0x1C
#define ETH_ARP_DST_MAC_P           0x20
#define ETH_ARP_DST_IP_P            0x26

// ******* IP *******
#define IP_HEADER_LEN               20
// ip.src
#define IP_SRC_P                    0x1A
#define IP_DST_P                    0x1E
#define IP_HEADER_LEN_VER_P         0x0E
#define IP_CHECKSUM_P               0x18
#define IP_TTL_P                    0x16
#define IP_FLAGS_P                  0x14
#define IP_P                        0x0E
#define IP_TOTLEN_H_P               0x10
#define IP_TOTLEN_L_P               0x11
#define IP_ID_H_P                   0x12
#define IP_ID_L_P                   0x13

#define IP_PROTO_P                  0x17

#define IP_PROTO_ICMP_V             1
#define IP_PROTO_TCP_V              6
#define IP_PROTO_UDP_V              0x11
// ******* ICMP *******
#define ICMP_TYPE_ECHOREPLY_V       0
#define ICMP_TYPE_ECHOREQUEST_V     8
//
#define ICMP_TYPE_P                 0x22
#define ICMP_CHECKSUM_P             0x24
#define ICMP_CHECKSUM_H_P           0x24
#define ICMP_CHECKSUM_L_P           0x25
#define ICMP_IDENT_H_P              0x26
#define ICMP_IDENT_L_P              0x27
#define ICMP_DATA_P                 0x2A

// ******* UDP *******
#define UDP_HEADER_LEN              8
//
#define UDP_SRC_PORT_H_P            0x22
#define UDP_SRC_PORT_L_P            0x23
#define UDP_DST_PORT_H_P            0x24
#define UDP_DST_PORT_L_P            0x25
//
#define UDP_LEN_H_P                 0x26
#define UDP_LEN_L_P                 0x27
#define UDP_CHECKSUM_H_P            0x28
#define UDP_CHECKSUM_L_P            0x29
#define UDP_DATA_P                  0x2A

// ******* TCP *******
#define TCP_SRC_PORT_H_P            0x22
#define TCP_SRC_PORT_L_P            0x23
#define TCP_DST_PORT_H_P            0x24
#define TCP_DST_PORT_L_P            0x25
// the tcp seq number is 4 bytes 0x26-0x29
#define TCP_SEQ_H_P                 0x26
#define TCP_SEQACK_H_P              0x2A
// flags: SYN=2
#define TCP_FLAGS_P                 0x2F
#define TCP_FLAGS_SYN_V             2
#define TCP_FLAGS_FIN_V             1
#define TCP_FLAGS_RST_V             4
#define TCP_FLAGS_PUSH_V            8
#define TCP_FLAGS_SYNACK_V          0x12
#define TCP_FLAGS_ACK_V             0x10
#define TCP_FLAGS_PSHACK_V          0x18
//  plain len without the options:
#define TCP_HEADER_LEN_PLAIN        20
#define TCP_HEADER_LEN_P            0x2E
#define TCP_WIN_SIZE                0x30
#define TCP_CHECKSUM_H_P            0x32
#define TCP_CHECKSUM_L_P            0x33
#define TCP_OPTIONS_P               0x36
//

// DHCP_OPTION_OFFSET is a relative to UDP_DATA_P
#define MAGIC_COOKIE_P              236
#define DHCP_OPTION_OFFSET          240
#define DHCP_SRV_SRC_PORT           67
#define DHCP_SRV_DST_PORT           68

#endif
//@}

