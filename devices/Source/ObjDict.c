/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#include "config.h"
#include "ext.h"

//////////////////////////
// Objects List

// Local subroutines
static uint8_t eepromReadOD(subidx_t *pSubidx, uint8_t *pLen, uint8_t *pBuf);
static uint8_t eepromWriteOD(subidx_t *pSubidx, uint8_t Len, uint8_t *pBuf);
static uint8_t readDeviceInfo(subidx_t *pSubidx, uint8_t *pLen, uint8_t *pBuf);

// Prototypes for callback functions
#ifdef EXTAIN_USED
static uint8_t cbWriteADCaverage(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf);
#endif  //  EXTAIN_USED

#ifdef LAN_NODE
static uint8_t cbWriteLANParm(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf);
static uint8_t cbReadLANParm(subidx_t *pSubidx, uint8_t *pLen, uint8_t *pBuf);
#endif  //  LAN_NODE

#ifdef ASLEEP
static uint8_t cbWriteTASleep(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf);
#endif  //  ASLEEP

// List of pre defined objects
static const indextable_t listPredefOD[] = 
{
    {{objEEMEM, objString, eeNodeName},
        objNodeName, (cbRead_t)&eepromReadOD, (cbWrite_t)&eepromWriteOD, NULL},
#ifdef ASLEEP
    {{objEEMEM, objUInt16, eeTASleep},
        objTASleep, (cbRead_t)&eepromReadOD,  (cbWrite_t)&cbWriteTASleep, NULL},
#endif  //  ASLEEP
#ifdef EXTAIN_USED
    {{objEEMEM, objUInt16, eeADCaverage},
        objADCaverage, (cbRead_t)&eepromReadOD, (cbWrite_t)&cbWriteADCaverage, NULL},
#endif  //  EXTAIN_USED
#ifdef RF_ADDR_t
    {{objEEMEM, RF_ADDR_TYPE, eeNodeID},
        objRFNodeId, (cbRead_t)&eepromReadOD, (cbWrite_t)&eepromWriteOD, NULL},
    {{objEEMEM, RF_ADDR_TYPE, eeGateID},
        objRFGateId, (cbRead_t)&eepromReadOD, (cbWrite_t)&eepromWriteOD, NULL},
#ifdef OD_DEFAULT_GROUP
    {{objEEMEM, objUInt16, eeGroupID},
        objRFGroup, (cbRead_t)&eepromReadOD, (cbWrite_t)&eepromWriteOD, NULL},
#endif  //  OD_DEFAULT_GROUP
#ifdef OD_DEFAULT_CHANNEL
    {{objEEMEM, objUInt8, eeChannel},
        objRFChannel, (cbRead_t)&eepromReadOD, (cbWrite_t)&eepromWriteOD, NULL},
#endif  //  OD_DEFAULT_CHANNEL
#endif  //  RF_ADDR_t
#ifdef LAN_NODE
    {{objEEMEM, objArray, eeMACAddr},
        objMACAddr, (cbRead_t)&cbReadLANParm, (cbWrite_t)&cbWriteLANParm, NULL},
    {{objEEMEM, objArray, eeIPAddr},
        objIPAddr, (cbRead_t)&cbReadLANParm, (cbWrite_t)&cbWriteLANParm, NULL},
    {{objEEMEM, objArray, eeIPMask},
        objIPMask, (cbRead_t)&cbReadLANParm, (cbWrite_t)&cbWriteLANParm, NULL},
    {{objEEMEM, objArray, eeIPRouter},
        objIPRouter, (cbRead_t)&cbReadLANParm, (cbWrite_t)&cbWriteLANParm, NULL},
    {{objEEMEM, objArray, eeIPBroker},
        objIPBroker, (cbRead_t)&cbReadLANParm, (cbWrite_t)&cbWriteLANParm, NULL},
#endif  //  LAN_NODE
    // Read Only Objects
    {{objPROGMEM, objString, 0},
        objDeviceTyp, (cbRead_t)&readDeviceInfo, NULL, NULL},
    {{objPROGMEM, objArray, 1},
        objPHY1addr, (cbRead_t)&readDeviceInfo, NULL, NULL},
#ifdef PHY2_NodeId
    {{objPROGMEM, objArray, 2},
        objPHY2addr, (cbRead_t)&readDeviceInfo, NULL, NULL}
#endif  //  PHY2_NodeId
};

