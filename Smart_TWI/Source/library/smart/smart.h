/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org

BSD New License
See LICENSE file for license details.
 */

#ifndef __SMART_H_
#define __SMART_H_

// Variables definition record
typedef struct
{
  uint8_t   lenC;       // Record length  type + offs + sizeof(desc)
  uint8_t   type;       // Variable type and access
                        //  Bit 7 - Read variable
                        //  Bit 6 - Write Variable
                        //  Bits 5-0 - Variable Type (eObjTyp - 0x40)
  uint8_t   offs;       // Variable offset
  uint8_t   desc[13];   // Description
}s_SMART_CONFIG_t;

// Variable type
enum
{
  objBool   = ('z' - 0x40),  // bool
  objInt8   = ('b' - 0x40),  // int8
  objUInt8  = ('B' - 0x40),  // uint8
  objInt16  = ('w' - 0x40),  // int16
  objUInt16 = ('W' - 0x40),  // uint16
  objInt32  = ('d' - 0x40),  // int32
  objUInt32 = ('D' - 0x40),  // uint32
  objInt64  = ('q' - 0x40),  // int64
  objString = ('s' - 0x40),  // UTF8 '0' - terminated string
  objArray  = ('a' - 0x40),  // byte array
}eObjTyp;

// Access
enum
{
  accRead = 0x80,
  accWrite = 0x40
}eAccessMode;

#define REG_MIN_USER        0
#define REG_MAX_USER        0x7F
#define REG_DEVICE_DESCR    0xB0
#define REG_VARIABLE_DESCR  0xC0
#define REG_STATUS          0xF0
#define REG_CONTROL         0xF0
#define REG_UNDEF           0xFF

void    smart_reset_reg(void);
uint8_t smart_read_data(void);
uint8_t smart_write_data(uint8_t data);

#endif  //  __SMART_H_

