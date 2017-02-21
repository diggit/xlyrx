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

//most protocol knowledge was gathered from https://www.rcgroups.com/forums/showthread.php?1667453-DIY-FrSky-RX
//thank you guys!

#include "protocol.h"
#include "cc2500.h"
#include "misc.h"
#include "flash.h"
#include "uart.h"
#include "timers.h"
#include "adc.h"

uint8_t frsky_channel_data[FRSKY_CHANNEL_COUNT][4];//channel, FSCAL3, FSCAL2, FSCAL1

union frsky_id
{
	uint16_t id;
	struct
	{
		uint8_t low;
		uint8_t high;
	}nibble;
};

union frsky_id frsky_tx_id={.id=0};//set default ID to 0, means not yet set
//in fact, this id is id of link between RX and TX, TX requires same ID in telemetry packets


//OFFSET CALIBRATION

uint8_t *offset_cal_table;
volatile int8_t offset_cal_table_idx;

volatile enum offset_cal_status
{
	FRSKY_OFFSET_CAL_FINISHED,
	FRSKY_OFFSET_CAL_WAITING,
	FRSKY_OFFSET_CAL_START,
	FRSKY_OFFSET_CAL_MEASURING,
} offset_cal_status;

void protocol_frsky_calibrate_offset_next(void)
{
	swdt_stop();
	if(offset_cal_status==FRSKY_OFFSET_CAL_START)
	{
		//first packet was received
		offset_cal_table_idx=INT8_MIN;
		offset_cal_status=FRSKY_OFFSET_CAL_MEASURING;
		uart_send_string_blocking("TX found, searching frequency offset...\n");
	}
	else
	{
		if(offset_cal_table_idx<INT8_MAX)
		{
			offset_cal_table_idx++;
			cc2500_write_reg(CC2500_FSCTRL0, (uint8_t)offset_cal_table_idx);
		}
		else if(offset_cal_status==FRSKY_OFFSET_CAL_MEASURING)//end and we were measuring
		{
			swdt_stop();
			cc2500_reset_callback(GDO0);
			offset_cal_status=FRSKY_OFFSET_CAL_FINISHED;
			cc2500_strobe(CC2500_SIDLE);
			return;
		}
		else//end of sweep cycle, but we have not found TX yet
		{
			offset_cal_table_idx=INT8_MIN;
		}
	}
	cc2500_strobe(CC2500_SIDLE);
	cc2500_write_reg(CC2500_FSCTRL0, (uint8_t)offset_cal_table_idx);
	cc2500_strobe(CC2500_SCAL);
	delay_ms(1);
	cc2500_strobe(CC2500_SRX);
	swdt_restart(30*SWDT_1ms);//some reasonable time in which valid packed should appear, when offset is close to ideal
}

void protocol_frsky_calibrate_offset_rxed_callback(void)
{
	uint8_t packet[64];
	uint8_t bytes = cc2500_read_fifo(packet);
	if(offset_cal_status==FRSKY_OFFSET_CAL_WAITING)
	{
		offset_cal_status=FRSKY_OFFSET_CAL_START;//finally some packet arrived
	}
	else
	{
		offset_cal_table[(uint8_t)offset_cal_table_idx]=packet[FRSKY_PKT_INX_RSSI(packet[FRSKY_BIND_INX_LENGTH])];
		uart_send_string_blocking("OFS: ");
		uart_send_string_blocking(itoa(offset_cal_table_idx,4));
		uart_send_string_blocking(" RSSI: ");
		uart_send_string_blocking(itoa(offset_cal_table[(uint8_t)offset_cal_table_idx],4));
		for(uint8_t i=0; i<bytes; i++)
		{
			uart_send_string_blocking(itoa(packet[i],5));
		}
		uart_send_byte_blocking('\n');
	}
	protocol_frsky_calibrate_offset_next();
}

void protocol_frsky_calibrate_offset_timeout_callback(void)//nothing received with set offset, just move on...
{
	offset_cal_table[(uint8_t)offset_cal_table_idx]=0;
	protocol_frsky_calibrate_offset_next();
}

