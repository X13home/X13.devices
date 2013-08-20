/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

// Object's Dictionary

#include "config.h"
#include "eeprom.h"
#include "ext.h"

// EEPROM Object's
#ifdef RF_NODE
const uint8_t  EEMEM ee_NodeID = 0xFF;                                  // Device Addr
const uint16_t EEMEM ee_GroupID = 0xFFFF;                               // Group ID
const uint8_t  EEMEM ee_Channel = 0xFF;                                 // Channel
#endif  //  RF_NODE

#ifdef LAN_NODE
const uint8_t  EEMEM ee_MAC_Addr[]  = OD_DEV_MAC;
const uint8_t  EEMEM ee_IP_Addr[]   = {0xFF,0xFF,0xFF,0xFF};
const uint8_t  EEMEM ee_IP_Mask[]   = {0xFF,0xFF,0xFF,0xFF};
const uint8_t  EEMEM ee_IP_Router[] = {0xFF,0xFF,0xFF,0xFF};
const uint8_t  EEMEM ee_IP_Broker[] = {0xFF,0xFF,0xFF,0xFF};
#endif  //  LAN_NODE

const uint8_t  EEMEM ee_NodeName[MQTTS_SIZEOF_CLIENTID-1] = {0};
#ifdef ASLEEP
const uint16_t EEMEM ee_TAsleep = 0;                                    // Sleep Time
#endif  //  ASLEEP
const uint8_t  EEMEM ee_listOd_bu[OD_MAX_INDEX_LIST*sizeof(subidx_t)] = {0xFF,};  // Backup listOD

#ifdef ASLEEP
// local callback function's
uint8_t cbWriteTASleep(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf)
{
    uint16_t val = *(uint16_t *)pBuf;
    mqtts_set_TASleep(val);
    return eepromWriteOD(pSubidx, Len, pBuf);
}
#endif  //  ASLEEP

#ifdef LAN_NODE
uint8_t cbWriteLANParm(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf)
{
  if(pSubidx->Base == (uint16_t)&ee_MAC_Addr)
    while(Len < 6)
      pBuf[Len++] = 0;

  uint8_t tmp, i, epnt, valid = 0;
  epnt = Len - 1;
  for(i = 0; i < epnt; i++)
  {
    tmp = pBuf[i];
    pBuf[i] = pBuf[epnt];
    pBuf[epnt] = tmp;
    epnt--;
    
    if(tmp != 0)
      valid = 1;
  }

  if(valid)
    return eepromWriteOD(pSubidx, Len, pBuf);
  else
    return MQTTS_RET_REJ_CONG;
}
#endif  //  LAN_NODE

#ifdef GATEWAY
#define OD_DEV_NETTYP_H    'G'
#define OD_DEV_NETTYP_L    'W'
#else   //  Node
#define OD_DEV_NETTYP_H    'N'
#define OD_DEV_NETTYP_L    'D'
#endif  //  GATEWAY

// Predefined Object's 
const PROGMEM uint8_t psDeviceTyp[] = {10,
                                        OD_DEV_NETTYP_H,        // Device Net Type
                                        OD_DEV_NETTYP_L,
                                        OD_DEV_TYP_0,           // Device HW/SW Type
                                        OD_DEV_TYP_1,
                                        OD_DEV_TYP_2,
                                        OD_DEV_TYP_3,
                                        '.',                    // Delimiter
                                        OD_DEV_SWVERSH,         // Software Version
                                        OD_DEV_SWVERSM,
                                        OD_DEV_SWVERSL};