// User objects list
static indextable_t ListOD[OD_MAX_INDEX_LIST];

// End Objects List
//////////////////////////

// Local variables
static uint16_t idxUpdate = 0;                                          // Poll pointer

//////////////////////////
// Callback functions
#ifdef EXTAIN_USED
void ainLoadAverage(void);
static uint8_t cbWriteADCaverage(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf)
{
    eepromWriteOD(pSubidx, Len, pBuf);
    ainLoadAverage();
    return MQTTSN_RET_ACCEPTED;
}
#endif  //  EXTAIN_USED

#ifdef LAN_NODE
// Convert raw data from mqtt-sn packet to LAN variables
static uint8_t cbWriteLANParm(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf)
{
    uint16_t Base = pSubidx->Base;

    if(Base == eeMACAddr)
    {
        if(Len != 6)
            return MQTTSN_RET_REJ_CONG;
    }
    else if(Len != 4)
        return MQTTSN_RET_REJ_CONG;

    eeprom_write(pBuf, Base, Len);
    return MQTTSN_RET_ACCEPTED;
}

static uint8_t cbReadLANParm(subidx_t *pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
    uint16_t Base = pSubidx->Base;

    if(Base == eeMACAddr)
        *pLen = 6;
    else
        *pLen = 4;

    eeprom_read(pBuf, Base, *pLen);
    return MQTTSN_RET_ACCEPTED;
}
#endif  //  LAN_NODE

#ifdef ASLEEP
static uint8_t cbWriteTASleep(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf)
{
    uint16_t val;
    val = pBuf[1];
    val <<= 8;
    val |= pBuf[0];
    MQTTSN_Set_ASleep(val);
    eeprom_write(pBuf, pSubidx->Base, Len);
    return MQTTSN_RET_ACCEPTED;
}
#endif  //  ASLEEP

// End callback's
//////////////////////////

//////////////////////////
// Local Subroutines

// Search Object by Index
static indextable_t * scanIndexOD(uint16_t index, uint8_t flags)
{
    uint16_t i;

    flags &= MQTTSN_FL_TOPICID_MASK;
    if(flags == MQTTSN_FL_TOPICID_NORM)
    {
        for(i = 0; i < OD_MAX_INDEX_LIST; i++)
            if(ListOD[i].Index == index)
                return &ListOD[i]; 
    }
    else if(flags == MQTTSN_FL_TOPICID_PREDEF)
    {
        for(i = 0; i < sizeof(listPredefOD)/sizeof(indextable_t); i++)
            if(listPredefOD[i].Index == index)
                return (indextable_t *)&listPredefOD[i];
    }

    return NULL;
}

// Convert Subindex to Length - pack/unpack objects
static uint8_t cvtSubidx2Len(subidx_t * pSubIdx)
{
    switch(pSubIdx->Place)
    {
        case objAin:
        case objPWM:
            return 2;
        case objDin:
        case objDout:
        case objSer:
            return 0;
        default:
            break;
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
        default:
            break;
    }
    return 0;
}

// Read predefined object from EEPROM
static uint8_t eepromReadOD(subidx_t *pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
    uint8_t Len;
    uint16_t Base;

    Base = pSubidx->Base;

    switch(pSubidx->Type)
    {
        case objBool:
        case objInt8:
        case objUInt8:
            *pLen = sizeof(uint8_t);
            break;
        case objInt16:
        case objUInt16:
            *pLen = sizeof(uint16_t);
            break;
        case objInt32:
        case objUInt32:
            *pLen = sizeof(uint32_t);
            break;
        case objString:
        case objArray:
            eeprom_read(&Len, Base, 1);
            *pLen = Len < *pLen ? Len : *pLen;
            if(*pLen == 0)
                return MQTTSN_RET_ACCEPTED;
            Base++;
            break;
        default:
            return MQTTSN_RET_REJ_NOT_SUPP;
    }
    eeprom_read(pBuf, Base, *pLen);

    return MQTTSN_RET_ACCEPTED;
}

