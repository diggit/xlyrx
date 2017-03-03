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
#include "misc.h"
#include "uart.h"
#include "spi.h"
#include "cc2500.h"
#include "protocol.h"
#include "irq.h"
#include "timers.h"
#include "flash.h"
#include "hardware.h"
#include "adc.h"
#include "systick.h"

#include <inttypes.h>


void system_clock_init(void)
{
	bit_set(RCC->CSR, RCC_CSR_LSION);//enable LSI for IWDG
	bit_set(FLASH->ACR, 1<<FLASH_ACR_LATENCY_Pos);//increase wait times for flash
	// led_flash(0);
	bit_clr(RCC->CR, RCC_CR_HSEON | RCC_CR_HSEBYP);//off ext OSC, off ext OSC bypass
	bit_set(RCC->CR, RCC_CR_HSEON);//reenable ext OSC
	while(bit_get(RCC->CR, RCC_CR_HSERDY)==0)//wait for OSC to settle
		NOP;

	RCC->CFGR = RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL9 | RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_ADCPRE_DIV8;
	bit_set(RCC->CR, RCC_CR_PLLON);//enable PLL
	while(bit_get(RCC->CR, RCC_CR_PLLRDY)==0)//wat for PLL to stabilize
		NOP;
	// led_flash(0);
	bit_mod(RCC->CFGR, RCC_CFGR_SW_Msk, RCC_CFGR_SW_PLL);//set system clock soutce to PLL
	//bit_clr(RCC->CR, RCC_CR_HSION);//off HSI

}

void draw_bar(uint8_t length, uint8_t filled, uint8_t ch)
{
	uart_send_byte_blocking('[');
	uint8_t cursor;
	for(cursor=0; cursor<filled; cursor++)
		uart_send_byte_blocking(ch);
	for(;cursor<length; cursor++)
		uart_send_byte_blocking(' ');
	uart_send_byte_blocking(']');
}

void uart_handler(uint8_t data)
{
	if(data=='B')
	{
		led_on(HARDWARE_LED_2);
		irq_disable();
		GPIOA->BSRR=GPIO_BSRR_BS12;//charge capacitor on BL pin
		delay_ms(10);

		//reset using IDWG, I was not successful using system reset
		IWDG->KR=0xCCCCU;//start IWDG
		IWDG->KR=0x5555U;//enable access to prescaler and preload regs.
		IWDG->RLR=1;
		IWDG->PR=0;
		while(IWDG->SR!=0)
			NOP;
		IWDG->KR=0xAAAAU;//reload IWDG

		while(1)
			NOP;
	}

}


uint8_t fifo[64];

int main(void)
{
	system_clock_init();
	gpio_init();
	swdt_init();
	adc_init_single();
	adc_calibrate();

	systick_init();

	// systick_blink_set(HARDWARE_LED_2, 0, SYSTICK_1s*0.1);


	irq_enable();
	irq_NVIC_ISE(EXTI9_5_IRQn);

	led_on(HARDWARE_LED_2);

	uart_init(NULL);
	uart_rx_handler_set(uart_handler);
	// uart_send_string_blocking("\x1B[1J\x1B[;H");
	uart_send_string_blocking("\nxlyRX starting...\n");
	uart_send_string_blocking("Build: ");
	#ifdef BUILD
	uart_send_string_blocking(itoa(BUILD,3));
	uart_send_string_blocking(" @ ");
	#endif
	uart_send_string_blocking(__DATE__);
	uart_send_byte_blocking(' ');
	uart_send_string_blocking(__TIME__);
	uart_send_byte_blocking('\n');

	spi_init(3);
	ppm_init();
	protocol_frsky_start(button_read());


	// led1_flash(100);

	while(1)
	{
		NOP;
	}
}
