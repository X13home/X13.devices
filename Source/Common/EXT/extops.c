#include "../config.h"

#ifdef EXTOPS_USED

#include "extops.h"
#include "exttwi.h"

#define SI1143_ADDR   0x5A

#define  ENTER_LEFT   0x01
#define  ENTER_RIGHT  0x02
#define  ENTER_BOTTOM 0x04
#define  ENTER_TOP    0x08
#define  ENTER_CENTER 0x0F
#define  LEAVE_LEFT   0x10
#define  LEAVE_RIGHT  0x20
#define  LEAVE_BOTTOM 0x40
#define  LEAVE_TOP    0x80
#define  LEAVE_CENTER 0xF0
#define  OPS_HW_ERROR 0x03

#define INIT_CMD_COUNT 51

static void opsGesture(volatile uint8_t* data);

static const uint8_t initSeq[INIT_CMD_COUNT]={
  0x0F, 0x03, 0x03,         // turn on interrupts
        0x10,               // turn on interrupt on PS3
        0x00,
        0x01,               // interrupt on ps3 measurement
        0x17,               //HW_KEY - The system must write the value 0x17 to this register for proper Si114x operation.
        0x6B,               // The device wakes up every 3 1/3 ms (0x6C x 31.25 µs)
        0x32,               // ALS Measurements made every 10 times the device wakes up.
        0x08,               // PS Measurements made every time the device wakes up
        0xFF, 0xFF,
        0xFF, 0xFF,
        0x77,               // LED current for LEDs 1 (red) & 2 (IR1)
        0x07,               // LED current for LED 3 (IR2)
  0x03, 0x17, 0x77, 0xA1,   // PARAM_CH_LIST - all measurements on
  0x03, 0x17, 0x21, 0xA2,   // PARAM_PSLED12_SELECT - select LEDs on
  0x03, 0x17, 0x04, 0xA3,   // PARAM_PSLED3_SELECT - 3 only
  0x03, 0x17, 0x03, 0xA7,   // PARAM_PS1_ADCMUX - PS1 photodiode select
  0x03, 0x17, 0x03, 0xA8,   // PARAM_PS2_ADCMUX - PS2 photodiode select
  0x03, 0x17, 0x03, 0xA9,   // PARAM_PS3_ADCMUX - PS3 photodiode select
  0x03, 0x17, 0x70, 0xAA,   // PARAM_PS_ADC_COUNTER - is default
  0x03, 0x17, 0x00, 0xAB,   // PARAM_PS_ADC_GAIN -
  0x02, 0x18, 0x0F          // starts an autonomous read loop
};

// Global variable used in HAL
extern volatile TWI_QUEUE_t  * pTwi_exchange;

static uint8_t dirCur=0, dirOld;
static int8_t opsState=0;
static TWI_QUEUE_t * opsPacket;

static uint32_t  _edge[6];
static int16_t _thresold[3]={0x0400, 0x0400, 0x0400};
static uint8_t flag=0, stateOld=0;

e_MQTTSN_RETURNS_t opsReadOD(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf){
  *pLen=1;
  pBuf[0]=dirCur;
  dirCur=0;
  return MQTTSN_RET_ACCEPTED;
}
uint8_t opsPollOD(subidx_t * pSubidx, uint8_t sleep) {
  if(dirCur!=0){
    return 1;
  }
  return 0;
}

void opsInit(){
  indextable_t * pIndex = getFreeIdxOD();
  opsPacket=mqAlloc(sizeof(MQ_t));
  if(pIndex == NULL || opsPacket == NULL)
  {
    return;
  }

  pIndex->cbRead     = &opsReadOD;
  pIndex->cbWrite    = NULL;
  pIndex->cbPoll     = &opsPollOD;
  pIndex->sidx.Place = objUsrExt;     // TWI object
  pIndex->sidx.Type  = objUInt8;      // Variable Type -  Byte Array
  pIndex->sidx.Base  = 43;            // Device address
  dirOld=0;
}

void opsProc()
{
    int8_t i;
    if(pTwi_exchange != NULL) 
    {
        uint8_t access = pTwi_exchange->frame.access;
        if((access & (TWI_ERROR | TWI_SLANACK | TWI_WD)) != 0)      // Error state
        {
            dirCur = OPS_HW_ERROR;
            pTwi_exchange = NULL;
            opsState = -127;
        }
        else if(access & TWI_RDY) 
        {
            if(pTwi_exchange->frame.read != 0) 
            {
                if(opsState>=INIT_CMD_COUNT) 
                {
                    opsGesture(pTwi_exchange->frame.data);
                }
            }
            pTwi_exchange = NULL;
        }
    }

    if(pTwi_exchange == NULL)   // request
    {
        if(opsState < 0)
        {
            opsState++;   // Wait
        }
        else if(opsState < INIT_CMD_COUNT) 
        {
            int8_t len=initSeq[opsState++];
            opsPacket->frame.address=SI1143_ADDR;
            opsPacket->frame.access=TWI_WRITE;
            opsPacket->frame.write=len;
            opsPacket->frame.read=0;
            for(i=0; i<len; i++, opsState++)
            {
                opsPacket->frame.data[i]=initSeq[opsState];
            }
            pTwi_exchange = opsPacket;
        }
        else 
        {
            opsPacket->frame.address=SI1143_ADDR;
            opsPacket->frame.access=TWI_WRITE | TWI_READ;
            opsPacket->frame.write=1;
            opsPacket->frame.read=10;
            opsPacket->frame.data[0]=0x22;
            pTwi_exchange = opsPacket;
        }
    }
}

