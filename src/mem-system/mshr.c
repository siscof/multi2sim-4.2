
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

/*void mshr_event_init(int cycles)
{
	//EV_MSHR_DYNAMIC_SIZE_EVAL = esim_register_event_with_name(mshr_control,	gpu_domain_index, "mshr_eval");
	INTERVAL = cycles;
}*/

void mshr_init(struct mshr_t *mshr, int size)
{
	mshr->size = size;
	if(mshr->entries)
		free(mshr->entries);

	mshr->size = size;
	mshr->entries = calloc(size,sizeof(struct mshr_entry_t));
	for(int i = 0;i < mshr->size; i++)
	{
		mshr->entries[i].mshr = mshr;
	}

	//if(flag_mshr_dynamic_enabled && EV_MSHR_DYNAMIC_SIZE_EVAL == NULL)
	//	mshr_event_init(10000);
}

struct mshr_t *mshr_create(int size)
{
	struct mshr_t *mshr;
	mshr = calloc(1,sizeof(struct mshr_t));
	mshr->size = 2048;
	mshr->entries = calloc(mshr->size,sizeof(struct mshr_entry_t));
	mshr->occupied_entries = 0;
	mshr->waiting_list = list_create();
	mshr->access_list = list_create();
	mshr->blocked_list = list_create();
	for(int i = 0;i < mshr->size; i++)
	{
		mshr->entries[i].mshr = mshr;
	}
	return mshr;
}

/*int mshr_pre_lock(struct mshr_t *mshr, struct mod_stack_t *stack)
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
}*/

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
		if(wavefront->id == stack_access->ret_stack->wavefront->id)
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
		if(mshr->size > mshr->occupied_entries)
		{
			assert(list_index_of(mshr->access_list, stack) == -1);
			list_add(mshr->access_list,stack);

			mshr_lock_entry(mshr, stack);
			return 1;
		}else{
			return 0;
		}
	}
	else if(mshr_protocol == mshr_protocol_wavefront_fifo)// && mod->compute_unit && mod->compute_unit->vector_cache == mod && mod->level == 1)
	{
		if(mshr->size > mshr->occupied_entries)
		{
			assert(list_index_of(mshr->access_list, stack) == -1);
			list_add(mshr->access_list,stack);

			mshr_lock_entry(mshr, stack);
			return 1;
		}else{
			return 0;
		}
	}else if(mshr_protocol == mshr_protocol_default){
		if(mshr->size > mshr->occupied_entries)
		{
			assert(list_index_of(mshr->access_list, stack) == -1);
			list_add(mshr->access_list,stack);

			mshr_lock_entry(mshr, stack);
			return 1;
		}else{
			return 0;
		}
	}
	fatal("invalid mshr_protocol : mshr_lock()");
	return 0;
}

struct mshr_entry_t * mshr_find_free_entry(struct mshr_t * mshr)
{
	for(int i = 0;i < mshr->size;i++)
	{
		if(mshr->entries[i].stack == NULL)
			return &(mshr->entries[i]);
	}
	return NULL;
}

void mshr_lock_entry(struct mshr_t *mshr, struct mod_stack_t *stack)
{
	struct mshr_entry_t *entry = mshr_find_free_entry(mshr);
	if(entry == NULL)
		fatal("there are not free entries");

	mem_debug("  %lld %lld 0x%x %s mshr entry lock\n", esim_time, stack->id,
		stack->tag, stack->mod->name);

	entry->stack = stack;
	stack->mshr_entry = entry;
	entry->mshr->occupied_entries++;
}

void mshr_unlock_entry(struct mshr_entry_t *entry)
{
	mem_debug("  %lld %lld 0x%x %s mshr entry unlock\n", esim_time, entry->stack->id,
		entry->stack->tag, entry->stack->mod->name);
	entry->stack->mshr_entry = NULL;
	entry->stack = NULL;
	entry->mshr->occupied_entries--;
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


	mshr_lock_entry(mshr, next_stack);
}

void mshr_unlock(struct mod_t *mod, struct mod_stack_t *stack)
{
	struct mshr_t *mshr = mod->mshr;
	struct mod_stack_t *next_stack;

	assert(mshr->occupied_entries > 0);
	list_remove(mshr->access_list,stack);
	mshr_unlock_entry(stack->mshr_entry);

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

			mshr_lock_entry(mshr, stack);
		}
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

			mshr_lock_entry(mshr, stack);
		}
	}
}

void mshr_free(struct mshr_t *mshr)
{
	list_free(mshr->waiting_list);
	list_free(mshr->access_list);
	list_free(mshr->blocked_list);
	free(mshr->entries);
	free(mshr);
}
