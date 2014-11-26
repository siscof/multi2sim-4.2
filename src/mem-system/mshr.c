#include <assert.h>

#include <lib/esim/esim.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/misc.h>
#include <lib/util/debug.h>

#include "cache.h"
#include "mem-system.h"
#include "mod-stack.h"
#include "mshr.h"

void mshr_init(struct mshr_t *mshr, int size)
{
	mshr->size = size;
	mshr->entradasOcupadas = 0;
	mshr->waiting_list = list_create();

}

int mshr_lock(struct mshr_t *mshr, struct mod_stack_t *stack)
{
	if(mshr->size > mshr->entradasOcupadas)
	{
		mshr->entradasOcupadas++;
		return 1; 
	}
	
	return 0;
	
}

int mshrGetEntradasOcupadas(struct mshr_t *mshr)
{
	return mshr->entradasOcupadas;
}

void mshr_enqueue(struct mshr_t *mshr, struct mod_stack_t *stack, int event)
{
	stack->waiting_list_event = event;
	list_enqueue(mshr->waiting_list, stack);
}

void mshr_unlock(struct mshr_t *mshr)
{
	assert(mshr->entradasOcupadas > 0);

	if(list_count(mshr->waiting_list))
	{
		struct mod_stack_t*stack = (struct mod_stack_t *) list_dequeue(mshr->waiting_list);
		int event = stack->waiting_list_event;
		stack->waiting_list_event = 0;
		esim_schedule_event(event, stack, 0);
	}else{
	        mshr->entradasOcupadas--;
	}
}

