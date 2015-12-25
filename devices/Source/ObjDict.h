/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _OBJ_DICT_H_
#define _OBJ_DICT_H_

#ifdef __cplusplus
extern "C" {
#endif

#define OD_DEV_TYP_LEN          6

// Predefined variables
typedef enum
{
    // Global Settings
    objNodeName         = (uint16_t)0xFF00, // _sName<String>
    objTASleep          = (uint16_t)0xFF01, // cfg/XD_SleepTime<UIint16>
    objADCaverage       = (uint16_t)0xFF08, // cfg/XD_ADCintegrate<UIint16>
    // RF Node
    objRFNodeId         = (uint16_t)0xFF10, // cfg/XD_DeviceAddr<UInt8>
    objRFGroup          = (uint16_t)0xFF11, // cfg/XD_GroupID<UInt16>
    objRFChannel        = (uint16_t)0xFF12, // cfg/XD_Channel<UInt8>
    objRFGateId         = (uint16_t)0xFF14, // cfg/XD_GateId
    // Lan Node 
    objMACAddr          = (uint16_t)0xFF20, // cfg/Xa_MACAddr   - Array - Len 6
    objIPAddr           = (uint16_t)0xFF21, // cfg/Xa_IPAddr    - Array - Len 4
    objIPMask           = (uint16_t)0xFF22, // cfg/Xa_IPMask    - Array - Len 4
    objIPRouter         = (uint16_t)0xFF23, // cfg/Xa_IPRouter  - Array - Len 4
    objIPBroker         = (uint16_t)0xFF24, // cfg/Xa_IPBroker  - Array - Len 4
    // Read Only Variables
    objDeviceTyp        = (uint16_t)0xFFC0, // _declarer<String>
    objPHY1addr         = (uint16_t)0xFFC1, // cfg/_a_phy1
    objPHY2addr         = (uint16_t)0xFFC2, // cfg/_a_phy2
    objPHY3addr         = (uint16_t)0xFFC3, // cfg/_a_phy3
    objPHY4addr         = (uint16_t)0xFFC4, // cfg/_a_phy4
  
    objRSSI             = (uint16_t)0xFFC8, // cfg/XD_RSSI<UInt8>
    /*
    FFD0 .. FFDF - system variables
    */  
    // Debug Variables
    objLogD             = (uint16_t)0xFFE0, // Data logging, log level - Debug
    objLogI             = (uint16_t)0xFFE1, // Data logging, log level - Info
    objLogW             = (uint16_t)0xFFE2, // Data logging, log level - Warning
    objLogE             = (uint16_t)0xFFE3  // Data logging, log level - Error
}eObjList_t;

typedef enum
{
    objPROGMEM  = 'F',  // FLASH
    objEEMEM    = 'E',  // EEPROM
    objAin      = 'A',  // Analog Input
    objAout     = 'D',  // Analog Output
    objDin      = 'I',  // Digital Input
    objDout     = 'O',  // Digital Output
    objPWM      = 'P',  // LED PWM out
    objSer      = 'S',  // Serial port, S.0-5 (1200-38400)
    objMerker   = 'M',  // PLC variables
    objPLCctrl  = 'p',  // PLC Control
    objTWI      = 'T',  // TWI
    objUsrExt   = 'X',  // User extensions
}eObjPlace_t;

typedef enum
{
  objBool     = 'z',  // bool
  objInt8     = 'b',  // int8
  objUInt8    = 'B',  // uint8
  objInt16    = 'w',  // int16
  objUInt16   = 'W',  // uint16
  objInt32    = 'd',  // int32
  objUInt32   = 'D',  // uint32
  objInt64    = 'q',  // int64
  objString   = 's',  // UTF8 '0' - terminated string
  objArray    = 'a',  // array
  objUnknown  = 'U',  // unknown
// Port's
  objPinPNP   = 'p',  // Pin PNP
  objPinNPN   = 'n',  // Pin NPN ( Input - PullUp)
  objActPNP   = 'A',  // Node Active  = 1, sleep = 0
  objActNPN   = 'a',  // Node Active = 0, sleep = 1;
// Analog In
  objArefVcc  = 'v',  // Ref = Vcc(3,3V)
  objArefExt  = 'e',  // Ref = External(not connected)
  objArefInt1 = 'i',  // Ref = Internal(1,1V)
  objArefInt2 = 'I',  // Ref = Internal(2,56V)
// Serial port
  ObjSerRx    = 'r',  // Com to RF
  ObjSerTx    = 't',  // RF to COM, Object type - array
}eObjTyp_t;

// Variable description
typedef struct
{
  eObjPlace_t Place;
  eObjTyp_t   Type;
  uint16_t    Base;
}subidx_t;

typedef e_MQTTSN_RETURNS_t (*cbRead_t)(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf);  // Callback Read
typedef e_MQTTSN_RETURNS_t (*cbWrite_t)(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf);   // Callback Write
typedef uint8_t (*cbPoll_t)(subidx_t * pSubidx, uint8_t sleep);                  // Callback Poll

// Structure for creating entries
typedef struct
{
  subidx_t    sidx;
  uint16_t    Index;
  cbRead_t    cbRead;
  cbWrite_t   cbWrite;
  cbPoll_t    cbPoll;
}indextable_t;

// EEPROM Object's
enum
{
// Global
    eeNodeName = 2,
    eeNodeNamebody = eeNodeName + MQTTSN_SIZEOF_CLIENTID - 2,
// ASLEEP
    eeTASleep,
    eeTASleepbody,
    eeADCaverage,
    eeADCaveragebody,
// BackUp Objects
    eelistOdbu,
    eelistOdbubody = eelistOdbu + OD_MAX_INDEX_LIST * sizeof(subidx_t) - 1,
// LAN NODE
    eeMACAddr,
    eeMACAddrBody = eeMACAddr + 5,
    eeIPAddr,
    eeIPAddrbody = eeIPAddr + sizeof(uint32_t) - 1,
    eeIPMask,
    eeIPMaskbody = eeIPMask + sizeof(uint32_t) - 1,
    eeIPRouter,
    eeIPRouterbody = eeIPRouter + sizeof(uint32_t) - 1,
    eeIPBroker,
    eeIPBrokerbody = eeIPBroker + sizeof(uint32_t) - 1,
// RF NODE
#if (defined RF_ADDR_t)
    eeNodeID,
    eeNodeIDbody = eeNodeID + sizeof(RF_ADDR_t) - 1,
    eeGateID,
    eeGateIDbody = eeGateID + sizeof(RF_ADDR_t) - 1,
    eeGroupID,
    eeGroupIDbody,
    eeChannel,
#endif  //  RF_ADDR_t

#ifdef EXTPLC_USED
    eePLCprogram,
    eePLCprogram_body = eePLCprogram + PLC_SIZEOF_EEPROM - 1,
#endif  //  EXTPLC_USED

    eeNextFreeAddress
} eEEPROMAddr;

void InitOD(void);
e_MQTTSN_RETURNS_t ReadOD(uint16_t Id, uint8_t Flags, uint8_t *pLen, uint8_t *pBuf);
e_MQTTSN_RETURNS_t WriteOD(uint16_t Id, uint8_t Flags, uint8_t Len, uint8_t *pBuf);

uint8_t MakeTopicName(uint8_t RecNR, uint8_t *pBuf);
void RegAckOD(uint16_t index);
e_MQTTSN_RETURNS_t RegisterOD(MQTTSN_MESSAGE_t *pMsg);
indextable_t * getFreeIdxOD(void);

e_MQTTSN_RETURNS_t ReadODpack(uint16_t Id, uint8_t Flags, uint8_t *pLen, uint8_t *pBuf);
e_MQTTSN_RETURNS_t WriteODpack(uint16_t Id, uint8_t Flags, uint8_t Len, uint8_t *pBuf);

void OD_Poll(void);

#ifdef __cplusplus
}
#endif
#endif
