/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

#ifndef _MQTTS_H
#define _MQTTS_H

#define MQTTS_DEF_KEEPALIVE     300     // Keep Alive Time - 5 Min
#define MQTTS_DEF_NRETRY        3       // Number of retry's
#define MQTTS_DEF_TCONNECT      10      // Time between Connect's
#define MQTTS_DEF_TSGW          15      // Time between search gateway request

#define MQTTS_DEF_PROTOCOLID    0x01

#define MQTTS_UDP_PORT          1883

//#define MQTTS_DEF_MSGID_DELTA   10      // delta between last & actual messages ID

#ifndef SWAPWORD
    #define SWAPWORD(x)     ((uint16_t)(((((uint16_t)(x))&0xFF)<<8)+((((uint16_t)(x))&0xFF00)>>8)))
#endif

//uint8_t     Length;         // The Length field is either 1- or 3-octet long and specifies 
                              //    the total number of octets contained in the message 
                              //    (including the Length field itself)
//uint8_t     MsgType         // The MsgType field is 1-octet long and specifies the message type.
//uint8_t     ClientId[];     // As with MQTT, the ClientId field has a variable length 
                              //    and contains a 1-23 character long string that uniquely 
                              //    identifies the client to the broker.
//uint8_t     Data[];         // As with MQTT, the Data field has a variable length 
                              //    and contain the publish data.
//uint16_t    Duration;       // The Duration field is 2-octet long and specifies the duration 
                              //    of a time period in seconds. The maximum value that can be 
                              //    encoded is approximately 18 hours.
//uint8_t     Flags;          // The Flags field is 1-octet and contains the following flags
//uint8_t     GwAdd[];        // The GwAdd field has a variable length and contains 
                              //    the address of a GW. 
//uint8_t     GwId;           // The GwId field is 1-octet long and uniquely identifies a gateway.
//uint16_t    MsgId;          // The MsgId field is 2-octet long and corresponds to the MQTT 
                              //    ‘Message ID’ parameter. It allows the sender to match a 
                              //    message with its corresponding acknowledgement.
//uint8_t     ProtocolId;     // The ProtocolId is 1-octet long. It is only present in a CONNECT 
                              //    message and corresponds to the MQTT protocol name 
                              //    and ‘protocol version’.
//                            // It is coded 0x01. All other values are reserved.
//uint8_t     Radius:         // The Radius field is 1-octet long and indicates the value of the 
                              //    broadcast radius. The value 0x00 means “broadcast to all nodes 
                              //    in the network”.
//uint8_t     ReturnCode;     // The value and meaning of the 1-octet long ReturnCode
//uint16_t    TopicId;        // The TopicId field is 2-octet long and contains the value of the 
                              //    topic id. The values “0x0000” and “0xFFFF” are reserved and 
                              //    therefore should not be used.
//uint8_t     TopicName[];    // The TopicName field has a variable length and contains an 
                              //    UTF8-encoded string that specifies the topic name.
//uint8_t     WillMsg[];      // The WillMsg field has a variable length and contains the 
                              //    Will message.
//uint8_t     WillTopic[];    // The WillTopic field has a variable length and contains the 
                              //    Will topic name.