void protocol_frsky_calibrate_offset_start(void)
{
	uart_send_string_blocking("searching for TX frequency offset...\n");
	uint8_t table[256];
	offset_cal_table_idx=INT8_MIN;
	offset_cal_table=table;
	offset_cal_status=FRSKY_OFFSET_CAL_WAITING;

	//disable both amps, we are close
	cc2500_ex_pa(0);
	cc2500_ex_lna(0);

	cc2500_strobe(CC2500_SIDLE);
	//TODO? use pin mode 0x01
	cc2500_write_reg(CC2500_IOCFG0, 0x06);//GDO to indicate packet processing end
	cc2500_write_reg(CC2500_ADDR, FRSKY_BIND_ADDRESS );//address in bind mode
	cc2500_set_channel(FRSKY_BIND_CHANNEL);

	uint8_t MDMCFG4_backup = cc2500_read_reg(CC2500_MDMCFG4);//backup seetings
	cc2500_write_reg(CC2500_MDMCFG4, 0xCA);//decrease channel width

	uint8_t FOCCFG_backup = cc2500_read_reg(CC2500_FOCCFG);//backup
	cc2500_write_reg(CC2500_FOCCFG, 0x00);//disable Freq offset autocal

	cc2500_strobe(CC2500_SCAL);
	delay_ms(1);//I hope it is enough, polling MARCSTATE fro IDLE state is the other way
	cc2500_set_callback(GDO0, FALLING, protocol_frsky_calibrate_offset_rxed_callback, NOT_EMPTY);//set
	cc2500_strobe(CC2500_SRX);
	swdt_set_callback(protocol_frsky_calibrate_offset_timeout_callback);
	swdt_restart(30*SWDT_1ms);

	while(offset_cal_status!=FRSKY_OFFSET_CAL_FINISHED)
		NOP;

	cc2500_write_reg(CC2500_FOCCFG, 0x00);

	//find band where rssi is above threshold and set offset of middle of this band

	uint8_t max=0;

	//find max RSSI
	for(int8_t o=INT8_MIN; o<INT8_MAX; o++)
		if(offset_cal_table[(uint8_t)o]>max)
			max=offset_cal_table[(uint8_t)o];

	uart_send_string_blocking("MAX RSSI: ");
	uart_send_string_blocking(itoa(max,3));

	uart_send_byte_blocking('\n');

	max=max/2;//set threshold to wind band of acceptable RSSIs

	//locate first RSSI above threshold from bottom
	int8_t bottom, top, mid;
	for(bottom=INT8_MIN; offset_cal_table[(uint8_t)bottom]<max; bottom++)
		NOP;
	uart_send_string_blocking("bottom: ");
	uart_send_string_blocking(itoa(bottom,3));
	uart_send_byte_blocking('\n');

	//locate first RSSI above threshold from top
	for(top=INT8_MAX; offset_cal_table[(uint8_t)top]<max; top--)
		NOP;
	uart_send_string_blocking("top: ");
	uart_send_string_blocking(itoa(top,3));
	uart_send_byte_blocking('\n');//CAL measure cycle end

	//calculate mid of this band
	mid=(top-bottom)/2+bottom;

	uart_send_string_blocking("mid: ");
	uart_send_string_blocking(itoa(mid,3));
	uart_send_string_blocking(" RSSI: ");
	uart_send_string_blocking(itoa(offset_cal_table[(uint8_t)mid],3));
	uart_send_byte_blocking('\n');

	cc2500_write_reg(CC2500_FSCTRL0,offset_cal_table[(uint8_t)mid]);//set offset we calculated as best

	cc2500_write_reg(CC2500_MDMCFG4, MDMCFG4_backup);//restore register contents
	cc2500_write_reg(CC2500_FOCCFG, FOCCFG_backup);//restore register contents
}

//config storing

