/*
Copyright (c) 2011-2012 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

#ifndef _RFM23REG_H_
#define _RFM23REG_H_

// RFM23 Registers definition
#define RFM23_DT                                          0x00    // Device Type Code, RO
#define RFM23_VC                                          0x01    // Version Code, RO
#define RFM23_STATUS                                      0x02    // Device Status, RO
#define RFM23_IS1                                         0x03    // Interrupt/Status 1, RO
#define RFM23_IS2                                         0x04    // Interrupt/Status 2, RO
#define RFM23_IE1                                         0x05    // Interrupt Enable 1
#define RFM23_IE2                                         0x06    // Interrupt Enable 2
#define RFM23_OFC1                                        0x07    // Operating Mode and Function Control 1
#define RFM23_OFC2                                        0x08    // Operating Mode and Function Control 2
#define RFM23_XTALLC                                      0x09    // Crystal Oscillator Load Capacitance
#define RFM23_MCO                                         0x0A    // Microcontroller Output Clock
#define RFM23_GPIO0_CFG                                   0x0B    // GPIO0 Configuration
#define RFM23_GPIO1_CFG                                   0x0C    // GPIO1 Configuration
#define RFM23_GPIO2_CFG                                   0x0D    // GPIO2 Configuration
#define RFM23_IO_CFG                                      0x0E    //  I/O Port Configuration
#define RFM23_ADC_CCFG                                    0x0F    // ADC Configuration
#define RFM23_ADCOFFS                                     0x10    // ADC Sensor Amplifier Offset
#define RFM23_ADC                                         0x11    // ADC Value
#define RFM23_TS_CTRL                                     0x12    // Temperature Sensor Calibration
#define RFM23_TVOFFS                                      0x13    // Temperature Value Offset
#define RFM23_WTR                                         0x14    // Wake-Up Timer Period 1
#define RFM23_WTMH                                        0x15    // Wake-Up Timer Period 2
#define RFM23_WTML                                        0x16    // Wake-Up Timer Period 3
#define RFM23_WTVH                                        0x17    // Wake-Up Timer Value 1, RO
#define RFM23_WTWL                                        0x18    // Wake-Up Timer Value 2, RO
#define RFM23_LDC                                         0x19    // Low-Duty Cycle Mode Duration
#define RFM23_LBDT                                        0x1A    // Low Battery Detector Threshold
#define RFM23_VBAT                                        0x1B    // Battery Voltage Level, RO
#define RFM23_IFFBW                                       0x1C    // IF Filter Bandwidth
#define RFM23_AFC_LGO                                     0x1D    // AFC Loop Gearshift Override
#define RFM23_AFC_TC                                      0x1E    // AFC Timing Control
#define RFM23_CRGO                                        0x1F    // Clock Recovery Gearshift Override
#define RFM23_RXOSR                                       0x20    // Clock Recovery Oversampling Rate
#define RFM23_CRO2                                        0x21    // Clock Recovery Offset 2
#define RFM23_NCOFFH                                      0x22    // Clock Recovery Offset 1
#define RFM23_NVOFFL                                      0x23    // Clock Recovery Offset 0
#define RFM23_CRTLG1                                      0x24    // Clock Recovery Timing Loop Gain 1
#define RFM23_CRGAINL                                     0x25    // Clock Recovery Timing Loop Gain 0
#define RFM23_RSSI                                        0x26    // Received Signal Strength Indicator, RO
#define RFM23_RSSI_TH                                     0x27    // RSSI Threshold for Clear Channel Indicator
#define RFM23_ADRSSI1                                     0x28    // Antenna Diversity 1
#define RFM23_ADRSSI2                                     0x29    // Antenna Diversity 2
#define RFM23_AFCLIM                                      0x2A    // AFC Limiter
#define RFM23_AFC_CORRH                                   0x2B    // AFC Correction (MSBs)
#define RFM23_OOK_CV1                                     0x2C    // OOK Counter Value 1
#define RFM23_OOK_CV2                                     0x2D    // OOK Counter Value 2
#define RFM23_SLPH                                        0x2E    // Slicer Peak Holder
#define RFM23_DTAC                                        0x30    // Data Access Control
#define RFM23_EZMAC_STATUS                                0x31    // EZMAC® Status
#define RFM23_HDC1                                        0x32    // Header Control 1
#define RFM23_HDC2                                        0x33    // Header Control 2
#define RFM23_PREALEN                                     0x34    // Preamble Length
#define RFM23_PREA_DC                                     0x35    // Preamble Detection Control 1
#define RFM23_SYNC_HH                                     0x36    // Synchronization Word [31:24]
#define RFM23_SYNC_HL                                     0x37    // Synchronization Word [23:16]
#define RFM23_SYNC_LH                                     0x38    // Synchronization Word [15:8]
#define RFM23_SYNC_LL                                     0x39    // Synchronization Word [7:0]
#define RFM23_TXHD_HH                                     0x3A    // Transmit Header [31:24]
#define RFM23_TXHD_HL                                     0x3B    // Transmit Header [23:16]
#define RFM23_TXHD_LH                                     0x3C    // Transmit Header [15:8]
#define RFM23_TXHD_LL                                     0x3D    // Transmit Header [7:0]
#define RFM23_PKLEN                                       0x3E    //  Packet Length
#define RFM23_CHHD_HH                                     0x3F    // Check Header [31:24]
#define RFM23_CHHD_HL                                     0x40    // Check Header [23:16]
#define RFM23_CHHD_LH                                     0x41    // Check Header [15:8]
#define RFM23_CHHD_LL                                     0x42    // Check Header [7:0]
#define RFM23_HDEN_HH                                     0x43    // Header Enable [31:24]
#define RFM23_HDEN_HL                                     0x44    // Header Enable [23:16]
#define RFM23_HDEN_LH                                     0x45    // Header Enable [15:8]
#define RFM23_HDEN_LL                                     0x46    // Header Enable [7:0]
#define RFM23_RXHD_HH                                     0x47    // Received Header [31:24] , RO
#define RFM23_RXHD_HL                                     0x48    // Received Header [23:16] , RO
#define RFM23_RXHD_LH                                     0x49    // Received Header [15:8] , RO
#define RFM23_RXHD_LL                                     0x4A    // Received Header [7:0] , RO
#define RFM23_RXPLEN                                      0x4B    // Received Packet Length, RO
#define RFM23_ADC8                                        0x4F    // ADC8 Control
#define RFM23_ATB                                         0x50    // ANALOG_TEST_BUS
#define RFM23_DTB                                         0x51    // DIGITAL_TEST_BUS
#define RFM23_TX_RAMP_CONTROL                             0x52
#define RFM23_PLL_TUNE_TIME                               0x53
#define RFM23_INVALID_PREAMBLE_THRESHOLD_AND_PA_MISC      0x54
#define RFM23_CALIBRATION_CONTROL                         0x55
#define RFM23_MODEM_TEST                                  0x56
#define RFM23_CHARGEPUMP_TEST                             0x57
#define RFM23_CHARGEPUMP_CURRENT_TRIMMING_OVERRIDE        0x58
#define RFM23_DIVIDER_CURRENT_TRIMMING                    0x59
#define RFM23_VCO_CURRENT_TRIMMING                        0x5A
#define RFM23_VCO_CALIBRATION_OVERRIDE                    0x5B
#define RFM23_SYNTHESIZER_TEST                            0x5C
#define RFM23_BLOCK_ENABLE_OVERRIDE_1                     0x5D
#define RFM23_BLOCK_ENABLE_OVERRIDE_2                     0x5E
#define RFM23_BLOCK_ENABLE_OVERRIDE_3                     0x5F
#define RFM23_CHANNEL_FILTER_COEFFICIENT_ADDRESS          0x60    // Channel Filter Coefficient Address
#define RFM23_CHANNEL_FILTER_COEFFICIENT_VALUE            0x61
#define RFM23_CRYSTAL_OSCILLATOR_CONTROL_TEST             0x62    // Crystal Oscillator/Power-on-Reset Control
#define RFM23_RC_OSCILLATOR_COARSE_CALIBRATION_OVERRIDE   0x63
#define RFM23_RC_OSCILLATOR_FINE_CALIBRATION_OVERRIDE     0x64
#define RFM23_LDO_CONTROL_OVERRIDE                        0x65
#define RFM23_LDO_LEVEL_SETTING                           0x66
#define RFM23_DELTASIGMA_ADC_TUNING_1                     0x67
#define RFM23_DELTASIGMA_ADC_TUNING_2                     0x68
#define RFM23_AGC_OVERRIDE_1                              0x69    // AGC Override 1
#define RFM23_AGC_OVERRIDE_2                              0x6A
#define RFM23_GFSK_FIR_FILTER_COEFFICIENT_ADDRESS         0x6B
#define RFM23_GFSK_FIR_FILTER_COEFFICIENT_VALUE           0x6C
#define RFM23_TX_POWER                                    0x6D    //  TX Power
#define RFM23_TXDR_H                                      0x6E    // TX Data Rate  [15:8]
#define RFM23_TXDR_L                                      0x6F    // TX Data Rate [7:0]
#define RFM23_MODULATION_MODE_CONTROL_1                   0x70    // Modulation Mode Control 1
#define RFM23_MODULATION_MODE_CONTROL_2                   0x71    // Modulation Mode Control 2
#define RFM23_FD                                          0x72    // Frequency Deviation
#define RFM23_FO_L                                        0x73    // Frequency Offset [7:0]
#define RFM23_FO_H                                        0x74    // Frequency Offset [9:8]
#define RFM23_FREQUENCY_BAND_SELECT                       0x75    // Frequency Band Select
#define RFM23_FC_H                                        0x76    //  Nominal Carrier Frequency MSB
#define RFM23_FC_L                                        0x77    //  Nominal Carrier Frequency LSB
#define RFM23_MISCELLANEOUS_SETTINGS                      0x78
#define RFM23_FHCH                                        0x79    // Frequency Hopping Channel Select
#define RFM23_FHS                                         0x7A    // Frequency Hopping Step Size
#define RFM23_TURN_AROUND_AND_15_4_LENGTH_COMPLIANCE      0x7B
#define RFM23_TXAFTHR                                     0x7C    // TX FIFO Control 1
#define RFM23_TXFAETHR                                    0x7D    // TX FIFO Control 2
#define RFM23_RXAFTHR                                     0x7E    //  RX FIFO Control
#define RFM23_FIFOD                                       0x7F    // FIFO Access

//RFM23_DEVICE_TYPE @ 0x00
#define RFM23_DT_MASK             0x1F
// RFM23_DEVICE_VERSION @ 0x01
#define RFM23_VC_MASK             0x1F
// RFM23_DEVICE_STATUS @ 0x02
#define RFM23_CPS_MASK            0x03  //Chip Power State.
                                        //00: Idle State
                                        //01: RX State
                                        //10: TX State
#define RFM23_LOCKDET             0x04
#define RFM23_FREQERR             0x08  // Frequency Error Status.
                                        // The programmed frequency is outside of the operating range. The actual frequency is 
                                        // saturated to the max/min value.
#define RFM23_HEADERR             0x10  //  Header Error Status.  Indicates if the received packet has a header check error.
#define RFM23_RXFFEM              0x20  // RX FIFO Empty Status.
#define RFM23_FFUNFL              0x40  // RX/TX FIFO Underflow Status.
#define RFM23_FFOVFL              0x80  //  RX/TX FIFO Overflow Status.

// RFM23_INTERRUPT_STATUS_1 @ 0x03
#define RFM23_ICRCERROR           0x01  // CRC Error. When set to 1 the cyclic redundancy check is failed.
#define RFM23_IPKVALID            0x02  // Valid Packet Received.When set to 1 a valid packet has been received.
#define RFM23_IPKSENT             0x04  // Packet Sent Interrupt. When set to1 a valid packet has been transmitted.
#define RFM23_IEXT                0x08  // extExternal Interrupt.
                                        // When set to 1 an interrupt occurred on one of the GPIO’s if it is programmed so. 
                                        // The status can be checked in register 0Eh. See GPIOx Configuration section for the details.
#define RFM23_IRXFFAFULL          0x10  // RX FIFO Almost Full.When set to 1 the RX FIFO has met its almost full threshold and 
                                        // needs to be read by the microcontroller.
#define RFM23_ITXFFAEM            0x20  // TX FIFO Almost Empty. When set to 1 the TX FIFO is almost empty and needs to be filled.
#define RFM23_ITXFFAFULL          0x40  // TX FIFO Almost Full. When set to 1 the TX FIFO has met its almost full threshold and needs to be transmitted.
#define RFM23_IFFERR              0x80  // FIFO Underflow/Overflow Error. When set to 1 the TX or RX FIFO has overflowed or underflowed.

// RFM23_INTERRUPT_STATUS_2 @ 0x04
#define RFM23_IPOR                0x01  // Power-on-Reset (POR). When the chip detects a Power on Reset above the desired setting this bit will be set to 1.
#define RFM23_ICHIPRDY            0x02  // Chip Ready (XTAL). When a chip ready event has been detected this bit will be set to 1.
#define RFM23_ILBD                0x04  // Low Battery Detect. When a low battery event has been detected this bit will be set to 1. 
                                        // This interrupt event is saved even if it is not enabled by the mask register bit and causes an interrupt after it is enabled.
#define RFM23_IWUT                0x08  // Wake-Up-Timer. On the expiration of programmed wake-up timer this bit will be set to 1.
#define RFM23_IRSSI               0x10  // RSSI. When RSSI level exceeds the programmed threshold this bit will be set to 1.
#define RFM23_IPREAINVAL          0x20  // Invalid Preamble Detected. When the preamble is not found within a period of time 
                                        // set by the invalid preamble detection threshold in Register 60h, this bit will be set to 1.
#define RFM23_IPREAVAL            0x40  // Valid Preamble Detected. When a preamble is detected this bit will be set to 1.
#define RFM23_ISWDET              0x80  // Sync Word Detected. When a sync word is detected this bit will be set to 1.

// RFM23_INTERRUPT_ENABLE_1 @ 0x05
#define RFM23_ENCRCERROR          0x01  // Enable CRC Error. When set to 1 the CRC Error interrupt will be enabled.
#define RFM23_ENPKVALID           0x02  // Enable Valid Packet Received. When ipkvalid = 1 the Valid Packet Received Interrupt will be enabled.
#define RFM23_ENPKSENT            0x04  // Enable Packet Sent. When ipksent =1 the Packet Sense Interrupt will be enabled.
#define RFM23_ENEXT               0x08  // Enable External Interrupt. When set to 1 the External Interrupt will be enabled.
#define RFM23_ENRXFFAFULL         0x10  // Enable RX FIFO Almost Full. When set to 1 the RX FIFO Almost Full interrupt will be enabled.
#define RFM23_ENTXFFAEM           0x20  // Enable TX FIFO Almost Empty. When set to 1 the TX FIFO Almost Empty interrupt will be enabled.
#define RFM23_ENTXFFAFULL         0x40  // Enable TX FIFO Almost Full. When set to 1 the TX FIFO Almost Full interrupt will be enabled.
#define RFM23_ENFFERR             0x80  // Enable FIFO Underflow/Overflow. When set to 1 the FIFO Underflow/Overflow interrupt will be enabled.

// RFM23_INTERRUPT_ENABLE_2 @ 0x06
#define RFM23_ENPOR               0x01  // Enable POR. When set to 1 the POR interrupt will be enabled.
#define RFM23_ENCHIPRDY           0x02  // Enable Chip Ready (XTAL). When set to 1 the Chip Ready interrupt will be enabled.
#define RFM23_ENLBDI              0x04  // Enable Low Battery Detect. When set to 1 the Low Battery Detect interrupt will be enabled.
#define RFM23_ENWUT               0x08  // Enable Wake-Up Timer. When set to 1 the Wake-Up Timer interrupt will be enabled.
#define RFM23_ENRSSI              0x10  // Enable RSSI. When set to 1 the RSSI Interrupt will be enabled.
#define RFM23_ENPREAINVAL         0x20  // Enable Invalid Preamble Detected. When set to 1 the Invalid Preamble Detected Interrupt will be enabled.
#define RFM23_ENPREAVAL           0x40  // Enable Valid Preamble Detected. When set to 1 the Valid Preamble Detected Interrupt will be enabled.
#define RFM23_ENSWDET             0x80  // Enable Sync Word Detected. When set to 1 the Syn Word Detected Interrupt will be enabled.

// RFM23_OPERATING_FUNCTION_CONTROL_1 @ 0x07
#define RFM23_XTON                0x01  // READY Mode (Xtal is ON).
#define RFM23_PLLON               0x02  // TUNE Mode (PLL is ON). When pllon = 1 the PLL will remain enabled in Idle State. 
                                        // This allows for faster turnaround time at the cost of increased current consumption in Idle State. 
#define RFM23_RXON                0x04  // RX on in Manual Receiver Mode.  Automatically cleared if Multiple Packets config. is disabled and a valid packet received.
#define RFM23_TXON                0x08  // TX on in Manual Transmit Mode.  Automatically cleared in FIFO mode once the packet is sent.
#define RFM23_X32KSEL             0x10  // 32,768 kHz Crystal Oscillator Select. 0: RC oscillator 1: 32 kHz crystal
#define RFM23_ENWT                0x20  // Enable Wake-Up-Timer. Enabled when enwt = 1. If the Wake-up-Timer function is enabled 
                                        //  it will operate in any mode and notify the microcontroller through the GPIO interrupt when the timer expires.
#define RFM23_ENLBD               0x40  // Enable Low Battery Detect. When this bit is set to 1 the Low Battery Detector circuit and threshold comparison will be enabled. 
#define RFM23_SWRES               0x80  // Software Register Reset Bit. This bit may be used to reset all registers simultaneously to a DEFAULT state, without the 
                                        // need for sequentially writing to each individual register. The RESET is accomplished by 
                                        // setting swres = 1. This bit will be automatically cleared. 

// RFM23_OPERATING_FUNCTION_CONTROL_2 @ 0x08
#define RFM23_FFCLRTX             0x01  // TX FIFO Reset/Clear. This has to be a two writes operation: Setting ffclrtx =1 followed by ffclrtx = 0 will clear the contents of the TX FIFO.
#define RFM23_FFCLRRX             0x02  // RX FIFO Reset/Clear. This has to be a two writes operation: Setting ffclrrx =1 followed by ffclrrx = 0 will clear the contents of the RX FIFO. 
#define RFM23_ENLDM               0x04  // Enable Low Duty Cycle Mode. If this bit is set to 1 then the chip turns on the RX regularly. The frequency should be set 
                                        //  in the Wake-Up Timer Period register, while the minimum ON time should be set in the 
                                        //  Low-Duty Cycle Mode Duration register. The FIFO mode should be enabled also.
#define RFM23_AUTOTX              0x08  // Automatic Transmission.  When autotx = 1 the transceiver will enter automatically TX State when the FIFO is 
                                        //  almost full. When the FIFO is empty it will automatically return to the Idle State. 
#define RFM23_RXMPK               0x10  // RX Multi Packet. When the chip is selected to use FIFO Mode (dtmod[1:0]) and 
                                        //  RX Packet Handling (enpacrx) then it will fill up the FIFO with multiple valid packets 
                                        //  if this bit is set, otherwise the transceiver will automatically leave the RX State after 
                                        //  the first valid packet has been received.
#define RFM23_ANTDIV_MASK         0xE0  // Enable Antenna Diversity. 
                                        // The GPIO must be configured for Antenna Diversity for the algorithm to work properly.

// RFM23_CRYSTAL_OSCILLATOR_LOAD_CAPACITANCE @ 0x09
#define RFM23_XLC_MASK            0x7F  // Tuning Capacitance for the 30 MHz XTAL.
#define RFM23_XTALSHFT            0x80  //  Additional capacitance to coarse shift the frequency if xlc[6:0] is not sufficient. Not binary with xlc[6:0].

// RFM23_MICROCONTROLLER_OUTPUT_CLOCK @ 0x0A
#define RFM23_MCLK_MASK           0x07  // Microcontroller Clock.
#define RFM23_ENLFC               0x08  // Enable Low Frequency Clock. 
                                        // When enlfc = 1 and the chip is in Sleep mode then the 32.768 kHz clock will be provided 
                                        //  to the microcontroller no matter what the selection of mclk[2:0] is. For example if 
                                        //  mclk[2:0] = 000, 30 MHz will be available through the GPIO to output to the microcon-
                                        // troller in all Idle, TX, or RX states. When the chip is commanded to Sleep mode the 
                                        // 30 MHz clock will become 32.768 kHz.
#define RFM23_CLKT_MASK           0x30  // Clock Tail.

// RFM23_GPIO0_CONFIGURATION @ 0x0B
#define RFM23_GPIO0_MASK          0x1F  // GPIO0 Pin Function Select.
#define RFM23_PUP0                0x20  // Pullup Resistor Enable on GPIO0. When set to 1 a 200 kR resistor is connected internally 
                                        //  between VDD and the pin if the GPIO is configured as a digital input.
#define RFM23_GPIO0DRV_MASK       0xC0  // GPIO Driving Capability Setting.

// RFM23_GPIO1_CONFIGURATION @ 0x0C
#define RFM23_GPIO1_MASK          0x1F  // GPIO1 Pin Function Select.
#define RFM23_PUP1                0x20  // Pullup Resistor Enable on GPIO1.
#define RFM23_GPIO1DRV_MASK       0xC0  // GPIO Driving Capability Setting.

// RFM23_GPIO2_CONFIGURATION @ 0x0D
#define RFM23_GPIO2_MASK          0x1F  // GPIO2 Pin Function Select.
#define RFM23_PUP2                0x20  // Pullup Resistor Enable on GPIO2.
#define RFM23_GPIO2DRV_MASK       0xC0  // GPIO Driving Capability Setting.

// RFM23_IO_PORT_CONFIGURATION @ 0x0E
#define RFM23_DIO0                0x01  //  Direct I/O for GPIO0.
#define RFM23_DIO1                0x02  //  Direct I/O for GPIO1.
#define RFM23_DIO2                0x04  //  Direct I/O for GPIO2.
#define RFM23_ITSDO               0x08  // Interrupt Request Output on the SDO Pin. nIRQ output is present on the SDO pin if this bit is set and the nSEL input is inactive (high).
#define RFM23_EXTITST_0           0x10  //  External Interrupt Status. If the GPIO0 is programmed to be an external interrupt source then the status can be read here.
#define RFM23_EXTITST_1           0x20  //  External Interrupt Status. If the GPIO1 is programmed to be an external interrupt source then the status can be read here.
#define RFM23_EXTITST_2           0x40  //  External Interrupt Status. If the GPIO2 is programmed to be an external interrupt source then the status can be read here.

// RFM23_ADC_CONFIGURATION @ 0x0F
#define RFM23_ADCGAIN_MASK        0x03  // ADC Sensor Amplifier Gain Selection. 
#define RFM23_ADCREF_MASK         0x0C  // ADC Reference Voltage Selection.
                                        //The reference voltage of the internal 8-bit ADC can be selected as follows:
                                        //    0X: bandgap voltage (1.2 V)
                                        //    10: VDD/3
                                        //    11: VDD/2
#define RFM23_ADCSEL_MASK         0x70  //  ADC Input Source Selection. 
#define RFM23_ADCSTART            0x80  // ADC Measurement Start Bit.
                                        //  Set this bit=1 starts the ADC measurement process. This bit self-clears during the mea-
                                        //  surement cycle and returns high when the measurement is complete. The conversion 
                                        //  process is fast; reading this bit may always appear to return a 1.
#define RFM23_ADCDONE             0x80  //

// RFM23_ADC_SENSOR_AMPLIFIER_OFFSET @ 0x10
#define RFM23_ADCOFFS_MASK       0x0F

// RFM23_ADC_VALUE @ 0x11
// no bits or mask

// RFM23_TEMPERATURE_SENSOR_CONTROL @ 0x12
#define RFM23_TSTRIM_MASK         0x0F  // Temperature Sensor Trim Value.
#define RFM23_ENTSTRIM            0x10  // Temperature Sensor Trim Enable.
#define RFM23_ENTSOFFS            0x20  // Temperature Sensor Offset to Convert from K to ºC. Default is 1. Test mode only, to use set tsrange and entsoffs to 0.
#define RFM23_TSRANGE_MASK        0xC0  //  Temperature Sensor Range Selection. 

// RFM23_TEMPERATURE_VALUE_OFFSET @ 0x13
// no bits or mask

// RFM23_WAKE_UP_TIMER_PERIOD_1 @ 0x14
#define RFM23_WTD_MASK            0x03
#define RFM23_WTR_MASK            0x3C

// RFM23_WAKE_UP_TIMER_PERIOD_2 @ 0x15
// no bits or mask

// RFM23_WAKE_UP_TIMER_PERIOD_3 @ 0x16
#define RFM23_WTM_MASK            0x80

// RFM23_WAKE_UP_TIMER_VALUE_1 @ 0x17
// no bits or mask

// RFM23_WAKE_UP_TIMER_VALUE_2 @ 0x18
// no bits or mask

// RFM23_LOW_DUTY_CYCLE_MODE_DURATION @ 0x19
// no bits or mask

// RFM23_LOW_BATTERY_DETECTOR_THRESHOLD @ 0x1A
#define RFM23_LBDT_MASK           0x1F

// RFM23_BATTERY_VOLTAGE_LEVEL @ 0x1B
#define RFM23_VBAT_MASK           0x1F  // Vbat_meas=1.7V + vbat[4:0] x 50 mV

// RFM23_IF_FILTER_BANDWIDTH @ 0x1C
#define RFM23_FILSET_MASK         0x0F  // IF Filter Coefficient Sets.
#define RFM23_NDEC_MASK           0x70  // IF Filter Decimation Rates.
#define RFM23_DWN3_BYPASS         0x80  // Bypass Decimator by 3 (if set).

// RFM23_AFC_LOOP_GEARSHIFT_OVERRIDE @ 0x1D
#define RFM23_PH0SIZE             0x01  // If low, we will reset the Preamble detector if there are 5 consecutive zero phases. If high, the reset will happen after 3 consecutive zero phases.
#define RFM23_MATAP               0x02  // Number of taps for moving average filter during Antenna Diversity RSSI evaluation. 
                                        // Allows for reduced noise variation on measured RSSI value but with slower update rate. 
                                        // If high (1), filter tap length = 8*Tb. If low (0=default), filter tap length = 8*Tb prior to 
                                        // first PREAMBLE_VALID, and 4*Tb thereafter
#define RFM23_1P5BYPASS           0x04  // If high (1), select 0dB bias for the second phase antenna selection, if low (0), select 1.5 dB. The default is (1), selecting 0 dB.
#define RFM23_AFCGEARH_MASK       0x38  // AFC High Gear Setting. Feedback loop gain during AFC setting process is proportional to 2^(–afcgearh[2:0]).
#define RFM23_ENAFC               0x40  // AFC Enable.
#define RFM23_AFCBD               0x80  // AFC wideband enable.

// RFM23_AFC_TIMING_CONTROL @ 0x1E
#define RFM23_ANWAIT_MASK         0x07  // Antenna switching wait time.
#define RFM23_SHWAIT_MASK         0x38  // short wait periods after AFC correction used before preamble is detected.
#define RFM23_SWANT_TIMER_MASK    0xC0  // additional number of bit periods to wait for RSSI value to stabilize during Antenna Diversity 2nd phase antenna evaluation.

// RFM23_CLOCK_RECOVERY_GEARSHIFT_OVERRIDE @ 0x1F
#define RFM23_CRSLOW_MASK        0x07   // Clock Recovery Slow Gearshift Value.
#define RFM23_CRFAST_MASK        0x38   // Clock Recovery Fast Gearshift Value.
//#define RFM23_RXREADY            0x40

// RFM23_CLOCK_RECOVERY_OVERSAMPLING_RATIO @ 0x20
// no bits or mask

// RFM23_CLOCK_RECOVERY_OFFSET_2 @ 0x21
#define RFM23_NCOFF_MASK          0x0F  //  NCO Offset.
//#define RFM23_STALLCTRL           0x10
#define RFM23_SKIP2PHTH           0x10  // Skip 2nd Phase Ant Div Threshold.
#define RFM23_RXOSR_MASK          0xE0  // Oversampling Rate. Upper bits.

// RFM23_CLOCK_RECOVERY_OFFSET_1 @ 0x22
// no bits or mask

// RFM23_CLOCK_RECOVERY_OFFSET_0 @ 0x23
// no bits or mask

// RFM23_CLOCK_RECOVERY_TIMING_LOOP_GAIN_1 @ 0x24
#define RFM23_CRGAIN_MASK         0x07  // Clock Recovery Timing Loop Gain.
#define RFM23_CGAINX2             0x08  // Multiplying the CR Gain by 2.
#define RFM23_RXNCOCOMP           0x10  // Receive Compensation Enable for High Data Rate Offset.

// RFM23_CLOCK_RECOVERY_TIMING_LOOP_GAIN_0 @ 0x25
//  Clock Recovery Timing Loop Gain.
// no bits or mask

// RFM23_RECEIVED_SIGNAL_STRENGTH_INDICATOR @ 0x26
// Received Signal Strength Indicator Value.
// no bits or mask

// RFM23_RSSI_THRESHOLD @ 0x27
// RSSI Threshold. 
// Interrupt is set if the RSSI value is above this threshold.
// no bits or mask

// RFM23_ANTENNA_DIVERSITY_REGISTER_1 @ 0x28
// Measured RSSI Value on Antenna 2
// no bits or mask

// RFM23_ANTENNA_DIVERSITY_REGISTER_2 @ 0x29
// Measured RSSI Value on Antenna 2
// no bits or mask

// RFM23_DATA_ACCESS_CONTROL @ 0x30
#define RFM23_CRC_MASK            0x03
#define RFM23_CRC_16              0x01
#define RFM23_ENCRC               0x04
#define RFM23_ENPACTX             0x08
#define RFM23_CRCDONLY            0x20
#define RFM23_LSBFRST             0x40
#define RFM23_ENPACRX             0x80

// RFM23_EZ_MAC_STATUS @ 0x31
#define RFM23_PKSENT              0x01
#define RFM23_PKTX                0x02
#define RFM23_CRCERROR            0x04
#define RFM23_PKVALID             0x08
#define RFM23_PKRX                0x10
#define RFM23_PKSRCH              0x20
#define RFM23_RXCRC1              0x40

// RFM23_HEADER_CONTROL_1 @ 0x32
#define RFM23_HDCH_MASK           0x0F
#define RFM23_BCEN_MASK           0xF0
#define RFM23_BCEN                0xF0
#define RFM23_DISABLE_HFILTERS    0x00

// RFM23_HEADER_CONTROL_2 @ 0x33
#define RFM23_PREALEN_MASK        0x01
#define RFM23_SYNCLEN_MASK        0x06
#define RFM23_FIXPKLEN            0x08
#define RFM23_HDLEN_MASK          0x70
#define RFM23_SYNCLEN_2BYTE       0x02
#define RFM23_SYNCLEN_3BYTE       0x04
#define RFM23_HDLEN_2BYTE         0x20
#define RFM23_HDLEN_3BYTE         0x30
#define RFM23_HDLEN_4BYTE         0x40

// RFM23_PREAMBLE_LENGTH @ 0x34
// no bits or mask

// RFM23_SYNC_WORD_3 @ 0x36
// no bits or mask

// RFM23_SYNC_WORD_2 @ 0x37
// no bits or mask

// RFM23_SYNC_WORD_1 @ 0x38
// no bits or mask

// RFM23_SYNC_WORD_0 @ 0x39
// no bits or mask

// RFM23_TRANSMIT_HEADER_3 @ 0x3A
// no bits or mask

// RFM23_TRANSMIT_HEADER_2 @ 0x3B
// no bits or mask

// RFM23_TRANSMIT_HEADER_1 @ 0x3C
// no bits or mask

// RFM23_TRANSMIT_HEADER_0 @ 0x3D
// no bits or mask

// RFM23_TRANSMIT_PACKET_LENGTH @ 0x3E
// no bits or mask

// RFM23_CHECK_HEADER_3 @ 0x3F
// no bits or mask

// RFM23_CHECK_HEADER_2 @ 0x40
// no bits or mask

// RFM23_CHECK_HEADER_1 @ 0x41
// no bits or mask

// RFM23_CHECK_HEADER_0 @ 0x42
// no bits or mask

// RFM23_HEADER_ENABLE_3 @ 0x43
// no bits or mask

// RFM23_HEADER_ENABLE_2 @ 0x44
// no bits or mask

// RFM23_HEADER_ENABLE_1 @ 0x45
// no bits or mask

// RFM23_HEADER_ENABLE_0 @ 0x46
// no bits or mask

// RFM23_RECEIVED_HEADER_3 @ 0x47
// no bits or mask

// RFM23_RECEIVED_HEADER_2 @ 0x48
// no bits or mask

// RFM23_RECEIVED_HEADER_1 @ 0x49
// no bits or mask

// RFM23_RECEIVED_HEADER_0 @ 0x4A
// no bits or mask

// RFM23_RECEIVED_PACKET_LENGTH @ 0x4B
// no bits or mask

// RFM23_ANALOG_TEST_BUS @ 0x50
#define RFM23_ATB_MASK            0x1F

// RFM23_DIGITAL_TEST_BUS @ 0x51
#define RFM23_DTB_MASK            0x2F
#define RFM23_ENSCTEST            0x40

// RFM23_TX_RAMP_CONTROL @ 0x52
#define RFM23_TXRAMP_MASK         0x03
#define RFM23_LDORAMP_MASK        0x0C
#define RFM23_TXMOD_MASK          0x70

// RFM23_PLL_TUNE_TIME @ 0x53
#define RFM23_PLLT0_MASK          0x07
#define RFM23_PLLTS_MASK          0xF8

// RFM23_CALIBRATION_CONTROL @ 0x55
#define RFM23_SKIPVCO             0x01
#define RFM23_VCOCAL              0x02
#define RFM23_VCOCALDP            0x04
#define RFM23_RCCAL               0x08
#define RFM23_ENRCFCAL            0x10
#define RFM23_ADCCALDONE          0x20
#define RFM23_XTALSTARTHALF       0x40

// RFM23_MODEM_TEST @ 0x56
#define RFM23_IQSWITCH            0x01
#define RFM23_REFCLKINV           0x02
#define RFM23_REFCLKSEL           0x04
#define RFM23_AFCPOL              0x10
#define RFM23_DTTYPE              0x20
#define RFM23_SLICFBYP            0x40
#define RFM23_BCRFBYP             0x80

// RFM23_CHARGEPUMP_TEST @ 0x57
#define RFM23_CDCCUR_MASK         0x07
#define RFM23_CDCONLY             0x08
#define RFM23_CPFORCEDN           0x10
#define RFM23_CPFORCEUP           0x20
#define RFM23_FBDIV_RST           0x40
#define RFM23_PFDRST              0x80

// RFM23_CHARGEPUMP_CURRENT_TRIMMING_OVERRIDE @ 0x58
#define RFM23_CPCORR_MASK         0x1F
#define RFM23_CPCORROV            0x20
#define RFM23_CPCURR_MASK         0xC0

// RFM23_DIVIDER_CURRENT_TRIMMING @ 0x59
#define RFM23_D1P5TRIM_MASK       0x03
#define RFM23_D2TRIM_MASK         0x0C
#define RFM23_D3TRIM_MASK         0x30
#define RFM23_FBDIVHC             0x40
#define RFM23_TXCORBOOSTEN        0x80

// RFM23_VCO_CURRENT_TRIMMING @ 0x5A
#define RFM23_VCOCUR_MASK         0x03
#define RFM23_VCOCORR_MASK        0x3C
#define RFM23_VCOCORROV           0x40
#define RFM23_TXCURBOOSTEN        0x80

// RFM23_VCO_CALIBRATION_OVERRIDE @ 0x5B
#define RFM23_VCOCAL_MASK         0x7F
#define RFM23_VCOCALOV            0x80  //W
#define RFM23_VCDONE              0x80  //R

// RFM23_SYNTHESIZER_TEST @ 0x5C
#define RFM23_DSRST               0x01
#define RFM23_DSRSTMOD            0x02
#define RFM23_DSORDER_MASK        0x0C
#define RFM23_DSMOD               0x10
#define RFM23_ENOLOOP             0x20
#define RFM23_VCOTYPE             0x40
#define RFM23_DSMDT               0x80

// RFM23_BLOCK_ENABLE_OVERRIDE_1 @ 0x5D
#define RFM23_ENMX2               0x01
#define RFM23_ENBF12              0x02
#define RFM23_ENDV32              0x04
#define RFM23_ENBF5               0x08
#define RFM23_ENPA                0x10
#define RFM23_ENPGA               0x20
#define RFM23_ENLNA               0x40
#define RFM23_ENMIX               0x80

// RFM23_BLOCK_ENABLE_OVERRIDE_2 @ 0x5E
#define RFM23_PLLRESET            0x01
#define RFM23_ENBF2               0x02
#define RFM23_ENBF11              0x04
#define RFM23_ENBF3               0x08
#define RFM23_ENBF4               0x10
#define RFM23_ENMX3               0x20
#define RFM23_ENLDET              0x40
#define RFM23_ENDS                0x80

// RFM23_BLOCK_ENABLE_OVERRIDE_3 @ 0x5F
#define RFM23_ENBG                0x01
#define RFM23_ENCP                0x02
#define RFM23_ENVCO               0x04
#define RFM23_DVBSHUNT            0x08
#define RFM23_ENDV1P5             0x10
#define RFM23_ENDV2               0x20
#define RFM23_ENDV31              0x40
#define RFM23_ENFRDV              0x80

// RFM23_CHANNEL_FILTER_COEFFICIENT_ADDRESS @ 0x60
#define RFM23_CHFILADD_MASK       0x0F

// RFM23_CHANNEL_FILTER_COEFFICIENT_VALUE @ 0x61
#define RFM23_CHFILVAL_MASK       0x3F

// RFM23_CRYSTAL_OSCILLATOR_CONTROL_TEST @ 0x62
#define RFM23_ENBUF               0x01
#define RFM23_BUFOVR              0x02
#define RFM23_ENAMP2X             0x04
#define RFM23_ENBIAS2X            0x08
#define RFM23_CLKHYST             0x10
#define RFM23_PWST_MASK           0xE0

// RFM23_RC_OSCILLATOR_COARSE_CALIBRATION_OVERRIDE @ 0x63
#define RFM23_RCC_MASK            0x7F
#define RFM23_RCCOV               0x80

// RFM23_RC_OSCILLATOR_FINE_CALIBRATION_OVERRIDE @ 0x64
#define RFM23_RCF_MASK            0x7F
#define RFM23_RCFOV               0x80

// RFM23_LDO_CONTROL_OVERRIDE @ 0x65
#define RFM23_ENDIGPWDN           0x01
#define RFM23_ENDIGLDO            0x02
#define RFM23_ENPLLLDO            0x04
#define RFM23_ENRFLDO             0x08
#define RFM23_ENIFLDO             0x10
#define RFM23_ENVCOLDO            0x20
#define RFM23_ENBIAS              0x40
#define RFM23_ENSPOR              0x80

// RFM23_LDO_LEVEL_SETTING @ 0x66
#define RFM23_DIGLVL_MASK         0x07
#define RFM23_ENRC32              0x10
#define RFM23_ENTS                0x20
#define RFM23_ENXTAL              0x40
#define RFM23_ENOVR               0x80

// RFM23_DELTASIGMA_ADC_TUNING_1 @ 0x67
#define RFM23_ADCTUNE_MASK        0x0F
#define RFM23_ADCTUNEOVR          0x10
#define RFM23_ENADC               0x20
#define RFM23_ENREFDAC            0x40
#define RFM23_ADCRST              0x80

// RFM23_DELTASIGMA_ADC_TUNING_2 @ 0x68
#define RFM23_DSADCREF_MASK       0x07 // added DS to make unique
#define RFM23_ADCOLOOP            0x08
#define RFM23_ENVCM               0x10

// RFM23_AGC_OVERRIDE_1 @ 0x69
#define RFM23_PGA_MASK            0x0F
#define RFM23_LNAGAIN             0x10
#define RFM23_AGCEN               0x20

// RFM23_AGC_OVERRIDE_2 @ 0x6A
#define RFM23_PGATH_MASK          0x03
#define RFM23_LNACOMP_MASK        0x3F
#define RFM23_AGCSLOW             0x40
#define RFM23_AGCOVPM             0x80

// RFM23_GFSK_FIR_FILTER_COEFFICIENT_ADDRESS @ 0x6B
#define RFM23_FIRADD_MASK         0x07

// RFM23_GFSK_FIR_FILTER_COEFFICIENT_VALUE @ 0x6C
#define RFM23_FIRVAL_MASK         0x3F

// RFM23_TX_POWER @ 0x6D
#define RFM23_TXPOW_MASK          0x07

// RFM23_TX_DATA_RATE_1 @ 0x6E
// no bits or mask

// RFM23_TX_DATA_RATE_0 @ 0x6F
// no bits or mask

// RFM23_MODULATION_MODE_CONTROL_1 @ 0x70
#define RFM23_ENWHITE             0x01
#define RFM23_ENMANCH             0x02
#define RFM23_ENMANINV            0x04
#define RFM23_MANPPOL             0x08
#define RFM23_ENPHPWDN            0x10
#define RFM23_TXDTRTSCALE         0x20

// RFM23_MODULATION_MODE_CONTROL_2 @ 0x71
#define RFM23_MODTYP_MASK         0x03
#define RFM23_FD_MASK             0x04
#define RFM23_ENINV               0x08
#define RFM23_DTMOD_MASK          0x30
#define RFM23_TRCLK_MASK          0xC0
#define RFM23_MODTYP_GFSK         0x03
#define RFM23_FIFO_MODE           0x20
#define RFM23_TX_DATA_CLK_GPIO    0x40

// RFM23_FREQUENCY_DEVIATION @ 0x72
// no bits or mask

// RFM23_FREQUENCY_OFFSET_1 @ 0x73
// no bits or mask

// RFM23_FREQUENCY_OFFSET_2 @ 0x74
#define RFM23_FO_MASK             0x03

// RFM23_FREQUENCY_BAND_SELECTRESERVED @ 0x75
#define RFM23_FB_MASK             0x1F
#define RFM23_HBSEL               0x20
#define RFM23_SBSEL               0x40

// RFM23_NOMINAL_CARRIER_FREQUENCY_1 @ 0x76
// no bits or mask

// RFM23_NOMINAL_CARRIER_FREQUENCY_0 @ 0x77
// no bits or mask

// RFM23_FREQUENCY_HOPPING_CHANNEL_SELECT @ 0x79
// no bits or mask

// RFM23_FREQUENCY_HOPPING_STEP_SIZE @ 0x7A
// no bits or mask

// RFM23_TX_FIFO_CONTROL_1 @ 0x7C
#define RFM23_TXAFTHR_MASK        0x3F

// RFM23_TX_FIFO_CONTROL_2 @ 0x7D
#define RFM23_TXAETHR_MASK        0x3F

// RFM23_RX_FIFO_CONTROL @ 0x7E
#define RFM23_RXAFTHR_MASK        0x3F

// RFM23_FIFO_ACCESS @ 0x7F
// no bits or mask

#endif  //  _RFM23REG_H_