enum e_MQTTS_MSGTYPE
{
    MQTTS_MSGTYP_ADVERTISE      =   0x00,
    MQTTS_MSGTYP_SEARCHGW       =   0x01,
    MQTTS_MSGTYP_GWINFO         =   0x02,
    MQTTS_MSGTYP_CONNECT        =   0x04,
    MQTTS_MSGTYP_CONNACK        =   0x05,
    MQTTS_MSGTYP_WILLTOPICREQ   =   0x06,
    MQTTS_MSGTYP_WILLTOPIC      =   0x07,
    MQTTS_MSGTYP_WILLMSGREQ     =   0x08,
    MQTTS_MSGTYP_WILLMSG        =   0x09,
    MQTTS_MSGTYP_REGISTER       =   0x0A,
    MQTTS_MSGTYP_REGACK         =   0x0B,
    MQTTS_MSGTYP_PUBLISH        =   0x0C,
    MQTTS_MSGTYP_PUBACK         =   0x0D,
    MQTTS_MSGTYP_PUBCOMP        =   0x0E,
    MQTTS_MSGTYP_PUBREC         =   0x0F,
    MQTTS_MSGTYP_PUBREL         =   0x10,
    MQTTS_MSGTYP_SUBSCRIBE      =   0x12,
    MQTTS_MSGTYP_SUBACK         =   0x13,
    MQTTS_MSGTYP_UNSUBSCRIBE    =   0x14,
    MQTTS_MSGTYP_UNSUBACK       =   0x15,
    MQTTS_MSGTYP_PINGREQ        =   0x16,
    MQTTS_MSGTYP_PINGRESP       =   0x17,
    MQTTS_MSGTYP_DISCONNECT     =   0x18,
    MQTTS_MSGTYP_WILLTOPICUPD   =   0x1A,
    MQTTS_MSGTYP_WILLTOPICRESP  =   0x1B,
    MQTTS_MSGTYP_WILLMSGUPD     =   0x1C,
    MQTTS_MSGTYP_WILLMSGRESP    =   0x1D,
    MQTTS_MSGTYP_FORWARD        =   0xFE
};

#define MQTTS_FL_DUP                0x80    // DUP: same meaning as with MQTT, i.e. set to “0” 
                                            //  if message is sent for the first time; 
                                            //  set to “1” if retransmitted 
                                            //  (only relevant within PUBLISH messages);
#define MQTTS_FL_QOS_MASK           0x60
#define MQTTS_FL_QOS3               0x60    // QoS level -1 (only relevant within
                                            //  PUBLISH messages sent by a client);
#define MQTTS_FL_QOS2               0x40    // QoS: meaning as with MQTT for QoS level 0, 1, and 2;
#define MQTTS_FL_QOS1               0x20
#define MQTTS_FL_QOS0               0x00    //
#define MQTTS_FL_RETAIN             0x10    // Retain: same meaning as with MQTT
                                            //  (only relevant within PUBLISH messages);
#define MQTTS_FL_WILL               0x08    // Will: if set, indicates that client is asking 
                                            //  for Will topic and Will message prompting 
                                            //  (only relevant within CONNECT message);
#define MQTTS_FL_CLEANSESSION       0x04    // CleanSession: same meaning as with MQTT, however 
                                            //  extended for Will topic and Will message 
                                            //  (only relevant within CONNECT message);
#define MQTTS_FL_TOPICID_NORM       0x00    // Indicates whether the field TopicId or TopicName 
                                            //  included in this message contains a normal topic id,
#define MQTTS_FL_TOPICID_PREDEF     0x01    //  a pre-defined topic id
#define MQTTS_FL_TOPICID_SHORT      0x02    //  or a short topic name
#define MQTTS_FL_TOPICID_MASK       0x03

enum e_MQTTS_RETURNS
{
    MQTTS_RET_ACCEPTED      =   0x00,       // Accepted
    MQTTS_RET_REJ_CONG      =   0x01,       // Rejected: congestion
    MQTTS_RET_REJ_INV_ID    =   0x02,       // Rejected: invalid topic ID
    MQTTS_RET_REJ_NOT_SUPP  =   0x03,       // Rejected: not supported
};

// Messages
typedef struct
{
    uint8_t     GwId;
    uint16_t    Duration;
} sMQTTS_MSG_ADVERTISE_t;
#define MQTTS_SIZEOF_MSG_ADVERTISE      5

typedef struct
{
    uint8_t     Radius;
} sMQTTS_MSG_SEARCHGW_t;
#define MQTTS_SIZEOF_MSG_SEARCHGW       3

typedef struct
{
    uint8_t     GwId;
    uint8_t     GwAdd[MQTTS_MSG_SIZE - 1];  // address of the indicated GW; optional,
                                            //  only included if message is sent by a client
} sMQTTS_MSG_GWINFO_t;
#define MQTTS_SIZEOF_MSG_GWINFO         3

