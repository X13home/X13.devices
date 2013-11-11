/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

// MQTT-S Library

#include "config.h"

#define POOL_TMR_FREQ_FAST  (POOL_TMR_FREQ/4 - 1)

static MQTTS_VAR_t vMQTTS;

static uint16_t mqtts_new_msgid(void)
{
    vMQTTS.MsgID++;
    if(vMQTTS.MsgID > 0xFFFE)
        vMQTTS.MsgID = 1;
    return SWAPWORD(vMQTTS.MsgID);
}

static uint8_t mqtts_check_msgid(uint16_t msgid)
{
    uint16_t tmp = SWAPWORD(msgid);

    if(tmp == vMQTTS.inMsgId)   // Dup
        return 1;

//    uint8_t delta = vMQTTS.MsgID < msgid ? msgid - vMQTTS.MsgID : vMQTTS.MsgID - msgid;
//    if(delta > MQTTS_DEF_MSGID_DELTA)
//        return 2;

    vMQTTS.inMsgId = tmp;
    return 0;
}

static void mqtts_inc_tail(void)
{
    if(vMQTTS.fHead == vMQTTS.fTail)
        return;
    mqRelease(vMQTTS.fBuf[vMQTTS.fTail++]);
    if(vMQTTS.fTail >= MQTTS_SIZEOF_SEND_FIFO)
        vMQTTS.fTail -= MQTTS_SIZEOF_SEND_FIFO;
    if(vMQTTS.fHead == vMQTTS.fTail)
    {
      vMQTTS.fHead = 0;
      vMQTTS.fTail = 0;
    
      vMQTTS.Tretry = 0;
      vMQTTS.pfCnt = (POOL_TMR_FREQ - 1);
    }

    vMQTTS.Nretry = MQTTS_DEF_NRETRY;
}

static uint8_t MQTTS_Push(MQ_t * pBuf)
{
    uint8_t tmphead = vMQTTS.head + 1;
    if(tmphead >= MQTTS_SIZEOF_SEND_FIFO)
        tmphead -= MQTTS_SIZEOF_SEND_FIFO;
    if(tmphead == vMQTTS.tail)          // Overflow
    {
        mqRelease(pBuf);
        return 0xFF;
    }
        
    vMQTTS.buf[vMQTTS.head] = pBuf;
    vMQTTS.head = tmphead;
    return 0;
}

static uint8_t MQTTS_ToBuf(MQ_t * pBuf)
{
    uint8_t qos;
    if(pBuf->mq.MsgType == MQTTS_MSGTYP_PUBLISH)
        qos = pBuf->mq.m.publish.Flags & MQTTS_FL_QOS_MASK;
    else if(pBuf->mq.MsgType == MQTTS_MSGTYP_SUBSCRIBE)
        qos = pBuf->mq.m.subscribe.Flags & MQTTS_FL_QOS_MASK;
    else if(pBuf->mq.MsgType == MQTTS_MSGTYP_REGISTER)
        qos = MQTTS_FL_QOS1;
    else
        qos = 0;

    if((qos == MQTTS_FL_QOS1) || (qos == MQTTS_FL_QOS2))
    {
        uint8_t tmphead = vMQTTS.fHead + 1;
        if(tmphead >= MQTTS_SIZEOF_SEND_FIFO)
            tmphead -= MQTTS_SIZEOF_SEND_FIFO;
        if(tmphead == vMQTTS.fTail)          // Overflow
        {
            mqRelease(pBuf);
            return 0xFF;
        }

        MQ_t * ppBuf = mqAssert();
        if(ppBuf == NULL)
        {
            mqRelease(pBuf);
            return 0xFF;
        }
        memcpy(ppBuf, pBuf, sizeof(MQ_t));
        
        if(pBuf->mq.MsgType == MQTTS_MSGTYP_PUBLISH)
            ppBuf->mq.m.publish.Flags |= MQTTS_FL_DUP;
        else if(pBuf->mq.MsgType == MQTTS_MSGTYP_SUBSCRIBE)
            ppBuf->mq.m.subscribe.Flags |= MQTTS_FL_DUP;

        if(vMQTTS.fHead == vMQTTS.fTail)
        {
            vMQTTS.Tretry = 0;
            vMQTTS.pfCnt = POOL_TMR_FREQ_FAST;
        }

        vMQTTS.fBuf[vMQTTS.fHead] = ppBuf;
        vMQTTS.fHead = tmphead;
    }

    return MQTTS_Push(pBuf);
}

