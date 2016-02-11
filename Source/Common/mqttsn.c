/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// MQTT-SN Library, Version 3.0.2 from 13.Dec.2015

#include "config.h"

// Keep Alive Time
#define MQTTSN_T_KEEPALIVE      300

#define MQTTSN_T_SHORT          (const uint16_t)(2 * POLL_TMR_FREQ)
#define MQTTSN_T_AWAKE          (const uint16_t)(3 * POLL_TMR_FREQ)

// Number of retries
#define MQTTSN_N_RETRIES        (const uint8_t)3
// Time between Connect's
// Random 1 - 5 Sec
#define MQTTSN_T_CONNECT        mqttsn_get_random_delay((5 * POLL_TMR_FREQ), POLL_TMR_FREQ)
// Time between search gateway requests
// Random 1-5/6-10/11-15 Sec
#define MQTTSN_T_SEARCHGW       mqttsn_get_random_delay((5 * POLL_TMR_FREQ * ((MQTTSN_N_RETRIES + 1) - vMQ_nRetry)), \
                                                        (POLL_TMR_FREQ * (5 * (MQTTSN_N_RETRIES - vMQ_nRetry) + 1)))
// Time between incoming search gateway request and GWInfo for Node
// Random 1 - 5 Sec
#define MQTTSN_T_GWINFO         mqttsn_get_random_delay((5 * POLL_TMR_FREQ), POLL_TMR_FREQ)
// Random 1 - 2 Sec
#define MQTTSN_T_ACCEL          mqttsn_get_random_delay((2 * POLL_TMR_FREQ), POLL_TMR_FREQ);
// Delay retries based
#define MQTTSN_T_RETR_BASED     mqttsn_get_random_delay(((MQTTSN_N_RETRIES - vMQ_nRetry)*(POLL_TMR_FREQ/5)), \
                                                        ((MQTTSN_N_RETRIES - vMQ_nRetry)*(POLL_TMR_FREQ/10)))
// Pause on Disconnect State, 30 Sec.
#define MQTTSN_T_DISCONNECT     (uint16_t)(30 * POLL_TMR_FREQ)

#define MQTTSN_MAX_RADIUS       3   // Hops to Gateway

static const PHY1_ADDR_t    addr1_undef = ADDR_UNDEF_PHY1;
static const PHY1_ADDR_t    addr1_broad = ADDR_BROADCAST_PHY1;
    
#ifdef PHY2_ADDR_t
static const PHY2_ADDR_t    addr2_undef = ADDR_UNDEF_PHY2;
static const PHY2_ADDR_t    addr2_broad = ADDR_BROADCAST_PHY2;
#endif  //  PHY2_ADDR_t

// Local Variables
static uint8_t              vMQ_GatewayAddr[sizeof(PHY1_ADDR_t)];   // Gateway Address
static uint8_t              vMQ_GwId;                               // Unique Gateway ID
static uint8_t              vMQ_Radius;                             // Broadcast Radius
static e_MQTTSN_STATUS_t    vMQ_Status;                             // Actual status
static uint16_t             vMQ_MsgId;                              // Message ID
// Timeout's
static uint16_t             vMQ_tRetry;                             // Time between retry's
static uint8_t              vMQ_nRetry;                             // Retry number
#if (defined MQTTSN_USE_MESH)
static uint16_t             vMQ_tGWinfo1;                           // Timeout to send GWInfo message
#endif  //  (defined MQTTSN_USE_MESH)
#ifdef PHY2_ADDR_t
static uint16_t             vMQ_tGWinfo2;
#endif
#ifdef  ASLEEP
static uint16_t             vMQ_tASleep;
#endif  //  ASLEEP

// Register / Subscribe / Publish variables
static e_MQTTSN_MSGTYPE_t   vMQ_MsgType;                            // Send data type
static MQ_t               * vMQ_pMessage;
static uint16_t             vMQ_oMsgId;                             // Old Message ID for publish

// Generate random from Min to Max
static uint16_t mqttsn_get_random_delay(uint16_t delayMax, uint16_t delayMin)
{
    uint32_t ulTmp = delayMax - delayMin;
    ulTmp *= HAL_RNG();
  
    return (uint16_t)(ulTmp>>16) + delayMin;
}

// Get new outgoing message ID
static uint16_t mqttsn_new_msgid(void)
{
    vMQ_MsgId++;
    if(vMQ_MsgId > 0xFFFE)
        vMQ_MsgId = 1;
    return vMQ_MsgId;
}

#ifdef DIAG_USED
void mqttsn_trace_msg(uint8_t Level, MQ_t * pMessage)
{
    if(pMessage == NULL)
        return;
    
    if(vMQ_Status != MQTTSN_STATUS_CONNECT)
    {
        mqFree(pMessage);
        return;
    }

    if(Level > lvlERROR)
        Level = lvlERROR;

    uint16_t TopicId = objLogD;
    TopicId += Level;
    mqttsn_new_msgid();
    
    pMessage->mq.MsgType = MQTTSN_MSGTYP_PUBLISH;
    pMessage->mq.publish.Flags = (MQTTSN_FL_QOS0 | MQTTSN_FL_TOPICID_PREDEF);
    pMessage->mq.publish.MsgId[0] = vMQ_MsgId>>8;
    pMessage->mq.publish.MsgId[1] = vMQ_MsgId & 0xFF;
    pMessage->mq.publish.TopicId[0] = TopicId>>8;
    pMessage->mq.publish.TopicId[1] = TopicId & 0xFF;
    pMessage->Length += MQTTSN_SIZEOF_MSG_PUBLISH;
    pMessage->mq.Length = pMessage->Length;
    memcpy(pMessage->phy1addr, vMQ_GatewayAddr, sizeof(PHY1_ADDR_t));
    PHY1_Send(pMessage);
}
#endif  //  DIAG_USED

////////////////////////////////////////////////////////////////////////
// Parse incoming messages
#if (defined MQTTSN_USE_MESH)
static void mqtts_forward_to_gate(MQ_t *pMsg)
{
    // Forward message on PHY1 from Remote Node to Gateway
    uint8_t Size = (MQTTSN_SIZEOF_MSG_FORWARD + sizeof(PHY1_ADDR_t) + 1);
    uint8_t Length = pMsg->Length + Size;
    uint8_t pos;

    if(Length > sizeof(MQTTSN_MESSAGE_t))
    {
        mqFree(pMsg);
        return;
    }

    // Don't use memcpy !
    for(pos = (Length - 1); pos >= Size; pos--)
        pMsg->raw[pos] = pMsg->raw[pos - Size];

    // Make forward message header
    pMsg->Length = Length;
    pMsg->mq.Length = Size;
    pMsg->mq.MsgType = MQTTSN_MSGTYP_FORWARD;
    pMsg->mq.forward.Ctrl = 0;   // ?? TTL
    pMsg->mq.forward.wNodeID[0] = 1;     // PHY1
    memcpy(&pMsg->mq.forward.wNodeID[1], pMsg->phy1addr, sizeof(PHY1_ADDR_t));
    memcpy(pMsg->phy1addr, vMQ_GatewayAddr, sizeof(PHY1_ADDR_t));
    PHY1_Send(pMsg);
}
#endif  //  (defined MQTTSN_USE_MESH)

