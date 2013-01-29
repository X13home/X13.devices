/*
Copyright (c) 2011-2012 <comparator@gmx.de>

This file is part of the X13.Home project.
https://github.com/X13home

BSD License
See LICENSE.txt file for license details.
*/

#ifndef _RFM12REG_H_
#define _RFM12REG_H_

// RFM12 Registers definition

//=== Configuration Setting ===
// POR Value:  8008
#define RFM12_CMD_CFG       0x8000      // Configuration Setting
#define RFM12_CFG_EL        0x80        // enables the internal data register.
#define RFM12_CFG_EF        0x40        // Bit ef enables the FIFO mode. If ef=0 then DATA (pin 6) and DCLK (pin 7) are used for data and data clock output.
#define RFM12_BAND_433      0x10
#define RFM12_BAND_868      0x20
#define RFM12_BAND_915      0x30
#define RFM12_BAND_MASK     0x30
#define RFM12_XTAL_12PF     0x07
// Capacitor Value = (x3:x0) * 0.5  + 8.5 pF

//=== Power Managment ===
// POR Value: 8208
#define RFM12_CMD_PWRMGT    0x8200      // Power Managment
#define RFM12_PWRMGT_ER     0x80        // Enables the whole receiver chain RF front end, baseband, synthesizer, oscillator
#define RFM12_PWRMGT_EBB    0x40        // The receiver baseband circuit can be separately switched on Baseband
#define RFM12_PWRMGT_ET     0x20        // Switches on the PLL, the power amplifier, and starts the transmission (If TX register is enabled) 
#define RFM12_PWRMGT_ES     0x10        // Turns on the synthesizer
#define RFM12_PWRMGT_EX     0x08        // Turns on the crystal oscillator
#define RFM12_PWRMGT_EB     0x04        // Enables the low battery detector
#define RFM12_PWRMGT_EW     0x02        // Enables the wake-up timer
#define RFM12_PWRMGT_DC     0x01        // Disables the clock output (pin 8)

//=== Frequency-setting ===
// POR Valie: A680
#define RFM12_CMD_FREQUENCY 0xA000
#define RFM12_FREQUENCY_CALC_433(f) (((f)-430000000UL)/2500UL)
#define RFM12_FREQUENCY_CALC_868(f) (((f)-860000000UL)/5000UL)
#define RFM12_FREQUENCY_CALC_915(f) (((f)-900000000UL)/7500UL)
#define RFM12_FREQ_MASK 0x0FFF
// 96 < [f11:f0] < 3903

//=== Data-Rate ===
// POR Value: C623
#define RFM12_CMD_DATARATE 0xC600
enum e_RFM12_BAUD
{
    RFM12_BAUD_115K2    = 0x02,
    RFM12_BAUD_57K6     = 0x05,
    RFM12_BAUD_49K2     = 0x06,
    RFM12_BAUD_38K4     = 0x08,
    RFM12_BAUD_28K8     = 0x0B,
    RFM12_BAUD_19K2     = 0x11,
    RFM12_BAUD_14K4     = 0x17,
    RFM12_BAUD_9K6      = 0x23,
    RFM12_BAUD_4K8      = 0x47,
    RFM12_BAUD_2K4      = 0x91,
    RFM12_BAUD_1K2      = 0xA3,
    RFM12_BAUD_600      = 0xC7
};

