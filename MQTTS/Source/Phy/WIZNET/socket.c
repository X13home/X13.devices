/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

#include "socket.h"

static uint16_t get_new_localport(void)
{
  static uint16_t local_port = 1024;
  local_port++;
  if(local_port < 1024)
    local_port = 1024;
  return local_port;
}

/**
@brief  This function close the socket and parameter is "s" which represent the socket number
*/ 
void close(SOCKET s)
{
  wiz_write(Sn_CR(s),Sn_CR_CLOSE);
  while(wiz_read(Sn_CR(s)));

#ifdef __DEF_WIZNET_INT__
  putISR(s, 0x00); // all clear
#else   //  !__DEF_WIZNET_INT__
  wiz_write(Sn_IR(s), 0xFF);
#endif  //  __DEF_WIZNET_INT__
}

/**
@brief  This Socket function initialize the channel in particular mode, and set the port and wait for W5200 done it.
@return 1 for success else 0.
*/  
uint8_t socket(SOCKET s, uint8_t protocol, uint16_t port, uint8_t flag)
{
  uint16_t uiTmp;

  if ((protocol == Sn_MR_TCP) || (protocol == Sn_MR_UDP) || (protocol == Sn_MR_IPRAW) || (protocol == Sn_MR_MACRAW) || (protocol == Sn_MR_PPPOE))
  {
    close(s);
    wiz_write(Sn_MR(s), protocol | flag);
    if(port != 0) 
    {
      wiz_write(Sn_PORT0(s), port>>8);
      wiz_write(Sn_PORT0(s) + 1, port & 0xff);
    }
    else  // if don't set the source port, set local_port number.
    {
      uiTmp = get_new_localport();    
      wiz_write(Sn_PORT0(s), uiTmp>>8);
      wiz_write((Sn_PORT0(s) + 1),uiTmp & 0xff);
    }
    wiz_write(Sn_CR(s),Sn_CR_OPEN);   // run sockinit Sn_CR

    while(wiz_read(Sn_CR(s)));
    return 1;
  }
  return 0;
}