// Write predefined object to EEPROM
static uint8_t eepromWriteOD(subidx_t *pSubidx, uint8_t Len, uint8_t *pBuf)
{
    uint16_t Base;

    Base = pSubidx->Base;

    switch(pSubidx->Type)
    {
        case objBool:
        case objInt8:
        case objUInt8:
            Len = sizeof(uint8_t);
            break;
        case objInt16:
        case objUInt16:
            Len = sizeof(uint16_t);
            break;
        case objInt32:
        case objUInt32:
            Len = sizeof(uint32_t);
            break;
        case objString:
        case objArray:
            eeprom_write(&Len, Base, 1);
            if(Len == 0)
                return MQTTSN_RET_ACCEPTED;
            Base++;
            break;
        default:
            return MQTTSN_RET_REJ_NOT_SUPP;
    }
    eeprom_write(pBuf, Base, Len);
    return MQTTSN_RET_ACCEPTED;
}

// Predefined Object's
static const uint8_t psDeviceTyp[] = {
                                OD_DEV_UC_TYPE,           // uC Family
                                OD_DEV_UC_SUBTYPE,        // uC SubType
                                OD_DEV_PHY1,              // PHY1 type
                                OD_DEV_PHY2,              // PHY2 type
                                OD_DEV_HW_TYP_H,          // HW Version High
                                OD_DEV_HW_TYP_L,          // HW Version Low
                                '.',                      // Delimiter
                                OD_DEV_SWVERSH,           // Software Version
                                OD_DEV_SWVERSM,
                                OD_DEV_SWVERSL};

static uint8_t readDeviceInfo(subidx_t *pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
    uint16_t Base = pSubidx->Base;

    if(Base == 0)           // Read Device type
    {
        uint16_t Len = sizeof(psDeviceTyp);
        if(Len > *pLen)
            Len = *pLen;

        memcpy((void *)pBuf, (const void *)psDeviceTyp, Len);
        *pLen = Len;

        return MQTTSN_RET_ACCEPTED;
    }
    
    // Read PHY address
    uint16_t index = 0xFFFF;

    if(Base == 1)
    {
        index = PHY1_NodeId;
    }
#ifdef PHY2_NodeId
    else if(Base == 2)
    {
        index = PHY2_NodeId;
    }
#endif  //  PHY2_NodeId

    return ReadOD(index, MQTTSN_FL_TOPICID_PREDEF, pLen, pBuf);
}

static void RestoreSubindex(uint16_t sidxn, subidx_t *pSubidx)
{
    uint16_t addr = sidxn;
    addr *= sizeof(subidx_t);
    addr += eelistOdbu;
    eeprom_read((uint8_t *)pSubidx, addr, sizeof(subidx_t));
}

static void SaveSubindex(uint16_t sidxn, subidx_t *pSubidx)
{
    uint16_t addr = sidxn;
    addr *= sizeof(subidx_t);
    addr += eelistOdbu;
    eeprom_write((uint8_t *)pSubidx, addr, sizeof(subidx_t));
}

// delete object
static void deleteIndexOD(uint8_t id)
{
    if(id >= OD_MAX_INDEX_LIST)
        return;
    
    ListOD[id].Index = 0xFFFF;

    // delete from EEPROM
    uint16_t i;
    subidx_t subidx;
    for(i = 0; i < OD_MAX_INDEX_LIST; i++)
    {
        RestoreSubindex(i, &subidx);
        if(memcmp((const void *)&subidx, (const void *)&ListOD[id].sidx, sizeof(subidx_t)) == 0)
        {
            subidx.Place = 0xFF;
            subidx.Type = 0xFF;
            subidx.Base  = 0xFFFF;
            SaveSubindex(i, &subidx);
            break;
        }
    }

    extDeleteOD(&ListOD[id].sidx);
}
// End Local Subroutines
//////////////////////////