//=== RX Control ===
// POR Value 9080
#define RFM12_CMD_RXCTRL            0x9000      // RX Control
#define RFM12_RXCTRL_P16_VDI        0x400       // Function of pin 16, 0 - Interrupt input / 1 - VDI output
#define RFM12_RXCTRL_VDI_FAST       0x000       // VDI (valid data indicator) signal response time setting FAST
#define RFM12_RXCTRL_VDI_MEDIUM     0x100
#define RFM12_RXCTRL_VDI_SLOW       0x200       // VDI (valid data indicator) signal response time setting SLOW
#define RFM12_RXCTRL_VDI_ALWAYS_ON  0x300       // VDI (valid data indicator) allways ON
#define RFM12_RXCTRL_BW_400         0x20        // Receiver baseband bandwidth (BW) select: 400 kHz
#define RFM12_RXCTRL_BW_340         0x40
#define RFM12_RXCTRL_BW_270         0x60
#define RFM12_RXCTRL_BW_200         0x80
#define RFM12_RXCTRL_BW_134         0xA0
#define RFM12_RXCTRL_BW_67          0xC0        // Receiver baseband bandwidth (BW) select: 67 kHz
// 0x00 & 0xE0 - reserved
#define RFM12_RXCTRL_LNA_0          0x00        // LNA gain select relative to maximum [dB]: 0
#define RFM12_RXCTRL_LNA_6          0x08
#define RFM12_RXCTRL_LNA_14         0x10
#define RFM12_RXCTRL_LNA_20         0x18        // LNA gain select relative to maximum [dB]: -20 dB
#define RFM12_RXCTRL_RSSI_103       0x00        // RSSI detector threshold [dBm]: -103
#define RFM12_RXCTRL_RSSI_97        0x01
#define RFM12_RXCTRL_RSSI_91        0x02
#define RFM12_RXCTRL_RSSI_85        0x03
#define RFM12_RXCTRL_RSSI_79        0x04
#define RFM12_RXCTRL_RSSI_73        0x05        // RSSI detector threshold [dBm]: -73
//#define RFM12_RXCTRL_RSSI_67        0x06        // RSSI detector threshold [dBm]: reserved
//#define RFM12_RXCTRL_RSSI_61        0x07        // RSSI detector threshold [dBm]: reserved
//The RSSI threshold depends on the LNA gain, the real RSSI threshold can be calculated: RSSIth=RSSIsetth+GLNA

//=== Data Filter ===
// POR Value: C22C
#define RFM12_CMD_DATAFILTER        0xC228  // Data Filter
#define RFM12_DATAFILTER_AL         0x80    // Clock recovery (CR) auto lock control, 
//  if set. CR will start in fast mode, then after locking it will automatically switch to slow mode.Bit 6 (ml) has no effect.
//  else manual mode, the clock recovery mode is set by Bit 6 (ml)
#define RFM12_DATAFILTER_ML         0x40    // Clock recovery lock control
//  1: fast mode, fast attack and fast release (4 to 8 bit preamble (1010...) is recommended)
//  0: slow mode, slow attack and slow release (12 to 16 bit preamble is recommended)
//  Using the slow mode requires more accurate bit timing (see Data Rate Command).
#define RFM12_DATAFILTER_S          0x10    // Select the type of the data filter: 0 Digital filter/ 1 Analog RC filter
//Digital: This is a digital realization of an analog RC filter followed by a comparator with hysteresis. The time constant is
//automatically adjusted to the bit rate defined by the Data Rate Command.
//Note: Bit rate can not exceed 115 kpbs in this mode.
//Analog RC filter: The demodulator output is fed to pin 7 over a 10 kOhm resistor. The filter cut-off frequency is set by the
//external capacitor connected to this pin and VSS.
#define RFM12_DQD_THRESH_7          0x07    //DQD threshold parameter.
#define RFM12_DQD_THRESH_6          0x06    // Note: To let the DQD report "good signal quality" the 
#define RFM12_DQD_THRESH_5          0x05    //  threshold parameter should be 4 in cases where the bitrate is close to the
#define RFM12_DQD_THRESH_4          0x04    //  deviation. At higher deviation/bitrate settings, a higher threshold parameter 
#define RFM12_DQD_THRESH_3          0x03    //  can report "good signal quality" as well.
#define RFM12_DQD_THRESH_2          0x02    // The DQD parameter can be calculated with the following formula:
#define RFM12_DQD_THRESH_1          0x01    // DQD par = 4 x (deviation – TX-RXoffset ) / bit rate
#define RFM12_DQD_THRESH_0          0x00

//=== FIFO und RESET-Mode ===
// POR Value: CA80
#define RFM12_CMD_FIFORESET         0xCA00
#define RFM12_FIFOITLVL_OFS         4       // Bits 7-4 (f3 to f0): FIFO IT level. The FIFO generates IT when the number of received data bits reaches this level.
#define RFM12_FIFORESET_SP          0x08    // Select the length of the synchron pattern:
//  sp    Byte1       Byte0(POR)  Synchron Pattern (Byte1+Byte0)
//  0     2Dh         D4h         2DD4h
//  1     Not used    D4h         D4h
#define RFM12_FIFORESET_AL          0x04    // Set the input of the FIFO fill start condition: 0-Synchron pattern / 1-Always fill
#define RFM12_FIFORESET_FF          0x02    // FIFO fill will be enabled after synchron pattern reception. The FIFO fill stops when this bit is cleared.
#define RFM12_FIFORESET_DR          0x01    // Disables the highly sensitive RESET mode.
//Note: To restart the synchron pattern recognition, bit 1 (ef, FIFO fill enable) should be cleared and set.