const PROGMEM indextable_t listPredefOD[] = 
{
  {{objEEMEM, objString, (uint16_t)&ee_NodeName},
    objNodeName, (cbRead_t)&eepromReadOD, (cbWrite_t)&eepromWriteOD, NULL},
#ifdef ASLEEP
  {{objEEMEM, objUInt16, (uint16_t)&ee_TAsleep},
    objTAsleep, (cbRead_t)&eepromReadOD,  (cbWrite_t)&cbWriteTASleep, NULL},
#endif  //  ASLEEP
#ifdef RF_NODE
  {{objEEMEM, objUInt8, (uint16_t)&ee_GroupID},
    objRFNodeId, (cbRead_t)&eepromReadOD, (cbWrite_t)&eepromWriteOD, NULL},
  {{objEEMEM, objUInt16, (uint16_t)&ee_NodeID},
    objRFGroup, (cbRead_t)&eepromReadOD, (cbWrite_t)&eepromWriteOD, NULL},
  {{objEEMEM, objUInt8, (uint16_t)&ee_Channel},
    objRFChannel, (cbRead_t)&eepromReadOD, (cbWrite_t)&eepromWriteOD, NULL},
#endif  //  RF_NODE
#ifdef LAN_NODE
  {{objEEMEM, objArray, (uint16_t)&ee_MAC_Addr},
    objMACAddr, (cbRead_t)&eepromReadOD, (cbWrite_t)&cbWriteLANParm, NULL},
  {{objEEMEM, objUInt32, (uint16_t)&ee_IP_Addr},
    objIPAddr, (cbRead_t)&eepromReadOD, (cbWrite_t)&cbWriteLANParm, NULL},
  {{objEEMEM, objUInt32, (uint16_t)&ee_IP_Mask},
    objIPMask, (cbRead_t)&eepromReadOD, (cbWrite_t)&cbWriteLANParm, NULL},
  {{objEEMEM, objUInt32, (uint16_t)&ee_IP_Router},
    objIPRouter, (cbRead_t)&eepromReadOD, (cbWrite_t)&cbWriteLANParm, NULL},
  {{objEEMEM, objUInt32, (uint16_t)&ee_IP_Broker},
    objIPBroker, (cbRead_t)&eepromReadOD, (cbWrite_t)&cbWriteLANParm, NULL},
#endif  //  LAN_NODE
  {{objPROGMEM, objString, (uint16_t)&psDeviceTyp},
    objDeviceTyp, (cbRead_t)&progmemReadOD, NULL, NULL}
};

// Local Variables
static indextable_t ListOD[OD_MAX_INDEX_LIST];                          // Object's List
static indextable_t tObjectOD;                                          // Temporary Object
static uint8_t idxUpdate, idxSubscr;                                    // Pool variable

// Search Object by Index
static indextable_t * scanIndexOD(uint16_t index, uint8_t topic)
{
    uint16_t i = 0;
    if(topic == MQTTS_FL_TOPICID_NORM)
    {
        for(i = 0; (i < OD_MAX_INDEX_LIST) && (ListOD[i].Index != 0xFFFF) ; i++)
            if(ListOD[i].Index == index)
                return &ListOD[i]; 
    }
    else if(topic == MQTTS_FL_TOPICID_PREDEF)
    {
        for(i = 0; i < sizeof(listPredefOD)/sizeof(indextable_t); i++)
        {
            memcpy_P(&tObjectOD, &listPredefOD[i], sizeof(indextable_t));
            if(tObjectOD.Index == index)
                return &tObjectOD;
        }
    }
    return NULL;
}

// delete object
static void deleteIndexOD(uint8_t id)
{
    if(id >= OD_MAX_INDEX_LIST)
        return;
        
    // delete from EEPROM
    uint8_t i;
    uint16_t addr;
    subidx_t subidx;
    
    for(i = 0; i < OD_MAX_INDEX_LIST; i++)
    {
        addr = (uint16_t)&ee_listOd_bu +  i * sizeof(subidx_t);
        eepromReadListOD(addr, &subidx);
        
        if(memcmp((const void *)&subidx, (const void *)&ListOD[id].sidx, sizeof(subidx_t)) == 0)
        {
            subidx.Place = 0xFF;
            subidx.Type = 0xFF;
            subidx.Base  = 0xFFFF;
            eepromWriteListOD(addr, &subidx);
            break;
        }
    }

    extDeleteOD(&ListOD[id].sidx);

    // Pack Index Table
    while((id < (OD_MAX_INDEX_LIST - 1)) && (ListOD[id+1].Index != 0xFFFF))
    {
        ListOD[id] = ListOD[id+1];
        id++;
    }
    ListOD[id].Index = 0xFFFF;
}


