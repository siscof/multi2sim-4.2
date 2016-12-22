
#include <mem-system/mshr.h>
#include <assert.h>

#include <lib/esim/esim.h>

#include "mod-stack.h"
#include <lib/util/list.h>

#include "cache.h"
#include "module.h"
#include "mem-system.h"
#include "directory.h"
#include <arch/southern-islands/timing/gpu.h>
#include <arch/southern-islands/timing/compute-unit.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

int EV_MSHR_DYNAMIC_SIZE_EVAL;
int INTERVAL;
//struct list_t *access_list;

void mshr_event_init(int cycles)
{
	//EV_MSHR_DYNAMIC_SIZE_EVAL = esim_register_event_with_name(mshr_control,	gpu_domain_index, "mshr_eval");
	INTERVAL = cycles;
}

void mshr_init(struct mshr_t *mshr, int size)
{
	mshr->size = size;
	//if(flag_mshr_dynamic_enabled && EV_MSHR_DYNAMIC_SIZE_EVAL == NULL)
	//	mshr_event_init(10000);
}

struct mshr_t *mshr_create()
{
	struct mshr_t *mshr;
	mshr = calloc(1,sizeof(struct mshr_t));
	mshr->size = 2048;
	mshr->entradasOcupadas = 0;
	mshr->waiting_list = list_create();
	mshr->access_list = list_create();
	mshr->blocked_list = list_create();
	return mshr;
}

int mshr_pre_lock(struct mshr_t *mshr, struct mod_stack_t *stack)
{
	if(mshr_protocol == mshr_protocol_wavefront_occupancy)
	{
		if(mshr->mod->compute_unit && mshr->mod->compute_unit->vector_cache == mshr->mod && mshr->mod->level == 1 &&
			stack->wavefront->wavefront_pool_entry->wait_for_mem == 0)
		{
			return 1;
		}else{
			return 0;
		}
	}
	else if(mshr_protocol == mshr_protocol_wavefront_fifo)
	{
		if(mshr->mod->compute_unit && mshr->mod->compute_unit->vector_cache == mshr->mod && mshr->mod->level == 1 &&
			stack->wavefront->wavefront_pool_entry->wait_for_mem == 0)
		{
			return 1;
		}else{
			return 0;
		}
	}
	else if(mshr_protocol == mshr_protocol_default)
	{
		return 0;
	}
	return 0;
}

void mshr_delay(struct mshr_t *mshr, struct mod_stack_t *stack, int event)
{
	stack->waiting_list_event = event;
	list_enqueue(mshr->blocked_list, stack);
}

void mshr_wavefront_wakeup(struct mshr_t *mshr, struct si_wavefront_t *wavefront)
{
	struct mod_stack_t *stack_access;
	for(int j = 0; j < list_count(mshr->blocked_list); j++)
	{
		stack_access = list_get(mshr->blocked_list,j);
		assert(stack_access);
		if(wavefront->id == stack_access->wavefront->id)
		{
			stack_access->event = stack_access->waiting_list_event;
			stack_access->waiting_list_event = 0;
			esim_schedule_mod_stack_event(stack_access, 0);

			list_remove_at(mshr->blocked_list,j);
			j--;
		}
	}
}

int mshr_lock(struct mshr_t *mshr, struct mod_stack_t *stack)
{
	//struct mod_t *mod = stack->mod;

	if(mshr_protocol == mshr_protocol_wavefront_occupancy)
	{
		if(mshr->size > mshr->entradasOcupadas)
		{
			assert(list_index_of(mshr->access_list, stack) == -1);
			list_add(mshr->access_list,stack);
			mshr->entradasOcupadas++;
			return 1;
		}else{
			return 0;
		}
	}
	else if(mshr_protocol == mshr_protocol_wavefront_fifo)// && mod->compute_unit && mod->compute_unit->vector_cache == mod && mod->level == 1)
	{
		if(mshr->size > mshr->entradasOcupadas)
		{
			assert(list_index_of(mshr->access_list, stack) == -1);
			list_add(mshr->access_list,stack);
			mshr->entradasOcupadas++;
			return 1;
		}else{
			return 0;
		}
	}else if(mshr_protocol == mshr_protocol_default){
		if(mshr->size > mshr->entradasOcupadas)
		{
			assert(list_index_of(mshr->access_list, stack) == -1);
			list_add(mshr->access_list,stack);
			mshr->entradasOcupadas++;
			return 1;
		}else{
			return 0;
		}
	}
	fatal("invalid mshr_protocol : mshr_lock()");
	return 0;
}

void mshr_enqueue(struct mshr_t *mshr, struct mod_stack_t *stack, int event)
{
	stack->waiting_list_event = event;
	list_enqueue(mshr->waiting_list, stack);
}