// Parse incoming messages from PHY1
void mqttsn_parser_phy1(MQ_t * pPHY1outBuf)
{
    bool msg_from_gw = (memcmp(pPHY1outBuf->phy1addr, vMQ_GatewayAddr, sizeof(PHY1_ADDR_t)) == 0);
    e_MQTTSN_MSGTYPE_t MsgType = pPHY1outBuf->mq.MsgType;

    switch(MsgType)
    {
        case MQTTSN_MSGTYP_ADVERTISE:
        {
            if(vMQ_Status == MQTTSN_STATUS_SEARCHGW)
            {
                memcpy(vMQ_GatewayAddr, pPHY1outBuf->phy1addr, sizeof(PHY1_ADDR_t));
                vMQ_GwId = pPHY1outBuf->mq.advertise.GwId;
                vMQ_Radius = 1;
                vMQ_Status = MQTTSN_STATUS_OFFLINE;
                vMQ_tRetry = MQTTSN_T_ACCEL;
                vMQ_nRetry = MQTTSN_N_RETRIES;
            }
#if ((defined MQTTSN_USE_MESH) || (defined PHY2_Send))
            else if(vMQ_Status == MQTTSN_STATUS_CONNECT)
            {
                if(msg_from_gw)
                {
#ifdef PHY2_Send
                    if(vMQ_Radius == 1)
                    {
#if (defined MQTTSN_USE_MESH)
                        vMQ_tGWinfo1 = 0;
#endif	//	(defined MQTTSN_USE_MESH)
                        vMQ_tGWinfo2 = 0;
                        PHY2_Send(pPHY1outBuf);
                        return;
                    }
#endif
                }
#if (defined MQTTSN_USE_MESH)
                else
                    vMQ_tGWinfo1 = 0;
#endif  //  (defined MQTTSN_USE_MESH)
            }
#endif  //  ((defined MQTTSN_USE_MESH) || (defined PHY2_Send))
            break;
        }
        // Search gateway request from another node
        case MQTTSN_MSGTYP_SEARCHGW:
        {
            if(vMQ_Status == MQTTSN_STATUS_SEARCHGW)
            {
                vMQ_tRetry = MQTTSN_T_SEARCHGW;
            }
#if (defined MQTTSN_USE_MESH)
            else if(vMQ_Status == MQTTSN_STATUS_CONNECT)
            {
                if((pPHY1outBuf->mq.searchgw.Radius == (vMQ_Radius + 1)) ||
                   (pPHY1outBuf->mq.searchgw.Radius == 0))
                {
                    vMQ_tGWinfo1 = MQTTSN_T_GWINFO;
                }
            }
#endif  //  (defined MQTTSN_USE_MESH)
            break;
        }
        // Advertise message, equivalent GWINFO
        case MQTTSN_MSGTYP_GWINFO:
        {
            if(vMQ_Status == MQTTSN_STATUS_SEARCHGW)
            {
                memcpy(vMQ_GatewayAddr, pPHY1outBuf->phy1addr, sizeof(PHY1_ADDR_t));
                vMQ_GwId = pPHY1outBuf->mq.gwinfo.GwId;
                vMQ_Status = MQTTSN_STATUS_OFFLINE;
                vMQ_tRetry = MQTTSN_T_ACCEL;
                vMQ_nRetry = MQTTSN_N_RETRIES;
            }
#if (defined MQTTSN_USE_MESH)
            else if(vMQ_Status == MQTTSN_STATUS_CONNECT)
            {
                vMQ_tGWinfo1 = 0;
            }
#endif  //  (defined MQTTSN_USE_MESH)
            break;
        }
        // Connect message from another node
#ifdef MQTTSN_USE_MESH
        case MQTTSN_MSGTYP_CONNECT:
        {
            if(!msg_from_gw && (vMQ_Status == MQTTSN_STATUS_CONNECT))
            {
                mqtts_forward_to_gate(pPHY1outBuf);
                return;
            }
            break;
        }
#endif  //  MQTTSN_USE_MESH
        // Connack message
        case MQTTSN_MSGTYP_CONNACK:
        {
            if(msg_from_gw)
            {
                if(pPHY1outBuf->mq.connack.ReturnCode == MQTTSN_RET_ACCEPTED)
                {
                    if(vMQ_Status == MQTTSN_STATUS_OFFLINE)
                    {
                        vMQ_Status = MQTTSN_STATUS_PRE_CONNECT;
                        vMQ_tRetry = MQTTSN_T_SHORT;
                        vMQ_nRetry = MQTTSN_N_RETRIES;
                    }
#ifdef  ASLEEP
                    else if(vMQ_Status == MQTTSN_STATUS_AWAKE)
                    {
                        vMQ_Status = MQTTSN_STATUS_CONNECT;
                        vMQ_tRetry = 1;
                        vMQ_nRetry = MQTTSN_N_RETRIES;
                    }
#endif  //  ASLEEP
                    // else
                    // ToDo
                    // Message lost, broker - gateway Problems, Connected another Node with same Address.
                    // Potential dangerous
                }
            }
            break;
        }
/*
        case MQTTSN_MSGTYP_WILLTOPICREQ:
        case MQTTSN_MSGTYP_WILLTOPIC:
        case MQTTSN_MSGTYP_WILLMSGREQ:
        case MQTTSN_MSGTYP_WILLMSG:
*/
        // Register Topic request
        case MQTTSN_MSGTYP_REGISTER:
        {
            if(vMQ_Status == MQTTSN_STATUS_CONNECT)
            {
                if(msg_from_gw)
                {
                    if(vMQ_MsgType == MQTTSN_MSGTYP_PINGREQ)
                    {
#ifdef ASLEEP
                        if(vMQ_tASleep != 0)
                            vMQ_tRetry = MQTTSN_T_SHORT;
                        else
#endif  //  ASLEEP
                            vMQ_tRetry = (const uint16_t)(MQTTSN_T_KEEPALIVE * POLL_TMR_FREQ);
                    }

                    pPHY1outBuf->mq.regack.ReturnCode = RegisterOD(&pPHY1outBuf->mq);
                    pPHY1outBuf->Length = MQTTSN_SIZEOF_MSG_REGACK;
                    pPHY1outBuf->mq.Length = MQTTSN_SIZEOF_MSG_REGACK;
                    pPHY1outBuf->mq.MsgType = MQTTSN_MSGTYP_REGACK;
                    PHY1_Send(pPHY1outBuf);
                }
#ifdef MQTTSN_USE_MESH
                else
                {
                    mqtts_forward_to_gate(pPHY1outBuf);
                }
#endif  //  MQTTSN_USE_MESH
                return;
            }
            break;
        }
        // RegAck Answer
        case MQTTSN_MSGTYP_REGACK:
        {
            if(msg_from_gw)
            {
                if((vMQ_Status == MQTTSN_STATUS_PRE_CONNECT) ||
                   (vMQ_Status == MQTTSN_STATUS_CONNECT))
                {
                    if((vMQ_MsgType == MQTTSN_MSGTYP_REGISTER) &&
                       (vMQ_pMessage->mq.regist.MsgId[0] == pPHY1outBuf->mq.regack.MsgId[0]) &&
                       (vMQ_pMessage->mq.regist.MsgId[1] == pPHY1outBuf->mq.regack.MsgId[1]))
                    {
                        mqFree(vMQ_pMessage);
                        vMQ_MsgType = MQTTSN_MSGTYP_PINGREQ;

                        uint16_t index;
                        if(pPHY1outBuf->mq.regack.ReturnCode == MQTTSN_RET_ACCEPTED)
                            index = (pPHY1outBuf->mq.regack.TopicId[0]<<8) |
                                     pPHY1outBuf->mq.regack.TopicId[1];
                        else
                            index = 0;
                        RegAckOD(index);
                        
                        vMQ_tRetry = MQTTSN_T_SHORT;
                        vMQ_nRetry = MQTTSN_N_RETRIES;
                    }
                }
            }
#ifdef MQTTSN_USE_MESH
            else if(vMQ_Status == MQTTSN_STATUS_CONNECT)
            {
                mqtts_forward_to_gate(pPHY1outBuf);
                return;
            }
#endif  //  MQTTSN_USE_MESH
            break;
        }
        // Publish Topic request
        case MQTTSN_MSGTYP_PUBLISH:
        {
            if(msg_from_gw)
            {
                if((vMQ_Status == MQTTSN_STATUS_CONNECT)
#ifdef ASLEEP
                    || (vMQ_Status == MQTTSN_STATUS_AWAKE)
#endif  //  ASLEEP
                   )
                {
                    uint8_t Flags = pPHY1outBuf->mq.publish.Flags;
                    uint16_t TopicId = (pPHY1outBuf->mq.publish.TopicId[0]<<8) |
                                        pPHY1outBuf->mq.publish.TopicId[1];
                    uint16_t MsgId =   (pPHY1outBuf->mq.publish.MsgId[0]<<8) |
                                        pPHY1outBuf->mq.publish.MsgId[1];

                    if(((Flags & MQTTSN_FL_DUP) == 0) || (vMQ_oMsgId != MsgId))
                    {
                        pPHY1outBuf->mq.puback.ReturnCode = WriteODpack(
                                        TopicId,
                                        Flags, 
                                        (pPHY1outBuf->mq.Length - MQTTSN_SIZEOF_MSG_PUBLISH),
                                        (uint8_t *)pPHY1outBuf->mq.publish.Data);
                        vMQ_oMsgId = MsgId;
                    }

                    // ToDo Not Supported QOS2
                    if((Flags & MQTTSN_FL_QOS_MASK) == MQTTSN_FL_QOS1)           // Need Ack
                    {
                        if(vMQ_MsgType == MQTTSN_MSGTYP_PINGREQ)
                        {
#ifdef ASLEEP
                            if(vMQ_tASleep != 0)
                                vMQ_tRetry = MQTTSN_T_SHORT;
                            else
#endif  //  ASLEEP
                                vMQ_tRetry = (const uint16_t)(MQTTSN_T_KEEPALIVE * POLL_TMR_FREQ);
                        }

                        pPHY1outBuf->Length = MQTTSN_SIZEOF_MSG_PUBACK;
                        pPHY1outBuf->mq.Length = MQTTSN_SIZEOF_MSG_PUBACK;
                        pPHY1outBuf->mq.MsgType = MQTTSN_MSGTYP_PUBACK;
                        pPHY1outBuf->mq.puback.TopicId[0] = TopicId>>8;
                        pPHY1outBuf->mq.puback.TopicId[1] = TopicId & 0xFF;
                        pPHY1outBuf->mq.puback.MsgId[0] = MsgId>>8;
                        pPHY1outBuf->mq.puback.MsgId[1] = MsgId & 0xFF;
                        PHY1_Send(pPHY1outBuf);
                        return;
                    }
                }
            }
#ifdef MQTTSN_USE_MESH
            else if(vMQ_Status == MQTTSN_STATUS_CONNECT)
            {
                mqtts_forward_to_gate(pPHY1outBuf);
                return;
            }
#endif  //  MQTTSN_USE_MESH
            break;
        }
        // PubAck Answer
        case MQTTSN_MSGTYP_PUBACK:
        {
            if(msg_from_gw)
            {
                if((vMQ_Status == MQTTSN_STATUS_CONNECT) || 
                   (vMQ_Status == MQTTSN_STATUS_PRE_CONNECT))
                {
                    if((vMQ_MsgType == MQTTSN_MSGTYP_PUBLISH) &&
                       (vMQ_pMessage->mq.publish.MsgId[0] == pPHY1outBuf->mq.puback.MsgId[0]) &&
                       (vMQ_pMessage->mq.publish.MsgId[1] == pPHY1outBuf->mq.puback.MsgId[1]))
                    {
                        mqFree(vMQ_pMessage);
                        vMQ_MsgType = MQTTSN_MSGTYP_PINGREQ;
                        vMQ_nRetry = MQTTSN_N_RETRIES;
#ifdef ASLEEP
                        if(vMQ_tASleep != 0)
                            vMQ_tRetry = MQTTSN_T_SHORT;
                        else
#endif  //  ASLEEP
                            vMQ_tRetry = (const uint16_t)(MQTTSN_T_KEEPALIVE * POLL_TMR_FREQ);
                    }
                }
            }
#ifdef MQTTSN_USE_MESH
            else if(vMQ_Status == MQTTSN_STATUS_CONNECT)
            {
                mqtts_forward_to_gate(pPHY1outBuf);
                return;
            }
#endif  //  MQTTSN_USE_MESH
            break;
        }
/*
        case MQTTSN_MSGTYP_PUBCOMP:
        case MQTTSN_MSGTYP_PUBREC:
        case MQTTSN_MSGTYP_PUBREL:
*/
        // Subscribe message
#ifdef MQTTSN_USE_MESH
        case MQTTSN_MSGTYP_SUBSCRIBE:
        {
            if(vMQ_Status == MQTTSN_STATUS_CONNECT)
            {
                mqtts_forward_to_gate(pPHY1outBuf);
                return;
            }
            break;
        }
#endif  //  MQTTSN_USE_MESH
        // SubAck answer
        case MQTTSN_MSGTYP_SUBACK:
        {
            if(msg_from_gw)
            {
                if(vMQ_Status == MQTTSN_STATUS_PRE_CONNECT)
                {
                    if((vMQ_MsgType == MQTTSN_MSGTYP_SUBSCRIBE) &&
                       (vMQ_pMessage->mq.subscribe.MsgId[0] == pPHY1outBuf->mq.suback.MsgId[0]) &&
                       (vMQ_pMessage->mq.subscribe.MsgId[1] == pPHY1outBuf->mq.suback.MsgId[1]))
                    {
                        mqFree(vMQ_pMessage);

                        vMQ_MsgType = MQTTSN_MSGTYP_PINGREQ;
                        vMQ_Status = MQTTSN_STATUS_CONNECT;
                        vMQ_nRetry = MQTTSN_N_RETRIES;
#ifdef ASLEEP
                        if(vMQ_tASleep != 0)
                            vMQ_tRetry = MQTTSN_T_SHORT;
                        else
#endif  //  ASLEEP
                            vMQ_tRetry = (const uint16_t)(MQTTSN_T_KEEPALIVE * POLL_TMR_FREQ);
                    }
                }
            }
            break;
        }
/*
        case MQTTSN_MSGTYP_UNSUBSCRIBE:
        case MQTTSN_MSGTYP_UNSUBACK:
*/
        // Ping Request
#ifdef MQTTSN_USE_MESH
        case MQTTSN_MSGTYP_PINGREQ:
        {
            if(vMQ_Status == MQTTSN_STATUS_CONNECT)
            {
                mqtts_forward_to_gate(pPHY1outBuf);
                return;
            }
            break;
        }
#endif  //  MQTTSN_USE_MESH
        // Ping Response
        case MQTTSN_MSGTYP_PINGRESP:
        {
            if(msg_from_gw)
            {
                if(vMQ_Status == MQTTSN_STATUS_CONNECT)
                {
                    vMQ_tRetry = (const uint16_t)(MQTTSN_T_KEEPALIVE * POLL_TMR_FREQ);
                    vMQ_nRetry = MQTTSN_N_RETRIES;
                }
#ifdef ASLEEP
                else if(vMQ_Status == MQTTSN_STATUS_AWAKE)
                {
                    vMQ_tRetry = MQTTSN_T_SHORT;
                    vMQ_Status = MQTTSN_STATUS_ASLEEP;
                }
#endif  //  ASLEEP
            }
            break;
        }
        // Disconnect Request
        case MQTTSN_MSGTYP_DISCONNECT:
        {
            if(vMQ_Status < MQTTSN_STATUS_OFFLINE)
            {
                vMQ_Radius = 1;
                vMQ_tRetry = 1;
                vMQ_nRetry = MQTTSN_N_RETRIES;
            }
            else if(msg_from_gw)
            {
                vMQ_tRetry = (const uint16_t)(POLL_TMR_FREQ/10);
                vMQ_nRetry = MQTTSN_N_RETRIES;
#ifdef ASLEEP
                if(vMQ_Status == MQTTSN_STATUS_ASLEEP_DISCONNECT)
                    vMQ_Status = MQTTSN_STATUS_ASLEEP;
                else
#endif  //  ASLEEP
                    vMQ_Status = MQTTSN_STATUS_DISCONNECTED;
            }
#ifdef MQTTSN_USE_MESH
            else if(vMQ_Status == MQTTSN_STATUS_CONNECT)
            {
                mqtts_forward_to_gate(pPHY1outBuf);
                return;
            }
#endif  //  MQTTSN_USE_MESH
            break;
        }
/*
        case MQTTSN_MSGTYP_WILLTOPICUPD:
        case MQTTSN_MSGTYP_WILLTOPICRESP:
        case MQTTSN_MSGTYP_WILLMSGUPD:
        case MQTTSN_MSGTYP_WILLMSGRESP:
*/
#ifdef MQTTSN_USE_DHCP
        // NOT STANDARD MESSAGE, DON'T USE WITH ANOTHER SYSTEMS
        // DHCP request from another node
        case MQTTSN_MSGTYP_DHCPREQ:
        {
            if(vMQ_Status == MQTTSN_STATUS_DHCP)
                vMQ_tRetry = MQTTSN_T_SEARCHGW;
#ifdef MQTTSN_USE_MESH
            else if(vMQ_Status == MQTTSN_STATUS_CONNECT)
            {
                mqtts_forward_to_gate(pPHY1outBuf);
                return;
            }
#endif  //  MQTTSN_USE_MESH
            break;
        }
        // DHCP Response
        case MQTTSN_MSGTYP_DHCPRESP:
        {
            if(vMQ_Status == MQTTSN_STATUS_DHCP)
            {
                uint16_t msgid = pPHY1outBuf->mq.dhcpresp.MsgId[0];
                msgid <<= 8;
                msgid |= pPHY1outBuf->mq.dhcpresp.MsgId[1];

                if(msgid == vMQ_MsgId) // Own message
                {
                    uint8_t Mask = 0;
                    uint8_t * pData = pPHY1outBuf->mq.dhcpresp.addr;
                    uint8_t Length = MQTTSN_SIZEOF_MSG_DHCPRESP;
                    
                    if(memcmp(PHY1_GetAddr(), &addr1_undef, sizeof(PHY1_ADDR_t)) == 0)
                    {
                        Length += sizeof(PHY1_ADDR_t);
                        Mask = 1; 
                    }
#ifdef PHY2_ADDR_t
                    if(memcmp(PHY2_GetAddr(), &addr2_undef, sizeof(PHY2_ADDR_t))== 0)
                    {
                        Length += sizeof(PHY2_ADDR_t);
                        Mask |= 2; 
                    }
#endif  //  PHY2_ADDR_t
                    if(pPHY1outBuf->mq.Length != Length)
                        break;

                    if(Mask & 1)
                    {
                        // Check, own address != Gateway Address
                        if(memcmp(PHY1_GetAddr(), pData, sizeof(PHY1_ADDR_t)) == 0)
                            break;

                        WriteOD(PHY1_NodeId, MQTTSN_FL_TOPICID_PREDEF, sizeof(PHY1_ADDR_t), (uint8_t *)pData);
                    }
#ifdef PHY2_ADDR_t
                    if(Mask & 2)
                    {
                        if(Mask & 1)
                            pData += sizeof(PHY1_ADDR_t);
                        
                        WriteOD(PHY2_NodeId, MQTTSN_FL_TOPICID_PREDEF, sizeof(PHY2_ADDR_t), (uint8_t *)pData);
                    }
#endif  //  PHY2_ADDR_t

                    vMQ_Status = MQTTSN_STATUS_DISCONNECTED;
                    vMQ_tRetry = 1;
                }
            }
            break;
        }
#endif  //  MQTTSN_USE_DHCP
        // Forward message
#if ((defined MQTTSN_USE_MESH) || (defined PHY2_Send))
        case MQTTSN_MSGTYP_FORWARD:
        {
            if(vMQ_Status == MQTTSN_STATUS_CONNECT)
            {
                if(msg_from_gw)   // message from Gateway to Node
                {
                    uint8_t Length = pPHY1outBuf->mq.Length;
                    uint8_t phynr = pPHY1outBuf->mq.forward.wNodeID[0];
#ifdef MQTTSN_USE_MESH
                    // Direction: Gateway to Remote node on PHY1
                    if((phynr == 1) && (Length == (MQTTSN_SIZEOF_MSG_FORWARD + sizeof(PHY1_ADDR_t) + 1)))
                    {
                        memcpy(pPHY1outBuf->phy1addr, &pPHY1outBuf->mq.forward.wNodeID[1], sizeof(PHY1_ADDR_t));
                        // truncate header
                        pPHY1outBuf->Length -= Length;
                        memcpy(&pPHY1outBuf->raw[0], &pPHY1outBuf->raw[Length], pPHY1outBuf->Length);
                        PHY1_Send(pPHY1outBuf);
                        return;
                    }
#endif  //  MQTTSN_USE_MESH
#ifdef PHY2_Send
                    // Direction Gateway to PHY2
                    if((phynr == 2) && (Length == (MQTTSN_SIZEOF_MSG_FORWARD + sizeof(PHY2_ADDR_t) + 1)))
                    {
                        memcpy(pPHY1outBuf->phy2addr, &pPHY1outBuf->mq.forward.wNodeID[1], sizeof(PHY2_ADDR_t));
                        // truncate header
                        pPHY1outBuf->Length -= Length;
                        memcpy(&pPHY1outBuf->raw[0], &pPHY1outBuf->raw[Length], pPHY1outBuf->Length);
                        PHY2_Send(pPHY1outBuf);
                        return;
                    }
#endif  //  PHY2_Send
                }
#ifdef MQTTSN_USE_MESH
                else
                {
                    mqtts_forward_to_gate(pPHY1outBuf);
                    return;
                }
#endif  //  MQTTSN_USE_MESH
            }
            break;
        }
#endif  //  ((defined MQTTSN_USE_MESH) || (defined PHY2_Send))
        default:
            break;
    }
    mqFree(pPHY1outBuf);
}

