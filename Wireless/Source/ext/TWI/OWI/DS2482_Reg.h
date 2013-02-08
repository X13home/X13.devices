/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

#ifndef _DS2482_REG_H
#define _DS2482_REG_H

// Configuration register
#define DS2482_G_APU    0x01    // Active pullup
#define DS2482_G_SPU    0x04    // Strong Pullup
#define DS2482_G_1WS    0x08    // 1-wire Speed

// Status Register
#define DS2482_S_1WB    0x01    // 1-Wire Busy
#define DS2482_S_PPD    0x02    //  Presence-Pulse Detect
#define DS2482_S_SD     0x04    // Short Detected
#define DS2482_S_LL     0x08    // Logic level
#define DS2482_S_RST    0x10    // Device Reset
#define DS2482_S_SBR    0x20    // Single Bit Result
#define DS2482_S_TSB    0x40    // Triplet Second Bit
#define DS2482_S_DIR    0x80    // Branch Direction Taken

// Internal DS2482 TWI Commands
#define DS2482_C_DRST   0xF0    // Device Reset
#define DS2482_C_SRP    0xE1    // Set Read Pointer
#define DS2482_C_WCFG   0xD2    // Write Configuration
#define DS2482_C_1WRS   0xB4    // 1-wire Reset
#define DS2482_C_1WSB   0x87    // 1-wire Single Bit
#define DS2482_C_1WWB   0xA5    // 1-wire Write Byte
#define DS2482_C_1WRB   0x96    // 1-wire Read Byte
#define DS2482_C_1WT    0x78    // 1-wire Triplet
#define DS2482_C_CHSL   0xC3    // Channel Select, only for DS2482-800

// Channel Selection Codes, to be written
#define DS2482_CSW_0    0xF0    // Select Channel 0 - Default
#define DS2482_CSW_1    0xE1    // Select Channel 1
#define DS2482_CSW_2    0xD2    // Select Channel 2
#define DS2482_CSW_3    0xC3    // Select Channel 3
#define DS2482_CSW_4    0xB4    // Select Channel 4
#define DS2482_CSW_5    0xA5    // Select Channel 5
#define DS2482_CSW_6    0x96    // Select Channel 6
#define DS2482_CSW_7    0x87    // Select Channel 7

// Channel Selection Codes, read back
#define DS2482_CSR_0    0xB8
#define DS2482_CSR_1    0xB1
#define DS2482_CSR_2    0xAA
#define DS2482_CSR_3    0xA3
#define DS2482_CSR_4    0x9C
#define DS2482_CSR_5    0x95
#define DS2482_CSR_6    0x8E
#define DS2482_CSR_7    0x87

// Available registers
#define DS2482_P_STATUS 0xF0    // Status register
#define DS2482_P_DATA   0xE1    // Read Data register
#define DS2482_P_CONFIG 0xC3    // Configuration Register
#define DS2482_P_CSEL   0xD2    // Channel Select register ,only for DS2482-800

#define DS2482_START_ADDR   0x18    // I2C Slave Address
#define DS2482_STOP_ADDR    0x1B    // For DS2482-100
//#define DS2482_STOP_ADDR    0x1F    // For Ds2482-800

#endif  //  _DS2482_REG_H