//we basically use last FLASH page (1kiB) to store our binding data
int8_t protocol_frsky_write_nvm(void)
{
	uart_send_string_blocking("SAVING CONFIG...\n");
	uint16_t *destination=(uint16_t*)FLASH_PAGE_ADDR(FLASH_PAGE_LAST);
	flash_unlock();
	flash_page_erase(destination);//erase is necessary to be able to write new data
	//some header
	flash_write(destination++, MEM_MAGIC);
	flash_write(destination++, MEM_VERSION);
	//TX ID
	flash_write(destination++, frsky_tx_id.id);
	//frequency offset
	flash_write(destination++, cc2500_read_reg(CC2500_FSCTRL0));
	//hop table
	for(uint8_t ch=0; ch<FRSKY_CHANNEL_COUNT; ch++)
	{
		flash_write(destination++,frsky_channel_data[ch][0]);//yep bytes and we store them to words, and what.... :D (this could be optimized)
	}
	return 0;
}

int8_t protocol_frsky_read_nvm(void)
{
	uint16_t buffer;

	uart_send_string_blocking("LOADING...\n");
	uint16_t *destination=(uint16_t*)FLASH_PAGE_ADDR(FLASH_PAGE_LAST);
	//some header
	buffer=*destination++;
	if(buffer!=MEM_MAGIC)
	{
		uart_send_string_blocking("FLASH contents not recognized...\n");
		return -1;
	}
	buffer=*destination++;
	if(buffer!=MEM_VERSION)
	{
		uart_send_string_blocking("FLASH contents version mismatch...\n");
		return -2;
	}
	//TX ID
	frsky_tx_id.id=*destination++;
	//frequency offset
	cc2500_write_reg(CC2500_FSCTRL0, *destination++);
	//hop table
	for(uint8_t ch=0; ch<FRSKY_CHANNEL_COUNT; ch++)
	{
		frsky_channel_data[ch][0]=*destination++;
	}
	return 0;
}


//BINDING

/*
(HTI - Hop Table Index, RI - RSSI)
LEN|ADR ID|TX  ID |HTI| h1| h2| h2| h4| h5| NUL|NUL|NUL|NUL|NUL|NUL| ?| RI| PQI
17   3   1 168 237   0   0 145  55 200 110   0   0   0   0   0   0  34  60 195
17   3   1 168 237   5  20 165  75 221 130   0   0   0   0   0   0  34  59 204
17   3   1 168 237  10  40 185  95   5 150   0   0   0   0   0   0  34  59 201
17   3   1 168 237  15  60 205 115  25 170   0   0   0   0   0   0  34  60 201
17   3   1 168 237  20  80 225 135  45 190   0   0   0   0   0   0  34  60 200
17   3   1 168 237  25 100  10 155  65 210   0   0   0   0   0   0  34  59 200
17   3   1 168 237  30 120  30 175  85 230   0   0   0   0   0   0  34  59 197
17   3   1 168 237  35 140  50 195 105  15   0   0   0   0   0   0  34  59 199
17   3   1 168 237  40 160  70 215 125  35   0   0   0   0   0   0  34  60 200
17   3   1 168 237  45 180  91  34   0   0   0   0   0   0   0   0  34  60 206

221 and 91 are out of +=5 rule, 220 and 90 are not used, hmmm...
34 is not channel because we use ch. 35 (too close), CRC?

235 seems to be top channel (excluded) -> 47 unique channels
*/

#define FRSKY_HOP_TABLE_COMPLETE		0x03FF	//bitewise flags of received hop table groups
#define FRSKY_HOP_TABLE_VALID			0xFFFF	//signal to finish binding a continue in prg. exec.

volatile uint16_t frsky_hop_channels=0;

volatile uint8_t frsky_channel_index=0;

void protocol_frsky_bind_finished(void)
{
	cc2500_strobe(CC2500_SIDLE);//stop RX
	cc2500_reset_callback(GDO0);
	uart_send_string_blocking("HOP sequence:\n");
	for (uint8_t i = 0; i < FRSKY_CHANNEL_COUNT; i++)
	{
		uart_send_string_blocking(itoa(frsky_channel_data[i][0],3));
	}
	frsky_hop_channels=FRSKY_HOP_TABLE_VALID;
}