void InitOD(void)
{
    eeprom_init_hw();

    // Check Settings
    uint8_t     Len = 1;
    uint8_t     ucTmp = 0;
    uint16_t    uiTmp;

    ReadOD(objNodeName, MQTTSN_FL_TOPICID_PREDEF, &Len, &ucTmp);
    if(ucTmp == 0xFF)                                                                       // Not Configured
    {
        // Load Default Settings
#ifdef RF_ADDR_t
#ifndef ADDR_DEFAULT_RF
#define ADDR_DEFAULT_RF ADDR_UNDEF_RF    // DHCP
#endif  //  ADDR_DEFAULT_RF
        RF_ADDR_t rfAddr = ADDR_DEFAULT_RF;
        WriteOD(objRFNodeId, MQTTSN_FL_TOPICID_PREDEF, sizeof(RF_ADDR_t), &rfAddr);         // Node address
        RF_ADDR_t rfGw = ADDR_UNDEF_RF;
        WriteOD(objRFGateId, MQTTSN_FL_TOPICID_PREDEF, sizeof(RF_ADDR_t), &rfGw);             // Gateway address
#ifdef OD_DEFAULT_GROUP
        uiTmp = OD_DEFAULT_GROUP;
        WriteOD(objRFGroup, MQTTSN_FL_TOPICID_PREDEF, sizeof(uiTmp), (uint8_t *)&uiTmp);    // Group Id
#endif  //  OD_DEFAULT_GROUP
#ifdef OD_DEFAULT_CHANNEL
        ucTmp = OD_DEFAULT_CHANNEL;
        WriteOD(objRFChannel, MQTTSN_FL_TOPICID_PREDEF, sizeof(ucTmp), &ucTmp);             // Channel
#endif  //  OD_DEFAULT_CHANNEL
#endif  //  RF_ADDR_t
#ifdef LAN_NODE
#ifndef OD_DEF_IP_ADDR
#define OD_DEF_IP_ADDR      0xFFFFFFFF      // Default IP - use DHCP
#endif  //  OD_DEF_IP_ADDR
#ifndef OD_DEF_IP_MASK
#define OD_DEF_IP_MASK      0xFFFFFFFF      // Default IP Mask - use DHCP
#endif  //  OD_DEF_IP_MASK
#ifndef OD_DEF_IP_ROUTER
#define OD_DEF_IP_ROUTER    0xFFFFFFFF      // Default IP Gateway - use DHCP
#endif  //  OD_DEF_IP_ROUTER
#ifndef OD_DEF_IP_BROKER
#define OD_DEF_IP_BROKER    0xFFFFFFFF      // Default IP Broker - auto resolve
#endif  //  OD_DEF_IP_BROKER
        uint32_t  ulTmp;
        ulTmp = OD_DEF_IP_BROKER;
        WriteOD(objIPBroker, MQTTSN_FL_TOPICID_PREDEF, 4, (uint8_t *)&ulTmp);
        ulTmp = OD_DEF_IP_ROUTER;
        WriteOD(objIPRouter, MQTTSN_FL_TOPICID_PREDEF, 4, (uint8_t *)&ulTmp);
        ulTmp = OD_DEF_IP_MASK;
        WriteOD(objIPMask,   MQTTSN_FL_TOPICID_PREDEF, 4, (uint8_t *)&ulTmp);
        ulTmp = OD_DEF_IP_ADDR;
        WriteOD(objIPAddr,   MQTTSN_FL_TOPICID_PREDEF, 4, (uint8_t *)&ulTmp);
        uint8_t   defMAC[] = OD_DEV_MAC;
        WriteOD(objMACAddr, MQTTSN_FL_TOPICID_PREDEF, 6, (uint8_t *)&defMAC);       // Default MAC
#endif  //  LAN_NODE
#ifdef EXTAIN_USED
        uiTmp = 80;
        WriteOD(objADCaverage, MQTTSN_FL_TOPICID_PREDEF, sizeof(uiTmp), (uint8_t *)&uiTmp);   // ADC conversion delay
#endif  //  EXTAIN_USED
#ifdef ASLEEP
        uiTmp = OD_DEFAULT_TASLEEP;
        WriteOD(objTASleep, MQTTSN_FL_TOPICID_PREDEF, sizeof(uiTmp), (uint8_t *)&uiTmp);    // Sleep Time
#endif  //  ASLEEP
        ucTmp = 0;
        WriteOD(objNodeName, MQTTSN_FL_TOPICID_PREDEF, 0, &ucTmp);                          // Device Name
    }

    // Clear listOD
    for(uiTmp = 0; uiTmp < OD_MAX_INDEX_LIST; uiTmp++)
        ListOD[uiTmp].Index = 0xFFFF;

    // Clear Poll Variables
    idxUpdate = 0x0000;

    extInit();

    // Load Saved Variables
    uint16_t pos = 0;
    for(uiTmp = 0; uiTmp < OD_MAX_INDEX_LIST; uiTmp++)
    {
        // Skip PNP objects
        while(ListOD[pos].Index != 0xFFFF)
        {
            pos++;
            if(pos == OD_MAX_INDEX_LIST)
                return;
        }

        RestoreSubindex(uiTmp, &ListOD[pos].sidx);

        if((ListOD[pos].sidx.Place == 0xFF) || (ListOD[pos].sidx.Place == 0x00) ||
           (extRegisterOD(&ListOD[pos]) != MQTTSN_RET_ACCEPTED))
            continue;

        ListOD[pos++].Index = 0x0000;
    }
}

