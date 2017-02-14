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

#ifndef _STM_UART
	#define _STM_UART

	#include "config.h"
	#include <inttypes.h>

	#ifndef UART_BAUD
		#error "UART_BAUD not defined"
	#endif
	void uart_init(void (*rx_handler)(uint8_t rxed));
	void uart_rx_handler_set(void (*rx_handler)(uint8_t rxed));
	void uart_send_byte_array_blocking(const uint8_t *data);
	void uart_send_byte_blocking(uint8_t data);
	void (*uart_rx_handler)(uint8_t rxed); //pointer to function called on RXed character

	//helpers
	#define uart_send_string_blocking(str) uart_send_byte_array_blocking((uint8_t*)str)

#endif