uint8_t MQTTS_DataRdy(void)
{
  return (vMQTTS.tail != vMQTTS.head) ? 1 : 0;
}

MQ_t * MQTTS_Get(void)
{
    if(vMQTTS.tail == vMQTTS.head)
        return NULL;
    uint8_t tmptail = vMQTTS.tail;
    vMQTTS.tail++;
    if(vMQTTS.tail >= MQTTS_SIZEOF_SEND_FIFO)
        vMQTTS.tail -= MQTTS_SIZEOF_SEND_FIFO;
    return vMQTTS.buf[tmptail];
}

static void MQTTS_Disconnect(void)
{
    // Reset buffers
    vMQTTS.head = 0;
    vMQTTS.tail = 0;
    vMQTTS.fHead = 0;
    vMQTTS.fTail = 0;
// (Re)Initialize Memory manager
    mqInit();
#ifndef GATEWAY
    vMQTTS.Status = MQTTS_STATUS_SEARCHGW;
#else   // Gateway
    vMQTTS.Status = MQTTS_STATUS_OFFLINE;
#endif  // Gateway
    vMQTTS.pfCnt = POOL_TMR_FREQ - 1;
    vMQTTS.Tretry = 0;
    vMQTTS.Nretry = MQTTS_DEF_NRETRY;
    vMQTTS.MsgID = 0;
    vMQTTS.ReturnCode = 0xFF;

    CleanOD();
#ifdef GATEWAY
    vMQTTS.GatewayID = rf_GetNodeID();
#endif  //  GATEWAY
}

#ifdef ASLEEP
void mqtts_set_TASleep(uint16_t tasleep)
{
    vMQTTS.Tasleep = tasleep;
}
#endif  //  ASLEEP

uint8_t MQTTS_Publish(uint16_t TopicID, uint8_t Flags, uint8_t Size, uint8_t * ipBuf)
{
    uint8_t mSize = Size + MQTTS_SIZEOF_MSG_PUBLISH + 1;
    if(mSize > sizeof(MQ_t))
        return 0xFF;
    
    MQ_t * pBuf = mqAssert();
    if(pBuf != NULL)    // no memory
    {
        pBuf->addr = vMQTTS.GatewayID;
        pBuf->mq.Length = mSize - 1;
        pBuf->mq.MsgType = MQTTS_MSGTYP_PUBLISH;
        pBuf->mq.m.publish.Flags = Flags;
        pBuf->mq.m.publish.TopicId = SWAPWORD(TopicID);
        pBuf->mq.m.publish.MsgId = mqtts_new_msgid();
        memcpy(&pBuf->mq.m.publish.Data, ipBuf, Size);
        
        return MQTTS_ToBuf(pBuf);
    }
    return 0xFF;
}

uint8_t MQTTS_Subscribe(uint8_t Flags, uint8_t Size, uint8_t * ipBuf)
{
    uint8_t mSize = Size + MQTTS_SIZEOF_MSG_SUBSCRIBE + 1;
    if(mSize > sizeof(MQ_t))
        return 0xFF;

    MQ_t * pBuf = mqAssert();
    if(pBuf != NULL)
    {
        pBuf->addr = vMQTTS.GatewayID;
        pBuf->mq.Length = mSize - 1;
        pBuf->mq.MsgType = MQTTS_MSGTYP_SUBSCRIBE;
        pBuf->mq.m.subscribe.Flags = Flags;
        pBuf->mq.m.subscribe.MsgId = mqtts_new_msgid();
        memcpy(&pBuf->mq.m.subscribe.Topic, ipBuf, Size);

        return MQTTS_ToBuf(pBuf);
    }
    return 0xFF;
}

uint8_t MQTTS_Register(uint16_t TopicID, uint8_t Size, uint8_t * ipBuf)
{
    uint8_t mSize = Size + MQTTS_SIZEOF_MSG_REGISTER + 1;
    if(mSize > sizeof(MQ_t))
        return 0xFF;

    MQ_t * pBuf = mqAssert();
    if(pBuf != NULL)
    {
        pBuf->addr = vMQTTS.GatewayID;
        pBuf->mq.Length = mSize - 1;
        pBuf->mq.MsgType = MQTTS_MSGTYP_REGISTER;
        pBuf->mq.m.regist.TopicId = SWAPWORD(TopicID);
        pBuf->mq.m.regist.MsgId =  mqtts_new_msgid();
        memcpy(&pBuf->mq.m.regist.TopicName, ipBuf, Size);
        
        return MQTTS_ToBuf(pBuf);
    }
    return 0xFF;
}

