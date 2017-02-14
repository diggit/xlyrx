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

#ifndef _LIB_GENERIC_STACK
	#define _LIB_GENERIC_STACK

	#include <inttypes.h>

	// #define STACK_TYPE	int16_t
	// #define STACK_NAME	s16
	// #define STACK_SIZE

	//OMG!
	#define helper2(a,b,c) a##_##b##_##c
	#define helper(a,b,c) helper2(a,b,c)
	#ifdef STACK_NAME
		#define _S(a) helper(stack,STACK_NAME,a)
	#else
		#define _S(a) stack_##a
	#endif

	#define STACK_ERROR_NONE		0
	#define STACK_ERROR_OVERFLOW	1
	#define STACK_ERROR_UNDERFLOW	2

	//TOP.......<-PTR->......BOTTOM, NULL~EMPTY

	struct stack_t
	{
		STACK_TYPE *top;
		STACK_TYPE *bottom;
		STACK_TYPE *pointer;
		uint8_t size;
		uint8_t used;
		uint8_t error;
	};


	void _S(init) (struct stack_t *stack, STACK_TYPE *buffer, uint8_t size);
	void _S(push) (struct stack_t *stack, uint16_t data);
	STACK_TYPE _S(pop)(struct stack_t *stack);

	// struct stack_init
#endif