void protocol_frsky_bind_process_packet(void)
{
	uint8_t packet[64];
	cc2500_read_fifo(packet);
	uart_send_string_blocking("GRP:");
	uart_send_string_blocking(itoa(packet[FRSKY_BIND_INX_HTI],3));
	uart_send_string_blocking("LEN:");
	uart_send_string_blocking(itoa(packet[FRSKY_BIND_INX_LENGTH],3));
	uart_send_byte_blocking('\n');
	uart_send_byte_blocking('\n');
	//parse TX ID
	union frsky_id IDbuff;
	IDbuff.nibble.high=packet[FRSKY_BIND_INX_TX_ADDR_H];
	IDbuff.nibble.low=packet[FRSKY_BIND_INX_TX_ADDR_L];

	if(frsky_tx_id.id==0)
	{
		frsky_tx_id.id=IDbuff.id;
		frsky_hop_channels=0;
		uart_send_string_blocking("ID LEARNED\n");
	}

	if(frsky_tx_id.id==IDbuff.id)
	{
		if(frsky_hop_channels==FRSKY_HOP_TABLE_COMPLETE)
			protocol_frsky_bind_finished();
		else
		{
			//parse
			for(uint8_t index=0; index<5; index++)
			{
				uint8_t hti=packet[FRSKY_BIND_INX_HTI];
				if(hti+index < FRSKY_CHANNEL_COUNT)
					frsky_channel_data[hti+index][0]=packet[FRSKY_BIND_INX_HOP1+index];//save channel number to channel config table
			}
			frsky_hop_channels|=(1<<(packet[FRSKY_BIND_INX_HTI]/5));//mark group as set
		}
	}

}

void protocol_frsky_bind(void)
{
	uart_send_string_blocking("BINDING...\n");
	frsky_tx_id.id=0;//protocol_frsky_bind_process_packet will parse ID when frsky_tx_id.id==0 or matching non zero value -> clear value to zero
	frsky_hop_channels=0;
	cc2500_strobe(CC2500_SIDLE);
	cc2500_write_reg(CC2500_FIFOTHR, 7);//more bytes than correct packet in RXFIFO to trigger -> triggered only by correct packets received, this way, we have least false positive interrupts from cc2500
	cc2500_write_reg(CC2500_IOCFG0, 0x01);//GDO to indicate packet processing end (or RXFIFO threshold crossed)
	cc2500_write_reg(CC2500_ADDR, FRSKY_BIND_ADDRESS );//address in bind mode
	cc2500_set_channel(FRSKY_BIND_CHANNEL);//all binding happens on channel 0 :/
	cc2500_strobe(CC2500_SCAL);//calibrate for this frequency (just for sure if no autocal enabled)
	delay_ms(1);//I hope it is enough, polling MARCSTATE fro IDLE state is the other way
	cc2500_set_callback(GDO0, RISING, protocol_frsky_bind_process_packet, NOT_EMPTY);//set callback on GDO0
	cc2500_strobe(CC2500_SRX);

	while(frsky_hop_channels!=FRSKY_HOP_TABLE_VALID)
		NOP;
	uart_send_string_blocking("BINDING finished!\n");
}

//to avoid waiting for PLL settlement after every hop, precalibrate all channels, might be necessary to recalibrate during runtime
void protocol_frsky_calibrate_channels(void)
{

	cc2500_strobe(CC2500_SIDLE);
	uart_send_string_blocking("calibrating channels...\n");

	cc2500_write_reg(CC2500_MCSM0, 0x08);//disable autocalibration
	for(uint8_t ch_index=0; ch_index<FRSKY_CHANNEL_COUNT; ch_index++)
	{
		cc2500_write_reg(CC2500_CHANNR, frsky_channel_data[ch_index][0]);
		cc2500_strobe(CC2500_SCAL);
		delay_ms(2);	//PLL should be settled, TODO: check for pll lock bit instead?
		frsky_channel_data[ch_index][1] = cc2500_read_reg(CC2500_FSCAL3);// & 0xCF);// ~FSCAL3_CHP_CURR_CAL_EN_msk); //set calibrated value, but disable charge-pump calibration bit
		frsky_channel_data[ch_index][2] = cc2500_read_reg(CC2500_FSCAL2);
		frsky_channel_data[ch_index][3] = cc2500_read_reg(CC2500_FSCAL1);
		cc2500_strobe(CC2500_SIDLE);

		uart_send_string_blocking("CH: ");
		uart_send_string_blocking(itoa(frsky_channel_data[ch_index][0],4));
		uart_send_string_blocking("FREQ: ");
		uart_send_string_blocking(itoa(frsky_channel_data[ch_index][1],4));
		uart_send_string_blocking(itoa(frsky_channel_data[ch_index][2],4));
		uart_send_string_blocking(itoa(frsky_channel_data[ch_index][3],4));
		uart_send_byte_blocking('\n');
	}
	uart_send_string_blocking("channels calibrated!\n");
}

