/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// ENC28J60 ethernet phy interface

#include "../../config.h"

#ifdef ENC_PHY

#include "enc28j60_hw.h"
#include "enc28j60_net.h"

static MQ_t *   enc_in_msg = NULL;
static uint8_t  enc_exchange_buf[MAX_FRAME_BUF];

// Process MQTT-SN Packet
void phy_mqttsn_filter(uint16_t len, eth_frame_t * pFrame)
{
#ifdef LED_On
    LED_On();
#endif  //  LED_On

    enc_in_msg = mqAlloc(sizeof(MQ_t));
    ip_packet_t *ip = (void*)(pFrame->data);

    enc28j60_GetPacket((void*)enc_in_msg->m.raw, len);
    memcpy(enc_in_msg->a.phy1addr, ip->sender_ip, 4);
    enc_in_msg->Length = len;
}


//////////////////////////////////////////////////////////////////////
// PHY_API
void ENC28J60_Init(void)
{
    enc_in_msg = NULL;
    enc28j60_init_net();
}

void ENC28J60_Send(void *pBuf)
{
    if(!ENC28J60_System_Ready())
    {
        mqFree(pBuf);
        return;
    }

#ifdef LED_On
    LED_On();
#endif  //  LED_On

    eth_frame_t * pFrame = (void *)enc_exchange_buf;

    ip_packet_t *ip = (void*)(pFrame->data);
    udp_packet_t *udp = (void*)(ip->data);

    memcpy(ip->target_ip, (((MQ_t *)pBuf)->a.phy1addr), 4);
    udp->target_port = MQTTSN_UDP_PORT;
    udp->sender_port = MQTTSN_UDP_PORT;
    uint16_t len = ((MQ_t *)pBuf)->Length;
    memcpy((void*)(udp->data), (((MQ_t *)pBuf)->m.raw), len);

    mqFree(pBuf);
    udp_send(len, pFrame);
}

void * ENC28J60_Get(void)
{
#ifdef NET_WITH_DHCP
    dhcp_poll((eth_frame_t *)enc_exchange_buf);
#endif  //  NET_WITH_DHCP

    // Rx Section
    if(en28j60_DataRdy())
    {
        uint16_t len = enc28j60_GetPacketLen();
        if(len > sizeof(eth_frame_t))
        {
            eth_frame_t * pFrame = (void *)enc_exchange_buf;
            enc28j60_GetPacket((void *)pFrame,  sizeof(eth_frame_t));
            eth_filter(len, pFrame);
        }
        enc28j60_ClosePacket();
    }

    if(enc_in_msg != NULL)
    {
        MQ_t * retval = enc_in_msg;
        enc_in_msg = NULL;
        return retval;
    }

    return NULL;
}

#endif  //  ENC_PHY
