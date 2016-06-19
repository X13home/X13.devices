/*
Copyright (c) 2011-2015 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _RFM69_REG_H_
#define _RFM69_REG_H_

// Source http://www.hoperf.com/upload/rfchip/RF69-V1.2.pdf

// RFM69 Registers definition

#define RFM69_REG_FIFO          0x00    // FIFO read/write access
#define RFM69_REG_OPMODE        0x01    // Operating modes of the transceiver
#define RFM69_REG_DATAMODUL     0x02    // Data operation mode and Modulation settings
#define RFM69_REG_BITRATEMSB    0x03    // Bit Rate setting, Most Significant Bits
#define RFM69_REG_BITRATELSB    0x04    // Bit Rate setting, Least Significant Bits
#define RFM69_REG_FDEVMSB       0x05    // Frequency Deviation setting, Most Significant Bits
#define RFM69_REG_FDEVLSB       0x06    // Frequency Deviation setting, Least Significant Bits
#define RFM69_REG_FRFMSB        0x07    // RF Carrier Frequency, Most Significant Bits
#define RFM69_REG_FRFMID        0x08    // RF Carrier Frequency, Intermediate Bits
#define RFM69_REG_FRFLSB        0x09    // RF Carrier Frequency, Least Significant Bits
#define RFM69_REG_OSC1          0x0A    // RC Oscillators Settings
#define RFM69_REG_AFCCTRL       0x0B    // AFC control in low modulation index situations
//#define RFM69_REG_LOWBAT        0x0C
#define RFM69_REG_LISTEN1       0x0D    // Listen Mode settings
#define RFM69_REG_LISTEN2       0x0E    // Listen Mode Idle duration
#define RFM69_REG_LISTEN3       0x0F    // Listen Mode Rx duration
#define RFM69_REG_VERSION       0x10
#define RFM69_REG_PALEVEL       0x11    // PA selection and Output Power control
#define RFM69_REG_PARAMP        0x12    // Control of the PA ramp time in FSK mode
#define RFM69_REG_OCP           0x13    // Over Current Protection control
//#define RFM69_REG_AGCREF        0x14
//#define RFM69_REG_AGCTHRESH1    0x15
//#define RFM69_REG_AGCTHRESH2    0x16
//#define RFM69_REG_AGCTHRESH3    0x17
#define RFM69_REG_LNA           0x18    // LNA settings
#define RFM69_REG_RXBW          0x19    // Channel Filter BW Control
#define RFM69_REG_AFCBW         0x1A    // Channel Filter BW control during the AFC routine
#define RFM69_REG_OOKPEAK       0x1B    // OOK demodulator selection and control in peak mode
#define RFM69_REG_OOKAVG        0x1C    // Average threshold control of the OOK demodulato
#define RFM69_REG_OOKFIX        0x1D    // Fixed threshold control of the OOK demodulator
#define RFM69_REG_AFCFEI        0x1E    // AFC and FEI control and status
#define RFM69_REG_AFCMSB        0x1F    // MSB of the frequency correction of the AFC
#define RFM69_REG_AFCLSB        0x20    // LSB of the frequency correction of the AFC
#define RFM69_REG_FEIMSB        0x21    // MSB of the calculated frequency error
#define RFM69_REG_FEILSB        0x22    // LSB of the calculated frequency error
#define RFM69_REG_RSSICONFIG    0x23    // RSSI-related settings
#define RFM69_REG_RSSIVALUE     0x24    // RSSI value in dBm
#define RFM69_REG_DIOMAPPING1   0x25    // Mapping of pins DIO0 to DIO3
#define RFM69_REG_DIOMAPPING2   0x26    // Mapping of pins DIO4 and DIO5, ClkOut frequency
#define RFM69_REG_IRQFLAGS1     0x27    // Status register: PLL Lock state, Timeout, RSSI > Threshold...
#define RFM69_REG_IRQFLAGS2     0x28    // Status register: FIFO handling flags...
#define RFM69_REG_RSSITHRESH    0x29    // RSSI Threshold control
#define RFM69_REG_RXTIMEOUT1    0x2A    // Timeout duration between Rx request and RSSI detection
#define RFM69_REG_RXTIMEOUT2    0x2B    // Timeout duration between RSSI detection and PayloadReady
#define RFM69_REG_PREAMBLEMSB   0x2C    // Preamble length, MSB
#define RFM69_REG_PREAMBLELSB   0x2D    // Preamble length, LSB
#define RFM69_REG_SYNCCONFIG    0x2E    // Sync Word Recognition control
#define RFM69_REG_SYNCVALUE1    0x2F    // Sync Word bytes, 1 through 8
#define RFM69_REG_SYNCVALUE2    0x30
#define RFM69_REG_SYNCVALUE3    0x31
#define RFM69_REG_SYNCVALUE4    0x32
#define RFM69_REG_SYNCVALUE5    0x33
#define RFM69_REG_SYNCVALUE6    0x34
#define RFM69_REG_SYNCVALUE7    0x35
#define RFM69_REG_SYNCVALUE8    0x36
#define RFM69_REG_PACKETCONFIG1 0x37    // Packet mode settings
#define RFM69_REG_PAYLOADLENGTH 0x38    // Payload length setting
#define RFM69_REG_NODEADRS      0x39    // Node address
#define RFM69_REG_BROADCASTADRS 0x3A    // Broadcast address
#define RFM69_REG_AUTOMODES     0x3B    // Auto modes settings
#define RFM69_REG_FIFOTHRESH    0x3C    // Fifo threshold, Tx start condition
#define RFM69_REG_PACKETCONFIG2 0x3D    // Packet mode settings
#define RFM69_REG_AESKEY1       0x3E    // 16 bytes of the cypher key
#define RFM69_REG_AESKEY2       0x3F
#define RFM69_REG_AESKEY3       0x40
#define RFM69_REG_AESKEY4       0x41
#define RFM69_REG_AESKEY5       0x42
#define RFM69_REG_AESKEY6       0x43
#define RFM69_REG_AESKEY7       0x44
#define RFM69_REG_AESKEY8       0x45
#define RFM69_REG_AESKEY9       0x46
#define RFM69_REG_AESKEY10      0x47
#define RFM69_REG_AESKEY11      0x48
#define RFM69_REG_AESKEY12      0x49
#define RFM69_REG_AESKEY13      0x4A
#define RFM69_REG_AESKEY14      0x4B
#define RFM69_REG_AESKEY15      0x4C
#define RFM69_REG_AESKEY16      0x4D
#define RFM69_REG_TEMP1         0x4E    // Temperature Sensor control
#define RFM69_REG_TEMP2         0x4F    // Temperature readout
#define RFM69_REG_TESTLNA       0x58    // Sensitivity boost
#define RFM69_REG_TESTPA1       0x5A    // High Power PA settings
#define RFM69_REG_TESTPA2       0x5C    // High Power PA settings
#define RFM69_REG_TESTDAGC      0x6F    // Fading Margin Improvement
#define RFM69_REG_TESTAFC       0x71    // AFC offset for low modulation index AFC
// 0x50+ RegTest - Internal Test Registers
#define RFM69_REG_WRITE         0x80    // Write Access
#define RFM69_REG_READ          0x00


//******************************************************
// RF69/SX1231 bit control definition
//******************************************************

////////////////////////////////////////////////////////
// RegOpMode - 0x01
// POR Value: 0x00
#define RFM69_OPMODE_SEQUENCER_OFF  0x80    // Controls the automatic Sequencer (see section 4.2 ):
                                            //  0 - Operating mode as selected with Mode bits in RegOpMode is automatically reached with the Sequencer
                                            //  1 - Mode is forced by the user
#define RFM69_OPMODE_LISTEN_ON      0x40    // Enables Listen mode, should be enabled whilst in Standby mode:
                                            //  0 - Off (see section 4.3)
                                            //  1 - On
#define RFM69_OPMODE_LISTENABORT    0x20    // Aborts Listen mode when set together with ListenOn = 0
                                            // See section 4.3.4 for details. Always reads 0.
#define RFM69_OPMODE_SLEEP          0x00    // Sleep mode
#define RFM69_OPMODE_STDBY          0x04    // Standby mode
#define RFM69_OPMODE_FS             0x08    // Frequency Synthesizer mode
#define RFM69_OPMODE_TX             0x0C    // Transmitter mode
#define RFM69_OPMODE_RX             0x10    // Receiver mode
// Bits 0,1 RO, not used
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegDataModul - 0x02
// POR Value: 0x00
// bit 7 unused
// bits 6-5 Data processing mode:
#define RFM69_DATAMODUL_DATAMODE_PACKET             0x00    // Packet mode
#define RFM69_DATAMODUL_DATAMODE_CONTINUOUS         0x40    // Continuous mode with bit synchronizer
#define RFM69_DATAMODUL_DATAMODE_CONTINUOUSNOBSYNC  0x60    // Continuous mode without bit synchronizer
// bits 4-3 Modulation scheme:
#define RFM69_DATAMODUL_MODULATIONTYPE_FSK          0x00    // FSK
#define RFM69_DATAMODUL_MODULATIONTYPE_OOK          0x08    // OOK
// bit2 unused

// bit 1-0 Data shaping:
#define RFM69_DATAMODUL_MODULATIONSHAPING_0         0x00    // no shaping
#define RFM69_DATAMODUL_MODULATIONSHAPING_1         0x01    // FSK: Gaussian filter, BT = 1.0 
                                                            // OOK: filtering with fcutoff = BR
#define RFM69_DATAMODUL_MODULATIONSHAPING_2         0x02    // FSK: Gaussian filter, BT = 0.5
                                                            // OOK: filtering with fcutoff = 2*BR
#define RFM69_DATAMODUL_MODULATIONSHAPING_3         0x03    // FSK: Gaussian filter, BT = 0.3
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegBitrateMsb - 0x03
// RegBitrateLsb - 0x04
// POR Value: 0x1A0B - 4800
#define RFM69_BITRATEMSB                            0x03    // 38400
#define RFM69_BITRATELSB                            0x41
// BitRate = FXO_SC/BitRateReg(15,0)
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegFdevMsb - 0x05
// RegFdevLsb - 0x06
// POR Value: 0x0052 - 5kHz
#define RFM69_FDEVMSB                               0x03    // 50 kHz
#define RFM69_FDEVLSB                               0x33
// Fdev = Fstep * FdevReg(13,0)
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegFrf 0x07-0x09     RF carrier frequency
// POR Value: 0xE4C000  - 915 MHz
// ToDo - Default frequency.

// Frf = Fstep * RegFrf(23,0)
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegOsc1 - 0x0A
#define RFM69_OSC1_RCCAL_START                      0x80
// Triggers the calibration of the RC oscillator when set.
// Always reads 0. RC calibration must be triggered in Standby mode
#define RFM69_OSC1_RCCAL_DONE                       0x40
// 0 - RC calibration in progress
// 1 - RC calibration is over
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegAfcCtrl - 0x0B
#define RFM69_AFCCTRL_LOWBETA_ON                    0x20
#define RFM69_AFCCTRL_LOWBETA_OFF                   0x00
// Improved AFC routine for signals with modulation index lower than 2. 
// Refer to section 3.4.16 for details
// 0 - Standard AFC routine
// 1 - Improved AFC routine
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegListen1 - 0x0D
// POR Value: 0x92
//
// bits 7-6
// Resolution of Listen mode Idle time (calibrated RC osc):
// ListenResolIdle
#define RFM69_LISTEN1_RESOL_IDLE_64                 0x40    // 64 us
#define RFM69_LISTEN1_RESOL_IDLE_4100               0x80    // 4,1 ms - default
#define RFM69_LISTEN1_RESOL_IDLE_262000             0xC0    // 262 ms
// bits 5-4
// Resolution of Listen mode Rx time (calibrated RC osc):
// ListenResolRx
#define RFM69_LISTEN1_RESOL_RX_64                   0x10    // 64us - default
#define RFM69_LISTEN1_RESOL_RX_4100                 0x20    // 4,1 ms
#define RFM69_LISTEN1_RESOL_RX_262000               0x30    // 262 ms
// bit 3
// Criteria for packet acceptance in Listen mode:
#define RFM69_LISTEN1_CRITERIA_RSSI                 0x00    // signal strength is above RssiThreshold
                                                            // default
#define RFM69_LISTEN1_CRITERIA_RSSIANDSYNC          0x08    // signal strength is above RssiThreshold and SyncAddress matched
// bits 2-1
// Action taken after acceptance of a packet in Listen mode
#define RFM69_LISTEN1_END_0                         0x00    // chip stays in Rx mode. Listen mode stops and must be disabled (see section 4.3).
#define RFM69_LISTEN1_END_1                         0x02    // chip stays in Rx mode until PayloadReady or Timeout interrupt occurs. It then goes to the mode defined by Mode. Listen mode stops and must be disabled (see section 4.3).
                                                            // default
#define RFM69_LISTEN1_END_2                         0x04    // chip stays in Rx mode until PayloadReady or Timeout interrupt occurs. Listen mode then resumes in Idle state. FIFO content is lost at next Rx wakeup.
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegListen2 - 0x0E   Duration of the Idle phase in Listen mode.
// POR Value: 0xF5
// tListenIdle = RegListen2 * ListenResolIdle
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegListen3 - 0x0F   Duration of the Rx phase in Listen mode (startup time included, see section 4.2.3)
// POR Value: 0x20
// tListenRx = RegListen3 * ListenResolRx
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegPaLevel - 0x11
// POR Value: 0x9F, PA0 +13 dBm
#define RFM69_PALEVEL_PA0_ON                        0x80    // Enables PA0, connected to RFIO and LNA
                                                            // default
#define RFM69_PALEVEL_PA1_ON                        0x40    // Enables PA1, on PA_BOOST pin
#define RFM69_PALEVEL_PA2_ON                        0x20    // Enables PA2, on PA_BOOST pin
#define RFM69_PALEVEL_MAX                           0x1F
// bits 4-0 Output Power
// Output power setting, with 1 dB steps
// Pout = -18 + OutputPower [dBm] , with PA0
// Pout = -18 + OutputPower [dBm] , with PA1**
// Pout = -14+ OutputPower [dBm] , with PA1 and PA2**
// Pout = -11 + OutputPower [dBm] , with PA1 and PA2, and high Power PA settings
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegPaRamp - 0x12
// Rise/Fall time of ramp up/down in FSK
// POR Value: 0x09, 40 us
#define RFM69_PARAMP_3400                           0x00
#define RFM69_PARAMP_2000                           0x01
#define RFM69_PARAMP_1000                           0x02
#define RFM69_PARAMP_500                            0x03
#define RFM69_PARAMP_250                            0x04
#define RFM69_PARAMP_125                            0x05
#define RFM69_PARAMP_100                            0x06
#define RFM69_PARAMP_62                             0x07
#define RFM69_PARAMP_50                             0x08
#define RFM69_PARAMP_40                             0x09
#define RFM69_PARAMP_31                             0x0A
#define RFM69_PARAMP_25                             0x0B
#define RFM69_PARAMP_20                             0x0C
#define RFM69_PARAMP_15                             0x0D
#define RFM69_PARAMP_12                             0x0E
#define RFM69_PARAMP_10                             0x0F
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegOcp - 0x13
// POR Value: 0x1A, OCP - On, 95 mA
                                                    // overload current protection (OCP) for the PA:
#define RFM69_OCP_ON                                0x10    //  Enabled
#define RFM69_OCP_OFF                               0x00    //  Disabled
// bits 3-0 Trimming of OCP current:
// Imax = 45 + 5 * OcpTrim
#define RFM69_OCP_TRIM_45                           0x00
#define RFM69_OCP_TRIM_50                           0x01
#define RFM69_OCP_TRIM_55                           0x02
#define RFM69_OCP_TRIM_60                           0x03
#define RFM69_OCP_TRIM_65                           0x04
#define RFM69_OCP_TRIM_70                           0x05
#define RFM69_OCP_TRIM_75                           0x06
#define RFM69_OCP_TRIM_80                           0x07
#define RFM69_OCP_TRIM_85                           0x08
#define RFM69_OCP_TRIM_90                           0x09
#define RFM69_OCP_TRIM_95                           0x0A   // Default
#define RFM69_OCP_TRIM_100                          0x0B
#define RFM69_OCP_TRIM_105                          0x0C
#define RFM69_OCP_TRIM_110                          0x0D
#define RFM69_OCP_TRIM_115                          0x0E
#define RFM69_OCP_TRIM_120                          0x0F
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegLna - 0x18
// POR Value: 0x88, ZIN - 200R, gain set by the internal AGC loop
// bit 7, LNA's input impedance
#define RFM69_LNA_ZIN_50                            0x00    // 50 ohms
#define RFM69_LNA_ZIN_200                           0x80    // 200 ohms - default
// bit 6
//#define RFM69_LNA_LOWPOWER_OFF                      0x00    // Default
//#define RFM69_LNA_LOWPOWER_ON                       0x40
// bits 5-3, RO, Current LNA gain, set either manually, or by the AGC
// bits 2-0, LNA gain setting:
#define RFM69_LNA_GAINSELECT_AUTO                   0x00    // gain set by the internal AGC loop
                                                            // default                      
#define RFM69_LNA_GAINSELECT_MAX                    0x01    // G1 = highest gain 
#define RFM69_LNA_GAINSELECT_MAX_6                  0x02    // G2 = highest gain - 6 dB
#define RFM69_LNA_GAINSELECT_MAX_12                 0x03    // G3 = highest gain - 12 dB
#define RFM69_LNA_GAINSELECT_MAX_24                 0x04    // G4 = highest gain - 24 dB
#define RFM69_LNA_GAINSELECT_MAX_36                 0x05    // G5 = highest gain - 36 dB
#define RFM69_LNA_GAINSELECT_MAX_48                 0x06    // G6 = highest gain - 48 dB
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegRxBw - 0x19 
// bits 7-5, Cut-off frequency of the DC offset canceller (DCC):
// fc = 4xRxBw/(2*pi*2^(DccFreq+2))
// approx. 4% of the RxBw by default
#define RFM69_RXBW_DCCFREQ_0                        0x00
#define RFM69_RXBW_DCCFREQ_1                        0x20
#define RFM69_RXBW_DCCFREQ_2                        0x40    // Recommended default
#define RFM69_RXBW_DCCFREQ_3                        0x60
#define RFM69_RXBW_DCCFREQ_4                        0x80    // Reset value
#define RFM69_RXBW_DCCFREQ_5                        0xA0
#define RFM69_RXBW_DCCFREQ_6                        0xC0
#define RFM69_RXBW_DCCFREQ_7                        0xE0
// bits 4-3, Channel filter bandwidth control:
#define RFM69_RXBW_MANT_16                          0x00    // RxBwMant = 16
                                                            // Reset value
#define RFM69_RXBW_MANT_20                          0x08    // RxBwMant = 20
#define RFM69_RXBW_MANT_24                          0x10    // RxBwMant = 24
                                                            // Recommended default
// bits 2-0, Channel filter bandwidth control:
#define RFM69_RXBW_EXP_0                            0x00
#define RFM69_RXBW_EXP_1                            0x01
#define RFM69_RXBW_EXP_2                            0x02
#define RFM69_RXBW_EXP_3                            0x03
#define RFM69_RXBW_EXP_4                            0x04
#define RFM69_RXBW_EXP_5                            0x05    // Recommended default
#define RFM69_RXBW_EXP_6                            0x06    // Reset value
#define RFM69_RXBW_EXP_7                            0x07
// FSK Mode: RxBw = FXOSC / (RxBwMant * 2^(RxBwExp + 2))
// OOK Mode: RxBw = FXOSC / (RxBwMant * 2^(RxBwExp + 3))
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegAfcBw - 0x1A
// bits 7-5, DccFreq parameter used during the AFC
#define RFM69_AFCBW_DCCFREQAFC_0                    0x00
#define RFM69_AFCBW_DCCFREQAFC_1                    0x20
#define RFM69_AFCBW_DCCFREQAFC_2                    0x40
#define RFM69_AFCBW_DCCFREQAFC_3                    0x60
#define RFM69_AFCBW_DCCFREQAFC_4                    0x80    // Default
#define RFM69_AFCBW_DCCFREQAFC_5                    0xA0
#define RFM69_AFCBW_DCCFREQAFC_6                    0xC0
#define RFM69_AFCBW_DCCFREQAFC_7                    0xE0
// bits 4-3, RxBwMant parameter used during the AFC
#define RFM69_AFCBW_MANTAFC_16                      0x00
#define RFM69_AFCBW_MANTAFC_20                      0x08    // Default
#define RFM69_AFCBW_MANTAFC_24                      0x10
// bits 2-0, RxBwExp parameter used during the AFC
#define RFM69_AFCBW_EXPAFC_0                        0x00
#define RFM69_AFCBW_EXPAFC_1                        0x01
#define RFM69_AFCBW_EXPAFC_2                        0x02
#define RFM69_AFCBW_EXPAFC_3                        0x03    // Default
#define RFM69_AFCBW_EXPAFC_4                        0x04
#define RFM69_AFCBW_EXPAFC_5                        0x05
#define RFM69_AFCBW_EXPAFC_6                        0x06
#define RFM69_AFCBW_EXPAFC_7                        0x07
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegOokPeak - 0x1B
// bits 7-6, Selects type of threshold in the OOK data slicer:
#define RFM69_OOKPEAK_THRESHTYPE_FIXED              0x00    // fixed
#define RFM69_OOKPEAK_THRESHTYPE_PEAK               0x40    // peak - efault
#define RFM69_OOKPEAK_THRESHTYPE_AVERAGE            0x80    // average
// bits 5-3, Size of each decrement of the RSSI threshold in the OOK demodulator:
#define RFM69_OOKPEAK_PEAKTHRESHSTEP_05             0x00    // Default - 0,5 dB
#define RFM69_OOKPEAK_PEAKTHRESHSTEP_10             0x08    //
#define RFM69_OOKPEAK_PEAKTHRESHSTEP_15             0x10    //
#define RFM69_OOKPEAK_PEAKTHRESHSTEP_20             0x18    //
#define RFM69_OOKPEAK_PEAKTHRESHSTEP_30             0x20
#define RFM69_OOKPEAK_PEAKTHRESHSTEP_40             0x28
#define RFM69_OOKPEAK_PEAKTHRESHSTEP_50             0x30
#define RFM69_OOKPEAK_PEAKTHRESHSTEP_60             0x38    // 6,0 dB 
// bits 2-0, Period of decrement of the RSSI threshold in the OOK demodulator:
#define RFM69_OOKPEAK_PEAKTHRESHDEC_0               0x00    // Default, once per chip
#define RFM69_OOKPEAK_PEAKTHRESHDEC_1               0x01    // once every 2 chips
#define RFM69_OOKPEAK_PEAKTHRESHDEC_2               0x02    // once every 4 chips
#define RFM69_OOKPEAK_PEAKTHRESHDEC_3               0x03    // once every 8 chips
#define RFM69_OOKPEAK_PEAKTHRESHDEC_4               0x04    // twice in each chip
#define RFM69_OOKPEAK_PEAKTHRESHDEC_5               0x05    // 4 times in each chip
#define RFM69_OOKPEAK_PEAKTHRESHDEC_6               0x06    // 8 times in each chip
#define RFM69_OOKPEAK_PEAKTHRESHDEC_7               0x07    // 16 times in each chip
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegOokAvg - 0x1C
// bits 7-6, Filter coefficients in average mode of the OOK demodulator:
#define RFM69_OOKAVG_AVERAGETHRESHFILT_0            0x00    // fC ≈ chip rate / 32pi
#define RFM69_OOKAVG_AVERAGETHRESHFILT_1            0x40    // fC ≈ chip rate / 8pi
#define RFM69_OOKAVG_AVERAGETHRESHFILT_2            0x80    // Default, fC ≈ chip rate / 4pi
#define RFM69_OOKAVG_AVERAGETHRESHFILT_3            0xC0    // fC ≈ chip rate / 2pi
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegOokFix - 0x1D
// Fixed threshold value (in dB) in the OOK demodulator.
// Used when OokThresType = 00
// Default Value: 0x06 - 6dB
#define RFM69_OOKFIX_FIXEDTHRESH_VALUE              0x06    // Default
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegAfcFei - 0x1E
#define RFM69_AFCFEI_FEI_DONE                       0x40    // 0 - FEI is on-going, 1 - FEI finished
#define RFM69_AFCFEI_FEI_START                      0x20    // Triggers a FEI measurement when set. Always reads 0.
#define RFM69_AFCFEI_AFC_DONE                       0x10    // 0 - AFC is on-going, 1 - AFC has finished
#define RFM69_AFCFEI_AFCAUTOCLEAR                   0x08    // Only valid if AfcAutoOn is set
                                                            // 0 - AFC register is not cleared before a new AFC phase
                                                            // 1 - AFC register is cleared before a new AFC phase
#define RFM69_AFCFEI_AFCAUTO                        0x04    // 0 - AFC is performed each time AfcStart is set
                                                            // 1 - AFC is performed each time Rx mode is entered
#define RFM69_AFCFEI_AFC_CLEAR                      0x02    // Clears the AfcValue if set in Rx mode. Always reads 0
#define RFM69_AFCFEI_AFC_START                      0x01    // Triggers an AFC when set. Always reads 0.
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegAfcMsb - 0x1F, RegAfcLsb - 0x20, RO
// AfcValue, 2's complement format
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegFeiMsb - 0x21, RegFeiLsb - 0x22, RO
// FeiValue, 2's complement format
// Frequency error = FeiValue x Fstep
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegRssiConfig - 0x23
#define RFM69_RSSI_DONE                             0x02    // 0 - RSSI is on-going
                                                            // 1 - RSSI sampling is finished, result available
#define RFM69_RSSI_START                            0x01    // Trigger a RSSI measurement when set. Always reads 0
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegRssiValue - 0x24
// Absolute value of the RSSI in dBm, 0.5dB steps. RSSI = -RssiValue/2 [dBm]
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegDioMapping1 0x25
// Mapping of pins DIO0 to DIO3
// See Table 21 for mapping in Continuous mode
// See Table 22 for mapping in Packet mode

#define RFM69_DIOMAPPING1_DIO0_0                    0x00  // Default
#define RFM69_DIOMAPPING1_DIO0_1                    0x40
#define RFM69_DIOMAPPING1_DIO0_2                    0x80
#define RFM69_DIOMAPPING1_DIO0_3                    0xC0

#define RFM69_DIOMAPPING1_DIO1_0                    0x00  // Default
#define RFM69_DIOMAPPING1_DIO1_1                    0x10
#define RFM69_DIOMAPPING1_DIO1_2                    0x20
#define RFM69_DIOMAPPING1_DIO1_3                    0x30

#define RFM69_DIOMAPPING1_DIO2_0                    0x00  // Default
#define RFM69_DIOMAPPING1_DIO2_1                    0x04
#define RFM69_DIOMAPPING1_DIO2_2                    0x08
#define RFM69_DIOMAPPING1_DIO2_3                    0x0C

#define RFM69_DIOMAPPING1_DIO3_0                    0x00  // Default
#define RFM69_DIOMAPPING1_DIO3_1                    0x01
#define RFM69_DIOMAPPING1_DIO3_2                    0x02
#define RFM69_DIOMAPPING1_DIO3_3                    0x03
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegDioMapping2 0x26
// Mapping of pins DIO4 to DIO5
// See Table 21 for mapping in Continuous mode
// See Table 22 for mapping in Packet mode
#define RFM69_DIOMAPPING2_DIO4_0                    0x00    // Default
#define RFM69_DIOMAPPING2_DIO4_1                    0x40
#define RFM69_DIOMAPPING2_DIO4_2                    0x80
#define RFM69_DIOMAPPING2_DIO4_3                    0xC0

#define RFM69_DIOMAPPING2_DIO5_0                    0x00    // Default
#define RFM69_DIOMAPPING2_DIO5_1                    0x10
#define RFM69_DIOMAPPING2_DIO5_2                    0x20
#define RFM69_DIOMAPPING2_DIO5_3                    0x30
// Selects CLKOUT frequency:
#define RFM69_DIOMAPPING2_CLKOUT_32                 0x00    //  FXOSC
#define RFM69_DIOMAPPING2_CLKOUT_16                 0x01    //  FXOSC/2
#define RFM69_DIOMAPPING2_CLKOUT_8                  0x02    //  FXOSC/4
#define RFM69_DIOMAPPING2_CLKOUT_4                  0x03    //  FXOSC/8
#define RFM69_DIOMAPPING2_CLKOUT_2                  0x04    //  FXOSC/16
#define RFM69_DIOMAPPING2_CLKOUT_1                  0x05    // Default, //  FXOSC/32
#define RFM69_DIOMAPPING2_CLKOUT_RC                 0x06    // RC (automatically enabled)
#define RFM69_DIOMAPPING2_CLKOUT_OFF                0x07    // Off
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegIrqFlags1 - 0x27
#define RFM69_IRQFLAGS1_MODEREADY                   0x80    // Set when the operation mode requested in Mode, is ready
                                                            // Sleep: Entering Sleep mode
                                                            // Standby: XO is running
                                                            // FS: PLL is locked
                                                            // Rx: RSSI sampling starts
                                                            // Tx: PA ramp-up completed
                                                            // Cleared when changing operating mode
#define RFM69_IRQFLAGS1_RXREADY                     0x40    // Set in Rx mode, after RSSI, AGC and AFC.
                                                            // Cleared when leaving Rx.
#define RFM69_IRQFLAGS1_TXREADY                     0x20    // Set in Tx mode, after PA ramp-up.
                                                            // Cleared when leaving Tx.
#define RFM69_IRQFLAGS1_PLLLOCK                     0x10    // Set (in FS, Rx or Tx) when the PLL is locked.
#define RFM69_IRQFLAGS1_RSSI                        0x08    // Set in Rx when the RssiValue exceeds RssiThreshold.
                                                            // Cleared when leaving Rx.
#define RFM69_IRQFLAGS1_TIMEOUT                     0x04    // Set when a timeout occurs (see TimeoutRxStart and TimeoutRssiThresh)
                                                            // Cleared when leaving Rx or FIFO is emptied.
#define RFM69_IRQFLAGS1_AUTOMODE                    0x02    // Set when entering Intermediate mode.
                                                            // Cleared when exiting Intermediate mode.
#define RFM69_IRQFLAGS1_SYNCADDRESSMATCH            0x01    // Set when Sync and Address (if enabled) are detected.
                                                            // Cleared when leaving Rx or FIFO is emptied.
                                                            // This bit is read only in Packet mode, rwc in Continuous mode
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegIrqFlags2 - 0x28
#define RFM69_IRQFLAGS2_FIFOFULL                    0x80    // Set when FIFO is full (i.e. contains 66 bytes), else cleared.
#define RFM69_IRQFLAGS2_FIFONOTEMPTY                0x40    // Set when FIFO contains at least one byte, else cleared.
#define RFM69_IRQFLAGS2_FIFOLEVEL                   0x20    // Set when the number of bytes in the FIFO strictly exceeds FifoThreshold, else cleared.
#define RFM69_IRQFLAGS2_FIFOOVERRUN                 0x10    // Set when FIFO overrun occurs. (except in Sleep mode)
                                                            // Flag(s) and FIFO are cleared when this bit is set. The FIFO then becomes immediately available for the next transmission / reception.
#define RFM69_IRQFLAGS2_PACKETSENT                  0x08    // Set in Tx when the complete packet has been sent.
                                                            // Cleared when exiting Tx.
#define RFM69_IRQFLAGS2_PAYLOADREADY                0x04    // Set in Rx when the payload is ready (i.e. last byte received and CRC, if enabled and CrcAutoClearOff is cleared, is Ok). Cleared when FIFO is empty.
#define RFM69_IRQFLAGS2_CRCOK                       0x02    // Set in Rx when the CRC of the payload is Ok. Cleared when FIFO is empty.
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegRssiThresh - 0x29
// RSSI trigger level for Rssi interrupt: - RssiThreshold / 2 [dBm]
// Default value: 0xE4
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegRxTimeout1 - 0x2A
// Timeout interrupt is generated TimeoutRxStart*16*Tbit after switching to Rx mode if Rssi interrupt doesn't occur (i.e. RssiValue > RssiThreshold)
// 0x00: TimeoutRxStart is disabled
// Default value: 0x00
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegRxTimeout2 - 0x2B
// Timeout interrupt is generated TimeoutRssiThresh*16*Tbit after Rssi interrupt if PayloadReady interrupt doesn't occur.
// 0x00: TimeoutRssiThresh is disabled
// Default value: 0x00
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegPreamble 0x2C-0x2D
// Size of the preamble to be sent (from TxStartCondition fulfilled).
// Default value: 0x0003
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegSyncConfig - 0x2E
#define RFM69_SYNC                                  0x80    // Default, Enables the Sync word generation and detection
#define RFM69_SYNC_FIFOFILL                         0x40    // FIFO filling condition:
                                                            // default, 0 - if SyncAddress interrupt occurs
                                                            // 1 - as long as FifoFillCondition is set
// Size of the Sync word, (SyncSize + 1) bytes
#define RFM69_SYNC_SIZE_1                           0x00
#define RFM69_SYNC_SIZE_2                           0x08
#define RFM69_SYNC_SIZE_3                           0x10
#define RFM69_SYNC_SIZE_4                           0x18    // Default, 4 bytes
#define RFM69_SYNC_SIZE_5                           0x20
#define RFM69_SYNC_SIZE_6                           0x28
#define RFM69_SYNC_SIZE_7                           0x30
#define RFM69_SYNC_SIZE_8                           0x38
// Number of tolerated bit errors in Sync word
#define RFM69_SYNC_TOL_0                            0x00    // Default
#define RFM69_SYNC_TOL_1                            0x01
#define RFM69_SYNC_TOL_2                            0x02
#define RFM69_SYNC_TOL_3                            0x03
#define RFM69_SYNC_TOL_4                            0x04
#define RFM69_SYNC_TOL_5                            0x05
#define RFM69_SYNC_TOL_6                            0x06
#define RFM69_SYNC_TOL_7                            0x07
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegSyncValue1-8 0x2F-0x36
// n'th byte of Sync word, Used if SyncOn is set.
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegPacketConfig1 - 0x37
// Packet mode settings
#define RFM69_PACKET1_FORMAT_VARIABLE               0x80
#define RFM69_PACKET1_DCFREE_OFF                    0x00    // Default
#define RFM69_PACKET1_DCFREE_MANCHESTER             0x20
#define RFM69_PACKET1_DCFREE_WHITENING              0x40
#define RFM69_PACKET1_CRC_ON                        0x10    // Default
#define RFM69_PACKET1_CRC_OFF                       0x00
#define RFM69_PACKET1_CRCAUTOCLEAR_ON               0x00    // Default
#define RFM69_PACKET1_CRCAUTOCLEAR_OFF              0x08
#define RFM69_PACKET1_ADRSFILTERING_OFF             0x00    // Default
#define RFM69_PACKET1_ADRSFILTERING_NODE            0x02
#define RFM69_PACKET1_ADRSFILTERING_NODEBROADCAST   0x04
#define RFM69_PACKET1_CRC_CCITT                     0x00    // default
#define RFM69_PACKET1_CRC_IBM                       0x01    // No documentation
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegFifoThresh - 0x3C
#define RFM69_FIFOTHRESH_TXSTART_FIFOTHRESH         0x00    // Reset value
#define RFM69_FIFOTHRESH_TXSTART_FIFONOTEMPTY       0x80    // Recommended default
#define RFM69_FIFOTHRESH_VALUE                      0x0F    // Default
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegPacketConfig2 - 0x3D
#define RFM69_PACKET2_RXRESTARTDELAY_1BIT           0x00    // Default
#define RFM69_PACKET2_RXRESTARTDELAY_2BITS          0x10
#define RFM69_PACKET2_RXRESTARTDELAY_4BITS          0x20
#define RFM69_PACKET2_RXRESTARTDELAY_8BITS          0x30
#define RFM69_PACKET2_RXRESTARTDELAY_16BITS         0x40
#define RFM69_PACKET2_RXRESTARTDELAY_32BITS         0x50
#define RFM69_PACKET2_RXRESTARTDELAY_64BITS         0x60
#define RFM69_PACKET2_RXRESTARTDELAY_128BITS        0x70
#define RFM69_PACKET2_RXRESTARTDELAY_256BITS        0x80
#define RFM69_PACKET2_RXRESTARTDELAY_512BITS        0x90
#define RFM69_PACKET2_RXRESTARTDELAY_1024BITS       0xA0
#define RFM69_PACKET2_RXRESTARTDELAY_2048BITS       0xB0
#define RFM69_PACKET2_RXRESTARTDELAY_NONE           0xC0
#define RFM69_PACKET2_RXRESTART                     0x04
#define RFM69_PACKET2_AUTORXRESTART_ON              0x02    // Default
#define RFM69_PACKET2_AUTORXRESTART_OFF             0x00
#define RFM69_PACKET2_AES_ON                        0x01
#define RFM69_PACKET2_AES_OFF                       0x00    // Default
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegTestLna - 0x58
#define RFM69_REG_TESTLNA_NORM                      0x1B    // Normal Mode
#define RFM69_REG_TESTLNA_HIGH                      0x2D    // High sensitivity mode
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegTestPa1 - 0x5A, RegTestPa1 - 0x5C
#define RFM69_REG_TESTPA1_LOW                       0x55    // Normal and Rx mode
#define RFM69_REG_TESTPA2_LOW                       0x70
#define RFM69_REG_TESTPA1_BOOST                     0x5D    // +20 dBm mode
#define RFM69_REG_TESTPA2_BOOST                     0x7C
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// RegTestDagc - 0x6F
// Fading Margin Improvement
#define RFM69_DAGC_NORMAL              0x00                 // Reset value
#define RFM69_DAGC_IMPROVED_LOWBETA1   0x20
#define RFM69_DAGC_IMPROVED_LOWBETA0   0x30                 // Recommended default
////////////////////////////////////////////////////////

#endif  //  _RFM69_REG_H_
