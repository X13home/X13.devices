/*********************************************
 * vim:sw=8:ts=8:si:et
 * To use the above modeline in vim you must have "set modeline" in your .vimrc
 * Author: Guido Socher 
 * Copyright: GPL V2
 *
 * This file can be used to decide which functionallity of the
 * TCP/IP stack shall be available.
 *
 *********************************************/
//@{
#ifndef IP_CONFIG_H
#define IP_CONFIG_H

// a spontanious sending UDP client (needed as well for DNS and DHCP)
#define UDP_client
//#define UDP_server

#define DHCP_client

#endif /* IP_CONFIG_H */
//@}
