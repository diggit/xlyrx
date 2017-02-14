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

#include "timers.h"
#include "misc.h"
#include "irq.h"
#include "uart.h"

void (*swdt_callback)(void)=NULL;

void swdt_init(void)
{
	bit_set(RCC->APB2ENR, RCC_APB2ENR_TIM1EN);
	TIM1->PSC = SWDT_F_IN/(1000000/SWDT_RESOLUTION) -1;//100us resolution;
	TIM1->CR1 = TIM_CR1_OPM;
	TIM1->DIER = TIM_DIER_CC1IE;
	TIM1->ARR = INT16_MAX;
	TIM1->EGR = TIM_EGR_UG;
	irq_NVIC_ISE(TIM1_CC_IRQn);
}

void swdt_restart(uint16_t timeout)
{
	bit_clr(TIM1->CR1, TIM_CR1_CEN);//stop timer
	TIM1->CNT=0;
	TIM1->CCR1 = timeout;
	// TIM1->EGR = TIM_EGR_UG;
	if(swdt_callback==NULL)
		return;
	bit_set(TIM1->CR1, TIM_CR1_CEN);
}

void swdt_set_callback(void (*callback)(void))
{
	bit_clr(TIM1->CR1, TIM_CR1_CEN);
	swdt_callback=callback;
}

uint16_t swdt_stop(void)
{
	bit_clr(TIM1->CR1, TIM_CR1_CEN);
	return TIM1->CNT;
}

void TIM1_CC_IRQHandler (void)
{
	bit_clr(TIM1->CR1, TIM_CR1_CEN);//stop timer
	// TIM1->CNT=0;
	bit_clr(TIM1->SR, TIM_SR_CC1IF);//clear int. flag of CC1
	// uart_send_string_blocking("CCR1: ");
	// uart_send_string_blocking(itoa(TIM1->CCR1,5));
	// uart_send_byte_blocking('\n');

	if(swdt_callback!=NULL)
		swdt_callback();


}

// void TIM4_IRQHandler (void)
// {
// 	bit_clr(TIM4->SR, TIM_SR_UIF);//clear flag
// 	GPIOC->BSRR=GPIO_BSRR_BR14;
// 	delay_ms(1);
// 	GPIOC->BSRR=GPIO_BSRR_BS14;
// }

void ppm_config_channel(TIM_TypeDef *tim)
{
	tim->PSC = PPM_CH_A_PRESCALER-1;
	tim->ARR = PPM_TIMER_MAX;
	tim->SMCR = 3<<TIM_SMCR_TS_Pos | 6<<TIM_SMCR_SMS_Pos; //ITR3 (TIM4), triggered
	tim->CR1 = TIM_CR1_OPM;

	tim->CCMR1 =  7<<TIM_CCMR1_OC1M_Pos | 7<<TIM_CCMR1_OC2M_Pos | TIM_CCMR1_OC1PE | TIM_CCMR1_OC2PE;// PWM
	tim->CCMR2 =  7<<TIM_CCMR2_OC3M_Pos | 7<<TIM_CCMR2_OC4M_Pos | TIM_CCMR2_OC3PE | TIM_CCMR2_OC4PE;// PWM
	tim->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;

	tim->CCR1 = 0;
	tim->CCR2 = 0;
	tim->CCR3 = 0;
	tim->CCR4 = 0;

	tim->EGR = TIM_EGR_UG;
}

void ppm_init(void)
{

	bit_set(RCC->APB1ENR, RCC_APB1ENR_TIM4EN);
	bit_set(RCC->APB1ENR, RCC_APB1ENR_TIM2EN);
	bit_set(RCC->APB1ENR, RCC_APB1ENR_TIM3EN);
	//
	PPM_TRIG_TMR->ARR = (T_us/PPM_TRIG_RESOLUTION_us/PPM_TRIG_F_OUT);
	PPM_TRIG_TMR->PSC = PPM_TRIG_F_IN/(T_us/PPM_TRIG_RESOLUTION_us) -1;
	PPM_TRIG_TMR->CR2 = 2<<TIM_CR2_MMS_Pos;
	PPM_TRIG_TMR->EGR = TIM_EGR_UG;

	ppm_config_channel(PPM_A_TMR);
	ppm_config_channel(PPM_B_TMR);

	// bit_set(TIM2->CR1, TIM_CR1_CEN);
	bit_set(PPM_TRIG_TMR->CR1, TIM_CR1_CEN);

}

