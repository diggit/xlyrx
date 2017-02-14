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

#include "stm32f103xb.h"
#include "uart.h"
#include "misc.h"

void (*uart_rx_handler)(uint8_t rxed)=NULL;

void uart_init(void (*rx_handler)(uint8_t rxed))
{
	bit_set(RCC->APB2ENR, RCC_APB2ENR_USART1EN);
	// USART1->BRR= 39<<USART_BRR_DIV_Mantissa_Pos | 1<<USART_BRR_DIV_Fraction_Pos;//115200 @ 72MHz (39.0625)
	USART1->BRR= 19<<USART_BRR_DIV_Mantissa_Pos | 8<<USART_BRR_DIV_Fraction_Pos;//230400 @ 72MHz
	USART1->CR1= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;
	uart_rx_handler = rx_handler;
}

void uart_rx_handler_set(void (*rx_handler)(uint8_t rxed))
{
	uart_rx_handler=rx_handler;
}

void uart_send_byte_array_blocking(const uint8_t *data)
{
	for(;*data!=0; data++)
	{
		uart_send_byte_blocking(*data);
	}
}

void uart_send_byte_blocking(uint8_t data)
{
	USART1->DR=data;
	while (!bit_get(USART1->SR,USART_SR_TXE))//wait for TX register empty
		NOP;
}

//IRQ handler
// void USART1_IRQHandler (void)
// {
// 	uint16_t SR=USART1->SR;
// 	uint8_t DR=USART1->DR;
// 	if(SR&USART_SR_RXNE)
// 	{
// 		if(uart_rx_handler!=NULL)
// 			uart_rx_handler(DR);
// 		// uart_send_byte_blocking(DR);
//
// 	}
// }