void InitOD(void)
{
  // Check Settings
  uint8_t ucTmp, Len;
  Len = 1;

  ReadOD(objNodeName, MQTTS_FL_TOPICID_PREDEF, &Len, &ucTmp);
  if(ucTmp == 0xFF)                                                           // Not Configured
  {
    ucTmp = 0;
    WriteOD(objNodeName, MQTTS_FL_TOPICID_PREDEF, 0, &ucTmp);                   // Device Name
#ifdef RF_NODE
    uint16_t  uiTmp;
#ifdef OD_DEFAULT_ADDR
    ucTmp = OD_DEFAULT_ADDR;
#else   //  OD_DEFAULT_ADDR
    ucTmp = 0xFF;       // DHCP
#endif  //  OD_DEFAULT_ADDR
    WriteOD(objRFNodeId, MQTTS_FL_TOPICID_PREDEF, sizeof(ucTmp), &ucTmp);           // Node Id
    ucTmp = 0;
    WriteOD(objNodeName, MQTTS_FL_TOPICID_PREDEF, 0, &ucTmp);                   // Device Name
    uiTmp = OD_DEFAULT_GROUP;
    WriteOD(objRFGroup, MQTTS_FL_TOPICID_PREDEF, sizeof(uiTmp), (uint8_t *)&uiTmp); // Group Id
    ucTmp = OD_DEFAULT_CHANNEL;
    WriteOD(objRFChannel, MQTTS_FL_TOPICID_PREDEF, sizeof(ucTmp), &ucTmp);          // Channel
#ifdef ASLEEP
    uiTmp = OD_DEFAULT_TASLEEP;
    WriteOD(objTAsleep, MQTTS_FL_TOPICID_PREDEF, sizeof(uiTmp), (uint8_t *)&uiTmp); // Sleep Time
#endif  //  ASLEEP
#endif  //  RF_NODE
#ifdef LAN_NODE
    uint32_t  ulTmp;
    uint8_t   defMAC[] = {0x01, 0x00, 0x00, 0xA3, 0x04, 0x00};
    WriteOD(objMACAddr, MQTTS_FL_TOPICID_PREDEF, 6, (uint8_t *)&defMAC);     // Default MAC
    ulTmp = 0xFFFFFFFF;
    WriteOD(objIPAddr, MQTTS_FL_TOPICID_PREDEF, 4, (uint8_t *)&ulTmp);       // Default IP - use DHCP
    WriteOD(objIPMask, MQTTS_FL_TOPICID_PREDEF, 4, (uint8_t *)&ulTmp);       // Default IP Mask - use DHCP
    WriteOD(objIPRouter, MQTTS_FL_TOPICID_PREDEF, 4, (uint8_t *)&ulTmp);     // Default IP Gateway - use DHCP
    WriteOD(objIPBroker, MQTTS_FL_TOPICID_PREDEF, 4, (uint8_t *)&ulTmp);     // Default IP Broker
#endif  //  LAN_NODE
  }

    // Clear listOD
    for(ucTmp = 0; ucTmp < OD_MAX_INDEX_LIST; ucTmp++)
        ListOD[ucTmp].Index = 0xFFFF;
        
    idxUpdate = 0;
    idxSubscr = POOL_TMR_FREQ - 1;

    // Clear ext Object's
    extClean();

    // Load listOD from EEPROM
    uint8_t pos = 0;

    for(ucTmp = 0; ucTmp < OD_MAX_INDEX_LIST; ucTmp++)
    {
        eepromReadListOD((uint16_t)&ee_listOd_bu +  (ucTmp * sizeof(subidx_t)), &ListOD[pos].sidx);
        
        if((ListOD[pos].sidx.Place == 0xFF) || (ListOD[pos].sidx.Place == 0x00) ||
           (extRegisterOD(&ListOD[pos]) != MQTTS_RET_ACCEPTED))
            continue;

        ListOD[pos++].Index = 0x0000;
    }

    // Configure PnP devises
    extConfig();
}

