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

#ifndef _PROTOCOL
	#define _PROTOCOL

	#include <inttypes.h>

	//at least for my TX...
	#define FRSKY_CHANNEL_COUNT			47

	//TX->RX BIND packets
	#define FRSKY_BIND_INX_LENGTH		 0
	#define FRSKY_BIND_INX_ADDR_H		 1
	#define FRSKY_BIND_INX_ADDR_L		 2
	#define FRSKY_BIND_INX_TX_ADDR_H	 3
	#define FRSKY_BIND_INX_TX_ADDR_L	 4
	#define FRSKY_BIND_INX_HTI			 5
	#define FRSKY_BIND_INX_HOP1			 6
	#define FRSKY_BIND_INX_HOP2			 7
	#define FRSKY_BIND_INX_HOP3			 8
	#define FRSKY_BIND_INX_HOP4			 9
	#define FRSKY_BIND_INX_HOP5			10
	//unknown/empty 7 bytes

	#define FRSKY_BIND_CHANNEL			 0
	#define FRSKY_BIND_ADDRESS			 3


	//RSSI and PQI are not in PKT length (added by RX cc2500)
	#define FRSKY_PKT_INX_RSSI(LENGTH)	(LENGTH+1)
	#define FRSKY_PKT_INX_PQI(LENGTH)	(LENGTH+2)


	//packet TX->RX
	#define FRSKY_D_RX_LENGTH			17
	#define FRSKY_D_RX_TOTAL_LENGTH	(FRSKY_D_RX_LENGTH+1+2)//+ 1 (LENGTH field) +2 RSSI and PQI
	#define FRSKY_D_RX_IDX_LENGTH		 0
	#define FRSKY_D_RX_IDX_TX_ADDR_H	 1
	#define FRSKY_D_RX_IDX_TX_ADDR_L	 2
	#define FRSKY_D_RX_IDX_PKT_ID		 3
	#define FRSKY_D_RX_IDX_UNKNOWN_1	 4
	#define FRSKY_D_RX_IDX_UNKNOWN_2	 5
	//ppm data
	#define FRSKY_D_RX_IDX_CH1_LSB		 6
	#define FRSKY_D_RX_IDX_CH2_LSB 	 7
	#define FRSKY_D_RX_IDX_CH3_LSB 	 8
	#define FRSKY_D_RX_IDX_CH4_LSB 	 9
	#define FRSKY_D_RX_IDX_CH12_MSB	10
	#define FRSKY_D_RX_IDX_CH34_MSB	11
	#define FRSKY_D_RX_IDX_CH5_LSB		12
	#define FRSKY_D_RX_IDX_CH6_LSB		13
	#define FRSKY_D_RX_IDX_CH7_LSB		14
	#define FRSKY_D_RX_IDX_CH8_LSB		15
	#define FRSKY_D_RX_IDX_CH56_MSB	16
	#define FRSKY_D_RX_IDX_CH78_MSB	17

	//packet RX->TX
	#define FRSKY_D_TX_LENGTH			17
	#define FRSKY_D_TX_IDX_LENGTH		 0
	#define FRSKY_D_TX_IDX_TX_ADDR_H	 1
	#define FRSKY_D_TX_IDX_TX_ADDR_L	 2
	#define FRSKY_D_TX_IDX_A1			 3
	#define FRSKY_D_TX_IDX_A2			 4
	#define FRSKY_D_TX_IDX_RX_RSSI		 5
	//rest of the packet are telemetry data

	//delay between Rxed packet a Txing start of telemetry packet
	#define FRSKY_D_TX_DELAY			(1.5*SWDT_1ms) //ms

	//TODO: implement external (UART) telemetry https://www.rcgroups.com/forums/showthread.php?2547257-FrSky-telemetry-hub-protocol-(on-air)

	//to easily get higher and lower nibble from byte
	#define FRSKY_EXTRACT_LOWER(A)	(A&0x0F)
	#define FRSKY_EXTRACT_HIGHER(A)	((A>>4)&0x0F)

	//frsky channel data -> pulse length translation for linear aproximation
	#define FRSKY_2ms					3000
	#define FRSKY_1ms					1500

	#define FRSKY_PACKETS_LOST_RESYNC_THRESHOLD	10 //number of consecutive packets lost to wait for total resync
	#define FRSKY_PACKET_TIMEOUT			(10*SWDT_1ms) //ms
	#define FRSKY_CYCLE_TIMEOUT				(480*SWDT_1ms) //ms

	#define FRSKY_RSSI_MAX					230 //RSSI value above which is LNA turmed off
	#define FRSKY_RSSI_MIN					120 //RSSI value below which is LNA turned on

	//exposed functions
	void protocol_frsky_init(void);//initializes cc2500 for FrSky protocol (preambles, modulation, PKT length,...)
	void protocol_frsky_bind(void);
	void protocol_frsky_start(uint8_t force_bind);
	// void protocol_frsky_extract(uint8_t *packet,uint16_t *channel_buffer);

#endif