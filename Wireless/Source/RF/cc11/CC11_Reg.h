/*
Copyright (c) 2011-2012 <comparator@gmx.de>

This file is part of the X13.Home project.
https://github.com/X13home

BSD License
See LICENSE.txt file for license details.
*/

#ifndef _CC11_REG_H
#define _CC11_REG_H

/**
 * Command strobes
 */
#define CC11_SRES                 0x30        // Reset CC1101 chip
#define CC11_SFSTXON              0x31        // Enable and calibrate frequency synthesizer (if MCSM0.FS_AUTOCAL=1). If in RX (with CCA):
                                                // Go to a wait state where only the synthesizer is running (for quick RX / TX turnaround).
#define CC11_SXOFF                0x32        // Turn off crystal oscillator
#define CC11_SCAL                 0x33        // Calibrate frequency synthesizer and turn it off. SCAL can be strobed from IDLE mode without
                                                // setting manual calibration mode (MCSM0.FS_AUTOCAL=0)
#define CC11_SRX                  0x34        // Enable RX. Perform calibration first if coming from IDLE and MCSM0.FS_AUTOCAL=1
#define CC11_STX                  0x35        // In IDLE state: Enable TX. Perform calibration first if MCSM0.FS_AUTOCAL=1.
                                                // If in RX state and CCA is enabled: Only go to TX if channel is clear
#define CC11_SIDLE                0x36        // Exit RX / TX, turn off frequency synthesizer and exit Wake-On-Radio mode if applicable
#define CC11_SWOR                 0x38        // Start automatic RX polling sequence (Wake-on-Radio) as described in Section 19.5 if
                                                // WORCTRL.RC_PD=0
#define CC11_SPWD                 0x39        // Enter power down mode when CSn goes high
#define CC11_SFRX                 0x3A        // Flush the RX FIFO buffer. Only issue SFRX in IDLE or RXFIFO_OVERFLOW states
#define CC11_SFTX                 0x3B        // Flush the TX FIFO buffer. Only issue SFTX in IDLE or TXFIFO_UNDERFLOW states
#define CC11_SWORRST              0x3C        // Reset real time clock to Event1 value
#define CC11_SNOP                 0x3D        // No operation. May be used to get access to the chip status byte

/**
 * Type of transfers
 */
#define CC11_BIT_BURST            0x40
#define CC11_BIT_READ             0x80

/**
 * Type of register
 */
#define CC11_CONFIG_REGISTER      0x80
#define CC11_STATUS_REGISTER      0xC0

/**
 * CC1101 configuration registers
 */
