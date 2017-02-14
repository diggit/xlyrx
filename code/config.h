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

#ifndef _CONFIG
	#define _CONFIG

	#include "stm32f103xb.h"
	#include <inttypes.h>

	#define XLY_PCB_VERSION	1

	#define MEM_MAGIC	0x71BA	//value in flash storage page to indicate valid data
	#define MEM_VERSION	1	//when flash storage format changes, increase this value to prevent loading corrupted data

	#define T_us	1000000U
	#define T_ms	1000

	#define F_CPU	72000000U
	#define F_APB1	(F_CPU/2)
	#define F_APB2	(F_CPU)

	#define UART_BAUD	115200

	#define CC2500_SPI_SLAVE_ID		0

	#define SWDT_RESOLUTION	100//time units
	#define SWDT_F_IN		(F_CPU)

	#define PPM_TRIG_F_IN		(F_APB1*2) //7.2 last note, LOL
	#define PPM_TRIG_F_OUT		50//Hz
	#define PPM_TRIG_RESOLUTION_us	10//

	#define PPM_CH_A_F_IN		(F_APB1*2) //7.2 last note, LOL
	#define PPM_CH_B_F_IN		(F_APB1*2) //7.2 last note, LOL

	#define PPM_PULSE_ABS_MAX_us		3000//3ms is acheaveable at least

	//SPI
	struct SPI_SLAVE
	{
		GPIO_TypeDef *PORT;
		const uint8_t pin;
	};
	//configure SPI slaves in config.c
	extern const struct SPI_SLAVE const SPI_SLAVE_LIST[];


#endif