// Register PnP objects, get pointer to a free ListOD record
indextable_t * getFreeIdxOD(void)
{
    uint8_t id;
    for(id = 0; id < OD_MAX_INDEX_LIST; id++)
    {
        if(ListOD[id].Index == 0xFFFF)
        {
            ListOD[id].cbRead   = NULL;
            ListOD[id].cbWrite  = NULL;
            ListOD[id].cbPoll   = NULL;
            ListOD[id].Index    = 0x0000;
            return &ListOD[id];
        }
    }
    return NULL;
}

e_MQTTSN_RETURNS_t ReadOD(uint16_t Id, uint8_t Flags, uint8_t *pLen, uint8_t *pBuf)
{
    indextable_t * pIndex = scanIndexOD(Id, Flags);
    if(pIndex == NULL)
    {
        *pLen = 0;
        return MQTTSN_RET_REJ_INV_ID;
    }
    if(pIndex->cbRead == NULL)
    {
        *pLen = 0;
        return MQTTSN_RET_REJ_NOT_SUPP;
    }
  
    e_MQTTSN_RETURNS_t retval;
    retval = (pIndex->cbRead)(&pIndex->sidx, pLen, pBuf);
    if(retval != MQTTSN_RET_ACCEPTED)
        *pLen = 0;

    return retval;
}

e_MQTTSN_RETURNS_t WriteOD(uint16_t Id, uint8_t Flags, uint8_t Len, uint8_t *pBuf)
{
    indextable_t * pIndex = scanIndexOD(Id, Flags);
    if(pIndex == NULL)
        return MQTTSN_RET_REJ_INV_ID;
    if(pIndex->cbWrite == NULL)
        return MQTTSN_RET_REJ_NOT_SUPP;

    return (pIndex->cbWrite)(&pIndex->sidx, Len, pBuf);
}

// Make Topic Name from record number
uint8_t MakeTopicName(uint8_t RecNR, uint8_t *pBuf)
{
    *(uint8_t*)(pBuf++) = ListOD[RecNR].sidx.Place;
    *(uint8_t*)(pBuf++) = ListOD[RecNR].sidx.Type;

    uint16_t addr = ListOD[RecNR].sidx.Base;
    // sprintf(pBuf,"%d",addr);
    uint16_t div = 10000;
    uint8_t ch, fl = 0, len = 3;

    while(div >= 10)
    {
        if(addr >= div)
        {
            ch = addr / div;
            addr = addr % div;
        }
        else
            ch = 0;

        div = div/10;

        if((ch != 0) || (fl != 0))
        {
            fl = 1;
            *(pBuf++) = ch + '0';
            len++;
        }
    }
    *pBuf = addr + '0';
    return len;
}