void protocol_frsky_hop(uint8_t hops)
{
	cc2500_strobe(CC2500_SIDLE);

	frsky_channel_index+=hops;
	if(frsky_channel_index>=FRSKY_CHANNEL_COUNT)
		frsky_channel_index-=FRSKY_CHANNEL_COUNT;


	cc2500_write_reg(CC2500_CHANNR, frsky_channel_data[frsky_channel_index][0]);//set new channel
	//load precalibrated values
	cc2500_write_reg(CC2500_FSCAL3, frsky_channel_data[frsky_channel_index][1]);// & ~FSCAL3_CHP_CURR_CAL_EN_msk); //set calibrated value, but disable charge-pump calibration
	cc2500_write_reg(CC2500_FSCAL2, frsky_channel_data[frsky_channel_index][2]);
	cc2500_write_reg(CC2500_FSCAL1, frsky_channel_data[frsky_channel_index][3]);

	// uart_send_string_blocking("HOP:");
	// uart_send_string_blocking(itoa(frsky_channel_data[frsky_channel_index][0],4));
	// uart_send_byte_blocking('\n');
}

//extract ppm data from packets
void protocol_frsky_extract(uint8_t *packet,uint16_t *channel_buffer)
{
	channel_buffer[0]=\
	((uint16_t)FRSKY_EXTRACT_LOWER(packet[FRSKY_2W_RX_IDX_CH12_MSB])<<8)|\
	((uint16_t)packet[FRSKY_2W_RX_IDX_CH1_LSB]);

	channel_buffer[1]=\
	((uint16_t)FRSKY_EXTRACT_HIGHER(packet[FRSKY_2W_RX_IDX_CH12_MSB])<<8)|\
	((uint16_t)packet[FRSKY_2W_RX_IDX_CH2_LSB]);

	channel_buffer[2]=\
	((uint16_t)FRSKY_EXTRACT_LOWER(packet[FRSKY_2W_RX_IDX_CH34_MSB])<<8)|\
	((uint16_t)packet[FRSKY_2W_RX_IDX_CH3_LSB]);

	channel_buffer[3]=\
	((uint16_t)FRSKY_EXTRACT_HIGHER(packet[FRSKY_2W_RX_IDX_CH34_MSB])<<8)|\
	((uint16_t)packet[FRSKY_2W_RX_IDX_CH4_LSB]);


	channel_buffer[4]=\
	((uint16_t)FRSKY_EXTRACT_LOWER(packet[FRSKY_2W_RX_IDX_CH56_MSB])<<8)|\
	((uint16_t)packet[FRSKY_2W_RX_IDX_CH5_LSB]);

	channel_buffer[5]=\
	((uint16_t)FRSKY_EXTRACT_HIGHER(packet[FRSKY_2W_RX_IDX_CH56_MSB])<<8)|\
	((uint16_t)packet[FRSKY_2W_RX_IDX_CH6_LSB]);

	channel_buffer[6]=\
	((uint16_t)FRSKY_EXTRACT_LOWER(packet[FRSKY_2W_RX_IDX_CH78_MSB])<<8)|\
	((uint16_t)packet[FRSKY_2W_RX_IDX_CH7_LSB]);

	channel_buffer[7]=\
	((uint16_t)FRSKY_EXTRACT_HIGHER(packet[FRSKY_2W_RX_IDX_CH78_MSB])<<8)|\
	((uint16_t)packet[FRSKY_2W_RX_IDX_CH8_LSB]);
}

//describes expected event
volatile enum frsky_state
{
	NOSYNC,
	RX_PKT1,
	RX_PKT2,
	RX_PKT3,
	TX_PKT,

}frsky_state;

volatile uint8_t frsky_last_rssi=0;

