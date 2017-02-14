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
#include "irq.h"

#ifndef CB_TYPE
	#error "used generic circular buffer, without CB_TYPE being set :("
#endif

#ifndef CB_SIZE
	#error "used generic circular buffer, without CB_SIZE being set :("
#endif

#ifndef CB_NAME
	#error "used generic circular buffer, without CB_NAME being set :("
#endif

#define CB_OK				0
#define CB_ERROR_UNDERFLOW	-1
#define CB_ERROR_OVERFLOW	-2

//OMG!
#define helper2(a,b) a##_##b
#define helper(a,b) helper2(a,b)
#define _CB(keyword) helper(CB_NAME,keyword) //Type Specific naming macro

extern CB_TYPE _CB(buffer)[];
extern volatile CB_TYPE *volatile _CB(head);
extern volatile CB_TYPE *volatile _CB(tail);
extern volatile CB_TYPE *const _CB(start);
extern volatile CB_TYPE *const _CB(end);
extern volatile uint8_t _CB(usage);

inline volatile CB_TYPE *volatile _CB(next)(volatile CB_TYPE *volatile cursor)
{
// #ifdef CB_ATOMIC
// ATOMIC_START();
// #endif
	if(cursor == _CB(end)-1)
		return _CB(start);
	else
		return cursor+1;
// #ifdef CB_ATOMIC
// ATOMIC_END_RESTORE();
// #endif
}

int8_t _CB(add)(CB_TYPE data);
int8_t _CB(get)(CB_TYPE *data);
int8_t _CB(peek)(CB_TYPE *data);
int8_t _CB(skip)(void);
void _CB(flush)(void);