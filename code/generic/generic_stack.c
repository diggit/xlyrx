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

#include <inttypes.h>
#include "generic_stack.h"

void _S(init) (struct stack_t *stack, STACK_TYPE *buffer, uint8_t size)
{
	stack->top = buffer;
	stack->pointer = 0;
	stack->bottom = stack->top+size-1;
	stack->used = 0;
	stack->size = size;
}
void _S(push) (struct stack_t *stack, uint16_t data)
{
	if(stack->pointer == 0)//push to empty
	{
		stack->pointer = stack->bottom;
		*stack->pointer=data;
		stack->used=1;
	}
	else if(stack->pointer > stack->top)//push next
	{
		*--stack->pointer=data;
		stack->used++;
	}

	else if(stack->pointer == stack->top)
		stack->error = STACK_ERROR_OVERFLOW;

}
STACK_TYPE _S(pop)(struct stack_t *stack)
{
	STACK_TYPE buff;
	if(stack->pointer == stack->bottom)
	{
		buff = *stack->pointer;
		stack->pointer = 0;
		stack->used=0;
		return buff;
	}
	else if(stack->pointer > stack->top)//push next
	{
		stack->used--;
		return *stack->pointer++;
	}
	else if(stack->pointer == 0)//push to empty
		stack->error = STACK_ERROR_OVERFLOW;
	return 1;
}
