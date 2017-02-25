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

#include "cc2500.h"
#include "spi.h"
#include "stm32f103xb.h"
#include "uart.h"
#include "timers.h"


// spi_start(CC2500_SPI_SLAVE_ID);
// spi_end();

volatile union cc2500_status cc2500_status;

void cc2500_wait_for_wake(uint8_t data)
{
	spi_start(CC2500_SPI_SLAVE_ID);
	//wait until MISO goes low
	while( bit_get(GPIOB->IDR,GPIO_IDR_IDR4)!=0)
		NOP;

	uint8_t buffer=data;
	spi_exchange(SPI_RW, 1, &buffer);
	cc2500_status.byte=buffer;
}

uint8_t cc2500_read_reg(uint8_t address)
{
	// spi_start(CC2500_SPI_SLAVE_ID);
	cc2500_wait_for_wake(address|CC2500_ACCESS_READ);
	//data
	uint8_t buffer;
	spi_exchange(SPI_R, 1, &buffer);
	spi_end();
	return buffer;
}

void cc2500_read_reg_burst(uint8_t address, uint8_t length, uint8_t *destination)
{
	cc2500_wait_for_wake(address|CC2500_ACCESS_READ|CC2500_ACCESS_BURST);
	//data
	spi_exchange(SPI_R, length, destination);
	spi_end();
}

void cc2500_write_reg(uint8_t address, uint8_t data)
{
	cc2500_wait_for_wake(address|CC2500_ACCESS_WRITE);
	//data
	spi_exchange(SPI_W, 1, &data);
	spi_end();
}

void cc2500_write_reg_burst(uint8_t address, uint8_t length, uint8_t *source)
{
	cc2500_wait_for_wake(address|CC2500_ACCESS_WRITE|CC2500_ACCESS_BURST);
	//data
	spi_exchange(SPI_W, length, source);
	spi_end();
}

void cc2500_strobe(uint8_t address)
{
	cc2500_wait_for_wake(address|CC2500_ACCESS_WRITE);
	spi_end();
}

void cc2500_write_fifo(uint8_t *data, uint8_t length)
{
	// uart_send_string_blocking("CC:");
	// uart_send_string_blocking(itoa(length,3));
	// uart_send_byte_blocking('\n');
	cc2500_wait_for_wake(CC2500_FIFO|CC2500_ACCESS_WRITE|CC2500_ACCESS_BURST);
	spi_exchange(SPI_W, length, data);
	spi_end();
}

uint8_t cc2500_read_fifo(uint8_t *data)
{
	uint8_t bytes=cc2500_read_reg(CC2500_RXBYTES);
	if(bytes==0)
		return 0;

	cc2500_wait_for_wake(CC2500_FIFO|CC2500_ACCESS_READ|CC2500_ACCESS_BURST);

	uint8_t length=0;
	for(; bytes>0; bytes--)
	{
		spi_exchange(SPI_R, 1, data);
		data++;
		length++;
	}
	spi_end();
	return length;
}

void cc2500_ex_pa(uint8_t state)
{
	if(state)
		GPIOB->BSRR = GPIO_BSRR_BS7;
	else
		GPIOB->BSRR = GPIO_BSRR_BR7;
}

void cc2500_ex_lna(uint8_t state)
{
	if(state)
		GPIOA->BSRR = GPIO_BSRR_BS15;
	else
		GPIOA->BSRR = GPIO_BSRR_BR15;
}

void cc2500_mode_tx(void)
{
	cc2500_ex_lna(0);
	cc2500_strobe(CC2500_SIDLE);
	cc2500_ex_pa(1);
	cc2500_strobe(CC2500_STX);
}

void cc2500_mode_rx(uint8_t LNA)
{
	cc2500_ex_pa(0);
	cc2500_strobe(CC2500_SIDLE);
	cc2500_ex_lna(LNA);
	cc2500_strobe(CC2500_SRX);
	// delay_us(90);//time to turn on rx
}

void cc2500_set_channel(uint8_t channel)
{
	cc2500_strobe(CC2500_SIDLE);
	cc2500_write_reg(CC2500_CHANNR, channel);
}