//=== Syncron Pattern ===
// POR Value: CED4
#define RFM12_CMD_SYNCPATTERN 0xCE00

//=== RX FIFO Read ===
// POR Value: B000
#define RFM12_CMD_READ 0xB000

//=== Automatic Frequenz Control ===
// POR Value C4F7
#define RFM12_CMD_AFC           0xC400
#define RFM12_AFC_AUTO_OFF      0x00    // Automatic operation mode selector: Auto mode off (Strobe is controlled by microcontroller)
#define RFM12_AFC_AUTO_ONCE     0x40    // Runs only once after each power-up
#define RFM12_AFC_AUTO_VDI      0x80    // Keep the foffset only during receiving (VDI=high)
#define RFM12_AFC_AUTO_KEEP     0xC0    // Keep the foffset value independently from the state of the VDI signal
#define RFM12_AFC_LIMIT_OFF     0x00    // +31/ -32    fres(315,433 - 2,5 kHz; 868 - 5 kHz; 915 - 7,5 kHz)
#define RFM12_AFC_LIMIT_16      0x10    //+15/-16
#define RFM12_AFC_LIMIT_8       0x20    // +7/-8
#define RFM12_AFC_LIMIT_4       0x30    // +3/-4
#define RFM12_AFC_ST            0x08    // Strobe edge, when st goes to high, the actual latest calculated frequency error is stored into the offset register of the AFC block.
#define RFM12_AFC_FI            0x04    // Switches the circuit to high accuracy (fine) mode. In this case, the processing time is about twice as long, but the measurement uncertainty is about half.
#define RFM12_AFC_OE            0x02    // Enables the frequency offset register
#define RFM12_AFC_EN            0x01    // Enables the calculation of the offset frequency by the AFC circuit.

//=== TX Configuration Control ===
// POR Value: 9800
#define RFM12_CMD_TXCONF        0x9800
#define RFM12_TXCONF_MP         0x100   // Spektr inversion
enum e_RFM12_POWER
{
    RFM12_TXCONF_POWER_0  = 0x00,       // Relative Output Power [0 dBm]
    RFM12_TXCONF_POWER_3  = 0x01,       // Relative Output Power [-3 dBm]
    RFM12_TXCONF_POWER_6  = 0x02,       // Relative Output Power [-6 dBm]
    RFM12_TXCONF_POWER_9  = 0x03,       // Relative Output Power [-9 dBm]
    RFM12_TXCONF_POWER_12 = 0x04,       // Relative Output Power [-12 dBm]
    RFM12_TXCONF_POWER_15 = 0x05,       // Relative Output Power [-15 dBm]
    RFM12_TXCONF_POWER_18 = 0x06,       // Relative Output Power [-18 dBm]
    RFM12_TXCONF_POWER_21 = 0x07        // Relative Output Power [-21 dBm]
};

enum e_RFM12_TXCONF_FS
{
    RFM12_TXCONF_FS_15    = 0x00,       // Frequency shift = 15 kHz
    RFM12_TXCONF_FS_30    = 0x10,
    RFM12_TXCONF_FS_45    = 0x20,
    RFM12_TXCONF_FS_60    = 0x30,
    RFM12_TXCONF_FS_75    = 0x40,
    RFM12_TXCONF_FS_90    = 0x50,
    RFM12_TXCONF_FS_105   = 0x60,
    RFM12_TXCONF_FS_120   = 0x70,
    RFM12_TXCONF_FS_135   = 0x80,
    RFM12_TXCONF_FS_150   = 0x90,
    RFM12_TXCONF_FS_165   = 0xA0,
    RFM12_TXCONF_FS_180   = 0xB0,
    RFM12_TXCONF_FS_195   = 0xC0,
    RFM12_TXCONF_FS_210   = 0xD0,
    RFM12_TXCONF_FS_225   = 0xE0,
    RFM12_TXCONF_FS_240   = 0xF0        // Frequency shift = 240 kHz
};

//=== PLL Setting ===
// POR Value: CC77
#define RFM12_CMD_PLL       0xCC12      // PLL Setting
#define RFM12_CLK_FRQ_HIGH  0x60        // uC CLK Frequency 5/10 MHz
#define RFM12_CLK_FRQ_MED   0x40        // uC CLK Frequency 3,3 MHz
#define RFM12_CLK_FRQ_LOW   0x00        // uC CLK Frequency <=2,5MHz
#define RFM12_PLL_DDY       0x08        // Switches on the delay in the phase detector when this bit is set.
#define RFM12_PLL_DDIT      0x04        // When set, disables the dithering in the PLL loop.
#define RFM12_PLL_BW0       0x01        // PLL bandwidth can be set for optimal TX RF performance.
//    bw0     Max bit rate [kbps]     Phase noise at 1MHz offset [dBc/Hz]
//    0       86.2                    -107
//    1       256                     -102