void protocol_frsky_packet_send(void)
{
	cc2500_strobe(CC2500_SIDLE);
	cc2500_strobe(CC2500_SFTX);//flush TX buffer, should not be necessary
	cc2500_strobe(CC2500_SFRX);//flush RX buffer, should not be necessary
	uint8_t packet[FRSKY_2W_TX_LENGTH+1]={0};

	packet[FRSKY_2W_TX_IDX_LENGTH]=FRSKY_2W_TX_LENGTH;
	packet[FRSKY_2W_TX_IDX_TX_ADDR_H]=frsky_tx_id.nibble.high;
	packet[FRSKY_2W_TX_IDX_TX_ADDR_L]=frsky_tx_id.nibble.low;
	packet[FRSKY_2W_TX_IDX_A1]=adc_measure_single_blocking(4)>>4;//12bit ADC, but only 8bit data space :(
	packet[FRSKY_2W_TX_IDX_A2]=adc_measure_single_blocking(5)>>4;
	packet[FRSKY_2W_TX_IDX_RX_RSSI]=frsky_last_rssi/2;
	cc2500_write_fifo(packet, FRSKY_2W_TX_LENGTH+1);
	delay_us(1000);//delay when it works.. TODO: turn delay into timeout action
	cc2500_mode_tx();
}

volatile uint8_t packets_lost=0;
volatile uint8_t lna_state=0;

void protocol_frsky_packet_rxed_callback(void)
{

	uint8_t bytes=cc2500_read_reg(CC2500_RXBYTES);

	if(bytes!=FRSKY_2W_RX_TOTAL_LENGTH)
	{
		if(bytes>FRSKY_2W_RX_TOTAL_LENGTH)
		{
			cc2500_strobe(CC2500_SIDLE);
			cc2500_strobe(CC2500_SFRX);
			cc2500_mode_rx(0);
		}
		uart_send_string_blocking("LE: ");//packet length error
		uart_send_string_blocking(itoa(bytes,2));
		uart_send_byte_blocking('\n');
		return;//won't restart swdt, that will result as TimeOut
	}


	// uart_send_string_blocking(itoa(swdt_get(),4));
	swdt_restart(FRSKY_PACKET_TIMEOUT);
	protocol_frsky_hop(1);
	// uart_send_string_blocking(" RX ");
	packets_lost=0;


	uint8_t packet[64];
	bytes = cc2500_read_fifo(packet);

	if(	packet[FRSKY_2W_RX_IDX_TX_ADDR_H]!=frsky_tx_id.nibble.high ||\
		packet[FRSKY_2W_RX_IDX_TX_ADDR_L]!=frsky_tx_id.nibble.low)
	{
		uart_send_string_blocking("TXID mismatch!\n");
		//kinda WTF, packet passed address: test, CRC check, length check and second byte of address is not matching?!
		return;//won't restart swdt, that will result as TimeOut
	}

	//YAY! packet was probably valid, continue processing...
	switch (frsky_state)
	{
		//expected packed and we goot it
		case RX_PKT1:
		case RX_PKT2:
		case RX_PKT3:
			frsky_state++;
			break;

		case TX_PKT:
			//telemetry packet shoul have been sent, but we got packet incomming, weitd situation
			frsky_state=RX_PKT1;
			uart_send_string_blocking("RXed while TXing!\n");
			break;

		case NOSYNC:
			break;
	}

	//sync state machine with packet from packet id 0,1,2,(3),4,5,6,(7),...
	uint8_t newstate=packet[FRSKY_2W_RX_IDX_PKT_ID]%4 + RX_PKT1 +1;
	if(frsky_state != newstate)
	{
		// uart_send_string_blocking("Qs ");
		frsky_state = newstate;
	}

	frsky_last_rssi=(int16_t)(int8_t)packet[FRSKY_PKT_INX_RSSI(FRSKY_2W_TX_LENGTH)]-INT8_MIN;//save RSSI value, we might need it in telemetry packet

	//disable LNA if RSSI gets too high (RX saturation)
	if(frsky_last_rssi > FRSKY_RSSI_MAX && lna_state!=0 )
	{
		lna_state=0;
		uart_send_string_blocking("LNA OFF\n");
	}

	//enable LNA when RSSI is low enough
	else if(frsky_last_rssi < FRSKY_RSSI_MIN && lna_state==0 )
	{
		uart_send_string_blocking("LNA ON\n");
		lna_state=1;
	}

	//set received data to ppm outputs
	uint16_t channels[8];
	protocol_frsky_extract(packet, channels);
	ppm_set_ticks(FRSKY_1ms, FRSKY_2ms, channels);

	if(frsky_state==TX_PKT)
	{
		protocol_frsky_packet_send();//sends telemetry packet after 3rx packet was received
	}
	else
	{
		cc2500_mode_rx(lna_state);//set correct LNA state and set cc2500 to RX mode
	}
}