// Parse Incoming messages from PHY2
#ifdef PHY2_Get
void mqttsn_parser_phy2(MQ_t * pPHY2outBuf)
{
    if(vMQ_Status == MQTTSN_STATUS_CONNECT)
    {
        switch(pPHY2outBuf->mq.MsgType)
        {
            case MQTTSN_MSGTYP_SEARCHGW:
                if((pPHY2outBuf->mq.searchgw.Radius == vMQ_Radius) ||
                   (pPHY2outBuf->mq.searchgw.Radius == 0))
                {
                    if(vMQ_Radius == 1)
                        vMQ_tGWinfo2 = 1;
                    else
                        vMQ_tGWinfo2 = MQTTSN_T_GWINFO;
                }
                break;
            case MQTTSN_MSGTYP_ADVERTISE:
            case MQTTSN_MSGTYP_GWINFO:
                vMQ_tGWinfo2 = 0;
                break;
            // Encapsulate message to Forward Packet and send to Gateway
            case MQTTSN_MSGTYP_CONNECT:
            case MQTTSN_MSGTYP_PINGREQ:
#ifdef PHY2_GetRSSI
            {
                MQ_t * pRSSI = mqAlloc(sizeof(MQ_t));
                uint8_t pos = (MQTTSN_SIZEOF_MSG_FORWARD + sizeof(PHY2_ADDR_t) + 1);
                // Make Forward message
                pRSSI->mq.Length = pos;
                pRSSI->mq.MsgType = MQTTSN_MSGTYP_FORWARD;
                pRSSI->mq.forward.Ctrl = 0;   // ?? TTL
                pRSSI->mq.forward.wNodeID[0] = 2;     // PHY2
                memcpy(&pRSSI->mq.forward.wNodeID[1], pPHY2outBuf->phy2addr, sizeof(PHY2_ADDR_t));
                // Make publish message
                pRSSI->raw[pos++] = (MQTTSN_SIZEOF_MSG_PUBLISH + 1);                // length of publish
                pRSSI->raw[pos++] = MQTTSN_MSGTYP_PUBLISH;                          // Message Type
                pRSSI->raw[pos++] = (MQTTSN_FL_QOSN1 | MQTTSN_FL_TOPICID_PREDEF);   // Flags
                pRSSI->raw[pos++] = (objRSSI>>8);                                   // Topic ID
                pRSSI->raw[pos++] = (objRSSI & 0xFF);
                pRSSI->raw[pos++] = 0xBA;                                           // Message ID
                pRSSI->raw[pos++] = 0xBA;
                uint8_t rssi = PHY2_GetRSSI();
                pRSSI->raw[pos++] = rssi;
                if((rssi & 0x80) == 0)
                    pRSSI->raw[pos++] = 0xFF;                                       // RSSI < 0
                pRSSI->Length = pos;
                PHY1_Send(pRSSI);
            }
#endif
            case MQTTSN_MSGTYP_REGISTER:
            case MQTTSN_MSGTYP_REGACK:
            case MQTTSN_MSGTYP_PUBLISH:
            case MQTTSN_MSGTYP_PUBACK:
            case MQTTSN_MSGTYP_SUBSCRIBE:
            case MQTTSN_MSGTYP_DISCONNECT:
#ifdef MQTTSN_USE_DHCP
            case MQTTSN_MSGTYP_DHCPREQ:
#endif  //  MQTTSN_USE_DHCP
            case MQTTSN_MSGTYP_FORWARD:
            {
                uint8_t Size = (MQTTSN_SIZEOF_MSG_FORWARD + sizeof(PHY2_ADDR_t) + 1);
                uint8_t Length = pPHY2outBuf->Length + Size;

                if(Length <= sizeof(MQTTSN_MESSAGE_t))
                {
                    uint8_t pos;
                    for(pos = (Length - 1); pos >= Size; pos--)
                        pPHY2outBuf->raw[pos] = pPHY2outBuf->raw[pos - Size];

                    // Make forward message
                    pPHY2outBuf->Length = Length;
                    pPHY2outBuf->mq.Length = Size;
                    pPHY2outBuf->mq.MsgType = MQTTSN_MSGTYP_FORWARD;
                    pPHY2outBuf->mq.forward.Ctrl = 0;   // ?? TTL
                    pPHY2outBuf->mq.forward.wNodeID[0] = 2;     // PHY2
                    memcpy(&pPHY2outBuf->mq.forward.wNodeID[1], pPHY2outBuf->phy2addr, sizeof(PHY2_ADDR_t));
                    memcpy(pPHY2outBuf->phy1addr, vMQ_GatewayAddr, sizeof(PHY1_ADDR_t));
                    PHY1_Send(pPHY2outBuf);
                    return;
                }
                break;
            }
            default:
                break;
        }
    }
    mqFree(pPHY2outBuf);
}
#endif  //  PHY2_Get

