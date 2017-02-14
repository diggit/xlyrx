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

#ifndef __MISC_LIB
	#define __MISC_LIB

	#include <inttypes.h>
	//for build time debug
	#define STR_HELPER(x) #x
	#define STR(x) STR_HELPER(x)

	#define uabs(x)  ( ( (x) < 0) ? -(x) : (x) )

	//global macros for bitwise manipulation
	#define bit_get(p,m) ((p) & (m))
	#define bit_set(p,m) ((p) |= (m))
	#define bit_clr(p,m) ((p) &= ~(m))
	#define bit_flip(p,m) ((p) ^= (m))
	#define bit_con(c,p,m) (c ? bit_set(p,m) : bit_clr(p,m)) //macro for conditional bit set/clr
	#define bit_mod(p,m,b) ((p) = ((p)&~(m))|(b) )//mask bits, then orr some
	#define BIT(x) (0x1U << (x))

	//this is epic idea... http://stackoverflow.com/a/7919546
	#define FILL_BITS_FROM_LSB(bits) ((1<<(bits))-1)
	#define FILL_BITS_FROM_MSB(bits) (~((0x80>>((bits)-1))-1)) //meh, you get it?

	//simple NOP, belongs to empty loops etc.
	#define NOP __asm__("NOP");

	#define array_length(A) sizeof(A)/sizeof(A[0])

	#define NULL ((void *)0)

	#define BUFF_SIZE		32

	uint32_t	reduce(uint32_t value, uint8_t decrease);
	char* stoa(const char *str);
	char* ctoa(char c);
	char* itoa_dec_fill(int32_t val,int min,char filler,int decimals);
	char* itoa_fill(int32_t val, int min, char filler);
	char* itoa(int32_t val, int min);
	char* itoa_dec(int32_t val,int min,int decimals);
	// int32_t atoi(char *start);
	void  delay_us(uint16_t delay);
	void  delay_ms(uint16_t delay);
	// uint8_t wait_timeout(volatile uint8_t *port, uint8_t mask, uint8_t timeout, uint8_t high);
	// uint8_t wait_timeout_simple(volatile uint8_t *port, uint8_t mask);
	int32_t linear_aproximation(int32_t x, int32_t x0, int32_t y0, int32_t x1, int32_t y1);

	inline uint16_t crop_ui16(uint16_t val,  uint16_t min, uint16_t max)
	{
		if(val<min)
			return min;
		if(val>max)
			return max;
		return val;
	}

	inline int16_t crop_i16(int16_t val,  int16_t min, int16_t max)
	{
		if(val<min)
			return min;
		if(val>max)
			return max;
		return val;
	}

	inline int32_t crop_i32(int32_t val,  int32_t min, int32_t max)
	{
		if(val<min)
			return min;
		if(val>max)
			return max;
		return val;
	}

	//for fun...
	union anything
	{
		uint8_t		ui8;
		int8_t		 i8;
		uint16_t	ui16;
		int16_t		 i16;
		uint32_t	ui32;
		int32_t		 i32;

		uint8_t		*ui8p;
		int8_t		 *i8p;
		uint16_t	*ui16p;
		int16_t		 *i16p;
		uint32_t	*ui32p;
		int32_t		 *i32p;
		void		*voidp;

		struct
		{
			uint16_t a;
			uint16_t b;
		}u16_pair;
	};

	struct fixed_point
	{
		int16_t number;
		uint8_t decimals;
	};




#endif
