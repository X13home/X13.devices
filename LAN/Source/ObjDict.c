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
const uint8_t  EEMEM ee_NodeID = 0xFF;                                  // Device Addr
const uint16_t EEMEM ee_GroupID = 0xFFFF;                               // Group ID
const uint8_t  EEMEM ee_Channel = 0xFF;                                 // Channel
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
    {{objPROGMEM, objString, (uint16_t)&psDeviceTyp},
        objDeviceTyp, (cbRead_t)&progmemReadOD, NULL, NULL},
    {{objEEMEM, objString, (uint16_t)&ee_NodeName},
        objNodeName, (cbRead_t)&eepromReadOD, (cbWrite_t)&eepromWriteOD, NULL},
#ifdef ASLEEP
    {{objEEMEM, objUInt16, (uint16_t)&ee_TAsleep},
        objTAsleep, (cbRead_t)&eepromReadOD,  (cbWrite_t)&cbWriteTASleep, NULL}
#endif  //  ASLEEP
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
    uint8_t ucTmp;
    uint8_t Len;
    
    ReadOD(objNodeName, MQTTS_FL_TOPICID_PREDEF, &Len, &ucTmp);
    if(ucTmp == 0xFF)                                                           // Not Configured
    {
        ucTmp = 0;
        WriteOD(objNodeName, MQTTS_FL_TOPICID_PREDEF, 0, &ucTmp);                   // Device Name
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

    indextable_t Index;
    for(ucTmp = 0; ucTmp < OD_MAX_INDEX_LIST; ucTmp++)
    {
        eepromReadListOD((uint16_t)&ee_listOd_bu +  (ucTmp * sizeof(subidx_t)), &Index.sidx);
        
        if((Index.sidx.Place == 0xFF) || (Index.sidx.Place == 0x00) ||
           (extRegisterOD(&Index) != MQTTS_RET_ACCEPTED))
            continue;

        Index.Index = 0x0000;
        ListOD[pos++] = Index;
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

#ifdef ASLEEP
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
            break;                                                  // Object exist but not registered

        if(((ListOD[id].Index == TopicId) && (TopicId != 0)) ||   // Object exist & registered
            (++id == OD_MAX_INDEX_LIST))                            // Table full
            return MQTTS_RET_REJ_INV_ID;
    }

    if(ListOD[id].Index == 0xFFFF)                                     // New variable
    {
        if(TopicId == 0)                                        // Try to delete not exist variable
            return MQTTS_RET_REJ_INV_ID;
        
        indextable_t Index;
        Index.sidx = Subidx;
        if((RetVal = extRegisterOD(&Index)) != MQTTS_RET_ACCEPTED)  // Variable overlapped
            return MQTTS_RET_REJ_INV_ID;

        Index.Index = TopicId;
        ListOD[id] = Index;

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
    }
    return (pIndex->cbWrite)(&pIndex->sidx, Len, pBuf);
}

uint16_t PoolOD(void)
{
    uint16_t Index;
    while((idxUpdate < OD_MAX_INDEX_LIST) && ((Index = ListOD[idxUpdate].Index) != 0xFFFF))
    {
        if(Index == 0)                  // Register
        {
            uint8_t iBuf[MQTTS_MSG_SIZE-2];
            uint8_t Len;

            Len = extCvtIdx2TopicId(&ListOD[idxUpdate].sidx, iBuf);
            if(MQTTS_Register(0, Len, iBuf) == 0)
                ListOD[idxUpdate].Index = 0xF000;
            return 0xFFFF;
        }
        else if(Index == 0xF000)        // Wait a RegAck
            return 0xFFFF;
            
            
        if((ListOD[idxUpdate].cbPool != NULL) && 
           (ListOD[idxUpdate].cbPool)(&ListOD[idxUpdate].sidx))
                return Index;
        idxUpdate++;
    }
    idxUpdate = 0;

    if(idxSubscr > 0)
    {
        idxSubscr--;
        if(idxSubscr == 0)  //  Send Subscribe '+'
        {
            uint8_t ch = '+';
            MQTTS_Subscribe(MQTTS_FL_QOS1, 1, &ch);
        }
    }
    return 0xFFFF;
}
