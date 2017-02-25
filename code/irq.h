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

#ifndef _IRQ_LIB
	#define _IRQ_LIB

	#include <inttypes.h>

	inline __attribute__( ( always_inline ) ) void irq_enable(void)
	{
		__asm__ volatile ("cpsie i" : : : "memory");
	}


	inline __attribute__( ( always_inline ) ) void irq_disable(void)
	{
		__asm__ volatile ("cpsid i" : : : "memory");
	}

	inline __attribute__( ( always_inline ) ) uint8_t irq_enable_prev(void)
	{
		uint32_t result;
		__asm__ volatile ("MRS %0, primask" : "=r" (result) );
		__asm__ volatile ("cpsie i" : : : "memory");
		return(result);
	}


	inline __attribute__( ( always_inline ) ) uint8_t irq_disable_prev(void)
	{
		uint32_t result;
		__asm__ volatile ("MRS %0, primask" : "=r" (result) );
		__asm__ volatile ("cpsid i" : : : "memory");
		return(result);
	}

	inline __attribute__( ( always_inline ) ) uint8_t irq_is_blocked(void)
	{
		uint32_t result;
		__asm__ volatile ("MRS %0, primask" : "=r" (result) );
		return(result);
	}

	#define ATOMIC_START() do{uint32_t irq_was_blocked=irq_disable_prev()
		//code...
	#define ATOMIC_END_RESTORE()\
		if((irq_was_blocked&0x1U)==0)\
			irq_enable();\
		}while(0)

	void irq_NVIC_ISE(int8_t interrupt);

#endif