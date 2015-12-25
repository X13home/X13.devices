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

#ifdef ENC28J60_PHY

#include "enc28j60_hw.h"
#include "enc28j60_net.h"

static Queue_t          enc_out_queue = {NULL, NULL, 4, 0};

// external subroutines, defined in enc28j60_net.c
void eth_filter(uint16_t len, eth_frame_t * pFrame);
void udp_send(uint16_t len, eth_frame_t *pFrame);
void enc28j60_init_net(void);
void dhcp_poll(void);

// Process MQTT-SN Packet
void phy_mqttsn_filter(uint16_t len, eth_frame_t * pFrame)
{
    MQ_t * pRx_buf = mqAlloc(sizeof(MQ_t));
    
#ifdef LED_On
    LED_On();
#endif  //  LED_On

    ip_packet_t *ip = (void*)(pFrame->data);

    enc28j60_GetPacket((void*)pRx_buf->raw, len);
    memcpy(pRx_buf->phy1addr, ip->sender_ip, 4);
    pRx_buf->Length = len;
    
    if(!mqEnqueue(&enc_out_queue, pRx_buf))
    {
        mqFree(pRx_buf);
    }
}


//////////////////////////////////////////////////////////////////////
// PHY_API
void ENC28J60_Init(void)
{
    MQ_t * pBuf;
    while((pBuf = mqDequeue(&enc_out_queue)) != NULL)
        mqFree(pBuf);
        
    enc28j60_init_net();
}

extern uint32_t ip_addr;

void ENC28J60_Send(void *pBuf)
{
    if(ip_addr == 0)
    {
        mqFree(pBuf);
        return;
    }

#ifdef LED_On
    LED_On();
#endif  //  LED_On

    eth_frame_t * pFrame = (void *)mqAlloc(MAX_FRAME_BUF);

    ip_packet_t *ip = (void*)(pFrame->data);
    udp_packet_t *udp = (void*)(ip->data);

    memcpy(ip->target_ip, (((MQ_t *)pBuf)->phy1addr), 4);
    udp->target_port = MQTTSN_UDP_PORT;
    udp->sender_port = MQTTSN_UDP_PORT;
    uint16_t len = ((MQ_t *)pBuf)->Length;
    memcpy((void*)(udp->data), (((MQ_t *)pBuf)->raw), len);

    mqFree(pBuf);
    udp_send(len, pFrame);
  
    mqFree(pFrame);
}

void * ENC28J60_Get(void)
{
#ifdef NET_WITH_DHCP
    dhcp_poll();
#endif  //  NET_WITH_DHCP

    // Rx Section
    if(en28j60_DataRdy())
    {
        uint16_t len = enc28j60_GetPacketLen();
        if(len > sizeof(eth_frame_t))
        {
            eth_frame_t * pFrame = (void *)mqAlloc(MAX_FRAME_BUF);
            enc28j60_GetPacket((void *)pFrame,  sizeof(eth_frame_t));
            eth_filter(len, pFrame);
            mqFree(pFrame);
        }
        enc28j60_ClosePacket();
    }

    return mqDequeue(&enc_out_queue);
}

void * ENC28J60_GetAddr(void)
{
    return &ip_addr;
}

#endif  //  ENC28J60_PHY
