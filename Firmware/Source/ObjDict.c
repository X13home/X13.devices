/*
Copyright (c) 2011-2012 <comparator@gmx.de>

This file is part of the X13.Home project.
https://github.com/X13home

BSD License
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

// Predefined Object's 
const PROGMEM uint8_t psDeviceTyp[] = OD_DEFAULT_TYP;

const PROGMEM indextable_t listPredefOD[] = 
{
    {{objPROGMEM, objString, (uint16_t)&psDeviceTyp},
        objDeviceTyp, (cbRead_t)&progmemReadOD, NULL, NULL},
    {{objEEMEM, objUInt8, (uint16_t)&ee_GroupID},
        objRFNodeId, (cbRead_t)&eepromReadOD, (cbWrite_t)&eepromWriteOD, NULL},
    {{objEEMEM, objUInt16, (uint16_t)&ee_NodeID},
        objRFGroup, (cbRead_t)&eepromReadOD, (cbWrite_t)&eepromWriteOD, NULL},
    {{objEEMEM, objUInt8, (uint16_t)&ee_Channel},
        objRFChannel, (cbRead_t)&eepromReadOD, (cbWrite_t)&eepromWriteOD, NULL},
    {{objEEMEM, objString, (uint16_t)&ee_NodeName},
        objNodeName, (cbRead_t)&eepromReadOD, (cbWrite_t)&eepromWriteOD, NULL},
#ifdef ASLEEP
    {{objEEMEM, objUInt16, (uint16_t)&ee_TAsleep},
        objTAsleep, (cbRead_t)&eepromReadOD,  (cbWrite_t)&cbWriteTASleep, NULL}
#endif  //  ASLEEP
};

// Local Variables
static indextable_t ListOD[OD_MAX_INDEX_LIST];                          // Object's List
static uint8_t idxUpdate, idxSubscr;                                    // Pool variable

// Search Object by Index
static uint8_t scanIndexOD(uint16_t index, uint8_t topic, indextable_t * pIndex)
{
    uint16_t i = 0;
    if(topic == MQTTS_FL_TOPICID_NORM)
    {
        for(i = 0; (i < OD_MAX_INDEX_LIST) && (ListOD[i].Index != 0xFFFF) ; i++)
            if(ListOD[i].Index == index)
            {
                memcpy(pIndex, &ListOD[i], sizeof(indextable_t));
                return 1;
            }
    }
    else if(topic == MQTTS_FL_TOPICID_PREDEF)
    {
        for(i = 0; i < sizeof(listPredefOD)/sizeof(indextable_t); i++)
        {
            memcpy_P(pIndex, &listPredefOD[i], sizeof(indextable_t));
            if(pIndex->Index == index)
                return 1;
        }
    }
    return 0;
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
    uint16_t uiTmp;
    uint8_t Len;
    ReadOD(objRFChannel, MQTTS_FL_TOPICID_PREDEF, &Len, &ucTmp);
    if(ucTmp == 0xFF)                                                           // Not Configured
    {
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

    // (Re)Load RF Configuration
    uint16_t uiTmp;
    uint8_t channel, NodeID, Len = sizeof(uint16_t);
    // Load config data
    ReadOD(objRFGroup, MQTTS_FL_TOPICID_PREDEF,   &Len, (uint8_t *)&uiTmp);
    ReadOD(objRFNodeId, MQTTS_FL_TOPICID_PREDEF,  &Len, &NodeID);
    ReadOD(objRFChannel, MQTTS_FL_TOPICID_PREDEF, &Len, &channel);
    rf_LoadCfg(channel, uiTmp, NodeID);
    
#ifdef ASLEEP
    ReadOD(objTAsleep, MQTTS_FL_TOPICID_PREDEF, &Len, (uint8_t *)&uiTmp);
    mqtts_set_TASleep(uiTmp);
#endif  //  ASLEEP
    
}

// Register PnP objects
uint8_t RegistIntOD(indextable_t * pIdx)
{
    uint8_t id;
    pIdx->Index = 0;        // Not registered

    for(id = 0; id < OD_MAX_INDEX_LIST; id++)
    {
        if(ListOD[id].Index == 0xFFFF)
        {
            ListOD[id] = *pIdx;
            return MQTTS_RET_ACCEPTED;
        }
    }
    return MQTTS_RET_REJ_INV_ID;
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

uint8_t ReadOD(uint16_t Id, uint8_t Flags, uint8_t *pLen, uint8_t *pBuf)
{
    indextable_t Index;
    if(scanIndexOD(Id, Flags & MQTTS_FL_TOPICID_MASK, &Index) == 0)
        return MQTTS_RET_REJ_INV_ID;
    if(Index.cbRead == NULL)
        return MQTTS_RET_REJ_NOT_SUPP;
    return (Index.cbRead)(&Index.sidx, pLen, pBuf);
}

uint8_t WriteOD(uint16_t Id, uint8_t Flags, uint8_t Len, uint8_t *pBuf)
{
    indextable_t Index;
    if(scanIndexOD(Id, Flags & MQTTS_FL_TOPICID_MASK, &Index) == 0)
        return MQTTS_RET_REJ_INV_ID;
    if(Index.cbWrite == NULL)
        return MQTTS_RET_REJ_NOT_SUPP;
    return (Index.cbWrite)(&Index.sidx, Len, pBuf);
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
            if(MQTTS_Register(0, 0, Len, iBuf) == 0)
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
            MQTTS_Subscribe(0, MQTTS_FL_QOS1, 1, &ch);
        }
    }
    return 0xFFFF;
}
