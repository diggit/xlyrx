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

#ifndef _HARDWARE_LIB
	#define _HARDWARE_LIB

	#include "stm32f103xb.h"
	#include "misc.h"
	#include <inttypes.h>

	void gpio_init(void);

	inline void led1_on(void)
	{
		GPIOC->BSRR=GPIO_BSRR_BR13;
	}

	inline void led1_off(void)
	{
		GPIOC->BSRR=GPIO_BSRR_BS13;
	}

	inline void led2_on(void)
	{
		GPIOC->BSRR=GPIO_BSRR_BR14;
	}

	inline void led2_off(void)
	{
		GPIOC->BSRR=GPIO_BSRR_BS14;
	}

	void led1_flash(uint8_t data);
	void led2_flash(uint8_t data);

#endif