#define CC11_IOCFG2               0x00        // GDO2 Output Pin Configuration
#define CC11_IOCFG1               0x01        // GDO1 Output Pin Configuration
#define CC11_IOCFG0               0x02        // GDO0 Output Pin Configuration
#define CC11_FIFOTHR              0x03        // RX FIFO and TX FIFO Thresholds
#define CC11_SYNC1                0x04        // Sync Word, High Byte, Default: 0xD3
#define CC11_SYNC0                0x05        // Sync Word, Low Byte, Default: 0x91
#define CC11_PKTLEN               0x06        // Packet Length
#define CC11_PKTCTRL1             0x07        // Packet Automation Control
#define CC11_PKTCTRL0             0x08        // Packet Automation Control
#define CC11_ADDR                 0x09        // Device Address
#define CC11_CHANNR               0x0A        // Channel Number
#define CC11_FSCTRL1              0x0B        // Frequency Synthesizer Control
#define CC11_FSCTRL0              0x0C        // Frequency Synthesizer Control
#define CC11_FREQ2                0x0D        // Frequency Control Word, High Byte
#define CC11_FREQ1                0x0E        // Frequency Control Word, Middle Byte
#define CC11_FREQ0                0x0F        // Frequency Control Word, Low Byte
#define CC11_MDMCFG4              0x10        // Modem Configuration
#define CC11_MDMCFG3              0x11        // Modem Configuration
#define CC11_MDMCFG2              0x12        // Modem Configuration
#define CC11_MDMCFG1              0x13        // Modem Configuration
#define CC11_MDMCFG0              0x14        // Modem Configuration
#define CC11_DEVIATN              0x15        // Modem Deviation Setting
#define CC11_MCSM2                0x16        // Main Radio Control State Machine Configuration
#define CC11_MCSM1                0x17        // Main Radio Control State Machine Configuration
#define CC11_MCSM0                0x18        // Main Radio Control State Machine Configuration
#define CC11_FOCCFG               0x19        // Frequency Offset Compensation Configuration
#define CC11_BSCFG                0x1A        // Bit Synchronization Configuration
#define CC11_AGCCTRL2             0x1B        // AGC Control
#define CC11_AGCCTRL1             0x1C        // AGC Control
#define CC11_AGCCTRL0             0x1D        // AGC Control
#define CC11_WOREVT1              0x1E        // High Byte Event0 Timeout
#define CC11_WOREVT0              0x1F        // Low Byte Event0 Timeout
#define CC11_WORCTRL              0x20        // Wake On Radio Control
#define CC11_FREND1               0x21        // Front End RX Configuration
#define CC11_FREND0               0x22        // Front End TX Configuration
#define CC11_FSCAL3               0x23        // Frequency Synthesizer Calibration
#define CC11_FSCAL2               0x24        // Frequency Synthesizer Calibration
#define CC11_FSCAL1               0x25        // Frequency Synthesizer Calibration
#define CC11_FSCAL0               0x26        // Frequency Synthesizer Calibration
#define CC11_RCCTRL1              0x27        // RC Oscillator Configuration
#define CC11_RCCTRL0              0x28        // RC Oscillator Configuration
#define CC11_FSTEST               0x29        // Frequency Synthesizer Calibration Control
#define CC11_PTEST                0x2A        // Production Test
#define CC11_AGCTEST              0x2B        // AGC Test
#define CC11_TEST2                0x2C        // Various Test Settings
#define CC11_TEST1                0x2D        // Various Test Settings
#define CC11_TEST0                0x2E        // Various Test Settings
#define CC11_PATABLE              0x3E        // PATABLE address
#define CC11_TXFIFO               0x3F        // TX FIFO address
#define CC11_RXFIFO               0x3F        // RX FIFO address

/**
 * Status registers
 */
#define CC11_PARTNUM              0x30        // Chip ID
#define CC11_VERSION              0x31        // Chip ID
#define CC11_FREQEST              0x32        // Frequency Offset Estimate from Demodulator
#define CC11_LQI                  0x33        // Demodulator Estimate for Link Quality
#define CC11_RSSI                 0x34        // Received Signal Strength Indication
#define CC11_MARCSTATE            0x35        // Control State Machine State
#define CC11_WORTIME1             0x36        // High Byte of WOR Time
#define CC11_WORTIME0             0x37        // Low Byte of WOR Time
#define CC11_PKTSTATUS            0x38        // Current GDOx Status and Packet Status
#define CC11_VCO_VC_DAC           0x39        // Current Setting from PLL Calibration Module
#define CC11_TXBYTES              0x3A        // Underflow and Number of Bytes in the TX FIFO
#define CC11_RXBYTES              0x3B        // Overflow and Number of Bytes in the RX FIFO
#define CC11_RCCTRL1_STATUS       0x3C        // Last RC Oscillator Calibration Result
#define CC11_RCCTRL0_STATUS       0x3D        // Last RC Oscillator Calibration Result 