void MQTTS_Init(void)
{
    MQTTS_Disconnect();
}

#ifndef GATEWAY
static void mqtts_send_search_gw(void)  // Send Search Gateway
{
    MQ_t * pBuf;
    pBuf = mqAssert();
    if(pBuf == NULL)    // no memory
        return;
    pBuf->addr = (s_Addr)AddrBroadcast;             // Broadcast
    pBuf->mq.Length = MQTTS_SIZEOF_MSG_SEARCHGW;
    pBuf->mq.MsgType = MQTTS_MSGTYP_SEARCHGW;
    pBuf->mq.m.searchgw.Radius = 0;
    MQTTS_Push(pBuf);
}
#endif  //  GATEWAY

static void mqtts_send_connect()        // Send  CONNECT
{
    MQ_t * pBuf;
    pBuf = mqAssert();
    if(pBuf == NULL)    // no memory
        return;

    pBuf->addr = vMQTTS.GatewayID;
    if(vMQTTS.MsgID == 0)
        pBuf->mq.m.connect.Flags = MQTTS_FL_CLEANSESSION;
    else
        pBuf->mq.m.connect.Flags = 0;
    
    pBuf->mq.m.connect.ProtocolId = MQTTS_DEF_PROTOCOLID;
    pBuf->mq.m.connect.Duration = SWAPWORD(MQTTS_DEF_KEEPALIVE);
    // Build Name
    uint8_t * ipBuf;
    ipBuf = pBuf->mq.m.connect.ClientId;
    pBuf->mq.Length = MQTTS_SIZEOF_CLIENTID - 3;
    ReadOD(objNodeName, MQTTS_FL_TOPICID_PREDEF, &pBuf->mq.Length, ipBuf);
    if(pBuf->mq.Length < 2)     // Node Name not defined, use default name
    {
        pBuf->mq.Length = OD_DEV_TYP_LEN;
        ReadOD(objDeviceTyp, MQTTS_FL_TOPICID_PREDEF, &pBuf->mq.Length, ipBuf);
        ipBuf += pBuf->mq.Length;
        *(ipBuf++) = '_';
        pBuf->mq.Length++;
        pBuf->mq.Length += PHY_BuildName(ipBuf);
    }
    pBuf->mq.Length += MQTTS_SIZEOF_MSG_CONNECT;
    pBuf->mq.MsgType = MQTTS_MSGTYP_CONNECT;
    MQTTS_Push(pBuf);
}

static void mqtts_send_ping()       // Send Ping request
{
    MQ_t * pBuf;
    pBuf = mqAssert();
    if(pBuf == NULL)    // no memory
        return;

    pBuf->addr = vMQTTS.GatewayID;
    pBuf->mq.Length = MQTTS_SIZEOF_MSG_PINGREQ;
    pBuf->mq.MsgType = MQTTS_MSGTYP_PINGREQ;
    MQTTS_Push(pBuf);
}