void (*cc2500_GDO0_callback)(void)=NULL;
enum gdo_filter cc2500_GDO0_callback_filter;
void (*cc2500_GDO2_callback)(void)=NULL;
enum gdo_filter cc2500_GDO2_callback_filter;

void cc2500_reset_callback(enum gdo gdo)
{
	switch(gdo)
	{
		case GDO0:
			cc2500_GDO0_callback=NULL;//not necessary
			bit_clr(EXTI->IMR, EXTI_IMR_IM8);
			bit_clr(EXTI->PR, EXTI_PR_PR8);
			bit_clr(EXTI->FTSR, EXTI_FTSR_FT8);
			bit_clr(EXTI->RTSR, EXTI_RTSR_RT8);
			break;

		case GDO2:
			cc2500_GDO2_callback=NULL;//not necessary
			bit_clr(EXTI->IMR, EXTI_IMR_IM9);//disable interrupt
			bit_set(EXTI->PR, EXTI_PR_PR9);//clean pending requasts
			bit_clr(EXTI->RTSR, EXTI_FTSR_FT9);//diable rising adge if there was any
			bit_clr(EXTI->FTSR, EXTI_RTSR_RT9);//diable falling adge if there was any
			break;

	}
}
void cc2500_set_callback(enum gdo gdo, enum edge edge, void(*callback)(void), uint8_t filter)
{
	// uart_send_string_blocking("setting callabck\n");

	volatile uint32_t *edge_register;
	switch(edge)
	{
		case RISING:
			// uart_send_string_blocking("\trising\n");
			edge_register = &EXTI->RTSR;
			break;

		case FALLING:
			// uart_send_string_blocking("\tfalling\n");
			edge_register = &EXTI->FTSR;
			break;

		default:
			return;
	}

	switch(gdo)
	{
		case GDO0:
			// uart_send_string_blocking("\tGDO0\n");
			cc2500_reset_callback(GDO0);//reset any previous state
			bit_set(*edge_register, EXTI_RTSR_TR8);
			cc2500_GDO0_callback=callback;
			cc2500_GDO0_callback_filter=filter;
			bit_set(EXTI->IMR, EXTI_IMR_IM8);//enable interrupt line
			break;

		case GDO2:
			// uart_send_string_blocking("\tGDO2\n");
			cc2500_reset_callback(GDO2);//reset any previous state
			bit_set(*edge_register, EXTI_RTSR_TR9);
			cc2500_GDO2_callback=callback;
			cc2500_GDO2_callback_filter=filter;
			bit_set(EXTI->IMR, EXTI_IMR_IM9);//enable interrupt line
			break;
	}
}


void EXTI9_5_IRQHandler (void)
{
	// uart_send_string_blocking("+");

	// cc2500_strobe(CC2500_SNOP);
	// uart_send_string_blocking("S: ");
	// uart_send_string_blocking(itoa(cc2500_status.bits.STATE,3));
	// uart_send_byte_blocking('\n');

	uint8_t buffer;
	if(bit_get(EXTI->PR,EXTI_PR_PR8))
	{
		bit_set(EXTI->PR,EXTI_PR_PR8);
		if(cc2500_GDO0_callback==NULL)
			return;

		switch(cc2500_GDO0_callback_filter)
		{
			case NOT_EMPTY:
				buffer=cc2500_read_reg(CC2500_RXBYTES);
				if(buffer==0)//no received bytes
				{
					// uart_send_string_blocking("_");
					return;
				}

			case NONE:
				break;
		}
		cc2500_GDO0_callback();
	}

	if(bit_get(EXTI->PR,EXTI_PR_PR9))
	{
		bit_set(EXTI->PR,EXTI_PR_PR9);
		if(cc2500_GDO2_callback==NULL)
			return;

		switch(cc2500_GDO2_callback_filter)
		{
			case NOT_EMPTY:
				buffer=cc2500_read_reg(CC2500_RXBYTES);
				if(buffer==0)//no received bytes
					return;

			case NONE:
				break;
		}
		cc2500_GDO2_callback();
	}

}