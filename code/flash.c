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

#include "flash.h"
#include "stm32f103xb.h"
#include "misc.h"
#include "uart.h"

void flash_unlock(void)
{
	if(bit_get(FLASH->CR, FLASH_CR_LOCK)!=0)
		if(bit_get(FLASH->CR, FLASH_CR_LOCK)!=0)
		{
			FLASH->KEYR = FLASH_KEY1;
			FLASH->KEYR = FLASH_KEY2;
		}
}

void flash_page_erase(uint16_t *destination)
{
	flash_unlock();
	while(bit_get(FLASH->SR, FLASH_SR_BSY)!=0)//wait until not BUSY
		NOP;

	bit_set(FLASH->CR, FLASH_CR_PER);//we want page erase
	FLASH->AR=(uint32_t)destination;//of page containing this address
	bit_set(FLASH->CR, FLASH_CR_STRT);//start erasing
	while(bit_get(FLASH->SR, FLASH_SR_BSY)!=0)//wait until not BUSY
		NOP;
	bit_clr(FLASH->CR, FLASH_CR_PER);//we deselect operation
	// uart_send_string_blocking("FLASH erase finished!\n");
}

void flash_write(uint16_t *destination, uint16_t data)
{
	flash_unlock();

	while(bit_get(FLASH->SR, FLASH_SR_BSY)!=0)//wait until not BUSY
		NOP;

	bit_set(FLASH->SR, FLASH_SR_WRPRTERR | FLASH_SR_PGERR );//clear flags


	bit_set(FLASH->CR, FLASH_CR_PG);//programming mode

	*destination=data;
	while(bit_get(FLASH->SR, FLASH_SR_BSY)!=0)//wait until not BUSY
		NOP;

	if(bit_get(FLASH->SR, FLASH_SR_EOP)==0)
	{
		uart_send_string_blocking("FLASH write error!\n");
	}


	if(*destination != data)
	{
		uart_send_string_blocking("FLASH witten data MATCH!\n");
	}
	bit_clr(FLASH->CR, FLASH_CR_PG);//leave programming mode
}