uint8_t MQTTS_Pool(uint8_t wakeup)
{
    if(vMQTTS.pfCnt)
    {
        vMQTTS.pfCnt--;
        return MQTTS_POOL_STAT_NOP;
    }

    switch(vMQTTS.Status)
    {
#ifndef GATEWAY
        case MQTTS_STATUS_SEARCHGW:
#ifndef ASLEEP
            vMQTTS.pfCnt = POOL_TMR_FREQ - 1;
            if(vMQTTS.Tretry)
            {
                vMQTTS.Tretry--;
                break;
            }
            vMQTTS.Tretry = MQTTS_DEF_TSGW - 1;

            if(vMQTTS.Nretry)
                vMQTTS.Nretry--;
            else
                SystemReset();
#else  //  ASLEEP
            if(vMQTTS.Tretry)
            {
                vMQTTS.Tretry--;
                if(vMQTTS.Tretry == (MQTTS_DEF_TSGW - 2))
                {
                    return MQTTS_POOL_STAT_ASLEEP;          // ASLeep
                }
                else if(vMQTTS.Tretry == 0)
                {
                    if(vMQTTS.Nretry)
                    {
                        vMQTTS.Nretry--;
                        vMQTTS.pfCnt = POOL_TMR_FREQ - 1;
                        return  MQTTS_POOL_STAT_AWAKE;          // WakeUp
                    }
                    // Not found many times
                    SystemReset();
                }
                break;
            }
            vMQTTS.pfCnt = POOL_TMR_FREQ - 1;
            vMQTTS.Tretry = MQTTS_DEF_TSGW - 1;
#endif  //  ASLEEP
            mqtts_send_search_gw();
            break;
#endif  //  GATEWAY
        case MQTTS_STATUS_OFFLINE:      // Connect to Broker
            vMQTTS.pfCnt = POOL_TMR_FREQ - 1;
            
            if(vMQTTS.Tretry)
            {
                vMQTTS.Tretry--;
                break;
            }

            if(vMQTTS.Nretry)
                vMQTTS.Nretry--;
            else
            {
#ifdef GATEWAY
                SystemReset();
#else   //    NODE
                vMQTTS.Nretry = MQTTS_DEF_NRETRY;
                vMQTTS.Status = MQTTS_STATUS_SEARCHGW;
                break;
#endif  // GATEWAY
            }
            
            vMQTTS.Tretry = MQTTS_DEF_TCONNECT - 1;
            mqtts_send_connect();
            break;
        case MQTTS_STATUS_CONNECT:
            vMQTTS.pfCnt = POOL_TMR_FREQ - 1;
            
            if(vMQTTS.Tretry)
            {
                vMQTTS.Tretry--;
                break;
            }

            if(vMQTTS.Nretry)
                vMQTTS.Nretry--;
            else
            {
                MQTTS_Disconnect();
                break;
            }

            if(vMQTTS.fTail == vMQTTS.fHead)        // Send Ping
            {
                if(wakeup == 0)
                    mqtts_send_ping();
            }
            else
            {
                MQ_t * pBuf;
                pBuf = mqAssert();
                if(pBuf != NULL)    // no memory
                {
                    memcpy(pBuf, vMQTTS.fBuf[vMQTTS.fTail], sizeof(MQ_t));
                    MQTTS_Push(pBuf);
                }

                vMQTTS.pfCnt = POOL_TMR_FREQ_FAST;
            }
            break;
#ifdef ASLEEP
        case MQTTS_STATUS_POST_CONNECT:
            {
            vMQTTS.pfCnt = (POOL_TMR_FREQ - 1);
            MQ_t * pBuf;
            pBuf = mqAssert();
            if(pBuf != NULL)    // no memory
            {
                // Send disconnect
                pBuf->mq.Length = MQTTS_SIZEOF_MSG_DISCONNECTL;
                pBuf->mq.MsgType = MQTTS_MSGTYP_DISCONNECT;
                pBuf->mq.m.disconnect.Duration = SWAPWORD(vMQTTS.Tasleep);
                MQTTS_Push(pBuf);
            }
            }
            if(vMQTTS.Nretry)
            {
                vMQTTS.Nretry--;
                break;
            }
        case MQTTS_STATUS_PRE_ASLEEP:
            vMQTTS.Status = MQTTS_STATUS_ASLEEP;
            vMQTTS.Tretry = vMQTTS.Tasleep;
            return MQTTS_POOL_STAT_ASLEEP;
        case MQTTS_STATUS_ASLEEP:
            if(vMQTTS.Tretry)
            {
                vMQTTS.Tretry--;
                break;
            }
            
            vMQTTS.pfCnt = POOL_TMR_FREQ - 1;
            vMQTTS.Status = MQTTS_STATUS_AWAKE;
            vMQTTS.Nretry = MQTTS_DEF_NRETRY;
            return MQTTS_POOL_STAT_AWAKE;
        case MQTTS_STATUS_AWAKE:
            vMQTTS.pfCnt = POOL_TMR_FREQ - 1;

            if(vMQTTS.Nretry)
            {
                vMQTTS.Nretry--;
                mqtts_send_ping();
            }
            else
                MQTTS_Disconnect();
            break;
        case MQTTS_STATUS_POST_AWAKE:
            if(wakeup != 0)
            {
                vMQTTS.Status = MQTTS_STATUS_OFFLINE;
                break;
            }

            vMQTTS.pfCnt = POOL_TMR_FREQ - 1;
            vMQTTS.Status = MQTTS_STATUS_PRE_ASLEEP;
            break;
#endif  //  ASLEEP
    }
    return MQTTS_POOL_STAT_NOP;
}