#define MQTTS_SIZEOF_CLIENTID           23
typedef struct
{
    uint8_t     Flags;      // Will: if set, indicates that client is requesting for Will topic 
                            //  and Will message prompting;
                            // CleanSession: same meaning as with MQTT, 
                            //  however extended for Will topic and Will message
    uint8_t     ProtocolId;
    uint16_t    Duration;
    uint8_t     ClientId[MQTTS_SIZEOF_CLIENTID];    // contains the client id which is a 
                                                    //  1-23 character long string
} sMQTTS_MSG_CONNECT_t;
#define MQTTS_SIZEOF_MSG_CONNECT        6

#if ((MQTTS_SIZEOF_CLIENTID + MQTTS_SIZEOF_MSG_CONNECT) > MQTTS_MSG_SIZE)
    #error MQTTS_MSG_SIZE < (MQTTS_SIZEOF_CLIENTID + MQTTS_SIZEOF_MSG_CONNECT)
#endif

typedef struct
{
    uint8_t     ReturnCode;
} sMQTTS_MSG_CONNACK_t;
#define MQTTS_SIZEOF_MSG_CONNACK        3

#define MQTTS_SIZEOF_MSG_WILLTOPICREQ   2

typedef struct
{
    uint8_t     Flags;      // QoS: same as MQTT, contains the Will QoS, 
                            // Retain: same as MQTT, contains the Will Retain flag
    uint8_t     WillTopic[MQTTS_MSG_SIZE-1];
} sMQTTS_MSG_WILLTOPIC_t;
#define MQTTS_SIZEOF_MSG_WILLTOPIC      3

#define MQTTS_SIZEOF_MSG_WILLMSGREQ     2

typedef struct
{
    uint8_t     WillMsg[MQTTS_MSG_SIZE];
} sMQTTS_MSG_WILLMSG_t;
#define MQTTS_SIZEOF_MSG_WILLMSG        3

typedef struct
{
    uint16_t    TopicId;
    uint16_t    MsgId;
    uint8_t     TopicName[MQTTS_MSG_SIZE-2];
} sMQTTS_MSG_REGISTER_t;
#define MQTTS_SIZEOF_MSG_REGISTER       6

typedef struct
{
    uint16_t    TopicId;
    uint16_t    MsgId;
    uint8_t     ReturnCode;
} sMQTTS_MSG_REGACK_t;
#define MQTTS_SIZEOF_MSG_REGACK         7

typedef struct
{
    uint8_t     Flags;  // DUP: same as MQTT, indicates whether message is sent for the 
                        //  first time or not.
                        // QoS: same as MQTT, contains the QoS level for this PUBLISH message.
                        // Retain: same as MQTT, contains the Retain flag.
                        // TopicIdType: indicates the type of the topic id contained in the 
                        //  TopicId field.
    uint16_t    TopicId;
    uint16_t    MsgId;
    uint8_t     Data[MQTTS_MSG_SIZE-5];
} sMQTTS_MSG_PUBLISH_t;
#define MQTTS_SIZEOF_MSG_PUBLISH        7

typedef struct
{
    uint16_t    TopicId;
    uint16_t    MsgId;
    uint8_t     ReturnCode;
} sMQTTS_MSG_PUBACK_t;
#define MQTTS_SIZEOF_MSG_PUBACK        7

typedef struct
{
    uint16_t    MsgId;
} sMQTTS_MSG_PUBCOMP_t;
#define MQTTS_SIZEOF_MSG_PUBCOMP        4

typedef struct
{
    uint16_t    MsgId;
} sMQTTS_MSG_PUBREC_t;
#define MQTTS_SIZEOF_MSG_PUBREC         4

typedef struct
{
    uint16_t    MsgId;
} sMQTTS_MSG_PUBREL_t;
#define MQTTS_SIZEOF_MSG_PUBREL         4