/*void mshr_unlock(struct mshr_t *mshr)
{
	assert(mshr->entradasOcupadas > 0);

	if(list_count(mshr->waiting_list))
	{
		struct mod_stack_t *stack = (struct mod_stack_t *) list_dequeue(mshr->waiting_list);
		int event = stack->waiting_list_event;
		stack->waiting_list_event = 0;
		esim_schedule_event(event, stack, 0);
	}else{
	        mshr->entradasOcupadas--;
	}
}*/
/*
bool_t mshr_wavefront_inflight(struct si_wavefront_t * wavefront)
{
	for(int j = 0; j < list_count(mshr->access_list); j++)
	{
		stack_access = list_get(mshr->access_list,j);
		assert(stack_access);
		if(wavefront->id == stack_access->wavefront->id)
		{
			return true;
		}
	}
	return false;
}

struct list_t * mshr_get_wavefront_list()
{
	struct list_t *wavefront_list = list_create();
	for(int j = 0; j < list_count(mshr->access_list); j++)
	{
		stack_access = list_get(mshr->access_list,j);
		assert(stack_access);

		if(list_index_of(wavefront_list, stack_access->wavefront) == -1)
				list_add(wavefront_list,stack_access->wavefront);
	}
}
*/

void mshr_wakeup(struct mshr_t *mshr, int index)
{
	struct mod_stack_t *next_stack = (struct mod_stack_t *) list_remove_at(mshr->waiting_list, index);
	int event = next_stack->waiting_list_event;
	next_stack->mshr_locked = 1;
	list_add(mshr->access_list,next_stack->ret_stack);
	if(next_stack->ret_stack)
		next_stack->ret_stack->latencias.lock_mshr = asTiming(si_gpu)->cycle - next_stack->ret_stack->latencias.start - next_stack->ret_stack->latencias.queue;
	next_stack->waiting_list_event = 0;
	next_stack->event = event;
	esim_schedule_mod_stack_event(next_stack, 0);
	mshr->entradasOcupadas++;
}

void mshr_unlock(struct mod_t *mod, struct mod_stack_t *stack)
{
	struct mshr_t *mshr = mod->mshr;
	struct mod_stack_t *next_stack;

	assert(mshr->entradasOcupadas > 0);
	list_remove(mshr->access_list,stack);
	mshr->entradasOcupadas--;

	assert(list_index_of(mshr->access_list, stack) == -1);

	if(mshr_protocol == mshr_protocol_wavefront_occupancy)
	{
		/* propuesta 2: */
	}
	else if(mshr_protocol == mshr_protocol_wavefront_fifo /*&& mod->compute_unit && mod->compute_unit->vector_cache == mod && mod->level == 1*/)
	{

		if(list_count(mshr->waiting_list))
		{
			next_stack = (struct mod_stack_t *) list_dequeue(mshr->waiting_list);
			int event = next_stack->waiting_list_event;
			next_stack->mshr_locked = 1;
			list_add(mshr->access_list,next_stack->ret_stack);
			if(next_stack->ret_stack)
				next_stack->ret_stack->latencias.lock_mshr = asTiming(si_gpu)->cycle - next_stack->ret_stack->latencias.start - next_stack->ret_stack->latencias.queue;
			next_stack->waiting_list_event = 0;
			next_stack->event = event;
			esim_schedule_mod_stack_event(next_stack, 0);
			//esim_schedule_event(event, next_stack, 0);
			mshr->entradasOcupadas++;
		}
		//get wavefront in mshr
/*		struct mod_stack_t *stack_access;
		struct list_t *wavefront_list = list_create();
		int inflight_accesses = 0;
		for(int i = 0; i < list_count(mshr->access_list); i++)
		{
			stack_access = list_get(mshr->access_list,i);
			if(list_index_of(wavefront_list, stack_access->wavefront) == -1)
			{
				list_add(wavefront_list, stack_access->wavefront);
				inflight_accesses += list_count(stack_access->wavefront->mem_accesses_list);
			}
		}

		for(int i = 0; i < list_count(wavefront_list); i++)
		{
			struct si_wavefront_t *wavefront = list_get(wavefront_list, i);
			for(int j = 0; j < list_count(mshr->waiting_list); j++)
			{
				stack = list_get(mshr->waiting_list, j);
				if(stack->wavefront->id == wavefront->id)
				{
					if(mshr->size > mshr->entradasOcupadas)
					{
						mshr_wakeup(mshr, j);
						j--;
					}else{
						list_free(wavefront_list);
						return;
					}
				}
			}
		}

		if(inflight_accesses < mshr->size  && mshr->size > mshr->entradasOcupadas && list_count(mshr->waiting_list) > 0)
		{
			for(int i = 0; i < list_count(mshr->waiting_list); i++)
			{
				stack = list_get(mshr->waiting_list, i);
				if(stack->wavefront->wavefront_pool_entry->wait_for_mem != 0)
				{
					mshr_wakeup(mshr, i);
					return;
				}
			}
		}*/
	}else if(mshr_protocol == mshr_protocol_default){
		/* default */
		if(list_count(mshr->waiting_list))
		{
			next_stack = (struct mod_stack_t *) list_dequeue(mshr->waiting_list);
			int event = next_stack->waiting_list_event;
			next_stack->mshr_locked = 1;
			list_add(mshr->access_list,next_stack->ret_stack);
			if(next_stack->ret_stack)
				next_stack->ret_stack->latencias.lock_mshr = asTiming(si_gpu)->cycle - next_stack->ret_stack->latencias.start - next_stack->ret_stack->latencias.queue;
			next_stack->waiting_list_event = 0;
			next_stack->event = event;
			esim_schedule_mod_stack_event(next_stack, 0);
			//esim_schedule_event(event, next_stack, 0);
			mshr->entradasOcupadas++;
		}
	}
}

void mshr_free(struct mshr_t *mshr)
{
	list_free(mshr->waiting_list);
	free(mshr);
}