//=== TX Register Write ===
// POR Value: B8AA
#define RFM12_CMD_TX        0xB800

//=== Wake-Up Timer ===
//Bit-Syntax: 111 | R4 | R3 | R2 | R1 | R0 | M7 | M6 | M5 | M4 | M3 | M2 | M1 | M0
//R = Exponent der Zeit
//M = Zeit
/*
	14. Wake-Up Timer Command
	Bit 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 POR
	1 1 1 r4 r3 r2 r1 r0 m7 m6 m5 m4 m3 m2 m1 m0 E196h
	The wake-up time period can be calculated by (m7 to m0) and (r4 to r0):
	Twake-up = 1.03 * M * 2^R + 0.5 [ms]
	Note:
	• For continual operation the ew bit should be cleared and set at the end of every cycle.
	• For future compatibility, use R in a range of 0 and 29.
*/
// POR Value: E196
#define RFM12_CMD_WAKEUP 0xE000

//=== Low Duty-Cycle ===
//Bestimmt die maximale Sendezeit pro Stunde. Dies ist wichtig, um sich an gesetzliche Frequenzzuteilungsrichtlinien zu halten, 
//  die bestimmen, wie lang jemand mit einer definierten Sendeleistung auf einer bestimmten Frequenz (mit eventuell definierter Betierbsart [Modulationstyp]) senden darf.)
//hex = 0xC800 + Bits
//Bit-Syntax: 1100100 | r| d6 | d5 | d4 | d3 | d2 | d1 | d0 | en
//r =  ??????????????????
//d... = Einschaltdauer während der zyklischen Einschaltung
//en = zyklischen Einschaltung akltivieren
/*
	15. Low Duty-Cycle Command
	Bit 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 POR
	1 1 0 0 1 0 0 0 d6 d5 d4 d3 d2 d1 d0 en C80Eh
	With this command, Low Duty-Cycle operation can be set in order to decrease the average power consumption in receiver mode.
	The time cycle is determined by the Wake-Up Timer Command.
	The Duty-Cycle can be calculated by using (d6 to d0) and M. (M is parameter in a Wake-Up Timer Command.)
	Duty-Cycle= (D * 2 +1) / M *100%
	The on-cycle is automatically extended while DQD indicates good received signal condition (FSK transmission is detected in the
	frequency range determined by Frequency Setting Command plus and minus the baseband filter bandwidth determined by the
	Receiver Control Command).
*/
// POR Value: C80E
#define RFM12_CMD_DUTYCYCLE 0xC800
#define RFM12_DUTYCYCLE_ENABLE 0x01

//=== LowBatt / µC Clock Control ===
//Hex = 0xC000 & xx
//Bit-Syntax: 11000000 | d2 | d1 | d0 | 0 | v3 | v2 | v1 | v0
//d... = Bestimmt den Teilungsfaktor für die Clockausgabe am CLK Pin in Abhängigkeit des Internen SystemTakts. (000=1 / 001=1.25 / 010=1.66 / 011=2 / 100=2.5 / 101=3.33 / 110=5 / 111=10)
//v... = Bestimmt die Minimalspannung, ab der ein Interrupt durchgeführt werden muss. (Ähnlich einer BrownOutDetection). Im Power Managment muss das eb Bit aktiv sein, damit dies funktioniert.
/*
	16. Low Battery Detector and Microcontroller Clock Divider Command
	Bit 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 POR
	1100 000 0 d2 d1 d0  0 v3 v2 v1 v0 C000h
	The 4 bit parameter (v3 to v0) represents the value V, which defines the threshold voltage Vlb of the detector:
	Vlb= 2.25 + V * 0.1 [V]

	The low battery detector and the clock output can be enabled or disabled by bits eb and dc, respectively, using the Power
	Management Command.
*/
// POR Value C000
#define RFM12_CMD_LBDMCD    0xC000
#define RFM12_MCD_DIV10     0x00    // Clock Output 1 MHz
#define RFM12_MCD_DIV8      0x20    // Clock Output 1,25 MHz
#define RFM12_MCD_DIV6      0x40    // Clock Output 1,66 MHz
#define RFM12_MCD_DIV5      0x60    // Clock Output 2 MHz
#define RFM12_MCD_DIV4      0x80    // Clock Output 2,5 MHz
#define RFM12_MCD_DIV3      0xA0    // Clock Output 3,33 MHz
#define RFM12_MCD_DIV2      0xC0    // Clock Output 5 MHz
#define RFM12_MCD_DIV1      0xE0    // Clock Output 10 MHz
#define RFM12_VLB_CALC(d) ((uint8_t)((((d-2.25)*10.0) + 0.5)))

