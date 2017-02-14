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

#include <inttypes.h>


void system_clock_init(void)
{
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


void gpio_init(void)
{
	bit_set(RCC->APB2ENR, \
			RCC_APB2ENR_IOPAEN |\
			RCC_APB2ENR_IOPBEN |\
			RCC_APB2ENR_IOPCEN |\
			RCC_APB2ENR_AFIOEN);// enable clock to ports A,B,C, USART1, AFIO

	AFIO->MAPR =	AFIO_MAPR_SPI1_REMAP | AFIO_MAPR_SWJ_CFG_JTAGDISABLE;

	GPIOA->CRL =	\
					//PA0	S0	(TIM2_CH1)
					GPIO_ALTERNATE			<<GPIO_CRL_MODE0_Pos|\
					//PA1	S1	(TIM2_CH2)
					GPIO_ALTERNATE			<<GPIO_CRL_MODE1_Pos|\
					//PA2	S5	(TIM2_CH3)
					GPIO_ALTERNATE			<<GPIO_CRL_MODE2_Pos|\
					//PA3	S6	(TIM2_CH4)
					GPIO_ALTERNATE			<<GPIO_CRL_MODE3_Pos|\
					//PA4	AIN_1	(ADC_IN4)
					GPIO_INPUT_ANALOG		<<GPIO_CRL_MODE4_Pos|\
					//PA5	AIN_2	(ADC_IN5)
					GPIO_INPUT_ANALOG		<<GPIO_CRL_MODE5_Pos|\
					//PA6	S3	(TIM3_CH1)
					GPIO_ALTERNATE			<<GPIO_CRL_MODE6_Pos|\
					//PA7	S4	(TIM3_CH2)
					GPIO_ALTERNATE			<<GPIO_CRL_MODE7_Pos;

	GPIOA->CRH = 	\
					//PA8	GPIO1	(GPIO)
					GPIO_INPUT				<<GPIO_CRH_MODE8_Pos|\
					//PA9	BL_FG_TX	(USART1_TX)
					GPIO_ALTERNATE			<<GPIO_CRH_MODE9_Pos|\
					//PA10	BL_FG_RX	(USART1_RX)
					GPIO_INPUT_PULL_UP_DOWN	<<GPIO_CRH_MODE10_Pos|\
					//PA11	GPIO2	(GPIO)
					GPIO_INPUT				<<GPIO_CRH_MODE11_Pos|\
					//PA12	BTLDR	(GPIO)
					GPIO_OUTPUT				<<GPIO_CRH_MODE12_Pos|\
					/*SWD
					GPIO_INPUT				<<GPIO_CRH_MODE13_Pos|\
					GPIO_INPUT				<<GPIO_CRH_MODE14_Pos|\
					*/
					//PA15	LNA_EN	(GPIO)
					GPIO_OUTPUT				<<GPIO_CRH_MODE15_Pos;

	GPIOA->ODR =	\
					//PA10 PUP
					GPIO_ODR_ODR10;

	GPIOB->CRL =	\
					//PB0	S7		(TIM3_CH3)
					GPIO_ALTERNATE			<<GPIO_CRL_MODE0_Pos|\
					//PB1	S8		(TIM3_CH4)
					GPIO_INPUT				<<GPIO_CRL_MODE1_Pos|\
					//PB2	BOOT1
					/*GPIO_INPUT				<<GPIO_CRL_MODE2_Pos|\*/
					//PB3	SCK		(SPI1_SCK) REMAP!
					GPIO_ALTERNATE_10MHz	<<GPIO_CRL_MODE3_Pos|\
					//PB4	MISO	(SPI1_MISO) REMAP!
					GPIO_INPUT				<<GPIO_CRL_MODE4_Pos|\
					//PB5	MOSI	(SPI1_MOSI) REMAP!
					GPIO_ALTERNATE_10MHz	<<GPIO_CRL_MODE5_Pos|\
					//PB6	CS		(GPIO)
					GPIO_OUTPUT_2MHz		<<GPIO_CRL_MODE6_Pos|\
					//PB7	PA_EN	(GPIO)
					GPIO_OUTPUT				<<GPIO_CRL_MODE7_Pos;

	GPIOB->CRH = 	\
					//PB8	GDO0	(GPIO)
					GPIO_INPUT				<<GPIO_CRH_MODE8_Pos|\
					//PB9	GDO2	(GPIO)
					GPIO_INPUT				<<GPIO_CRH_MODE9_Pos|\
					//PB10	TLMTRY_TX	(USART3_TX)
					GPIO_ALTERNATE			<<GPIO_CRH_MODE10_Pos|\
					//PB11	TLMTRY_RX	(USART3_RX)
					GPIO_INPUT_PULL_UP_DOWN	<<GPIO_CRH_MODE11_Pos|\
					//PB12	GPIO3	(GPIO)
					GPIO_INPUT				<<GPIO_CRH_MODE12_Pos|\
					//PB13	GPIO4	(GPIO)
					GPIO_INPUT				<<GPIO_CRH_MODE13_Pos|\
					//PB14	GPIO5	(GPIO)
					GPIO_INPUT				<<GPIO_CRH_MODE14_Pos|\
					//PB15	GPIO6	(GPIO)
					GPIO_INPUT				<<GPIO_CRH_MODE15_Pos;

	GPIOB->ODR =	\
					//PB6	HIGH
					GPIO_ODR_ODR6|\
					//PB11	PUP
					GPIO_ODR_ODR11;

	//only pins 13,14,15 present
	GPIOC->CRH = 	\
					//PC13	LED_1	(GPIO)
					GPIO_OUTPUT				<<GPIO_CRH_MODE13_Pos|\
					//PC14	LED_2	(GPIO)
					GPIO_OUTPUT				<<GPIO_CRH_MODE14_Pos|\
					//PC15	BTN		(GPIO)
					GPIO_INPUT				<<GPIO_CRH_MODE15_Pos;

	GPIOC->ODR =	\
					//PC13	LED(OFF)
					GPIO_ODR_ODR13|\
					//PC14	LED(OFF)
					GPIO_ODR_ODR14;


	//enable EXTI inttterupt for GDOs
	AFIO->EXTICR[2]=AFIO_EXTICR3_EXTI8_PB | AFIO_EXTICR3_EXTI9_PB;

}
void led1_flash(uint8_t data)
{
	GPIOC->BSRR=GPIO_BSRR_BR13;
	delay_ms(data);
	GPIOC->BSRR=GPIO_BSRR_BS13;
	delay_ms(data);
}

void led2_flash(uint8_t data)
{
	GPIOC->BSRR=GPIO_BSRR_BR14;
	delay_ms(data);
	GPIOC->BSRR=GPIO_BSRR_BS14;
	delay_ms(data);
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



uint8_t fifo[64];

int main(void)
{
	system_clock_init();
	gpio_init();
	swdt_init();

	irq_enable();
	irq_NVIC_ISE(EXTI9_5_IRQn);

	uart_init(NULL);
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
	protocol_frsky_start(0);


	led1_flash(100);

	while(1)
	{
		NOP;
	}
}