typedef struct
{
    uint8_t     Flags;      // DUP: same as MQTT, indicates whether message is sent for first time 
                            //  or not.
                            // QoS: same as MQTT, contains the requested QoS level for this topic.
                            // TopicIdType: indicates the type of information included at the end 
                            //  of the message, namely “0b00”
                            // topic name, “0b01” pre-defined topic id, “0b10” short topic name, 
                            //  and “0b11” reserved.
    uint16_t    MsgId;
    uint8_t     Topic[MQTTS_MSG_SIZE - 3];  // TopicName or TopicId
} sMQTTS_MSG_SUBSCRIBE_t;
#define MQTTS_SIZEOF_MSG_SUBSCRIBE      5

typedef struct
{
    uint8_t     Flags;      // QoS: same as MQTT, contains the granted QoS level.
    uint16_t    TopicId;
    uint16_t    MsgId;
    uint8_t     ReturnCode;
} sMQTTS_MSG_SUBACK_t;
#define MQTTS_SIZEOF_MSG_SUBACK         8

typedef struct
{
    uint8_t     Flags;      // TopicIdType: indicates the type of information included at the end 
                            //  of the message, namely “0b00” topic name, 
                            //  “0b01” pre-defined topic id, “0b10” short topic name, 
                            //  and “0b11” reserved.
    uint16_t    MsgId;
    uint8_t     Topic[MQTTS_MSG_SIZE - 3];
} sMQTTS_MSG_UNSUBSCRIBE_t;
#define MQTTS_SIZEOF_MSG_UNSUBSCRIBE    5

typedef struct
{
    uint16_t    MsgId;
} sMQTTS_MSG_UNSUBACK_t;
#define MQTTS_SIZEOF_MSG_UNSUBACK       4

typedef struct
{
    uint8_t     ClientId[MQTTS_MSG_SIZE];   // Optional
} sMQTTS_MSG_PINGREQ_t;
#define MQTTS_SIZEOF_MSG_PINGREQ        2

#define MQTTS_SIZEOF_MSG_PINGRESP       2

typedef struct MQTTS_MSG_DISCONNECT
{
    uint16_t    Duration;                   // Optional, for ASleep node
} sMQTTS_MSG_DISCONNECT_t;
#define MQTTS_SIZEOF_MSG_DISCONNECT     2
#define MQTTS_SIZEOF_MSG_DISCONNECTL    4

typedef struct
{
    uint8_t     Flags;                      // QoS: same as MQTT, contains the Will QoS; 
                                            // Retain: same as MQTT, contains the Will Retain flag
    uint8_t     WillTopic[MQTTS_MSG_SIZE - 1];
} sMQTTS_MSG_WILLTOPICUPD_t;
#define MQTTS_SIZEOF_MSG_WILLTOPICUPD   3

typedef struct
{
    uint8_t     ReturnCode;
} sMQTTS_MSG_WILLTOPICRESP_t;
#define MQTTS_SIZEOF_MSG_WILLTOPICRESP  3

typedef struct
{
    uint8_t     WillMsg[MQTTS_MSG_SIZE];
} sMQTTS_MSG_WILLMSGUPD_t;
#define MQTTS_SIZEOF_MSG_WILLMSGUPD     2

typedef struct
{
    uint8_t     ReturnCode;
} sMQTTS_MSG_WILLMSGRESP_t;
#define MQTTS_SIZEOF_MSG_WILLMSGRESP    3

typedef struct
{
    uint8_t     Ctrl;
    uint8_t     wNodeID[MQTTS_MSG_SIZE - 1];
} sMQTTS_MSG_FORWARD_t;
#define MQTTS_SIZEOF_MSG_FORWARD        3

