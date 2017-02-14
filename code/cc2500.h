// Copyright (c) 2017 Patrik Bachan
//
// GNU GENERAL PUBLIC LICENSE
//    Version 3, 29 June 2007
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef _CC2500_LIB
	#define _CC2500_LIB

	#include "config.h"
	#include "misc.h"
	#include <inttypes.h>

	enum cc2500_w_registers
	{

		CC2500_SRES		= 0x30, //Reset chip.
		CC2500_SFSTXON	= 0x31, //Enable and calibrate frequency synthesizer (if MCSM0.FS_AUTOCAL=1). If in RX (with CCA): Go to a wait state where only the synthesizer is running (for quick RX / TX turnaround).
		CC2500_SXOFF	= 0x32, //Turn off crystal oscillator.
		CC2500_SCAL		= 0x33, //Calibrate frequency synthesizer and turn it off. SCAL can be strobed from IDLE mode without setting manual calibration mode (MCSM0.FS_AUTOCAL=0)
		CC2500_SRX		= 0x34, //Enable RX. Perform calibration first if coming from IDLE and MCSM0.FS_AUTOCAL=1.
		CC2500_STX		= 0x35, //In IDLE state: Enable TX. Perform calibration first if MCSM0.FS_AUTOCAL=1. If in RX state and CCA is enabled: Only go to TX if channel is clear.
		CC2500_SIDLE	= 0x36, //Exit RX / TX, turn off frequency synthesizer and exit Wake-On-Radio mode if applicable.
		CC2500_SWOR		= 0x38, //Start automatic RX polling sequence (Wake-on-Radio) as described in Section 19.5 if WORCTRL.RC_PD=0.
		CC2500_SPWD		= 0x39,	 //Enter power down mode when CSn goes high.
		CC2500_SFRX		= 0x3A,	 //Flush the RX FIFO buffer. Only issue SFRX in IDLE or RXFIFO_OVERFLOW states.
		CC2500_SFTX		= 0x3B,	 //Flush the TX FIFO buffer. Only issue SFTX in IDLE or TXFIFO_UNDERFLOW states.
		CC2500_SWORRST	= 0x3C,	 //Reset real time clock to Event1 value.
		CC2500_SNOP		= 0x3D,	 //No operation. May be used to get access to the chip status byte.
	};

	enum cc2500_rw_registers
	{
		CC2500_IOCFG2	= 0x00, 		//GDO2 output pin configuration
		CC2500_IOCFG1	= 0x01, 		//GDO1 output pin configuration
		CC2500_IOCFG0	= 0x02, 		//GDO0 output pin configuration
		CC2500_FIFOTHR	= 0x03, 		//RX FIFO and TX FIFO thresholds
		CC2500_SYNC1	= 0x04, 		//Sync word, high byte
		CC2500_SYNC0	= 0x05, 		//Sync word, low byte
		CC2500_PKTLEN	= 0x06, 		//Packet length
		CC2500_PKTCTRL1	= 0x07, 		//Packet automation control
		CC2500_PKTCTRL0	= 0x08, 		//Packet automation control
		CC2500_ADDR		= 0x09, 		//Device address
		CC2500_CHANNR	= 0x0A, 		//Channel number
		CC2500_FSCTRL1	= 0x0B, 		//Frequency synthesizer control
		CC2500_FSCTRL0	= 0x0C, 		//Frequency synthesizer control
		CC2500_FREQ2	= 0x0D, 		//Frequency control word, high byte
		CC2500_FREQ1	= 0x0E, 		//Frequency control word, middle byte
		CC2500_FREQ0	= 0x0F, 		//Frequency control word, low byte
		CC2500_MDMCFG4	= 0x10, 		//Modem configuration
		CC2500_MDMCFG3	= 0x11, 		//Modem configuration
		CC2500_MDMCFG2	= 0x12, 		//Modem configuration
		CC2500_MDMCFG1	= 0x13, 		//Modem configuration
		CC2500_MDMCFG0	= 0x14, 		//Modem configuration
		CC2500_DEVIATN	= 0x15, 		//Modem deviation setting
		CC2500_MCSM2	= 0x16, 		//Main Radio Control State Machine configuration
		CC2500_MCSM1	= 0x17, 		//Main Radio Control State Machine configuration
		CC2500_MCSM0	= 0x18, 		//Main Radio Control State Machine configuration
		CC2500_FOCCFG	= 0x19, 		//Frequency Offset Compensation configuration
		CC2500_BSCFG	= 0x1A, 		//Bit Synchronization configuration
		CC2500_AGCTRL2	= 0x1B, 		//AGC control
		CC2500_AGCTRL1	= 0x1C, 		//AGC control
		CC2500_AGCTRL0	= 0x1D, 		//AGC control
		CC2500_WOREVT1	= 0x1E, 		//High byte Event 0 timeout
		CC2500_WOREVT0	= 0x1F, 		//Low byte Event 0 timeout
		CC2500_WORCTRL	= 0x20, 		//Wake On Radio control
		CC2500_FREND1	= 0x21, 		//Front end RX configuration
		CC2500_FREND0	= 0x22, 		//Front end TX configuration
		CC2500_FSCAL3	= 0x23, 		//Frequency synthesizer calibration
		CC2500_FSCAL2	= 0x24, 		//Frequency synthesizer calibration
		CC2500_FSCAL1	= 0x25, 		//Frequency synthesizer calibration
		CC2500_FSCAL0	= 0x26, 		//Frequency synthesizer calibration
		CC2500_RCCTRL1	= 0x27, 		//RC oscillator configuration
		CC2500_RCCTRL0	= 0x28, 		//RC oscillator configuration
		CC2500_FSTEST	= 0x29, 		//Frequency synthesizer calibration control
		CC2500_PTEST	= 0x2A, 		//Production test
		CC2500_AGCTEST	= 0x2B, 		//AGC test
		CC2500_TEST2	= 0x2C, 		//Various test settings
		CC2500_TEST1	= 0x2D, 		//Various test settings
		CC2500_TEST0	= 0x2E, 		//Various test settings
		CC2500_PATABLE	= 0x3E,			//table of 8 values
		CC2500_FIFO		= 0x3F,			//FIFO access
	};

	enum cc2500_r_registers
	{
		CC2500_PARTNUM	= 0xF0, //CC2500 part number
		CC2500_VERSION	= 0xF1, //Current version number
		CC2500_FREQEST	= 0xF2, //Frequency offset estimate
		CC2500_LQI		= 0xF3, //Demodulator estimate for Link Quality
		CC2500_RSSI		= 0xF4, //Received signal strength indication
		CC2500_MARCSTATE= 0xF5, //Control state machine state
		CC2500_WORTIME1	= 0xF6, //High byte of WOR timer
		CC2500_WORTIME0	= 0xF7, //Low byte of WOR timer
		CC2500_PKTSTATUS = 0xF8, //Current GDOx status and packet status
		CC2500_VCO_VC_DAC = 0xF9, //Current setting from PLL calibration module
		CC2500_TXBYTES	= 0xFA, //Underflow and number of bytes in the TX FIFO
		CC2500_RXBYTES	= 0xFB, //Overflow and number of bytes in the RX FIFO
		CC2500_RCCTRL1_STATUS = 0xFC, //Last RC oscillator calibration result
		CC2500_RCCTRL0_STATUS = 0xFD, //Last RC oscillator calibration result
	};















	#define CC2500_ACCESS_READ		0x80
	#define CC2500_ACCESS_WRITE		0x00
	#define CC2500_ACCESS_BURST		0x40

	#define FIFOTHR_FIFO_THR_pos			0
	#define FIFOTHR_FIFO_THR_msk			(0xF<<FIFOTHR_FIFO_THR_pos)

	#define FIFOTHR_FIFO_THR_TX_61_RX_4		(0<<FIFOTHR_FIFO_THR_pos)
	#define FIFOTHR_FIFO_THR_TX_57_RX_8		(1<<FIFOTHR_FIFO_THR_pos)
	#define FIFOTHR_FIFO_THR_TX_53_RX_12	(2<<FIFOTHR_FIFO_THR_pos)
	#define FIFOTHR_FIFO_THR_TX_49_RX_16	(3<<FIFOTHR_FIFO_THR_pos)
	#define FIFOTHR_FIFO_THR_TX_45_RX_20	(4<<FIFOTHR_FIFO_THR_pos)
	#define FIFOTHR_FIFO_THR_TX_41_RX_24	(5<<FIFOTHR_FIFO_THR_pos)
	#define FIFOTHR_FIFO_THR_TX_37_RX_28	(6<<FIFOTHR_FIFO_THR_pos)
	#define FIFOTHR_FIFO_THR_TX_33_RX_32	(7<<FIFOTHR_FIFO_THR_pos)
	#define FIFOTHR_FIFO_THR_TX_29_RX_36	(8<<FIFOTHR_FIFO_THR_pos)
	#define FIFOTHR_FIFO_THR_TX_25_RX_40	(9<<FIFOTHR_FIFO_THR_pos)
	#define FIFOTHR_FIFO_THR_TX_21_RX_44	(10<<FIFOTHR_FIFO_THR_pos)
	#define FIFOTHR_FIFO_THR_TX_17_RX_48	(11<<FIFOTHR_FIFO_THR_pos)
	#define FIFOTHR_FIFO_THR_TX_13_RX_52	(12<<FIFOTHR_FIFO_THR_pos)
	#define FIFOTHR_FIFO_THR_TX_9_RX_56		(13<<FIFOTHR_FIFO_THR_pos)
	#define FIFOTHR_FIFO_THR_TX_5_RX_60		(14<<FIFOTHR_FIFO_THR_pos)
	#define FIFOTHR_FIFO_THR_TX_1_RX_64		(15<<FIFOTHR_FIFO_THR_pos)


	#define PKTCTRL1_ADD_CHK_pos			0
	#define PKTCTRL1_ADD_CHK_msk			(0x3<<PKTCTRL1_ADD_CHK_pos)

	#define PKTCTRL1_APPEND_STATUS_pos		2
	#define PKTCTRL1_APPEND_STATUS_msk		(0x1<<PKTCTRL1_APPEND_STATUS_pos)
		#define PKTCTRL1_APPEND_STATUS			(PKTCTRL1_APPEND_STATUS_msk)

	#define PKTCTRL1_CRC_AUTOFLUSH_pos		3
	#define PKTCTRL1_CRC_AUTOFLUSH_msk		(0x1<<PKTCTRL1_CRC_AUTOFLUSH_pos)
		#define PKTCTRL1_CRC_AUTOFLUSH			(PKTCTRL1_CRC_AUTOFLUSH_msk)

	#define PKTCTRL1_PQT_pos				5
	#define PKTCTRL1_PQT_msk				(0x7<<PKTCTRL1_PQT_pos)

	#define PKTCTRL0_LENGTH_CONFIG_pos		0
	#define PKTCTRL0_LENGTH_CONFIG_msk		(0x3<<PKTCTRL0_LENGTH_CONFIG_pos)
		#define PKTCTRL0_LENGTH_CONFIG_FIXED	(0<<PKTCTRL0_LENGTH_CONFIG_pos)
		#define PKTCTRL0_LENGTH_CONFIG_VARIABLE	(1<<PKTCTRL0_LENGTH_CONFIG_pos)
		#define PKTCTRL0_LENGTH_CONFIG_INFINITE	(2<<PKTCTRL0_LENGTH_CONFIG_pos)

	#define PKTCTRL0_CRC_EN_pos				2
	#define PKTCTRL0_CRC_EN_msk				(0x1<<PKTCTRL0_CRC_EN_pos)
		#define PKTCTRL0_CRC_EN					(PKTCTRL0_CRC_EN_msk)

	#define PKTCTRL0_CC2400_EN_pos			3
	#define PKTCTRL0_CC2400_EN_msk			(0x1<<PKTCTRL0_CC2400_EN_pos)
		#define PKTCTRL0_CC2400_EN				(PKTCTRL0_CC2400_EN_msk)

	#define PKTCTRL0_PKT_FORMAT_pos			4
	#define PKTCTRL0_PKT_FORMAT_msk			(0x3<<PKTCTRL0_PKT_FORMAT_pos)
		#define PKTCTRL0_PKT_FORMAT_NORMAL		(0<<PKTCTRL0_PKT_FORMAT_pos)
		#define PKTCTRL0_PKT_FORMAT_SYNC		(1<<PKTCTRL0_PKT_FORMAT_pos)
		#define PKTCTRL0_PKT_FORMAT_RANDOM		(2<<PKTCTRL0_PKT_FORMAT_pos)
		#define PKTCTRL0_PKT_FORMAT_ASYNC		(3<<PKTCTRL0_PKT_FORMAT_pos)

	#define PKTCTRL0_WHITE_DATA_pos			6
	#define PKTCTRL0_WHITE_DATA_msk			(0x1<<PKTCTRL0_WHITE_DATA_pos)
		#define PKTCTRL0_WHITE_DATA				(PKTCTRL0_WHITE_DATA_msk)



	#define FSCTRL1_FREQ_IF_pos				0
	#define FSCTRL1_FREQ_IF_msk				(0xF<<FSCTRL1_FREQ_IF_pos)


	#define MDMCFG4_DRATE_E_pos				0
	#define MDMCFG4_DRATE_E_msk				(0xF<<MDMCFG4_DRATE_E_pos)

	#define MDMCFG4_CHANBW_M_pos			4
	#define MDMCFG4_CHANBW_M_msk			(0x3<<MDMCFG4_CHANBW_M_pos)

	#define MDMCFG4_CHANBW_E_pos			6
	#define MDMCFG4_CHANBW_E_msk			(0x3<<MDMCFG4_CHANBW_E_pos)

	#define MDMCFG3_DRATE_M_pos				0
	#define MDMCFG3_DRATE_M_msk				(0xFF<<MDMCFG3_DRATE_M_pos)


	#define MDMCFG2_SYNC_MODE_pos			0
	#define MDMCFG2_SYNC_MODE_msk			(0x7<MDMCFG2_SYNC_MODE_pos)
		#define MDMCFG2_SYNC_MODE_NOSYNC	(0<<MDMCFG2_SYNC_MODE_pos)
		#define MDMCFG2_SYNC_MODE_15_16		(1<<MDMCFG2_SYNC_MODE_pos)
		#define MDMCFG2_SYNC_MODE_16_16		(2<<MDMCFG2_SYNC_MODE_pos)
		#define MDMCFG2_SYNC_MODE_30_32		(3<<MDMCFG2_SYNC_MODE_pos)
		#define MDMCFG2_SYNC_MODE_NOSYNC_CS	(4<<MDMCFG2_SYNC_MODE_pos)
		#define MDMCFG2_SYNC_MODE_15_16_CS	(5<<MDMCFG2_SYNC_MODE_pos)
		#define MDMCFG2_SYNC_MODE_16_16_CS	(6<<MDMCFG2_SYNC_MODE_pos)
		#define MDMCFG2_SYNC_MODE_30_32_CS	(7<<MDMCFG2_SYNC_MODE_pos)

	#define MDMCFG2_MANCHESTER_EN_pos		3
	#define MDMCFG2_MANCHESTER_EN_msk		(0x1<MDMCFG2_MANCHESTER_EN_pos)
		#define MDMCFG2_MANCHESTER_EN		(MDMCFG2_MANCHESTER_EN_msk)

	#define MDMCFG2_MOD_FORMAT_pos			4
	#define MDMCFG2_MOD_FORMAT_msk			(0x7<MDMCFG2_MOD_FORMAT_pos)
		#define MDMCFG2_MOD_FORMAT_2FSK		(0<<MDMCFG2_MOD_FORMAT_pos)
		#define MDMCFG2_MOD_FORMAT_GFSK		(1<<MDMCFG2_MOD_FORMAT_pos)
		#define MDMCFG2_MOD_FORMAT_OOK		(3<<MDMCFG2_MOD_FORMAT_pos)
		#define MDMCFG2_MOD_FORMAT_MSK		(7<<MDMCFG2_MOD_FORMAT_pos)

	#define MDMCFG2_DEM_DCFILT_OFF_pos		7
	#define MDMCFG2_DEM_DCFILT_OFF_msk		(0x1<MDMCFG2_DEM_DCFILT_OFF_pos)
		#define MDMCFG2_DEM_DCFILT_OFF		(MDMCFG2_DEM_DCFILT_OFF_msk)

	#define MDMCFG1_FEC_EN_pos				7
	#define MDMCFG1_FEC_EN_msk				(0x01<MDMCFG1_FEC_EN_pos)
		#define MDMCFG1_FEC_EN				(MDMCFG1_FEC_EN_msk)

	#define MDMCFG1_NUM_PREAMBLE_pos		4
	#define MDMCFG1_NUM_PREAMBLE_msk		(0x07<MDMCFG1_NUM_PREAMBLE_pos)
		#define MDMCFG1_NUM_PREAMBLE_2		(0<<MDMCFG1_NUM_PREAMBLE_pos)
		#define MDMCFG1_NUM_PREAMBLE_3		(1<<MDMCFG1_NUM_PREAMBLE_pos)
		#define MDMCFG1_NUM_PREAMBLE_4		(2<<MDMCFG1_NUM_PREAMBLE_pos)
		#define MDMCFG1_NUM_PREAMBLE_6		(3<<MDMCFG1_NUM_PREAMBLE_pos)
		#define MDMCFG1_NUM_PREAMBLE_8		(4<<MDMCFG1_NUM_PREAMBLE_pos)
		#define MDMCFG1_NUM_PREAMBLE_12		(5<<MDMCFG1_NUM_PREAMBLE_pos)
		#define MDMCFG1_NUM_PREAMBLE_16		(6<<MDMCFG1_NUM_PREAMBLE_pos)
		#define MDMCFG1_NUM_PREAMBLE_24		(7<<MDMCFG1_NUM_PREAMBLE_pos)

	#define MDMCFG1_CHANSPC_E_pos			0
	#define MDMCFG1_CHANSPC_E_msk			(0x03<MDMCFG1_CHANSPC_E_pos)

	#define MDMCFG0_CHANSPC_M_pos			0
	#define MDMCFG0_CHANSPC_M_msk			(0xFF<<MDMCFG0_CHANSPC_M_pos)

	#define DEVIATN_DEVIATION_E_pos			4
	#define DEVIATN_DEVIATION_E_msk			(0x07<DEVIATN_DEVIATION_E_pos)

	#define DEVIATN_DEVIATION_M_pos			0
	#define DEVIATN_DEVIATION_M_msk			(0x07<DEVIATN_DEVIATION_M_pos)



	#define BSCFG_BS_PRE_KI_pos				6
	#define BSCFG_BS_PRE_KI_msk				(0x03<<BSCFG_BS_PRE_KI_pos)

	#define BSCFG_BS_PRE_KP_pos				4
	#define BSCFG_BS_PRE_KP_msk				(0x03<<BSCFG_BS_PRE_KP_pos)

	#define BSCFG_BS_POST_KI_pos			3
	#define BSCFG_BS_POST_KI_msk			(0x01<<BSCFG_BS_POST_KI_pos)

	#define BSCFG_BS_POST_KP_pos			2
	#define BSCFG_BS_POST_KP_msk			(0x01<<BSCFG_BS_POST_KP_pos)

	#define BSCFG_BS_LIMIT_pos				0
	#define BSCFG_BS_LIMIT_msk				(0x03<<BSCFG_BS_LIMIT_pos)


	#define AGCCTRL2_MAX_DVGA_GAIN_pos		6
	#define AGCCTRL2_MAX_DVGA_GAIN_msk		(0x02<<AGCCTRL2_MAX_DVGA_GAIN_pos)

	#define AGCCTRL2_MAX_LNA_GAIN_pos		3
	#define AGCCTRL2_MAX_LNA_GAIN_msk		(0x07<<AGCCTRL2_MAX_LNA_GAIN_pos)

	#define AGCCTRL2_MAGN_TARGET_pos		0
	#define AGCCTRL2_MAGN_TARGET_msk		(0x07<<AGCCTRL2_MAGN_TARGET_pos)


	#define AGCCTRL1_AGC_LNA_PRIORITY_pos		6
	#define AGCCTRL1_AGC_LNA_PRIORITY_msk		(0x01<<AGCCTRL1_AGC_LNA_PRIORITY_pos)
		#define AGCCTRL1_AGC_LNA_PRIORITY		(AGCCTRL1_AGC_LNA_PRIORITY_msk)

	#define AGCCTRL1_CARRIER_SENSE_REL_THR_pos	4
	#define AGCCTRL1_CARRIER_SENSE_REL_THR_msk	(0x03<<AGCCTRL1_CARRIER_SENSE_REL_THR_pos)

	#define AGCCTRL1_CARRIER_SENSE_ABS_THR_pos	0
	#define AGCCTRL1_CARRIER_SENSE_ABS_THR_msk	(0x0F<<AGCCTRL1_CARRIER_SENSE_ABS_THR_pos)

	#define AGCCTRL0_HYST_LEVEL_pos			6
	#define AGCCTRL0_HYST_LEVEL_msk			(0x03<<AGCCTRL0_HYST_LEVEL_pos)

	#define AGCCTRL0_WAIT_TIME_pos			4
	#define AGCCTRL0_WAIT_TIME_msk			(0x03<<AGCCTRL0_WAIT_TIME_pos)

	#define AGCCTRL0_AGC_FREEZE_pos			2
	#define AGCCTRL0_AGC_FREEZE_msk			(0x03<<AGCCTRL0_AGC_FREEZE_pos)

	#define AGCCTRL0_FILTER_LENGTH_pos		0
	#define AGCCTRL0_FILTER_LENGTH_msk		(0x03<<AGCCTRL0_FILTER_LENGTH_pos)




	// 3:0 FSCAL3[3:0]
	#define FSCAL3_CHP_CURR_CAL_EN_pos		4
	#define FSCAL3_CHP_CURR_CAL_EN_msk		(0x3<<FSCAL3_CHP_CURR_CAL_EN_pos)
	// 7:6 FSCAL3[7:6]

	#define MCSM1_TXOFF_MODE_pos			0
	#define MCSM1_TXOFF_MODE_msk			(0x03<<MCSM1_TXOFF_MODE_pos)
		#define MCSM1_TXOFF_MODE_IDLE		(0<<MCSM1_TXOFF_MODE_pos)
		#define MCSM1_TXOFF_MODE_FSTXON		(1<<MCSM1_TXOFF_MODE_pos)
		#define MCSM1_TXOFF_MODE_TX			(2<<MCSM1_TXOFF_MODE_pos)
		#define MCSM1_TXOFF_MODE_RX			(3<<MCSM1_TXOFF_MODE_pos)

	#define MCSM1_RXOFF_MODE_pos			2
	#define MCSM1_RXOFF_MODE_msk			(0x03<<MCSM1_RXOFF_MODE_pos)
		#define MCSM1_RXOFF_MODE_IDLE		(0<<MCSM1_RXOFF_MODE_pos)
		#define MCSM1_RXOFF_MODE_FSTXON		(1<<MCSM1_RXOFF_MODE_pos)
		#define MCSM1_RXOFF_MODE_TX			(2<<MCSM1_RXOFF_MODE_pos)
		#define MCSM1_RXOFF_MODE_RX			(3<<MCSM1_RXOFF_MODE_pos)

	#define MCSM1_CCA_MODE_pos				4
	#define MCSM1_CCA_MODE_msk				(0x03<<MCSM1_CCA_MODE_pos)

	#define MCSM0_XOSC_FORCE_ON_pos			0
	#define MCSM0_XOSC_FORCE_ON_msk			(0x01<<MCSM0_XOSC_FORCE_ON_pos)
		#define MCSM0_XOSC_FORCE_ON			(MCSM0_XOSC_FORCE_ON_msk)

	#define MCSM0_PIN_CTRL_EN_pos			1
	#define MCSM0_PIN_CTRL_EN_msk			(0x01<<MCSM0_PIN_CTRL_EN_pos)
		#define MCSM0_PIN_CTRL_EN			(MCSM0_PIN_CTRL_EN_msk)

	#define MCSM0_PO_TIMEOUT_pos			2
	#define MCSM0_PO_TIMEOUT_msk			(0x03<<MCSM0_PO_TIMEOUT_pos)

	#define MCSM0_FS_AUTOCAL_pos			4
	#define MCSM0_FS_AUTOCAL_msk			(0x03<<MCSM0_FS_AUTOCAL_pos)

	//TODO:rest of registers...

	union cc2500_status
	{
		struct
		{
			uint8_t FIFO_BYTES_AVALABLE:4;
			uint8_t STATE:3;
			uint8_t CHIP_RDy:1;
		}bits;
		uint8_t byte;
	};

	enum edge{RISING, FALLING};
	enum gdo{GDO0, GDO2};
	enum gdo_filter{NONE,NOT_EMPTY};

	extern volatile union cc2500_status cc2500_status;
	uint8_t cc2500_read_reg(uint8_t address);
	void cc2500_read_reg_burst(uint8_t address, uint8_t length, uint8_t *destination);
	void cc2500_write_reg(uint8_t address, uint8_t data);
	void cc2500_write_reg_burst(uint8_t address, uint8_t length, uint8_t *source);
	void cc2500_strobe(uint8_t address);
	void cc2500_ex_pa(uint8_t state);
	void cc2500_ex_lna(uint8_t state);
	void cc2500_mode_tx(void);
	void cc2500_mode_rx(uint8_t LNA);
	uint8_t cc2500_read_fifo(uint8_t *data);
	void cc2500_write_fifo(uint8_t *data, uint8_t length);
	void cc2500_set_channel(uint8_t channel);
	void cc2500_reset_callback(enum gdo gdo);
	void cc2500_set_callback(enum gdo gdo, enum edge edge, void(*callback)(void), uint8_t filter);

	inline uint8_t cc2500_read_GDO0(void)
	{
		return (bit_get(GPIOB->IDR,BIT(8))>>8);
	}

	inline uint8_t cc2500_read_GDO2(void)
	{
		return (bit_get(GPIOB->IDR,BIT(9))>>9);
	}

#endif