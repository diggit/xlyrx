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

#include "systick.h"
#include "irq.h"
#include "hardware.h"
#include "uart.h"
#include "stm32f103xb.h"

struct led_blink_type
{
	volatile uint8_t LED;
	volatile uint8_t blinks;
	volatile uint16_t period;
	volatile uint16_t remains;//here is period counted down, period itself must stay untoucher for reloading
	volatile uint8_t state;//remember on/off state
};
struct led_blink_type leds[HARDWARE_LED_COUNT];
uint8_t btn_timer;

void (*volatile button_pressed_callback)(void)=NULL;
void (*volatile button_held_callback)(void)=NULL;
void (*volatile button_released_callback)(void)=NULL;


void systick_init(void)
{
	btn_timer=0;
	for(uint8_t l=0; l<HARDWARE_LED_COUNT; l++)
		leds[l].period=0;//diable all leds blinking

	STK->LOAD = SYSTICK_CALIBRATION;
	STK->CTRL = STK_CTRL_TICKINT | STK_CTRL_ENABLE;
}


int8_t systick_blink_set(uint8_t led, uint8_t blinks, uint16_t period)
{
	if(led>=HARDWARE_LED_COUNT)
		return -1;

	leds[led].LED=led;
	if(blinks==0)
		leds[led].blinks=0;//0 means blink forever
	else
		leds[led].blinks=blinks+1;//1 is out stop value, we must add 1 to blink count
	leds[led].period=period;
	leds[led].remains=0;
	leds[led].state=0;
	return 0;
}

int8_t systick_blink_stop(uint8_t led)
{
	if(led>=HARDWARE_LED_COUNT)
		return -1;

	leds[led].period=0;
	led_off(led);
	return 0;
}

void SysTick_Handler (void)
{

	//button handling
	if(button_read())
	{
		if(btn_timer==0)
		{
			button_state=PRESSED;
			if(button_pressed_callback!=NULL)
				button_pressed_callback();
		}


		if(btn_timer<HARDWARE_BUTTON_HOLD_THRESHOLD)
			btn_timer++;

		else if(btn_timer==HARDWARE_BUTTON_HOLD_THRESHOLD)
		{
			button_state=HELD;
			btn_timer++;
			if(button_held_callback!=NULL)
				button_held_callback();
		}
	}
	else
	{
		button_state=RELEASED;
		if(btn_timer!=0 && button_released_callback!=NULL)
			button_released_callback();
		btn_timer=0;
	}

	//blinking
	for(uint8_t l=0; l<HARDWARE_LED_COUNT; l++)
	{
		if(leds[l].period>0)
		{
			//end of on/off part of cycle
			if(leds[l].remains==0)
			{
				if(leds[l].state==0)
				{
					leds[l].state=1;
					led_on(l);
				}
				else
				{
					leds[l].state=0;
					led_off(l);

					//check if we are in blink counted mode (!=0)
					if(leds[l].blinks>1)
						leds[l].blinks--;
					if(leds[l].blinks==1)//it was last cycle
						leds[l].period=0;//disable
				}
				leds[l].remains=leds[l].period/2-1;//reload for next cycle
			}
			else
				leds[l].remains--;//remaining time of on/off cycle
		}
	}
}

