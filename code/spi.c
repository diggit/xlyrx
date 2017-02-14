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

#include "spi.h"
#include "misc.h"
#include "stm32f103xb.h"
#include "config.h"

#include "uart.h"

int8_t spi_selected_slave=-1;
int8_t spi_start(uint8_t slave_id)
{
	SPI_SLAVE_LIST[slave_id].PORT->BSRR = BIT(SPI_SLAVE_LIST[slave_id].pin)<<16;
	spi_selected_slave=slave_id;
	return 0;
}

int8_t spi_end(void)
{
	if(spi_selected_slave<0)
		return -1;

	SPI_SLAVE_LIST[spi_selected_slave].PORT->BSRR = BIT(SPI_SLAVE_LIST[spi_selected_slave].pin);
	return 0;
}

void spi_init(uint8_t prescaler)
{
	bit_set(RCC->APB2ENR, RCC_APB2ENR_SPI1EN);
	SPI1->CR1 = SPI_CR1_SSM | SPI_CR1_SSI | (prescaler&0x07)<<SPI_CR1_BR_Pos | SPI_CR1_MSTR | SPI_CR1_SPE;
}

void spi_exchange(uint8_t operation,uint8_t length ,uint8_t* data)
{
	while(!bit_get(SPI1->SR,SPI_SR_TXE))
		NOP;
	SPI1->DR;//clear

	for(uint8_t i=0; i<length; i++)
	{
		if(operation&SPI_W)
			SPI1->DR=data[i];
		else
			SPI1->DR=0;

		while(!bit_get(SPI1->SR,SPI_SR_TXE))
			NOP;

		while(!bit_get(SPI1->SR,SPI_SR_RXNE))
			NOP;

		if(operation&SPI_R)
		{
			data[i]=SPI1->DR;
		}
		else
		{
			SPI1->DR;//read but do not use
		}
	}

	//TODO: really necessary?
	while(bit_get(SPI1->SR, SPI_SR_BSY));
		NOP;

	if(bit_get(SPI1->SR, SPI_SR_OVR | SPI_SR_MODF | SPI_SR_UDR)!=0)
	{
		uart_send_string_blocking("SPIERR:");
		uart_send_string_blocking(itoa(SPI1->SR,4));
		uart_send_byte_blocking('\n');
	}
}

//interrupt handler