void opsDiag(uint8_t r, int8_t dx, int8_t dy, int8_t du){
  MQ_t * pMessage = mqAlloc(sizeof(MQ_t));
  if(pMessage == NULL)
  return;

  pMessage->mq.publish.Data[0] = r;
  pMessage->mq.publish.Data[1] = dx;
  pMessage->mq.publish.Data[2] = dy;
  pMessage->mq.publish.Data[3] = du;

  pMessage->Length = 4;

  mqttsn_trace_msg(lvlINFO, pMessage);

}


static void opsGesture(volatile uint8_t *data){
  uint32_t startTick, duration;  // *1ms
  int32_t dx, dy;
  uint8_t i, mask;
  int16_t _valueCur[3];
  _valueCur[0]=((pTwi_exchange->frame.data[7]<<8) | pTwi_exchange->frame.data[6]);
  _valueCur[1]=((pTwi_exchange->frame.data[5]<<8) | pTwi_exchange->frame.data[4]);
  _valueCur[2]=((pTwi_exchange->frame.data[9]<<8) | pTwi_exchange->frame.data[8]);
  
  //dirCur=0;

  for(i=0; i<3; i++) {
    mask=1<<i;
    if(_valueCur[i]>_thresold[i]){
      if((stateOld&mask)==0 && _edge[i]==0) {  // Rising Edge Detection
        _edge[i]=hal_get_ms();
        flag|=mask;
      }
      stateOld|=mask;
    }else {
      if((stateOld&mask)!=0 && _edge[i+3]==0) {  // Falling Edge Detection
        _edge[i+3]=hal_get_ms();
        flag|=mask<<4;
      }
      stateOld&=~mask;
    }
  }

  if((flag&0x07)==0x07) {  // Check if rising edge group is ready to be processed:
    startTick=_edge[0]<_edge[1]?_edge[0]:_edge[1];
    if(startTick>_edge[2]){
      startTick=_edge[2];
    }
    duration=_edge[0]>_edge[1]?_edge[0]:_edge[1];
    if(duration<_edge[2]){
      duration=_edge[2];
    }
    duration=(duration-startTick-1)/2;
    if(duration<2){
      duration=2;
    }
    // Process rising edge group (this code implements the conditional event/gesture table)
    dx=_edge[1]-_edge[0];
    dy=_edge[2]-_edge[0];
    if(dx>duration) {
      dirCur|=ENTER_LEFT;
    } else if(dx<-duration) {
      dirCur|=ENTER_RIGHT;
    }

    if(dy>duration) {
      dirCur|=ENTER_BOTTOM;
    } else if(dy<-duration) {
      dirCur|=ENTER_TOP;
    }

    if((dirCur & ENTER_CENTER)==0) {
      dirCur=ENTER_CENTER;
    }

    opsDiag(dirCur, (int8_t)(dx>>4), (int8_t)(dy>>4), (int8_t)(duration>>4));
    _edge[0]=0;
    _edge[1]=0;
    _edge[2]=0;
    flag&=~0x07;
  }

  if((flag&0x70)==0x70) {  // Check if rising edge group is ready to be processed:
    startTick=_edge[3]<_edge[4]?_edge[3]:_edge[4];
    if(startTick>_edge[5]){
      startTick=_edge[5];
    }
    duration=_edge[3]>_edge[4]?_edge[3]:_edge[4];
    if(duration<_edge[5]){
      duration=_edge[5];
    }
    duration=(duration-startTick-1)/2;
    if(duration<2){
      duration=2;
    }
    // Process rising edge group (this code implements the conditional event/gesture table)
    dx=_edge[4]-_edge[3];
    dy=_edge[5]-_edge[3];
    if(dx>duration) {
      dirCur|=LEAVE_RIGHT;
    } else if(dx<-duration) {
      dirCur|=LEAVE_LEFT;
    }

    if(dy>duration) {
      dirCur|=LEAVE_TOP;
    } else if(dy<-duration) {
      dirCur|=LEAVE_BOTTOM;
    }

    if((dirCur & LEAVE_CENTER)==0) {
      dirCur=LEAVE_CENTER;
    }

    opsDiag(dirCur, (int8_t)(dx>>4), (int8_t)(dy>>4), (int8_t)(duration>>4));
    _edge[3]=0;
    _edge[4]=0;
    _edge[5]=0;
    flag&=~0x70;
  }
  startTick=hal_get_ms()-2048;  // mS
  for(i=0; i<3; i++) {
    mask=1<<i;
    if((hal_get_ms() & 0x3FF)==5){
      _thresold[i]+=(_valueCur[i]+_valueCur[i]/4-_thresold[i])/16; // C + 1/4
    }    
    if((flag&mask)!=0 && _edge[i]<startTick) {
      _edge[i]=0;
      flag&=~mask;
    }
    if((flag&(mask<<4))!=0 && _edge[i+3]<startTick) {
      _edge[i+3]=0;
      flag&=~(mask<<4);
    }
  }
}

#endif    //  EXTTWI_USED