void CleanOD(void)
{
  idxUpdate = 0;
  idxSubscr = POOL_TMR_FREQ - 1;

  uint8_t ucTmp;
    
  for(ucTmp = 0; ucTmp < OD_MAX_INDEX_LIST; ucTmp++)
    if(ListOD[ucTmp].Index == 0xFFFF)
      break;
  else
    ListOD[ucTmp].Index = 0;

  // (Re)Load PHY Configuration
  PHY_LoadConfig();

#ifdef ASLEEP
  uint16_t uiTmp;
  uint8_t Len = sizeof(uint16_t);

  ReadOD(objTAsleep, MQTTS_FL_TOPICID_PREDEF, &Len, (uint8_t *)&uiTmp);
  mqtts_set_TASleep(uiTmp);
#endif  //  ASLEEP
}

// Register PnP objects
indextable_t * getFreeIdxOD(void)
{
  uint8_t id;
  for(id = 0; id < OD_MAX_INDEX_LIST; id++)
    if(ListOD[id].Index == 0xFFFF)
    {
      ListOD[id].Index = 0;
      return &ListOD[id];
    }
  return NULL;
}

uint8_t RegisterOD(MQ_t *pBuf)
{
    // Convert Topic Name to IDX record.
    subidx_t Subidx;
    uint8_t *pTopicName;
    pTopicName = (uint8_t *)&pBuf->mq.m.regist.TopicName;
    Subidx.Place = *(pTopicName++);
    Subidx.Type = *(pTopicName++);

    uint8_t i;
    uint16_t val = 0;
    uint8_t Len = pBuf->mq.Length;
    for(i = MQTTS_SIZEOF_MSG_REGISTER + 2; i < Len; i++)
    {
        uint8_t ch = *(pTopicName++);
        if(ch >= '0' && ch <= '9')
        {
            val *= 10;
            val += ch -'0';
        }
        else
            break;
    }
    Subidx.Base = val;
    uint8_t RetVal = extCheckIdx(&Subidx);

    if(RetVal != MQTTS_RET_ACCEPTED)
        return RetVal;

    uint16_t TopicId = SWAPWORD(pBuf->mq.m.regist.TopicId);
        
    // Get Last Index OD
    uint8_t id = 0;
    while(ListOD[id].Index != 0xFFFF)
    {
        if(memcmp((const void *)&Subidx, (const void *)&ListOD[id].sidx,sizeof(subidx_t)) == 0)
            break;                                                // Object exist but not registered

        if(((ListOD[id].Index == TopicId) && (TopicId != 0)) ||   // Object exist & registered
            (++id == OD_MAX_INDEX_LIST))                          // Table full
            return MQTTS_RET_REJ_INV_ID;
    }

    if(ListOD[id].Index == 0xFFFF)                                // New variable
    {
        if(TopicId == 0)                                          // Try to delete not exist variable
            return MQTTS_RET_REJ_INV_ID;
        
        ListOD[id].sidx = Subidx;
        if(extRegisterOD(&ListOD[id]) != MQTTS_RET_ACCEPTED)      // Variable overlapped
            return MQTTS_RET_REJ_INV_ID;

        ListOD[id].Index = TopicId;

        // Save to eeprom
        uint16_t i;
        uint16_t addr;
        subidx_t Subidx2;
        
        for(i = 0; i < OD_MAX_INDEX_LIST; i++)
        {
            addr = (uint16_t)&ee_listOd_bu +  i * sizeof(subidx_t);
            eepromReadListOD(addr, &Subidx2);
            if(memcmp((const void *)&Subidx, (const void *)&Subidx2, sizeof(subidx_t)) == 0)
                break;                                              // Variable Exist in EEPROM
            if((Subidx2.Place == 0xFF) || (Subidx2.Place == 0x00))
            {
                eepromWriteListOD(addr, &Subidx);
                break;
            }
        }
    }
    else if(TopicId == 0)       // Delete Variable
        deleteIndexOD(id);

    return MQTTS_RET_ACCEPTED;
}

void RegAckOD(uint16_t index)
{
    if(index != 0)
        ListOD[idxUpdate].Index = index;
    else    // Delete Message
    {
        deleteIndexOD(idxUpdate);
        idxUpdate = 0;
    }
}

// Convert Subindex to Length
static uint8_t cvtSubidx2Len(subidx_t * pSubIdx)
{
    switch(pSubIdx->Place)
    {
        case objAin:
            return 2;
        case objPWM:
            return 1;
        case objDin:
        case objDout:
        case objSer:
            return 0;
    }

    switch(pSubIdx->Type)
    {
        case objUInt8:
            return 1;
        case objInt16:
            return 0x82;
        case objUInt16:
            return 2;
        case objInt32:
            return 0x84;
        case objUInt32:
            return 4;
        case objInt64:
            return 0x88;
    }
    return 0;
}

