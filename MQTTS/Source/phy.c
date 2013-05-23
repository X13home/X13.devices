/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

// Phy Prototypes

#include "config.h"
#include "util.h"

#if (defined _RFM12_H)
#include "Phy/rfm12/rfm12.c"
#elif (defined _CC11_H)
#include "Phy/cc11/cc11.c"
#elif (defined _ENCPHY_H)
#include "Phy/enc28j60/encphy.c"
#else
#error Phy not defined
#endif  //  HWCONFIG

uint8_t PHY_BuildName(uint8_t * pBuf)
{
#ifdef RF_NODE
  sprinthex(&pBuf[0], rf_GetNodeID());
  return 2;
#endif  //  RF_NODE
#ifdef LAN_NODE
  sprinthex(&pBuf[0], ipaddr[0]);
  sprinthex(&pBuf[2], ipaddr[1]);
  sprinthex(&pBuf[4], ipaddr[2]);
  sprinthex(&pBuf[6], ipaddr[3]);
  return 8;
#endif
}