// End parse incoming messages
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// Poll Task

// Build Name
static uint8_t mqttsn_build_node_name(uint8_t * pBuf)
{
    uint8_t Length = MQTTSN_SIZEOF_CLIENTID;
    ReadOD(objNodeName, MQTTSN_FL_TOPICID_PREDEF, &Length, pBuf);
    if(Length > 1)
        return Length;

    // Node Name not defined, use default name
    uint8_t pos;
    Length = OD_DEV_TYP_LEN;
    ReadOD(objDeviceTyp, MQTTSN_FL_TOPICID_PREDEF, &Length, pBuf);
    pBuf += Length;
    *(pBuf++) = '_';
    Length++;
    
    uint8_t * pAddr = PHY1_GetAddr(); 

    for(pos = 0; pos < sizeof(PHY1_ADDR_t); pos++)
    {
        uint8_t zif = *(pAddr++);
    
        uint8_t ch = zif>>4;
        if(ch > 0x09)
            ch += 0x37;
        else
            ch += 0x30;
        *pBuf = ch;
        pBuf++;
    
        ch = zif & 0x0F;
        if(ch > 0x09)
            ch += 0x37;
        else
            ch += 0x30;
        *pBuf = ch;
        pBuf++;
    }
    Length += (sizeof(PHY1_ADDR_t)*2);

    return Length;
}

