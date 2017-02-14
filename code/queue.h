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

#ifndef _QUEUE
	#define _QUEUE


	#include "misc.h"
	#include "irq.h"

	// 0 NULL	(mem_start)
	// 1 task	(first)
	// 2 task
	// 3 task
	// 4 NULL	(last)
	// 5 NULL
	// 6 NULL	(mem_end)


	// void queue_init(struct queue *queue)
	// {
	// 	queue->mem_end=&queue->mem_start[QUEUE_SIZE-1];
	// 	queue->first=queue->mem_start;//not much important as we spin through buffer
	// 	queue->last=queue->mem_start;
	// }

	#define CB_NAME queue
	#define CB_SIZE 16
	#define CB_TYPE struct queue_task
	#define CB_ATOMIC
	#include "irq.h"
	#include "generic/generic_circular_buffer.c"

	int8_t queue_process(void)
	{
		if(queue_usage==0)//nothind process
			return 0;

		struct queue_task task=QUEUE_TASK_EMPTY();

		if(queue_get(&task)!=CB_OK)
			return -1;

		task.handler(task.parameter);

		return 1;

	}

#endif