uint8_t ReadOD(uint16_t Id, uint8_t Flags, uint8_t *pLen, uint8_t *pBuf)
{
    indextable_t * pIndex = scanIndexOD(Id, Flags & MQTTS_FL_TOPICID_MASK);
    if(pIndex == NULL)
        return MQTTS_RET_REJ_INV_ID;
    if(pIndex->cbRead == NULL)
        return MQTTS_RET_REJ_NOT_SUPP;
    
    uint8_t retval = (pIndex->cbRead)(&pIndex->sidx, pLen, pBuf);
    
    if((Flags & 0x80) && (retval == MQTTS_RET_ACCEPTED))    // Pack Object
    {
        uint8_t len;
        len = cvtSubidx2Len(&pIndex->sidx);
        
        if(len & 0x80)      // Signed
        {
            len &= 0x7F;
            while(len > 1)
            {
                if(((pBuf[len-1] == 0)    && ((pBuf[len-2] & 0x80) == 0)) ||
                   ((pBuf[len-1] == 0xFF) && ((pBuf[len-2] & 0x80) == 0x80)))
                        len--;
                else
                    break;
            }
            *pLen = len;
        }
        else if(len > 0)    // Unisigned
        {
            if(pBuf[len - 1] & 0x80)
            {
                pBuf[len] = 0;
                len++;
            }
            else while((len > 1) && (pBuf[len-1] == 0) && ((pBuf[len-2] & 0x80) == 0))
                len--;
            *pLen = len;
        }
    }

    return retval;
}

uint8_t WriteOD(uint16_t Id, uint8_t Flags, uint8_t Len, uint8_t *pBuf)
{
    indextable_t * pIndex = scanIndexOD(Id, Flags & MQTTS_FL_TOPICID_MASK);
    if(pIndex == NULL)
        return MQTTS_RET_REJ_INV_ID;
    if(pIndex->cbWrite == NULL)
        return MQTTS_RET_REJ_NOT_SUPP;

    if(Flags & 0x80)    // Unpack Object
    {
        uint8_t len = cvtSubidx2Len(&pIndex->sidx) & 0x7F;
        if(len > Len)
        {
            uint8_t fill;
            fill = (pBuf[Len-1] & 0x80) ? 0xFF: 0x00;
            while(Len < len)
                pBuf[Len++] = fill;
        }

        if(len)
          Len = len;
    }
    return (pIndex->cbWrite)(&pIndex->sidx, Len, pBuf);
}

uint16_t PoolOD(uint8_t sleep)
{
  uint16_t Index;
  uint8_t * piBuf;
  uint8_t ucTmp;
  
  if(sleep != 0)
    idxUpdate = 0;

  while((idxUpdate < OD_MAX_INDEX_LIST) && ((Index = ListOD[idxUpdate].Index) != 0xFFFF))
  {
    if(Index == 0)                  // Register
    {
      piBuf = (uint8_t *)mqAssert();
      if(piBuf != NULL)
      {
        ucTmp = extCvtIdx2TopicId(&ListOD[idxUpdate].sidx, piBuf);
        if(MQTTS_Register(0, ucTmp, piBuf) == 0)
          ListOD[idxUpdate].Index = 0xF000;
        mqRelease((MQ_t *)piBuf);
      }
      return 0xFFFF;
    }
    else if(Index == 0xF000)        // Wait a RegAck
      return 0xFFFF;

    if((ListOD[idxUpdate].cbPool != NULL) && 
       (ListOD[idxUpdate].cbPool)(&ListOD[idxUpdate].sidx, sleep))
      return Index;
      idxUpdate++;
  }
  idxUpdate = 0;

  if(idxSubscr > 0)
  {
    idxSubscr--;
    if(idxSubscr == 0)  //  Send Subscribe '+'
    {
      ucTmp = '#';
      MQTTS_Subscribe(MQTTS_FL_QOS1, 1, &ucTmp);
    }
  }
  return 0xFFFF;
}