#if ((defined MQTTSN_USE_MESH) || (defined PHY2_ADDR_t))
// Send Gateway Info message
static void mqttsn_send_gwinfo(uint8_t phy)
{
#if (defined MQTTSN_USE_MESH)
    if(phy == 1)
    {
        MQ_t * pGWInfo = mqAlloc(sizeof(MQ_t));
        memcpy(pGWInfo->phy1addr, &addr1_broad, sizeof(PHY1_ADDR_t));
        pGWInfo->Length = MQTTSN_SIZEOF_MSG_GWINFO;
        pGWInfo->mq.Length = MQTTSN_SIZEOF_MSG_GWINFO;
        pGWInfo->mq.MsgType = MQTTSN_MSGTYP_GWINFO;
        pGWInfo->mq.gwinfo.GwId = vMQ_GwId;
        //memcpy(pGWInfo->mq.gwinfo.GwAdd, vMQ_phy1addr, sizeof(PHY1_ADDR_t));
        PHY1_Send(pGWInfo);
        return;
    }
#endif  //  (defined MQTTSN_USE_MESH)
#if (defined PHY2_ADDR_t)
    if(phy == 2)
    {
        MQ_t * pGWInfo = mqAlloc(sizeof(MQ_t));
        memcpy(pGWInfo->phy2addr, &addr2_broad, sizeof(PHY2_ADDR_t));
        pGWInfo->Length = MQTTSN_SIZEOF_MSG_GWINFO;
        pGWInfo->mq.Length = MQTTSN_SIZEOF_MSG_GWINFO;
        pGWInfo->mq.MsgType = MQTTSN_MSGTYP_GWINFO;
        pGWInfo->mq.gwinfo.GwId = vMQ_GwId;
        //memcpy(pGWInfo->mq.gwinfo.GwAdd, vMQ_phy1addr, sizeof(PHY1_ADDR_t));
        PHY2_Send(pGWInfo);
        return;
    }
#endif  //  (defined PHY2_ADDR_t)
}
#endif  //  ((defined MQTTSN_USE_MESH) || (defined PHY2_ADDR_t))