/**
* Detailed register description
*/
// General Purpose / Test Output Control Pins configurations
enum e_CC11_IOCFG_GPO
{
    CC11_GDO_SYNC           =   0x06,           // Sync/Packet Send/Received
    CC11_GDO_CCA            =   0x09,           // High when RSSI level is below threshold (dependent on the current CCA_MODE setting).
    CC11_GDO_CS             =   0x0E,           // Carrier sense. High if RSSI level is above threshold. Cleared when entering IDLE mode.
    CC11_GDO_CRC_OK         =   0x0F,           // The last CRC comparison matched. Cleared when entering/restarting RX mode.
    CC11_GDO_PA_PD          =   0x1B,           // Low when transmit is active, low during sleep
    CC11_GDO_LNA_PD         =   0x1C,           // Low when receive is active, low during sleep
    CC11_GDO_CHIP_RDYn      =   0x29,           // Chip Ready
    CC11_GDO_XOSC_STABLE    =   0x2B,
    CC11_GDO_DISABLE        =   0x2E,           // High impedance (3-state)
};

/**
* Register 0x00 - CC11_IOCFG2
* GDO2 Output Pin Configuration
* Default: 0x29 - CHP_RDYn
*/
#define CC11_IOCFG2_GDO2_INV          6       // Invert output, i.e. select active low (1) / high (0)
#define CC11_IOCFG2_GDO2_CFG          0       // Pin Function, from e_CC11_IOCFG_GPO

/**
* Register  0x01 CC11_IOCFG1
* GDO1 Output Pin Configuration
* Default 0x2E - TriState
*/
#define CC11_IOCFG1_GDO_DS            7       // Set high (1) or low (0) output drive strength on the GDO pins.
#define CC11_IOCFG1_GDO1_INV          6       // Invert output, i.e. select active low (1) / high (0)
#define CC11_IOCFG1_GDO1_CFG          0       // Pin Function, from e_CC11_IOCFG_GPO

/**
* Register  0x02 CC11_IOCFG0
* GDO0 Output Pin Configuration
* Default 0x3F - CLK_XOSC/192
*/
#define CC11_IOCFG0_TS_EN             7       // Enable analog temperature sensor. Write 0 in all other register bits.
#define CC11_IOCFG0_GDO0_INV          6       // Invert output, i.e. select active low (1) / high (0)
#define CC11_IOCFG0_GDO0_CFG          0       // Pin Function, from e_CC11_IOCFG_GPO

/**
* Register 0x03: CC11_FIFOTHR
* RX FIFO and TX FIFO Thresholds
* Default: 0x07
*/
#define CC11_FIFOTHR_ADC_RETENTION    6
#define CC11_FIFOTHR_CLOSE_IN_RX      4       //  Rx Attenuation, 0 - dB, 1 - 6dB, 2-12dB, 3 - 18dB
#define CC11_FIFOTHR_FIF0_THR         0       //  Set the threshold for the TX FIFO and RX FIFO

/**
* Register 0x07: CC11_PKTCTRL1
* Packet Automation Control
* Default: 0x04
*/
#define CC11_PKTCTRL1_PQT             5       // Preamble quality estimator threshold
#define CC11_PKTCTRL1_CRC_AUTOFLUSH   3       // Enable automatic flush of RX FIFO when CRC is not OK.
#define CC11_PKTCTRL1_APPEND_STATUS   2       // Appended to the payload RSSI and LQI values
#define CC11_PKTCTRL1_ADR_CHK         0       // Controls address check configuration of received packages

/**
* Register 0x08: CC11_PKTCTRL0
* Packet Automation Control
* Default: 0x45
*/
#define CC11_PKTCTRL0_WHITE_DATA      6       // Turn data whitening on / off
#define CC11_PKTCTRL0_PKT_FORMAT      4       // Format of RX and TX data, Normal Sync, Rand, Async
#define CC11_PKTCTRL0_CRC_EN          2       // CRC calculation in TX and CRC check in RX
#define CC11_PKTCTRL0_LENGTH_CONFIG   0       // Configure the packet length, Fixed, Variable, Infinite

/**
* Register 0x10: CC11_MDMCFG4
* Modem Configuration
* Default: 0x8C
*/
#define CC11_MDMCFG4_CHANBW_E         6
#define CC11_MDMCFG4_CHANBW_M         4
#define CC11_MDMCFG4_DRATE_E          0

