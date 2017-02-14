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

#ifndef _TIMERS_LIB
	#define _TIMERS_LIB
	#include "config.h"
	#include "misc.h"
	#include <inttypes.h>

	#define SWDT_1ms	(1000/SWDT_RESOLUTION)

	void swdt_init(void);
	void swdt_restart(uint16_t timeout);
	void swdt_set_callback(void (*callback)(void));
	uint16_t swdt_stop(void);

	inline uint16_t swdt_get(void)
	{
		return TIM1->CNT;
	}

	#ifndef SWDT_F_IN
		#error "input frequency SWDT_F_IN not defined"
	#endif

	#ifndef SWDT_RESOLUTION
		#error SWDT_RESOLUTION not defined, (1~1000us)
	#endif

	#if SWDT_RESOLUTION >1000 || SWDT_RESOLUTION<1
		#error SWDT_RESOLUTION out of range (1~1000us)
	#endif

	#define PPM_TRIG_TMR	TIM4
	#define PPM_A_TMR		TIM2
	#define PPM_B_TMR		TIM3

	#define PPM_TIMER_MAX	65000U//with this value in CCxR we get minimal length pulse
	#define PPM_TIMER_MIN	1//on 1 we get maximal length, 0 does not generate pulses at all, loot into DSH, OPM mode

	#define PPM_CLAMP_MIN	900
	#define PPM_CLAMP_MAX	2600

	#define PPM_TIME_TO_TICKS(T)
	#define PPM_CH_A_PRESCALER	(PPM_CH_A_F_IN/(T_us/PPM_PULSE_ABS_MAX_us *PPM_TIMER_MAX) + 1)//+1 to be sure to be able to achieve PPM_PULSE_ABS_MAX long pulse

	#define PPM_CH_A_TIME_TO_TICKS(T) (PPM_TIMER_MAX-T*(PPM_CH_A_F_IN/(PPM_CH_A_PRESCALER*T_us)))

	void ppm_init(void);
	void ppm_set_ticks(uint16_t ref1000, uint16_t ref2000, uint16_t *channels);

#endif