//when packet does not arrive
void protocol_frsky_packet_timeout_callback(void)
{
	if(packets_lost>=FRSKY_PACKETS_LOST_RESYNC_THRESHOLD || frsky_state==NOSYNC)
	{
		frsky_state=NOSYNC;
		protocol_frsky_hop(1);
		cc2500_mode_rx(1);//when we lost signal, turn on LNA
		uart_send_string_blocking("NOSYNC CH: ");
		uart_send_string_blocking(itoa(frsky_channel_data[frsky_channel_index][0], 3));
		uart_send_byte_blocking('\n');
		swdt_restart(FRSKY_CYCLE_TIMEOUT*3);//n times higher TO than full cycle to not run ahead/after TX and sync soon
	}
	else
	{
		protocol_frsky_hop(1);
		swdt_restart(FRSKY_PACKET_TIMEOUT);
		switch(frsky_state)
		{
			//expected packet from TX, but nothing arrived in time
			case RX_PKT1:
			case RX_PKT2:
			case RX_PKT3:
				packets_lost++;
				break;

			case TX_PKT:
				frsky_state=RX_PKT1;
				break;

			case NOSYNC:
				break;
		}
		cc2500_mode_rx(lna_state);
	}
}

void protocol_frsky_start(uint8_t force_bind)
{
	protocol_frsky_init();
	//read config from flash
	if( force_bind || protocol_frsky_read_nvm()<0 )//if no valid data found
	{
		protocol_frsky_calibrate_offset_start();
		protocol_frsky_bind();
		protocol_frsky_write_nvm();
	}
	else
	{
		uart_send_string_blocking("config LOADED\n");
	}

	frsky_state=NOSYNC;
	cc2500_strobe(CC2500_SIDLE);
	protocol_frsky_calibrate_channels();
	cc2500_write_reg(CC2500_ADDR, frsky_tx_id.nibble.high);//hw can check only 1 byte of 2 byte ID, second part is checked by SW (just for sure)
	cc2500_set_callback(GDO0, RISING,	protocol_frsky_packet_rxed_callback, NOT_EMPTY);//set intterupt on falling edge of GDO0
	cc2500_write_reg(CC2500_FOCCFG, 0x16);//enable frequecy autotune
	cc2500_write_reg(CC2500_IOCFG0, 0x01);
	cc2500_write_reg(CC2500_FIFOTHR, 7);//5 ~ 24+ bytes in RXFIFO to trigger -> troggered on pkt end only
	protocol_frsky_hop(0);//load channel config
	cc2500_mode_rx(1);//LNA state should not matter, will be tuned dynamically
	swdt_set_callback(protocol_frsky_packet_timeout_callback);//callback, when timeout occurs
	swdt_restart(FRSKY_CYCLE_TIMEOUT);//run!
}