/**
* Register 0x12: CC11_MDMCFG2
* Modem Configuration
* Default: 
*/
#define CC11_MDMCFG2_DEM_DCFILT_OFF   7
#define CC11_MDMCFG2_MOD_FORMAT       5
#define CC11_MDMCFG2_MANCHESTER_EN    4
#define CC11_MDMCFG2_SYNC_MODE        0

/**
* Register 0x13: CC11_MDMCFG1
* Modem Configuration
* Default: 
*/
#define CC11_MDMCFG1_FEC_EN           7
#define CC11_MDMCFG1_NUM_PREAMBLE     4
#define CC11_MDMCFG1_SHANSPC_E        0

/**
* Register 0x15: CC11_DEVIATN
* Modem Deviation Setting
* Default: 
*/
#define CC11_DEVIATN_DEVIATION_E      4
#define CC11_DEVIATN_DEVIATION_M      0


/**
* CC11_LQI
* Demodulator Estimate for Link Quality
*/
#define CC11_LQI_CRC_OK                 0x80    // The last CRC comparison matched. Cleared when entering/restarting RX mode.

/**
 * MARC_STATE
 * Main Radio Control FSM State. 
 */
#define CC11_MARCSTATE_SLEEP            0x00
#define CC11_MARCSTATE_IDLE             0x01
#define CC11_MARCSTATE_XOFF             0x02
#define CC11_MARCSTATE_VCOON_MC         0x03
#define CC11_MARCSTATE_REGON_MC         0x04
#define CC11_MARCSTATE_MANCAL           0x05
#define CC11_MARCSTATE_VCOON            0x06
#define CC11_MARCSTATE_REGON            0x07
#define CC11_MARCSTATE_STARTCAL         0x08
#define CC11_MARCSTATE_BWBOOST          0x09
#define CC11_MARCSTATE_FS_LOCK          0x0A
#define CC11_MARCSTATE_IFADCON          0x0B
#define CC11_MARCSTATE_ENDCAL           0x0C
#define CC11_MARCSTATE_RX               0x0D
#define CC11_MARCSTATE_RX_END           0x0E
#define CC11_MARCSTATE_RX_RST           0x0F
#define CC11_MARCSTATE_TXRX_SWITCH      0x10
#define CC11_MARCSTATE_RXFIFO_OVERFLOW  0x11
#define CC11_MARCSTATE_FSTXON           0x12
#define CC11_MARCSTATE_TX               0x13
#define CC11_MARCSTATE_TX_END           0x14
#define CC11_MARCSTATE_RXTX_SWITCH      0x15
#define CC11_MARCSTATE_TXFIFO_UNDERFLOW 0x16

/**
* CC11_PKTSTATUS
* Current GDOx Status and Packet Status Bit Field
*/
#define CC11_PKTSTATUS_CRC_OK           0x80    // The last CRC comparison matched. Cleared when entering/restarting RX mode.
#define CC11_PKTSTATUS_CS               0x40    // Carrier sense. Cleared when entering IDLE mode.
#define CC11_PKTSTATUS_PQT_REACHED      0x20    // Preamble Quality reached. If leaving RX state when this bit is set it will remain asserted until the chip re-enters RX state (MARCSTATE=0x0D). 
                                                // The bit will also be cleared if PQI goes below the programmed PQT value.
#define CC11_PKTSTATUS_CCA              0x10    // Channel is clear
#define CC11_PKTSTATUS_SFD              0x08    // Start of Frame Delimiter. In RX, this bit is asserted when sync word has been received and de-asserted at the end of the packet. 
                                                // It will also deassert when a packet is discarded due to address or maximum length filtering or the radio enters RXFIFO_OVERFLOW state.
                                                // In TX this bit will always read as 0.
#define CC11_PKTSTATUS_GDO2             0x04    // Current non-inverted GDO2 value
#define CC11_PKTSTATUS_GDO0             0x01    // Current non-inverted GDO0 value

#endif  //  _CC11_REG_H
