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

#ifndef _FLASH
	#define _FLASH

	#include "stm32f103xb.h"
	#include <inttypes.h>

	#define FLASH_PAGE_SIZE			1024
	#define FLASH_PAGE_COUNT		64
	#define FLASH_PAGE_LAST			(FLASH_PAGE_COUNT-1)
	#define FLASH_PAGE_ADDR(PAGE)	(FLASH_BASE+(PAGE)*FLASH_PAGE_SIZE)

	void flash_unlock(void);
	void flash_page_erase(uint16_t *destination);
	void flash_write(uint16_t *destination, uint16_t data);

#endif