#ifdef ASLEEP
void mqttsn_asleep(void)
{
#ifdef PHY1_ASleep
    PHY1_ASleep();                  // Disable PHY
#endif  //  PHY1_ASleep
    HAL_ASleep(vMQ_tASleep);    // Enter to Sleep mode
#ifdef PHY1_AWake
    PHY1_AWake();                   // Enable PHY
#endif  //  PHY1_AWake
}
#endif  //  ASLEEP

void MQTTSN_Poll(void)
{
    MQ_t * pMessage;

#if (defined MQTTSN_USE_MESH)
    // Gateway Info messages
    if(vMQ_tGWinfo1 > 0)
    {
        vMQ_tGWinfo1--;
        if(vMQ_tGWinfo1 == 0)
            mqttsn_send_gwinfo(1);
    }
#endif  //  (defined MQTTSN_USE_MESH)

#ifdef PHY2_ADDR_t
    if(vMQ_tGWinfo2 > 0)
    {
        vMQ_tGWinfo2--;
        if(vMQ_tGWinfo2 == 0)
            mqttsn_send_gwinfo(2);
    }
#endif  //  PHY2_ADDR_t
    
    // Normal Messages
    if(vMQ_tRetry > 0)
    {
        vMQ_tRetry--;
        if(vMQ_tRetry != 0)
            return;
    }

    switch(vMQ_Status)
    {
        case MQTTSN_STATUS_CONNECT:
        {
            if(vMQ_nRetry > 0)
                vMQ_nRetry--;
            else
            {
#ifndef MQTTSN_REBOOT_ON_LOST
                vMQ_tRetry = 1;
                vMQ_Status = MQTTSN_STATUS_DISCONNECTED;
#else
                HAL_Reboot();
#endif  //  MQTTSN_REBOOT_ON_LOST
                return;
            }

            // Build Publish message
            if(vMQ_MsgType == MQTTSN_MSGTYP_PUBLISH)
            {
                vMQ_tRetry = MQTTSN_T_RETR_BASED;

                uint8_t qos = vMQ_pMessage->mq.publish.Flags & MQTTSN_FL_QOS_MASK;
                if(qos == MQTTSN_FL_QOS1)
                {
                    pMessage = mqAlloc(sizeof(MQ_t));
                    memcpy(pMessage, vMQ_pMessage, sizeof(MQ_t));
                    vMQ_pMessage->mq.publish.Flags |= MQTTSN_FL_DUP;
                }
                else    // ToDo, only for QoS0/QoS-1, QoS2 not supported yet.
                {
                    pMessage = vMQ_pMessage;
                    vMQ_MsgType = MQTTSN_MSGTYP_PINGREQ;
                }
            }
            else if(vMQ_MsgType == MQTTSN_MSGTYP_REGISTER)  // Send Register message
            {
                vMQ_tRetry = MQTTSN_T_RETR_BASED;
                pMessage = mqAlloc(sizeof(MQ_t));
                memcpy(pMessage, vMQ_pMessage, sizeof(MQ_t));
            }
            else    // No messages, send PingReq
            {
#ifdef ASLEEP
                if(vMQ_tASleep != 0)
                {
                    vMQ_nRetry = MQTTSN_N_RETRIES;
                    vMQ_Status = MQTTSN_STATUS_ASLEEP_DISCONNECT;
                    return;
                }
#endif  //  ASLEEP

                pMessage = mqAlloc(sizeof(MQ_t));
                memcpy(pMessage->phy1addr, vMQ_GatewayAddr, sizeof(PHY1_ADDR_t));
                pMessage->Length = MQTTSN_SIZEOF_MSG_PINGREQ;
                pMessage->mq.Length = MQTTSN_SIZEOF_MSG_PINGREQ;
                pMessage->mq.MsgType = MQTTSN_MSGTYP_PINGREQ;

                vMQ_tRetry = (const uint16_t)(MQTTSN_T_KEEPALIVE * POLL_TMR_FREQ);
            }
            break;
        }
        case MQTTSN_STATUS_DISCONNECTED:
        {
            PHY1_Init();
#ifdef PHY2_ADDR_t
            PHY2_Init();
#endif  //  PHY2_ADDR_t

            MQTTSN_Init();

            vMQ_tRetry = POLL_TMR_FREQ/10;
            return;
        }
#ifdef MQTTSN_USE_DHCP
        case MQTTSN_STATUS_DHCP:
        {
            if(vMQ_nRetry > 0)
                vMQ_nRetry--;
            else
            {
                if(vMQ_Radius < MQTTSN_MAX_RADIUS)
                {
                    vMQ_Radius++;
                    vMQ_nRetry = MQTTSN_N_RETRIES;
                    return;
                }
                else
                {
#ifdef ASLEEP
                    if(vMQ_tASleep != 0)
                    {
                        mqttsn_asleep();
                        vMQ_tRetry = 1;
                    }
                    else
#endif  //  ASLEEP
                        vMQ_tRetry = MQTTSN_T_DISCONNECT;

                    vMQ_Status = MQTTSN_STATUS_DISCONNECTED;
                    return;
                }
            }
            vMQ_tRetry = MQTTSN_T_SEARCHGW;

            pMessage = mqAlloc(sizeof(MQ_t));
            // Make DHCP request
            vMQ_MsgId = HAL_RNG();

            uint8_t Length = 0;
            if(memcmp(PHY1_GetAddr(), &addr1_undef, sizeof(PHY1_ADDR_t)) == 0)
                pMessage->mq.dhcpreq.hlen[Length++] = sizeof(PHY1_ADDR_t);
#ifdef PHY2_ADDR_t
            if(memcmp(PHY2_GetAddr(), &addr2_undef, sizeof(PHY2_ADDR_t))== 0)
                pMessage->mq.dhcpreq.hlen[Length++] = sizeof(PHY2_ADDR_t);
#endif  //  PHY2_ADDR_t

            memcpy(pMessage->phy1addr, &addr1_broad, sizeof(PHY1_ADDR_t));
            pMessage->mq.MsgType = MQTTSN_MSGTYP_DHCPREQ;
            pMessage->mq.dhcpreq.Radius = vMQ_Radius;
            pMessage->mq.dhcpreq.MsgId[0] = vMQ_MsgId >> 8;
            pMessage->mq.dhcpreq.MsgId[1] = vMQ_MsgId & 0xFF;
            Length += MQTTSN_SIZEOF_MSG_DHCPREQ;
            pMessage->Length = Length;
            pMessage->mq.Length = Length;
            break;
        }
#endif  //  MQTTSN_USE_DHCP
        case MQTTSN_STATUS_SEARCHGW:
        {
            if(vMQ_nRetry > 0)
                vMQ_nRetry--;
            else
            {
                if(vMQ_Radius < MQTTSN_MAX_RADIUS)
                {
                    vMQ_Radius++;
                    vMQ_nRetry = MQTTSN_N_RETRIES;
                    return;
                }
                else
                {
#ifdef ASLEEP
                    if(vMQ_tASleep != 0)
                    {
                        mqttsn_asleep();
                        vMQ_tRetry = 1;
                    }
                    else
#endif  //  ASLEEP
                        vMQ_tRetry = MQTTSN_T_DISCONNECT;
                    vMQ_Status = MQTTSN_STATUS_DISCONNECTED;
                    return;
                }
            }
            vMQ_tRetry = MQTTSN_T_SEARCHGW;

            pMessage = mqAlloc(sizeof(MQ_t));
            memcpy(pMessage->phy1addr, &addr1_broad, sizeof(PHY1_ADDR_t));
            pMessage->Length = MQTTSN_SIZEOF_MSG_SEARCHGW;
            pMessage->mq.Length = MQTTSN_SIZEOF_MSG_SEARCHGW;
            pMessage->mq.MsgType = MQTTSN_MSGTYP_SEARCHGW;
            pMessage->mq.searchgw.Radius = vMQ_Radius;
            break;
        }
        case MQTTSN_STATUS_OFFLINE:
        {
            if(vMQ_nRetry > 0)
                vMQ_nRetry--;
            else
            {
                vMQ_MsgId = 0;
                vMQ_Radius = 1;
                vMQ_nRetry = MQTTSN_N_RETRIES;
                vMQ_tRetry = MQTTSN_T_SEARCHGW;
                vMQ_Status = MQTTSN_STATUS_SEARCHGW;
                return;
            }
            vMQ_tRetry = MQTTSN_T_CONNECT;

            pMessage = mqAlloc(sizeof(MQ_t));

            // Make connect message
            memcpy(pMessage->phy1addr, vMQ_GatewayAddr, sizeof(PHY1_ADDR_t));
            pMessage->mq.MsgType = MQTTSN_MSGTYP_CONNECT;

            pMessage->mq.connect.Flags = MQTTSN_FL_CLEANSESSION;
            pMessage->mq.connect.ProtocolId = MQTTSN_PROTOCOLID;
            pMessage->mq.connect.Duration[0] = (const uint8_t)(MQTTSN_T_KEEPALIVE>>8);
            pMessage->mq.connect.Duration[1] = (const uint8_t)(MQTTSN_T_KEEPALIVE & 0xFF);

            uint8_t Length = mqttsn_build_node_name((uint8_t *)&pMessage->mq.connect.ClientId);
            Length += MQTTSN_SIZEOF_MSG_CONNECT;
            pMessage->Length = Length;
            pMessage->mq.Length = Length;
            break;
        }
        case MQTTSN_STATUS_PRE_CONNECT:
        {
            if(vMQ_nRetry > 0)
                vMQ_nRetry--;
            else
            {
                vMQ_tRetry = 1;
                vMQ_Status = MQTTSN_STATUS_DISCONNECTED;
                return;
            }

            vMQ_tRetry = MQTTSN_T_RETR_BASED;

            if((vMQ_MsgType == MQTTSN_MSGTYP_REGISTER) ||   // Send Register message
               (vMQ_MsgType == MQTTSN_MSGTYP_SUBSCRIBE))    // Subscribe message
            {
                pMessage = mqAlloc(sizeof(MQ_t));
                memcpy(pMessage, vMQ_pMessage, sizeof(MQ_t));
            }
            // Build Publish message
            else if(vMQ_MsgType == MQTTSN_MSGTYP_PUBLISH)
            {
                uint8_t qos = vMQ_pMessage->mq.publish.Flags & MQTTSN_FL_QOS_MASK;
                if(qos == MQTTSN_FL_QOS1)
                {
                    pMessage = mqAlloc(sizeof(MQ_t));
                    memcpy(pMessage, vMQ_pMessage, sizeof(MQ_t));
                    vMQ_pMessage->mq.publish.Flags |= MQTTSN_FL_DUP;
                }
                else    // ToDo, only for QoS0/QoS-1, QoS2 not supported yet.
                {
                    pMessage = vMQ_pMessage;
                    vMQ_MsgType = MQTTSN_MSGTYP_PINGREQ;
                }
            }
            else    // Paronoidal check
            {
                vMQ_MsgType = MQTTSN_MSGTYP_PINGREQ;
                return;
            }
            break;
        }

#ifdef ASLEEP
        case MQTTSN_STATUS_ASLEEP_DISCONNECT:
        {
            // Send Disconnect with Asleep duration
            if(vMQ_nRetry > 0)
                vMQ_nRetry--;
            else
            {
                vMQ_tRetry = 1;
                vMQ_Status = MQTTSN_STATUS_DISCONNECTED;
                return;
            }

            pMessage = mqAlloc(sizeof(MQ_t));
            memcpy(pMessage->phy1addr, vMQ_GatewayAddr, sizeof(PHY1_ADDR_t));
            pMessage->Length = MQTTSN_SIZEOF_MSG_DISCONNECTL;
            pMessage->mq.Length = MQTTSN_SIZEOF_MSG_DISCONNECTL;
            pMessage->mq.MsgType = MQTTSN_MSGTYP_DISCONNECT;
            pMessage->mq.disconnect.Duration[0] = (vMQ_tASleep>>8);
            pMessage->mq.disconnect.Duration[1] = (vMQ_tASleep & 0xFF);
            vMQ_tRetry = MQTTSN_T_SHORT;
            break;
        }
        case MQTTSN_STATUS_ASLEEP:
        {
            vMQ_nRetry = MQTTSN_N_RETRIES;
            vMQ_Status = MQTTSN_STATUS_AWAKE;

            if((vMQ_MsgType == MQTTSN_MSGTYP_PINGREQ) && (vMQ_tASleep != 0))
            {
                mqttsn_asleep();
                vMQ_tRetry = MQTTSN_T_AWAKE;
                return;
            }
            vMQ_tRetry = 1;
            return;
        }
        case MQTTSN_STATUS_AWAKE:
        {
            if(vMQ_nRetry > 0)
                vMQ_nRetry--;
            else
            {
                vMQ_tRetry = 1;
                vMQ_Status = MQTTSN_STATUS_DISCONNECTED;
                return;
            }
            vMQ_tRetry = MQTTSN_T_SHORT;

            pMessage = mqAlloc(sizeof(MQ_t));
            memcpy(pMessage->phy1addr, vMQ_GatewayAddr, sizeof(PHY1_ADDR_t));

            if((vMQ_MsgType == MQTTSN_MSGTYP_PINGREQ) && (vMQ_tASleep != 0))
            {
                // Make ping message
                pMessage->Length = MQTTSN_SIZEOF_MSG_PINGREQ;
                pMessage->mq.Length = MQTTSN_SIZEOF_MSG_PINGREQ;
                pMessage->mq.MsgType = MQTTSN_MSGTYP_PINGREQ;
            }
            else    // data present
            {
                // Make connect message
                pMessage->mq.MsgType = MQTTSN_MSGTYP_CONNECT;
                pMessage->mq.connect.Flags = 0;
                pMessage->mq.connect.ProtocolId = MQTTSN_PROTOCOLID;
                pMessage->mq.connect.Duration[0] = (const uint8_t)(MQTTSN_T_KEEPALIVE>>8);
                pMessage->mq.connect.Duration[1] = (const uint8_t)(MQTTSN_T_KEEPALIVE & 0xFF);

                uint8_t Length = mqttsn_build_node_name((uint8_t *)&pMessage->mq.connect.ClientId);
                Length += MQTTSN_SIZEOF_MSG_CONNECT;
                pMessage->Length = Length;
                pMessage->mq.Length = Length;
            }
            break;
        }
#endif  //  ASLEEP

        default:
            return;
    }

    PHY1_Send(pMessage);
}