uint8_t MQTTS_Parser(MQ_t * pBuf)
{
    uint8_t tmp;
    switch(pBuf->mq.MsgType)
    {
        case MQTTS_MSGTYP_SEARCHGW:
#ifdef GATEWAY
            // Local Answer
            pBuf->mq.Length = MQTTS_SIZEOF_MSG_GWINFO;
            pBuf->mq.MsgType = MQTTS_MSGTYP_GWINFO;
            pBuf->mq.m.gwinfo.GwId = rf_GetNodeID();
            MQTTS_Push(pBuf);
            return 1;
#else   //  !GATEWAY
            if(vMQTTS.Status == MQTTS_STATUS_SEARCHGW)
            {
                vMQTTS.Tretry = MQTTS_DEF_TSGW;
                vMQTTS.Nretry = MQTTS_DEF_NRETRY;
            }
            break;
        case MQTTS_MSGTYP_ADVERTISE:
        case MQTTS_MSGTYP_GWINFO:
            if(vMQTTS.Status == MQTTS_STATUS_SEARCHGW)
            {
                vMQTTS.GatewayID =  pBuf->addr;
                vMQTTS.Status = MQTTS_STATUS_OFFLINE;
                vMQTTS.Tretry = 0;
            }
            break;
#endif  //  GATEWAY
        case MQTTS_MSGTYP_CONNACK:
            if(vMQTTS.Status == MQTTS_STATUS_OFFLINE)
            {
                vMQTTS.pfCnt = POOL_TMR_FREQ - 1;
                if(pBuf->mq.m.connack.ReturnCode == MQTTS_RET_ACCEPTED)
                {
                    vMQTTS.Status = MQTTS_STATUS_CONNECT;
                    vMQTTS.Tretry = 0;
                    vMQTTS.Nretry = MQTTS_DEF_NRETRY;
                    vMQTTS.inMsgId = 0;

                    // Publish Device Type
                    if((vMQTTS.MsgID == 0) && (rf_GetNodeID() != 0xFF))
                    {
                        pBuf->mq.Length = MQTTS_SIZEOF_CLIENTID - 1;
                        ReadOD(objDeviceTyp, MQTTS_FL_TOPICID_PREDEF, &pBuf->mq.Length, (uint8_t *)&pBuf->mq.m.raw);
                        MQTTS_Publish(objDeviceTyp, MQTTS_FL_QOS1 | MQTTS_FL_TOPICID_PREDEF,
                                      pBuf->mq.Length, (uint8_t *)&pBuf->mq.m.raw);
                    }
                }
                else
                {
                    MQTTS_Disconnect();
                    return 1;
                }
            }
            break;
        case MQTTS_MSGTYP_REGISTER:
            if(vMQTTS.Status < MQTTS_STATUS_CONNECT)
                break;

            tmp = mqtts_check_msgid(pBuf->mq.m.regist.MsgId);
            if(tmp == 0)    // New message
                vMQTTS.ReturnCode = RegisterOD(pBuf);
            else if(tmp > 1)
                break;

            // Build RegAck Message
            pBuf->mq.Length = MQTTS_SIZEOF_MSG_REGACK;
            pBuf->mq.MsgType = MQTTS_MSGTYP_REGACK;
            pBuf->mq.m.regack.ReturnCode = vMQTTS.ReturnCode;
            MQTTS_Push(pBuf);
            return 1;
         case MQTTS_MSGTYP_REGACK:
            if((vMQTTS.Status == MQTTS_STATUS_CONNECT) &&
               (vMQTTS.fBuf[vMQTTS.fTail]->mq.MsgType == MQTTS_MSGTYP_REGISTER))
            {
                uint16_t index = 0;

                if(pBuf->mq.m.regack.ReturnCode == MQTTS_RET_ACCEPTED)
                    index = SWAPWORD(pBuf->mq.m.regack.TopicId);

                RegAckOD(index);
                mqtts_inc_tail();
            }
            break;
        case MQTTS_MSGTYP_PUBLISH:
            if((vMQTTS.Status != MQTTS_STATUS_CONNECT)
#ifdef ASLEEP
            && (vMQTTS.Status !=MQTTS_STATUS_AWAKE)
#endif  //  ASLEEP
            )
                break;

            vMQTTS.pfCnt = (POOL_TMR_FREQ - 1);
            vMQTTS.Nretry = MQTTS_DEF_NRETRY;

            tmp = mqtts_check_msgid(pBuf->mq.m.publish.MsgId);

            if(tmp == 0)   // New message
            {
                vMQTTS.ReturnCode = WriteOD(SWAPWORD(pBuf->mq.m.publish.TopicId),
                                            pBuf->mq.m.publish.Flags | 0x80,
                                            pBuf->mq.Length - MQTTS_SIZEOF_MSG_PUBLISH,
                                            (uint8_t *)&pBuf->mq.m.publish.Data);
            }
            else if((!(pBuf->mq.m.publish.Flags & MQTTS_FL_DUP)) || (tmp > 1))
                break;

            uint8_t qos = pBuf->mq.m.publish.Flags & MQTTS_FL_QOS_MASK;
            if(qos == MQTTS_FL_QOS1)                                    // Need Ack
            {
                pBuf->mq.Length = MQTTS_SIZEOF_MSG_PUBACK;
                pBuf->mq.MsgType = MQTTS_MSGTYP_PUBACK;
                pBuf->mq.m.puback.TopicId = pBuf->mq.m.publish.TopicId;
                pBuf->mq.m.puback.MsgId = pBuf->mq.m.publish.MsgId;
                pBuf->mq.m.puback.ReturnCode = vMQTTS.ReturnCode;
                MQTTS_Push(pBuf);
                return 1;
            }
            break;
        case MQTTS_MSGTYP_PUBACK:
            if((vMQTTS.Status == MQTTS_STATUS_CONNECT) &&
               (vMQTTS.fBuf[vMQTTS.fTail]->mq.MsgType == MQTTS_MSGTYP_PUBLISH) &&
               (vMQTTS.fBuf[vMQTTS.fTail]->mq.m.publish.MsgId == pBuf->mq.m.puback.MsgId))
                mqtts_inc_tail();
            break;
        case MQTTS_MSGTYP_SUBACK:
            if((vMQTTS.Status == MQTTS_STATUS_CONNECT) &&
               (vMQTTS.fBuf[vMQTTS.fTail]->mq.MsgType == MQTTS_MSGTYP_SUBSCRIBE) &&
               (vMQTTS.fBuf[vMQTTS.fTail]->mq.m.subscribe.MsgId == pBuf->mq.m.suback.MsgId))
                mqtts_inc_tail();
            break;
        case MQTTS_MSGTYP_PINGRESP:
#ifdef ASLEEP
          if(vMQTTS.Status == MQTTS_STATUS_AWAKE)
          {
            vMQTTS.Status = MQTTS_STATUS_POST_AWAKE;
          }
          else
#endif  //  ASLEEP
          if((vMQTTS.Status == MQTTS_STATUS_CONNECT) && (vMQTTS.tail == vMQTTS.head))
          {
            vMQTTS.Tretry = MQTTS_DEF_KEEPALIVE;
#ifdef ASLEEP
            if(vMQTTS.Tasleep != 0)
            {
              vMQTTS.Status = MQTTS_STATUS_POST_CONNECT;
              vMQTTS.pfCnt = 0;
            }
#endif	// ASLEEP
          }
          else
            break;
            
          vMQTTS.Nretry = MQTTS_DEF_NRETRY;
          vMQTTS.pfCnt = POOL_TMR_FREQ - 1;

          break;
        case MQTTS_MSGTYP_DISCONNECT:
#ifdef ASLEEP
            if(vMQTTS.Status == MQTTS_STATUS_POST_CONNECT)
            {
                vMQTTS.Status = MQTTS_STATUS_PRE_ASLEEP;
                vMQTTS.pfCnt = POOL_TMR_FREQ - 1;
                break;
            }
#endif  //  ASLEEP
            MQTTS_Disconnect();
            return 1;
    }
    return 0;
}

uint8_t MQTTS_GetStatus(void)
{
    return vMQTTS.Status;
}