
#include <mem-system/mshr.h>
#include <assert.h>

#include <lib/esim/esim.h>

#include "mod-stack.h"
#include <lib/util/list.h>

#include "cache.h"
#include "mem-system.h"
#include "directory.h"
#include <stdlib.h> 

void mshr_init(struct mshr_t *mshr, int size)
{
	mshr->size = size;

}

struct mshr_t *mshr_create()
{
	struct mshr_t *mshr;
	mshr = calloc(1,sizeof(struct mshr_t));
	mshr->size = 2048;
	mshr->entradasOcupadas = 0;
	mshr->waiting_list = list_create();
	return mshr;
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

void mshr_unlock2(struct mod_t *mod)
{
	struct mshr_t *mshr = mod->mshr;
	
	assert(mshr->entradasOcupadas > 0);
	
	
	mshr->entradasOcupadas--;

	if(list_count(mshr->waiting_list))
	{
		struct mod_stack_t *stack_next = (struct mod_stack_t *) list_dequeue(mshr->waiting_list);
		int event = stack_next->waiting_list_event;
		stack_next->waiting_list_event = 0;
		esim_schedule_event(event, stack_next, 0);	
	}

}

void mshr_free(struct mshr_t *mshr)
{
	list_free(mshr->waiting_list);
	free(mshr);
}
/*void mshr_control(int latencia)
{
	int flag = 1;
	for (int k = 0; k < list_count(mem_system->mod_list); k++)
	{
        	struct mod_t *mod = list_get(mem_system->mod_list, k);
		
		if(latencia > 2000)
		{
			if(mod->level == 1 && (mod->dir->ysize * mod->dir->xsize) > mod->mshr_size)
			{
        			mod->mshr_size *= 2;	
			}
		}else{
		     	if(mod->level == 1 && 4 < mod->mshr_size)
                        {
                                mod->mshr_size /= 2;
                        }
		}
		
		if(flag && mod->level == 1)
		{
			flag = 0;
			printf("mshr = %d\n",mod->mshr_size);
		}

	}
}*/