void RegAckOD(uint16_t index)
{
    if(index != 0xFFFF)
        ListOD[idxUpdate].Index = index;
    else    // Delete Message
        deleteIndexOD(idxUpdate);
    idxUpdate++;
}

e_MQTTSN_RETURNS_t RegisterOD(MQTTSN_MESSAGE_t *pMsg)
{
    // Convert Topic Name to IDX record.
    subidx_t    Subidx;
    uint8_t     *pTopicName;
    pTopicName   = (uint8_t *)&pMsg->regist.TopicName;
    Subidx.Place = *(pTopicName++);
    Subidx.Type  = *(pTopicName++);

    // atoi
    {
    uint8_t i;
    uint16_t val = 0;
    uint8_t Len = pMsg->Length;
    for(i = MQTTSN_SIZEOF_MSG_REGISTER + 2; i < Len; i++)
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
    }

    if(!extCheckSubidx(&Subidx))
        return MQTTSN_RET_REJ_NOT_SUPP;

    uint16_t TopicId = (pMsg->regist.TopicId[0]<<8) | pMsg->regist.TopicId[1];

    // Get Index OD
    uint16_t id = 0xFFFF;
    {
    uint16_t pos;
    for(pos = 0; pos < OD_MAX_INDEX_LIST; pos++)
    {
        if(ListOD[pos].Index == 0xFFFF)
        {
            if(id == 0xFFFF)
                id = pos;
        }
        else
        {
            if(memcmp((const void *)&Subidx, (const void *)&ListOD[pos].sidx, sizeof(subidx_t)) == 0)
            {
                id = pos;
                break;                                                  // Object exist    
            }
            
            if(ListOD[pos].Index == TopicId)                            // TopicId exist but with another subidx
                return MQTTSN_RET_REJ_INV_ID;
        }
    }
    }

    if(id == 0xFFFF)                                                    // Table is full
        return MQTTSN_RET_REJ_CONG;

    if(ListOD[id].Index == 0xFFFF)                                      // New variable
    {
        if(TopicId == 0xFFFF)                                           // Try to delete not exist variable
            return MQTTSN_RET_REJ_INV_ID;

        ListOD[id].sidx = Subidx;
        if(extRegisterOD(&ListOD[id]) != MQTTSN_RET_ACCEPTED)           // Variable overlapped
            return MQTTSN_RET_REJ_INV_ID;

        ListOD[id].Index = TopicId;
    
        // Save to eeprom
        {
        uint16_t i;
        subidx_t Subidx2;
        for(i = 0; i < OD_MAX_INDEX_LIST; i++)
        {
            RestoreSubindex(i, &Subidx2);
            if(memcmp((const void *)&Subidx, (const void *)&Subidx2, sizeof(subidx_t)) == 0)
                break;                                                  // Variable Exist in EEPROM
            if((Subidx2.Place == 0xFF) || (Subidx2.Place == 0x00))
            {
                SaveSubindex(i, &Subidx);
                break;
            }
        }
        }
    }
    else if(TopicId == 0xFFFF)          // Delete Variable
        deleteIndexOD(id);
    else                                // Renew Topic ID, or duplicate message
        ListOD[id].Index = TopicId;

    return MQTTSN_RET_ACCEPTED;
}

// Read and pack object by Index. 
e_MQTTSN_RETURNS_t ReadODpack(uint16_t Id, uint8_t Flags, uint8_t *pLen, uint8_t *pBuf)
{
    indextable_t * pIndex = scanIndexOD(Id, Flags);
    if(pIndex == NULL)
    {
        *pLen = 0;
        return MQTTSN_RET_REJ_INV_ID;
    }
    if(pIndex->cbRead == NULL)
    {
        *pLen = 0;
        return MQTTSN_RET_REJ_NOT_SUPP;
    }

    e_MQTTSN_RETURNS_t retval = (pIndex->cbRead)(&pIndex->sidx, pLen, pBuf);

    if(retval == MQTTSN_RET_ACCEPTED)    // Pack Object
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
        else if(len > 0)    // Unsigned
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
    else
        *pLen = 0;

    return retval;
}