//=== Status Read ===
//Dieses Kommando löst die Rückgabe des Statusregisters aus, welches nach der ersten 0 im ersten Bit syncron übertragen wird.
//Hex = 0x0000
//Bit-Syntax: 0000000000000000<000>
//Rückgabe-Syntax: x0 | x1 | x2 | x3 | x4 | x5 | x6 | x7 | x8 | x9 | x10 | x11 | x12 | x13 | x14 | x15 | x16 | x17 | x18
//x0 -> x5 = Interrupt bits
//x6 -> x15 = Status Bits
//x16 -> x18 = FIFO
//x0 = FFIT / RGIT (RGIT = TX Register ist bereit neue Daten zu senden ... kann mit dem TX Register gelöscht werden)(FFIT = Die anzahl der Datenbits im FIFO Puffer hat das eingestellte Limit erreicht. Kann mit einer der FIFO-Lese methoden gelöscht werden)
//x1 = POR (PowerOnReset)
//x2 = FFOV / RGUR (RGUR = Der Datenstrom beim Senden ist abgerissen, da nicht schnell genug Daten nachgeladen wurden)(FFOV = Der RX FIFO ist übergelaufen)
//x3 = WKUP
//x4 = EXT
//x5 = LBD
//x6 = FFBM (Der FIFO Puffer ist leer)
//x7 = RSSI/ATS (ATS = )(RSSI = Die Signalstärke ist über dem eingestelltem Limit)
//x8 = DQD 
//x9 = CRL 
//x10 = ATGL
//x11 = OFFS_4
//x12 = OFFS_3
//x13 = OFFS_2
//x14 = OFFS_1
//x15 = OFFS_0
//x16 = FO
//x17 = FO+1
//x18 = FO+2
/*
17. Status Read Command
The read command starts with a zero, whereas all other control commands start with a one. If a read command is identified, the
status bits will be clocked out on the SDO pin as follows:

    4	OFFS(6) MSB of the measured frequency offset (sign of the offset value)
    3	OFFS(3) -OFFS(0) Offset value to be added to the value of the frequency control parameter (Four LSB bits)
*/
#define RFM12_CMD_STATUS 	0x0000
#define RFM12_STATUS_RGIT 	0x8000  // RGIT TX register is ready to receive the next byte (Can be cleared by Transmitter Register Write Command)
#define RFM12_STATUS_FFIT 	0x8000  // FFIT The number of data bits in the RX FIFO has reached the pre-programmed limit (Can be cleared by any of the FIFO read methods)
#define RFM12_STATUS_POR 	0x4000  // POR Power-on reset (Cleared after Status Read Command)
#define RFM12_STATUS_RGUR 	0x2000  // RGUR TX register under run, register over write (Cleared after Status Read Command)
#define RFM12_STATUS_FFOV 	0x2000  // FFOV RX FIFO overflow (Cleared after Status Read Command)
#define RFM12_STATUS_WKUP 	0x1000  // WKUP Wake-up timer overflow (Cleared after Status Read Command)
#define RFM12_STATUS_EXT 	0x0800  // EXT Logic level on interrupt pin (pin 16) changed to low (Cleared after Status Read Command)
#define RFM12_STATUS_LBD 	0x0400  // LBD Low battery detect, the power supply voltage is below the pre-programmed limit
#define RFM12_STATUS_FFEM 	0x0200  // FFEM FIFO is empty
#define RFM12_STATUS_ATS 	0x0100  // ATS Antenna tuning circuit detected strong enough RF signal
#define RFM12_STATUS_RSSI 	0x0100  // RSSI The strength of the incoming signal is above the pre-programmed limit
#define RFM12_STATUS_DQD 	0x0080  // DQD Data quality detector output
#define RFM12_STATUS_CRL 	0x0040  // CRL Clock recovery locked
#define RFM12_STATUS_ATGL 	0x0020  // ATGL Toggling in each AFC cycle

#endif
