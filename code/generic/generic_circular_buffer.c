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

#include "generic_circular_buffer.h"
#include <inttypes.h>

CB_TYPE _CB(buffer)[(CB_SIZE)];
CB_TYPE volatile* volatile _CB(head)=_CB(buffer);
CB_TYPE volatile* volatile _CB(tail)=_CB(buffer);
CB_TYPE volatile *const _CB(start)=_CB(buffer);
CB_TYPE volatile *const _CB(end)=_CB(buffer)+(CB_SIZE);//points after buffer, not usable mem
uint8_t volatile _CB(usage)=0;


int8_t _CB(add)(CB_TYPE data)
{
	if(_CB(usage) == (CB_SIZE))
		return CB_ERROR_OVERFLOW;
#ifdef CB_ATOMIC
	ATOMIC_START();
#endif
		*_CB(tail)=data;
		_CB(tail)=_CB(next)(_CB(tail));
		_CB(usage)++;
#ifdef CB_ATOMIC
	ATOMIC_END_RESTORE();
#endif
	return CB_OK;
}

int8_t _CB(get)(CB_TYPE *data)
{
	if(_CB(usage) == 0)
		return CB_ERROR_UNDERFLOW;

#ifdef CB_ATOMIC
	ATOMIC_START();
#endif
		*data=*_CB(head);
		_CB(head)=_CB(next)(_CB(head));
		_CB(usage)--;
#ifdef CB_ATOMIC
	ATOMIC_END_RESTORE();
#endif
	return CB_OK;
}

int8_t _CB(peek)(CB_TYPE *data)
{
	if(_CB(usage) == 0)
		return CB_ERROR_UNDERFLOW;

#ifdef CB_ATOMIC
	ATOMIC_START();
#endif
		*data=*_CB(head);
#ifdef CB_ATOMIC
	ATOMIC_END_RESTORE();
#endif
	return CB_OK;
}

int8_t _CB(skip)(void)
{
	if(_CB(usage) == 0)
		return CB_ERROR_UNDERFLOW;

#ifdef CB_ATOMIC
	ATOMIC_START();
#endif
		_CB(head)=_CB(next)(_CB(head));
		_CB(usage)--;
#ifdef CB_ATOMIC
	ATOMIC_END_RESTORE();
#endif
	return CB_OK;
}


void _CB(flush)(void)
{
#ifdef CB_ATOMIC
	ATOMIC_START();
#endif
		_CB(head)=_CB(start);
		_CB(tail)=_CB(end);
		_CB(usage)=0;
#ifdef CB_ATOMIC
	ATOMIC_END_RESTORE();
#endif
}

//prevent affection of other CBs
#ifdef CB_NAME
	#undef CB_NAME
#endif

#ifdef CB_SIZE
	#undef CB_SIZE
#endif

#ifdef CB_TYPE
	#undef CB_TYPE
#endif

#ifdef CB_ATOMIC
	#undef CB_ATOMIC
#endif
