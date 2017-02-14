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

#ifndef _SPI_LIB
	#define _SPI_LIB

	#include "config.h"
	#include "misc.h"
	#include <inttypes.h>

	int8_t spi_start(uint8_t slave_id);
	int8_t spi_end(void);

	void spi_init(uint8_t prescaler);
	void spi_exchange(uint8_t operation,uint8_t length ,uint8_t* data);

	#define SPI_W		1
	#define SPI_R		2 //do not use as standalone
	#define SPI_RW		(SPI_R|SPI_W)

	enum{
		SPI_PSC_2=0,
		SPI_PSC_4,
		SPI_PSC_8,
		SPI_PSC_16,
		SPI_PSC_32,
		SPI_PSC_64,
		SPI_PSC_128,
		SPI_PSC_256
	};


#endif