// Unpack and write object by Index.
e_MQTTSN_RETURNS_t WriteODpack(uint16_t Id, uint8_t Flags, uint8_t Len, uint8_t *pBuf)
{
    indextable_t * pIndex = scanIndexOD(Id, Flags);
    if(pIndex == NULL)
        return MQTTSN_RET_REJ_INV_ID;
    if(pIndex->cbWrite == NULL)
        return MQTTSN_RET_REJ_NOT_SUPP;

    // Unpack Object
    uint8_t len;
    len = cvtSubidx2Len(&pIndex->sidx) & 0x7F;
    if(len > Len)
    {
        uint8_t fill;
        fill = (pBuf[Len-1] & 0x80) ? 0xFF: 0x00;
        while(Len < len)
            pBuf[Len++] = fill;
    }

    if(len)
        Len = len;

    return (pIndex->cbWrite)(&pIndex->sidx, Len, pBuf);
}

// OD Main task
void OD_Poll(void)
{
    // Read/Update IOs state
    extProc();

    // Send Data to Broker
    e_MQTTSN_STATUS_t status = MQTTSN_GetStatus();
    
    if((status == MQTTSN_STATUS_CONNECT)
#ifdef ASLEEP
        || (status == MQTTSN_STATUS_AWAKE) || (status == MQTTSN_STATUS_ASLEEP)
#endif  //  ASLEEP
        )
    {
        if(idxUpdate >= OD_MAX_INDEX_LIST)
            idxUpdate = 0;

        while(MQTTSN_CanSend() && (idxUpdate < OD_MAX_INDEX_LIST))
        {
            uint16_t index = ListOD[idxUpdate].Index;

            if(index == 0x0000)                         // Register new variable
            {
                MQTTSN_Send(MQTTSN_MSGTYP_REGISTER,     // Message type
                            idxUpdate,                  // Flags
                            index);                     // Topic Id
            }
            else if(index != 0xFFFF)                    // Poll data & publish on ready
            {
                if((ListOD[idxUpdate].cbPoll != NULL) &&
                   (ListOD[idxUpdate].cbPoll)(&ListOD[idxUpdate].sidx, 0))
                {
                    MQTTSN_Send(MQTTSN_MSGTYP_PUBLISH,
                               (MQTTSN_FL_QOS1 | MQTTSN_FL_TOPICID_NORM),
                               index);
                }
            }
            idxUpdate++;
        }
    }
    else if(status == MQTTSN_STATUS_PRE_CONNECT)
    {
        // Register variables
        if(idxUpdate < OD_MAX_INDEX_LIST)
        {
            if(ListOD[idxUpdate].Index != 0xFFFF)
            {
                if(MQTTSN_CanSend())
                    MQTTSN_Send(MQTTSN_MSGTYP_REGISTER,     // Message type
                                idxUpdate,                  // Flags
                                ListOD[idxUpdate].Index);   // Topic Id
            }
            else
                idxUpdate++;
        }
        // Publish device info
        else if(idxUpdate < 0xFFFF)
        {
            if(idxUpdate < 0xFFC0)
                idxUpdate = 0xFFC0;
            if(MQTTSN_CanSend())
            {
                if(scanIndexOD(idxUpdate, MQTTSN_FL_TOPICID_PREDEF) != NULL)
                {
                    MQTTSN_Send(MQTTSN_MSGTYP_PUBLISH,
                                (MQTTSN_FL_QOS1 | MQTTSN_FL_RETAIN | MQTTSN_FL_TOPICID_PREDEF),
                                idxUpdate);
                    idxUpdate++;
                }
                else
                    idxUpdate = 0xFFFF;
            }
        }
        // Send Subscribe
        else
        {
            if(MQTTSN_CanSend())
                MQTTSN_Send(MQTTSN_MSGTYP_SUBSCRIBE, (MQTTSN_FL_QOS1 | MQTTSN_FL_TOPICID_NORM), 0);
        }
    }
    else
        idxUpdate = 0x00;
}