// End Poll Task
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// API

// Initialize MQTTSN tasks
void MQTTSN_Init(void)
{
    uint8_t uTmp = sizeof(PHY1_ADDR_t);
    ReadOD(PHY1_GateId, MQTTSN_FL_TOPICID_PREDEF, &uTmp, (uint8_t *)vMQ_GatewayAddr);

#ifdef MQTTSN_USE_DHCP
    if(memcmp(PHY1_GetAddr(), &addr1_undef, sizeof(PHY1_ADDR_t)) == 0)
        vMQ_Status = MQTTSN_STATUS_DHCP;
    else
#ifdef PHY2_ADDR_t
    if(memcmp(PHY2_GetAddr(), &addr2_undef, sizeof(PHY2_ADDR_t)) == 0)
        vMQ_Status = MQTTSN_STATUS_DHCP;
    else
#endif  //  PHY2_ADDR_t
#endif  //  MQTTSN_USE_DHCP
    if(memcmp(vMQ_GatewayAddr, &addr1_undef, sizeof(PHY1_ADDR_t)) == 0)
        vMQ_Status = MQTTSN_STATUS_SEARCHGW;
    else
        vMQ_Status = MQTTSN_STATUS_OFFLINE;

    vMQ_GwId = 0;
    vMQ_Radius = 1;
    vMQ_tRetry = (POLL_TMR_FREQ*2);
    vMQ_nRetry = MQTTSN_N_RETRIES;

    vMQ_MsgId = 0;
    vMQ_oMsgId = 0xFFFF;

    if((vMQ_MsgType != MQTTSN_MSGTYP_PINGREQ) && (vMQ_pMessage != NULL))
        mqFree(vMQ_pMessage);
    vMQ_MsgType = MQTTSN_MSGTYP_PINGREQ;
    
#if (defined MQTTSN_USE_MESH)
    vMQ_tGWinfo1 = 0;
#endif  //  (defined MQTTSN_USE_MESH)
#ifdef PHY2_ADDR_t
    vMQ_tGWinfo2 = 0;
#endif  //  PHY2_ADDR_t
#ifdef  ASLEEP
    uTmp = sizeof(vMQ_tASleep);
    ReadOD(objTASleep, MQTTSN_FL_TOPICID_PREDEF, &uTmp, (uint8_t *)&vMQ_tASleep);
#endif  //  ASLEEP
}