void ppm_set_ticks(uint16_t ref1000, uint16_t ref2000, uint16_t *channels)
{
	volatile uint16_t *target;
	for(uint8_t ch=0; ch<8; ch++)
	{
		switch(ch)
		{
			case 0:	target=&PPM_A_TMR->CCR1; break;
			case 1:	target=&PPM_A_TMR->CCR2; break;
			case 2:	target=&PPM_B_TMR->CCR1; break;
			case 3:	target=&PPM_B_TMR->CCR2; break;
			case 4:	target=&PPM_A_TMR->CCR3; break;
			case 5:	target=&PPM_A_TMR->CCR4; break;
			case 6:	target=&PPM_B_TMR->CCR3; break;
			case 7:	target=&PPM_B_TMR->CCR4; break;
		}
		if(channels[ch]!=0)
		{
			*target = 	linear_aproximation(\
								channels[ch],\
								ref1000,\
								PPM_CH_A_TIME_TO_TICKS(1000),\
								ref2000,\
								PPM_CH_A_TIME_TO_TICKS(2000));
			// if(ch==4)
			// 	if(*target < 37500 || *target > 38500)
			// 	{
			// 		uart_send_string_blocking(itoa(*target,7));
			// 		uart_send_byte_blocking('\n');
			// 	}
		}
	}
	//CH1
	// PPM_A_TMR->CCR1=crop_i16(linear_aproximation(channels[0], ref1000, PPM_CH_A_TIME_TO_TICKS(1000), ref2000, PPM_CH_A_TIME_TO_TICKS(2000)), PPM_CLAMP_MIN, PPM_CLAMP_MAX);
	//
	// //CH2
	// PPM_A_TMR->CCR2=crop_i16(linear_aproximation(channels[1], ref1000, PPM_CH_A_TIME_TO_TICKS(1000), ref2000, PPM_CH_A_TIME_TO_TICKS(2000)), PPM_CLAMP_MIN, PPM_CLAMP_MAX);
	//
	// ///CH3
	// PPM_B_TMR->CCR1=crop_i16(linear_aproximation(channels[2], ref1000, PPM_CH_A_TIME_TO_TICKS(1000), ref2000, PPM_CH_A_TIME_TO_TICKS(2000)), PPM_CLAMP_MIN, PPM_CLAMP_MAX);
	//
	// //CH4
	// PPM_B_TMR->CCR2=crop_i16(linear_aproximation(channels[3], ref1000, PPM_CH_A_TIME_TO_TICKS(1000), ref2000, PPM_CH_A_TIME_TO_TICKS(2000)), PPM_CLAMP_MIN, PPM_CLAMP_MAX);
	//
	// //CH5
	// PPM_A_TMR->CCR3=crop_i16(linear_aproximation(channels[4], ref1000, PPM_CH_A_TIME_TO_TICKS(1000), ref2000, PPM_CH_A_TIME_TO_TICKS(2000)), PPM_CLAMP_MIN, PPM_CLAMP_MAX);
	//
	// //CH6
	// PPM_A_TMR->CCR4=crop_i16(linear_aproximation(channels[5], ref1000, PPM_CH_A_TIME_TO_TICKS(1000), ref2000, PPM_CH_A_TIME_TO_TICKS(2000)), PPM_CLAMP_MIN, PPM_CLAMP_MAX);
	//
	// //CH7
	// PPM_B_TMR->CCR3=crop_i16(linear_aproximation(channels[6], ref1000, PPM_CH_A_TIME_TO_TICKS(1000), ref2000, PPM_CH_A_TIME_TO_TICKS(2000)), PPM_CLAMP_MIN, PPM_CLAMP_MAX);
	//
	// //CH8
	// PPM_B_TMR->CCR4=crop_i16(linear_aproximation(channels[7], ref1000, PPM_CH_A_TIME_TO_TICKS(1000), ref2000, PPM_CH_A_TIME_TO_TICKS(2000)), PPM_CLAMP_MIN, PPM_CLAMP_MAX);
}