void protocol_frsky_init(void)
{
	cc2500_strobe(CC2500_SRES);
	delay_ms(2);
	cc2500_strobe(CC2500_SIDLE);
	cc2500_write_reg(CC2500_MCSM1, MCSM1_TXOFF_MODE_RX | MCSM1_RXOFF_MODE_RX);
	cc2500_write_reg(CC2500_MCSM0, 1<<MCSM0_FS_AUTOCAL_pos | 2<<MCSM0_PO_TIMEOUT_pos);
	cc2500_write_reg(CC2500_PKTLEN, 25);
	cc2500_write_reg(CC2500_PKTCTRL1, PKTCTRL1_APPEND_STATUS | PKTCTRL1_CRC_AUTOFLUSH | 1<<PKTCTRL1_ADD_CHK_pos);
	cc2500_write_reg(CC2500_PKTCTRL0, PKTCTRL0_LENGTH_CONFIG_VARIABLE | PKTCTRL0_CRC_EN);
	// PATABLE(0) = 0xFF (+1dBm)
	cc2500_write_reg(CC2500_PATABLE, 0xFF);
	//FREQ_IF = 0x08 (IF = 203.125kHz)
	cc2500_write_reg(CC2500_FSCTRL1, 8<<FSCTRL1_FREQ_IF_pos);
	// there is some offset, determined ta binding time
	cc2500_write_reg(CC2500_FSCTRL0, 0);
	//FREQ = 0x5C7627 (F = 2404MHz)
	cc2500_write_reg(CC2500_FREQ2, 0x5C);
	cc2500_write_reg(CC2500_FREQ1, 0x76);
	cc2500_write_reg(CC2500_FREQ0, 0x27);
	cc2500_write_reg(CC2500_MDMCFG4, 2<<MDMCFG4_CHANBW_E_pos | 2<<MDMCFG4_CHANBW_M_pos | 10<<MDMCFG4_DRATE_E_pos);
	//Bitrate = 31044 bps
	cc2500_write_reg(CC2500_MDMCFG3, 57<<MDMCFG3_DRATE_M_pos);
	cc2500_write_reg(CC2500_MDMCFG2, MDMCFG2_MOD_FORMAT_GFSK | MDMCFG2_SYNC_MODE_15_16);
	cc2500_write_reg(CC2500_MDMCFG1, MDMCFG1_NUM_PREAMBLE_4 | 3<<MDMCFG1_CHANSPC_E_pos);
	// Channel Spacing = 299927Hz
	cc2500_write_reg(CC2500_MDMCFG0, 122<<MDMCFG0_CHANSPC_M_pos);
	// Deviation = 31738Hz
	cc2500_write_reg(CC2500_DEVIATN, 4<<DEVIATN_DEVIATION_E_pos | 2<<DEVIATN_DEVIATION_M_pos);
	cc2500_write_reg(CC2500_BSCFG, 1<<BSCFG_BS_PRE_KI_pos | 2<<BSCFG_BS_PRE_KP_pos | 1<<BSCFG_BS_POST_KI_pos | 1<<BSCFG_BS_POST_KP_pos);
	cc2500_write_reg(CC2500_AGCTRL2, 3<<AGCCTRL2_MAGN_TARGET_pos);
	cc2500_write_reg(CC2500_AGCTRL1, AGCCTRL1_AGC_LNA_PRIORITY);

	//TODO: check following settings
	cc2500_write_reg(CC2500_FOCCFG, 0x16);
	// 24 - (0x1D 0x91) AGCCTRL0 = 0x91
	cc2500_write_reg(CC2500_AGCTRL0, 0x91);
	// 25 - (0x21 0x56) FREND1
	cc2500_write_reg(CC2500_FREND1, 0x56);
	// 26 - (0x22 0x10) FREND0: LODIV_BUF_CURRENT = 1
	cc2500_write_reg(CC2500_FREND0, 0x10);
	// 31 - (0x29 0x59) FSTEST = 0x59  (Same as specified in datasheet)
	cc2500_write_reg(CC2500_FSTEST, 0x59);
	// 32 - (0x2C 0x88) TEST2 = 0x88 (Same as specified in datasheet and by SmartRF sw)
	cc2500_write_reg(CC2500_TEST2, 0x88);
	// 33 - (0x2D 0x31) TEST1 = 0x31 (Same as specified in datasheet and by SmartRF sw)
	cc2500_write_reg(CC2500_TEST1, 0x31);
	// 34 - (0x2E 0x0B) TEST0 = 0x0B (Same as specified in datasheet and by SmartRF sw)
	cc2500_write_reg(CC2500_TEST0, 0x0B);
	cc2500_write_reg(CC2500_FIFOTHR, 0x07);

	//set gpio pins as outputs, low
	cc2500_write_reg(CC2500_IOCFG0, 0x2F);//HW low
	cc2500_write_reg(CC2500_IOCFG2, 0x2F);//HW low
}