#ifdef ASLEEP
void MQTTSN_Set_ASleep(uint16_t val)
{
    vMQ_tASleep = val;
}
#endif  //  ASLEEP

// Get MQTTSN Status
e_MQTTSN_STATUS_t MQTTSN_GetStatus(void)
{
    return vMQ_Status;
}

bool MQTTSN_CanSend(void)
{
    return (vMQ_MsgType == MQTTSN_MSGTYP_PINGREQ);
}

void MQTTSN_Send(e_MQTTSN_MSGTYPE_t      MsgType,
                 uint8_t                 Flags,
                 uint16_t                TopicId)
{
    MQ_t * pMessage = mqAlloc(sizeof(MQ_t));
    uint8_t Length;
    uint16_t MessageId = mqttsn_new_msgid();

    if(MsgType == MQTTSN_MSGTYP_PUBLISH)
    {
        // Make Publish message
        pMessage->mq.publish.Flags = Flags;
        pMessage->mq.publish.TopicId[0] = TopicId>>8;
        pMessage->mq.publish.TopicId[1] = TopicId & 0xFF;
        pMessage->mq.publish.MsgId[0] = MessageId>>8;
        pMessage->mq.publish.MsgId[1] = MessageId & 0xFF;

        Length = (MQTTSN_MSG_SIZE - 5);
        ReadODpack(TopicId, Flags, &Length, pMessage->mq.publish.Data);
        Length += MQTTSN_SIZEOF_MSG_PUBLISH;
    }
    else if(MsgType == MQTTSN_MSGTYP_REGISTER)
    {
        // Make Register message
        pMessage->mq.regist.TopicId[0] = TopicId>>8;
        pMessage->mq.regist.TopicId[1] = TopicId & 0xFF;
        pMessage->mq.regist.MsgId[0] = MessageId>>8;
        pMessage->mq.regist.MsgId[1] = MessageId & 0xFF;
        Length = MakeTopicName(Flags, pMessage->mq.regist.TopicName);
        Length += MQTTSN_SIZEOF_MSG_REGISTER;
    }
    else if(MsgType == MQTTSN_MSGTYP_SUBSCRIBE)
    {
        // Make Subscribe message
        pMessage->mq.subscribe.Flags = Flags;
        pMessage->mq.subscribe.MsgId[0] = MessageId>>8;
        pMessage->mq.subscribe.MsgId[1] = MessageId & 0xFF;
        pMessage->mq.subscribe.Topic[0] = '#';
        Length = (MQTTSN_SIZEOF_MSG_SUBSCRIBE + 1);
    }
    else    // Parnoidal section, unknown message
    {
        mqFree(pMessage);
        return;
    }

    memcpy(pMessage->phy1addr, vMQ_GatewayAddr, sizeof(PHY1_ADDR_t));
    
    pMessage->Length = Length;
    pMessage->mq.Length = Length;

    vMQ_MsgType  = MsgType;
    pMessage->mq.MsgType = MsgType;
    
    vMQ_pMessage = pMessage;

    if((vMQ_Status == MQTTSN_STATUS_PRE_CONNECT) ||
       (vMQ_Status == MQTTSN_STATUS_CONNECT))
    {
        vMQ_tRetry = 0;
        vMQ_nRetry = MQTTSN_N_RETRIES;
    }
}
