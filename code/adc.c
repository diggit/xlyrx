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
#include "adc.h"
#include "misc.h"


#define ADC_SEQ_SMPR	7

void adc_init_single(void)
{
	bit_set(RCC->APB2ENR, RCC_APB2ENR_ADC1EN);
	ADC1->CR1=0;
	ADC1->CR2=0;

	//discontinoous mode, num of discon. channels
	bit_set(ADC1->CR1, ADC_CR1_DISCEN | 0<<ADC_CR1_DISCNUM_Pos );
	//enable vrefint and int temp sensor
	bit_set(ADC1->CR2, ADC_CR2_TSVREFE | ADC_CR2_EXTTRIG | 7<<ADC_CR2_EXTSEL_Pos | ADC_CR2_EXTTRIG );
	//setup sample times
	ADC1->SMPR1 =	ADC_SEQ_SMPR <<ADC_SMPR1_SMP10_Pos|\
					ADC_SEQ_SMPR <<ADC_SMPR1_SMP11_Pos|\
					ADC_SEQ_SMPR <<ADC_SMPR1_SMP12_Pos|\
					ADC_SEQ_SMPR <<ADC_SMPR1_SMP13_Pos|\
					ADC_SEQ_SMPR <<ADC_SMPR1_SMP14_Pos|\
					ADC_SEQ_SMPR <<ADC_SMPR1_SMP15_Pos|\
					ADC_SEQ_SMPR <<ADC_SMPR1_SMP16_Pos|\
					ADC_SEQ_SMPR <<ADC_SMPR1_SMP17_Pos;

	ADC1->SMPR2 =	ADC_SEQ_SMPR <<ADC_SMPR2_SMP0_Pos |\
					ADC_SEQ_SMPR <<ADC_SMPR2_SMP1_Pos|\
					ADC_SEQ_SMPR <<ADC_SMPR2_SMP2_Pos|\
					ADC_SEQ_SMPR <<ADC_SMPR2_SMP3_Pos|\
					ADC_SEQ_SMPR <<ADC_SMPR2_SMP4_Pos|\
					ADC_SEQ_SMPR <<ADC_SMPR2_SMP5_Pos|\
					ADC_SEQ_SMPR <<ADC_SMPR2_SMP6_Pos|\
					ADC_SEQ_SMPR <<ADC_SMPR2_SMP7_Pos|\
					ADC_SEQ_SMPR <<ADC_SMPR2_SMP8_Pos|\
					ADC_SEQ_SMPR <<ADC_SMPR2_SMP9_Pos;


	// num of channels to convert: 1
	ADC1->SQR1 = 0U;
	ADC1->SQR2 = 0U;
	ADC1->SQR3 = 0U;

	// turn on ADC
	bit_set(ADC1->CR2, ADC_CR2_ADON);
}

void adc_calibrate(void)
{

	// uart_send_string_blocking("ADC RSTCAL\n");
	//initialize(reset) calibration regs
	bit_set(ADC1->CR2, ADC_CR2_RSTCAL);
	//wait for complete
	while (bit_get(ADC1->CR2, ADC_CR2_RSTCAL) != 0)
		NOP;

	// uart_send_string_blocking("ADC CAL\n");
	//start calibration
	bit_set(ADC1->CR2, ADC_CR2_CAL);
	//wait for complete
	while (bit_get(ADC1->CR2, ADC_CR2_CAL) != 0)
		NOP;
}

#define ADC_AVG	100
uint16_t adc_measure_single_blocking(uint8_t input)
{
	bit_mod(ADC1->SQR1, ADC_SQR1_L_Msk, 0<<ADC_SQR1_L_Pos);
	bit_mod(ADC1->SQR3, ADC_SQR3_SQ1_Msk, input);

	uint32_t buff=0;
	//start conversion
	// GPIOC->BSRR=GPIO_BSRR_BR13;
	for(uint8_t i=0; i<ADC_AVG; i++)
	{
		bit_set(ADC1->CR2, ADC_CR2_SWSTART);
		//wait for End Of Conversion
		while(bit_get(ADC1->SR,ADC_SR_EOC)==0)
			NOP;
		buff+=ADC1->DR;
		// delay_ms(40);
	}
	// GPIOC->BSRR=GPIO_BSRR_BS13;

	return buff/ADC_AVG;
}