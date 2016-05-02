/*
Copyright (c) 2011-2016 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _MQ_TYPES_H
#define _MQ_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#define MQTTSN_PROTOCOLID           0x01

typedef enum e_MQTTSN_RETURNS
{
    MQTTSN_RET_ACCEPTED         =   0x00,       // Accepted
    MQTTSN_RET_REJ_CONG         =   0x01,       // Rejected: congestion
    MQTTSN_RET_REJ_INV_ID       =   0x02,       // Rejected: invalid topic ID
    MQTTSN_RET_REJ_NOT_SUPP     =   0x03,       // Rejected: not supported
}e_MQTTSN_RETURNS_t;

typedef enum e_MQTTSN_MSGTYPE
{
    MQTTSN_MSGTYP_ADVERTISE     =   0x00,
    MQTTSN_MSGTYP_SEARCHGW      =   0x01,
    MQTTSN_MSGTYP_GWINFO        =   0x02,
    MQTTSN_MSGTYP_CONNECT       =   0x04,
    MQTTSN_MSGTYP_CONNACK       =   0x05,
    MQTTSN_MSGTYP_WILLTOPICREQ  =   0x06,
    MQTTSN_MSGTYP_WILLTOPIC     =   0x07,
    MQTTSN_MSGTYP_WILLMSGREQ    =   0x08,
    MQTTSN_MSGTYP_WILLMSG       =   0x09,
    MQTTSN_MSGTYP_REGISTER      =   0x0A,
    MQTTSN_MSGTYP_REGACK        =   0x0B,
    MQTTSN_MSGTYP_PUBLISH       =   0x0C,
    MQTTSN_MSGTYP_PUBACK        =   0x0D,
    MQTTSN_MSGTYP_PUBCOMP       =   0x0E,
    MQTTSN_MSGTYP_PUBREC        =   0x0F,
    MQTTSN_MSGTYP_PUBREL        =   0x10,
    MQTTSN_MSGTYP_SUBSCRIBE     =   0x12,
    MQTTSN_MSGTYP_SUBACK        =   0x13,
    MQTTSN_MSGTYP_UNSUBSCRIBE   =   0x14,
    MQTTSN_MSGTYP_UNSUBACK      =   0x15,
    MQTTSN_MSGTYP_PINGREQ       =   0x16,
    MQTTSN_MSGTYP_PINGRESP      =   0x17,
    MQTTSN_MSGTYP_DISCONNECT    =   0x18,
    MQTTSN_MSGTYP_WILLTOPICUPD  =   0x1A,
    MQTTSN_MSGTYP_WILLTOPICRESP =   0x1B,
    MQTTSN_MSGTYP_WILLMSGUPD    =   0x1C,
    MQTTSN_MSGTYP_WILLMSGRESP   =   0x1D,

    MQTTSN_MSGTYP_DHCPREQ       =   0x43,   // Not standard options, Request Address from Node to Gateway
    MQTTSN_MSGTYP_DHCPRESP      =   0x44,   // Not standard options, Assign address to Node from Gateway.

    MQTTSN_MSGTYP_FORWARD       =   0xFE
}e_MQTTSN_MSGTYPE_t;

#define MQTTSN_FL_DUP               0x80    // DUP: same meaning as with MQTT, i.e. set to “0” 
                                            //  if message is sent for the first time; 
                                            //  set to “1” if retransmitted 
                                            //  (only relevant within PUBLISH messages);
#define MQTTSN_FL_QOS_MASK          0x60
#define MQTTSN_FL_QOSN1             0x60    // QoS level -1 (only relevant within
                                            //  PUBLISH messages sent by a client);
#define MQTTSN_FL_QOS2              0x40    // QoS: meaning as with MQTT for QoS level 0, 1, and 2;
#define MQTTSN_FL_QOS1              0x20
#define MQTTSN_FL_QOS0              0x00    //
#define MQTTSN_FL_RETAIN            0x10    // Retain: same meaning as with MQTT
                                            //  (only relevant within PUBLISH messages);
#define MQTTSN_FL_WILL              0x08    // Will: if set, indicates that client is asking 
                                            //  for Will topic and Will message prompting 
                                            //  (only relevant within CONNECT message);
#define MQTTSN_FL_CLEANSESSION      0x04    // CleanSession: same meaning as with MQTT, however 
                                            //  extended for Will topic and Will message 
                                            //  (only relevant within CONNECT message);
#define MQTTSN_FL_TOPICID_NORM      0x00    // Indicates whether the field TopicId or TopicName 
                                            //  included in this message contains a normal topic id,
#define MQTTSN_FL_TOPICID_PREDEF    0x01    //  a pre-defined topic id
#define MQTTSN_FL_TOPICID_SHORT     0x02    //  or a short topic name
#define MQTTSN_FL_TOPICID_MASK      0x03

// Message Variable Part

//uint8_t     Length          // The Length field is either 1- or 3-octet long and specifies 
                              //    the total number of octets contained in the message 
                              //    (including the Length field itself)
//uint8_t     MsgType         // The MsgType field is 1-octet long and specifies the message type.
//uint8_t     ClientId[]      // As with MQTT, the ClientId field has a variable length 
                              //    and contains a 1-23 character long string that uniquely 
                              //    identifies the client to the broker.
//uint8_t     Data[]          // As with MQTT, the Data field has a variable length 
                              //    and contain the publish data.
//uint16_t    Duration        // The Duration field is 2-octet long and specifies the duration 
                              //    of a time period in seconds. The maximum value that can be 
                              //    encoded is approximately 18 hours.
//uint8_t     Flags           // The Flags field is 1-octet and contains the flags
//uint8_t     GwAdd[]         // The GwAdd field has a variable length and contains 
                              //    the address of a GW. 
//uint8_t     GwId            // The GwId field is 1-octet long and uniquely identifies a gateway.
//uint16_t    MsgId           // The MsgId field is 2-octet long and corresponds to the MQTT 
                              //    ‘Message ID’ parameter. It allows the sender to match a 
                              //    message with its corresponding acknowledgement.
//uint8_t     ProtocolId      // The ProtocolId is 1-octet long. It is only present in a CONNECT 
                              //    message and corresponds to the MQTT ‘protocol name’ 
                              //    and ‘protocol version’.
//                            // It is coded 0x01. All other values are reserved.
//uint8_t     Radius          // The Radius field is 1-octet long and indicates the value of the 
                              //    broadcast radius. The value 0x00 means “broadcast to all nodes 
                              //    in the network”.
//uint8_t     ReturnCode      // The value and meaning of the 1-octet long ReturnCode
//uint16_t    TopicId         // The TopicId field is 2-octet long and contains the value of the 
                              //    topic id. The values “0x0000” and “0xFFFF” are reserved and 
                              //    therefore should not be used.
//uint8_t     TopicName[]     // The TopicName field has a variable length and contains an 
                              //    UTF8-encoded string that specifies the topic name.
//uint8_t     WillMsg[]       // The WillMsg field has a variable length and contains the 
                              //    Will message.
//uint8_t     WillTopic[]     // The WillTopic field has a variable length and contains the 
                              //    Will topic name.

// Format of Individual Messages

typedef struct sMQTTSN_MSG_ADVERTISE
{
    uint8_t     GwId;
    uint8_t     Duration[2];
} sMQTTSN_MSG_ADVERTISE_t;
#define MQTTSN_SIZEOF_MSG_ADVERTISE     5

typedef struct sMQTTSN_MSG_SEARCHGW
{
    uint8_t     Radius;
} sMQTTSN_MSG_SEARCHGW_t;
#define MQTTSN_SIZEOF_MSG_SEARCHGW      3

typedef struct sMQTTSN_MSG_GWINFO
{
    uint8_t     GwId;
    uint8_t     GwAdd[MQTTSN_MSG_SIZE - 1];  // address of the indicated GW; optional,
                                            //  only included if message is sent by a client
} sMQTTSN_MSG_GWINFO_t;
#define MQTTSN_SIZEOF_MSG_GWINFO        3

#define MQTTSN_SIZEOF_CLIENTID          23
typedef struct sMQTTSN_MSG_CONNECT
{
    uint8_t     Flags;      // Will: if set, indicates that client is requesting for Will topic 
                            //  and Will message prompting;
                            // CleanSession: same meaning as with MQTT, 
                            //  however extended for Will topic and Will message
    uint8_t     ProtocolId;
    uint8_t     Duration[2];
    uint8_t     ClientId[MQTTSN_SIZEOF_CLIENTID];    // contains the client id which is a 
                                                    //  1-23 character long string
} sMQTTSN_MSG_CONNECT_t;
#define MQTTSN_SIZEOF_MSG_CONNECT       6

#if ((MQTTSN_SIZEOF_CLIENTID + MQTTSN_SIZEOF_MSG_CONNECT) > MQTTSN_MSG_SIZE)
    #error MQTTSN_MSG_SIZE < (MQTTSN_SIZEOF_CLIENTID + MQTTSN_SIZEOF_MSG_CONNECT)
#endif

typedef struct sMQTTSN_MSG_CONNACK
{
    uint8_t     ReturnCode;
} sMQTTSN_MSG_CONNACK_t;
#define MQTTSN_SIZEOF_MSG_CONNACK       3

#define MQTTSN_SIZEOF_MSG_WILLTOPICREQ  2

typedef struct sMQTTSN_MSG_WILLTOPIC
{
    uint8_t     Flags;      // QoS: same as MQTT, contains the Will QoS, 
                            // Retain: same as MQTT, contains the Will Retain flag
    uint8_t     WillTopic[MQTTSN_MSG_SIZE-1];
} sMQTTSN_MSG_WILLTOPIC_t;
#define MQTTSN_SIZEOF_MSG_WILLTOPIC     3

#define MQTTSN_SIZEOF_MSG_WILLMSGREQ    2

typedef struct sMQTTSN_MSG_WILLMSG
{
    uint8_t     WillMsg[MQTTSN_MSG_SIZE];
} sMQTTSN_MSG_WILLMSG_t;
#define MQTTSN_SIZEOF_MSG_WILLMSG       3

typedef struct sMQTTSN_MSG_REGISTER
{
    uint8_t     TopicId[2];
    uint8_t     MsgId[2];
    uint8_t     TopicName[MQTTSN_MSG_SIZE-4];
} sMQTTSN_MSG_REGISTER_t;
#define MQTTSN_SIZEOF_MSG_REGISTER      6

typedef struct sMQTTSN_MSG_REGACK
{
    uint8_t     TopicId[2];
    uint8_t     MsgId[2];
    uint8_t     ReturnCode;
} sMQTTSN_MSG_REGACK_t;
#define MQTTSN_SIZEOF_MSG_REGACK        7

typedef struct sMQTTSN_MSG_PUBLISH
{
    uint8_t     Flags;  // DUP: same as MQTT, indicates whether message is sent for the 
                        //  first time or not.
                        // QoS: same as MQTT, contains the QoS level for this PUBLISH message.
                        // Retain: same as MQTT, contains the Retain flag.
                        // TopicIdType: indicates the type of the topic id contained in the 
                        //  TopicId field.
    uint8_t     TopicId[2];
    uint8_t     MsgId[2];
    uint8_t     Data[MQTTSN_MSG_SIZE-5];
} sMQTTSN_MSG_PUBLISH_t;
#define MQTTSN_SIZEOF_MSG_PUBLISH       7

typedef struct sMQTTSN_MSG_PUBACK
{
    uint8_t     TopicId[2];
    uint8_t     MsgId[2];
    uint8_t     ReturnCode;
} sMQTTSN_MSG_PUBACK_t;
#define MQTTSN_SIZEOF_MSG_PUBACK        7

typedef struct sMQTTSN_MSG_PUBCOMP
{
    uint8_t     MsgId[2];
} sMQTTSN_MSG_PUBCOMP_t;
#define MQTTSN_SIZEOF_MSG_PUBCOMP       4

typedef struct sMQTTSN_MSG_PUBREC
{
    uint8_t     MsgId[2];
} sMQTTSN_MSG_PUBREC_t;
#define MQTTSN_SIZEOF_MSG_PUBREC        4

typedef struct sMQTTSN_MSG_PUBREL
{
    uint8_t     MsgId[2];
} sMQTTSN_MSG_PUBREL_t;
#define MQTTSN_SIZEOF_MSG_PUBREL        4

typedef struct sMQTTSN_MSG_SUBSCRIBE
{
    uint8_t     Flags;      // DUP: same as MQTT, indicates whether message is sent for first time 
                            //  or not.
                            // QoS: same as MQTT, contains the requested QoS level for this topic.
                            // TopicIdType: indicates the type of information included at the end 
                            //  of the message, namely “0b00”
                            // topic name, “0b01” pre-defined topic id, “0b10” short topic name, 
                            //  and “0b11” reserved.
    uint8_t     MsgId[2];
    uint8_t     Topic[MQTTSN_MSG_SIZE - 3];  // TopicName or TopicId
} sMQTTSN_MSG_SUBSCRIBE_t;
#define MQTTSN_SIZEOF_MSG_SUBSCRIBE     5

typedef struct sMQTTSN_MSG_SUBACK
{
    uint8_t     Flags;      // QoS: same as MQTT, contains the granted QoS level.
    uint8_t     TopicId[2];
    uint8_t     MsgId[2];
    uint8_t     ReturnCode;
} sMQTTSN_MSG_SUBACK_t;
#define MQTTSN_SIZEOF_MSG_SUBACK        8

typedef struct sMQTTSN_MSG_UNSUBSCRIBE
{
    uint8_t     Flags;      // TopicIdType: indicates the type of information included at the end 
                            //  of the message, namely “0b00” topic name, 
                            //  “0b01” pre-defined topic id, “0b10” short topic name, 
                            //  and “0b11” reserved.
    uint8_t     MsgId[2];
    uint8_t     Topic[MQTTSN_MSG_SIZE - 3];
} sMQTTSN_MSG_UNSUBSCRIBE_t;
#define MQTTSN_SIZEOF_MSG_UNSUBSCRIBE   5

typedef struct sMQTTSN_MSG_UNSUBACK
{
    uint8_t     MsgId[2];
} sMQTTSN_MSG_UNSUBACK_t;
#define MQTTSN_SIZEOF_MSG_UNSUBACK      4

typedef struct sMQTTSN_MSG_PINGREQ
{
    uint8_t     ClientId[MQTTSN_MSG_SIZE];   // Optional
} sMQTTSN_MSG_PINGREQ_t;
#define MQTTSN_SIZEOF_MSG_PINGREQ       2

#define MQTTSN_SIZEOF_MSG_PINGRESP      2

typedef struct sMQTTSN_MSG_DISCONNECT
{
    uint8_t     Duration[2];                   // Optional, for ASleep node
} sMQTTSN_MSG_DISCONNECT_t;
#define MQTTSN_SIZEOF_MSG_DISCONNECT    2
#define MQTTSN_SIZEOF_MSG_DISCONNECTL   4

typedef struct sMQTTSN_MSG_WILLTOPICUPD
{
    uint8_t     Flags;                      // QoS: same as MQTT, contains the Will QoS; 
                                            // Retain: same as MQTT, contains the Will Retain flag
    uint8_t     WillTopic[MQTTSN_MSG_SIZE - 1];
} sMQTTSN_MSG_WILLTOPICUPD_t;
#define MQTTSN_SIZEOF_MSG_WILLTOPICUPD  3

typedef struct sMQTTSN_MSG_WILLTOPICRESP
{
    uint8_t     ReturnCode;
} sMQTTSN_MSG_WILLTOPICRESP_t;
#define MQTTSN_SIZEOF_MSG_WILLTOPICRESP 3

typedef struct sMQTTSN_MSG_WILLMSGUPD
{
    uint8_t     WillMsg[MQTTSN_MSG_SIZE];
} sMQTTSN_MSG_WILLMSGUPD_t;
#define MQTTSN_SIZEOF_MSG_WILLMSGUPD    2

typedef struct sMQTTSN_MSG_WILLMSGRESP
{
    uint8_t     ReturnCode;
} sMQTTSN_MSG_WILLMSGRESP_t;
#define MQTTSN_SIZEOF_MSG_WILLMSGRESP   3


// Not standard message
// Request Address from Node to Gateway
typedef struct sMQTTSN_MSG_DHCPREQ
{
    uint8_t     Radius;
    uint8_t     MsgId[2];                   // unique transaction id
    uint8_t     hlen[];                     // phy's address length(bytes) 
} sMQTTSN_MSG_DHCPREQ_t;
#define MQTTSN_SIZEOF_MSG_DHCPREQ       5

// Not standard message
// Assign address to Node from Gateway
typedef struct sMQTTSN_MSG_DHCPRESP
{
    uint8_t     GwId;
    uint8_t     MsgId[2];                   // unique transaction id
    uint8_t     addr[];
} sMQTTSN_MSG_DHCPRESP_t;
#define MQTTSN_SIZEOF_MSG_DHCPRESP      5

typedef struct sMQTTSN_MSG_FORWARD
{
    uint8_t     Ctrl;                             // Bits 0,1 - Radius
    uint8_t     wNodeID[MQTTSN_MSG_SIZE - 1];
} sMQTTSN_MSG_FORWARD_t;
#define MQTTSN_SIZEOF_MSG_FORWARD       3

typedef struct sMQTTSN_MESSAGE
{
  uint8_t Length;
  e_MQTTSN_MSGTYPE_t MsgType;
  union
  {
    uint8_t                     raw[MQTTSN_MSG_SIZE];
    sMQTTSN_MSG_ADVERTISE_t     advertise;
    sMQTTSN_MSG_SEARCHGW_t      searchgw;
    sMQTTSN_MSG_GWINFO_t        gwinfo;
    sMQTTSN_MSG_CONNECT_t       connect;
    sMQTTSN_MSG_CONNACK_t       connack;
    sMQTTSN_MSG_WILLTOPIC_t     willtopic;
    sMQTTSN_MSG_WILLMSG_t       willmsg;
    sMQTTSN_MSG_REGISTER_t      regist;
    sMQTTSN_MSG_REGACK_t        regack;
    sMQTTSN_MSG_PUBLISH_t       publish;
    sMQTTSN_MSG_PUBACK_t        puback;
    sMQTTSN_MSG_PUBCOMP_t       pubcomp;
    sMQTTSN_MSG_PUBREC_t        pubrec;
    sMQTTSN_MSG_PUBREL_t        pubrel;
    sMQTTSN_MSG_SUBSCRIBE_t     subscribe;
    sMQTTSN_MSG_SUBACK_t        suback;
    sMQTTSN_MSG_UNSUBSCRIBE_t   unsubscribe;
    sMQTTSN_MSG_UNSUBACK_t      unsuback;
    sMQTTSN_MSG_PINGREQ_t       pingreq;
    sMQTTSN_MSG_DISCONNECT_t    disconnect;
    sMQTTSN_MSG_WILLTOPICUPD_t  willtopicupd;
    sMQTTSN_MSG_WILLTOPICRESP_t willtopicresp;
    sMQTTSN_MSG_WILLMSGUPD_t    willmsgupd;
    sMQTTSN_MSG_WILLMSGRESP_t   willmsgresp;
    sMQTTSN_MSG_DHCPREQ_t       dhcpreq;
    sMQTTSN_MSG_DHCPRESP_t      dhcpresp;
    sMQTTSN_MSG_FORWARD_t       forward;
  }m;
} MQTTSN_MESSAGE_t;

typedef struct sMQ
{
    struct sMQ * pNext;

    union
    {
        uint8_t             phy1addr[sizeof(PHY1_ADDR_t)];
#ifdef PHY2_ADDR_t
        uint8_t             phy2addr[sizeof(PHY2_ADDR_t)];
#endif
    }a;
  
    uint8_t                 Length;
    union
    {
        uint8_t             raw[sizeof(MQTTSN_MESSAGE_t)];
        MQTTSN_MESSAGE_t    mq;
    }m;
} MQ_t;

#ifdef __cplusplus
}
#endif

#endif  //    _MQ_TYPES_H