typedef struct 
{
    uint8_t Length;
    uint8_t MsgType;
    union
    {
        uint8_t                     raw[MQTTS_MSG_SIZE];
        sMQTTS_MSG_ADVERTISE_t      advertise;
        sMQTTS_MSG_SEARCHGW_t       searchgw;
        sMQTTS_MSG_GWINFO_t         gwinfo;
        sMQTTS_MSG_CONNECT_t        connect;
        sMQTTS_MSG_CONNACK_t        connack;
        sMQTTS_MSG_WILLTOPIC_t      willtopic;
        sMQTTS_MSG_WILLMSG_t        willmsg;
        sMQTTS_MSG_REGISTER_t       regist;
        sMQTTS_MSG_REGACK_t         regack;
        sMQTTS_MSG_PUBLISH_t        publish;
        sMQTTS_MSG_PUBACK_t         puback;
        sMQTTS_MSG_PUBCOMP_t        pubcomp;
        sMQTTS_MSG_PUBREC_t         pubrec;
        sMQTTS_MSG_PUBREL_t         pubrel;
        sMQTTS_MSG_SUBSCRIBE_t      subscribe;
        sMQTTS_MSG_SUBACK_t         suback;
        sMQTTS_MSG_UNSUBSCRIBE_t    unsubscribe;
        sMQTTS_MSG_UNSUBACK_t       unsuback;
        sMQTTS_MSG_PINGREQ_t        pingreq;
        sMQTTS_MSG_DISCONNECT_t     disconnect;
        sMQTTS_MSG_WILLTOPICUPD_t   willtopicupd;
        sMQTTS_MSG_WILLTOPICRESP_t  willtopicresp;
        sMQTTS_MSG_WILLMSGUPD_t     willmsgupd;
        sMQTTS_MSG_WILLMSGRESP_t    willmsgresp;
        sMQTTS_MSG_FORWARD_t        forward;
    } m;
} MQTTS_MESSAGE_t;

typedef struct
{
    s_Addr          addr;
    MQTTS_MESSAGE_t mq;
}MQ_t;

// Local Variables
// Current Status
typedef enum e_MQTTS_STATUS
{
#ifndef GATEWAY
    MQTTS_STATUS_SEARCHGW = 0,
#endif  //  GATEWAY
    MQTTS_STATUS_OFFLINE,
    MQTTS_STATUS_CONNECT,
#ifdef ASLEEP
    MQTTS_STATUS_POST_CONNECT,
    MQTTS_STATUS_PRE_ASLEEP,
    MQTTS_STATUS_ASLEEP,
    MQTTS_STATUS_AWAKE,
    MQTTS_STATUS_POST_AWAKE
#endif  //  ASLEEP
}e_MQTTS_STATUS_t;

enum e_MQTTS_POOL_STATUS
{
    MQTTS_POOL_STAT_NOP = 0,
    MQTTS_POOL_STAT_DATA,
#ifdef ASLEEP
    MQTTS_POOL_STAT_ASLEEP,
    MQTTS_POOL_STAT_AWAKE,
#endif  //  ASLEEP
};

typedef struct
{
    s_Addr                  GatewayID;      // Gateway ID
    e_MQTTS_STATUS_t        Status;         // Actual status
    // Timeouts
    uint8_t                 pfCnt;
    uint16_t                Tretry;         // Keep Alive Timeout
    uint8_t                 Nretry;         // Rertry's number
#ifdef ASLEEP
    uint16_t                Tasleep;        // ASleep Time, if = 0, no sleep mode
#endif
    // Send FIFO
    uint8_t                 tail;
    uint8_t                 head;
    MQ_t                  * buf[MQTTS_SIZEOF_SEND_FIFO];
    // Pool FIFO
    uint8_t                 fTail;
    uint8_t                 fHead;
    MQ_t                  * fBuf[MQTTS_SIZEOF_POOL_FIFO];
    uint16_t                MsgID;
    uint16_t                inMsgId;
    
    uint8_t                 ReturnCode;
}MQTTS_VAR_t;

uint8_t MQTTS_DataRdy(void);
MQ_t * MQTTS_Get(void);
#ifdef ASLEEP
void mqtts_set_TASleep(uint16_t tasleep);
#endif  //  ASLEEP
uint8_t MQTTS_Publish(uint16_t TopicID, uint8_t Flags, uint8_t Size, uint8_t * ipBuf);
uint8_t MQTTS_Subscribe(uint8_t Flags, uint8_t Size, uint8_t * ipBuf);
uint8_t MQTTS_Register(uint16_t TopicID, uint8_t Size, uint8_t * ipBuf);

void MQTTS_Init(void);
uint8_t MQTTS_Pool(uint8_t wakeup);
uint8_t MQTTS_Parser(MQ_t * pBuf);

uint8_t MQTTS_GetStatus(void);
#endif
