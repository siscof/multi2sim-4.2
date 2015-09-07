
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

int EV_MSHR_DYNAMIC_SIZE_EVAL;
int INTERVAL;

void mshr_event_init(int cycles)
{
	EV_MSHR_DYNAMIC_SIZE_EVAL = esim_register_event_with_name(mshr_control,	gpu_domain_index, "mshr_eval");
/* fixme */
	INTERVAL = cycles;
	//esim_schedule_event(EV_MSHR_DYNAMIC_SIZE_EVAL, NULL, INTERVAL);
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
	/*for(int i = 0; i < list_count(mshr->waiting_list);i++)
	{
		struct mod_stack_t *stack_in_list = (struct mod_stack_t *) list_get(mshr->waiting_list, i);
		//assert(stack_in_list->id != stack->id);
	}*/
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

void mshr_unlock(struct mod_t *mod)
{
	struct mshr_t *mshr = mod->mshr;

	assert(mshr->entradasOcupadas > 0);

	if(list_count(mshr->waiting_list))
	{
		struct mod_stack_t *stack = (struct mod_stack_t *) list_dequeue(mshr->waiting_list);
		int event = stack->waiting_list_event;
		stack->mshr_locked = 1;
		if(stack->ret_stack)
			stack->ret_stack->latencias.lock_mshr = asTiming(si_gpu)->cycle - stack->ret_stack->latencias.start - stack->ret_stack->latencias.queue;
		stack->waiting_list_event = 0;
		esim_schedule_event(event, stack, 0);
	}else{
	        mshr->entradasOcupadas--;
	}

}

void mshr_free(struct mshr_t *mshr)
{
	list_free(mshr->waiting_list);
	free(mshr);
}

int temporizador_reinicio = 5;

void mshr_control(int latencia, int opc)
{
  int flag = 1;
	int accion = 0;
  struct mod_t *mod;
	int mshr_size;

  for (int k = 0; k < list_count(mem_system->mod_list); k++)
  {
    mod = list_get(mem_system->mod_list, k);
		if(mod->level == 1)
  		break;
	}

	//GPU running?

	//finalizar test
	if(mod->mshr->testing == 1){
		mshr_size = mshr_evaluar_test();
		accion = 4;
	}else{

	  //reinicio
		temporizador_reinicio--;

		if(temporizador_reinicio <= 0)
		{
			temporizador_reinicio = 5;
			accion = 3;
		}

		// primera decision
		if(!mod->mshr->size_anterior)
		{
			if(latencia > 1000)
				accion = 2;
			else
				accion = 1;
		}

		// dependiendo del OPC
		if(accion == 0 && opc < (mod->mshr->ipc_anterior * 0.95))
		{
			if((mod->mshr->latencia_anterior*1.05) < latencia)
			{
				if(mod->mshr->size > mod->mshr->size_anterior)
					accion = 2;
				else if(mod->mshr->size < mod->mshr->size_anterior)
					accion = 1;
				else
				{
					if(mod->mshr->size == (mod->dir->ysize * mod->dir->xsize))
						accion = 2;
					else
						accion = 1;
				}

			}
		}else if(accion == 0 && opc > (mod->mshr->ipc_anterior * 1.05)){
	  	if(mod->mshr->size > mod->mshr->size_anterior)
	      accion = 1;
	    else if(mod->mshr->size < mod->mshr->size_anterior)
	      accion = 2;
		}
	}
	//dependiendo de la latencia

	/*(latencia > 1000)
        {
		accion = 1;
	}else if(latencia <= 1000){
		accion = 2;
	}*/

	//if(accion)
	//{
    mod->mshr->size_anterior = mod->mshr->size;
		mod->mshr->ipc_anterior = opc;
		mod->mshr->latencia_anterior = latencia;
	//}

	for (int k = 0; k < list_count(mem_system->mod_list); k++)
  {
    mod = list_get(mem_system->mod_list, k);

		if(mod->level == 1)
		{
			switch(accion)
			{
				case 1:	if((mod->mshr->size - 32) < (mod->dir->ysize * mod->dir->xsize))
					{
						if(mod->mshr->size >= 64)
						{
							mod->mshr->size += 32;
							mod->mshr_size += 32;
						}else{
							mod->mshr->size *= 2;
							mod->mshr_size *= 2;
						}
					}
					break;

				case 2:	if(mod->mshr->size > 8)
					{
						if(mod->mshr->size >= 64)
						{
							mod->mshr->size -= 32;
							mod->mshr_size -= 32;
						}else{
							mod->mshr->size /= 2;
							mod->mshr_size /= 2;
						}
					}
					break;

				case 3: mod->mshr->size_anterior = 0;
					mshr_test_sizes();
					break;

				case 4: mod->mshr->size = mshr_size;

				default : break;
			}
		}
  }
	//if(flag_mshr_dynamic_enabled)
		//esim_schedule_event(EV_MSHR_DYNAMIC_SIZE_EVAL, NULL, INTERVAL);

}

void mshr_test_sizes(){
	struct mod_t *mod;
	int testing_cu = 0;
	int max_mshr_size;
	int min_mshr_size = 4;

	for (int k = 0; k < list_count(mem_system->mod_list); k++)
	{
		mod = list_get(mem_system->mod_list, k);

		if(mod_is_vector_cache(mod) && mod->level == 1)
		{

			max_mshr_size = mod->dir->ysize * mod->dir->xsize;
			mod->mshr->testing = 1;

			/*FIXME*/
			int testing_size = min_mshr_size * 2 * (testing_cu + 1);

			if(testing_size > max_mshr_size)
				break;

			mod->mshr->size = testing_size;
			testing_cu++;

			mod->mshr->cycle = mod->compute_unit->cycle;
			mod->mshr->oper_count = mod->compute_unit->oper_count;
		}
	}

}

int mshr_evaluar_test(){

	int opc = 0;
	int testing_cu = 0;
	int best_mshr_size = 0;
	struct mod_t *mod;

	for (int k = 0; k < list_count(mem_system->mod_list); k++)
	{
		mod = list_get(mem_system->mod_list, k);

		if(mod_is_vector_cache(mod) && mod->mshr->testing == 1)
		{
			if(mod->compute_unit->oper_count - mod->mshr->oper_count == 0)
				continue;

			mod->mshr->testing = 0;
			if(opc < ((mod->compute_unit->oper_count - mod->mshr->oper_count) / (mod->compute_unit->cycle - mod->mshr->cycle))){
				opc = ((mod->compute_unit->oper_count - mod->mshr->oper_count) / (mod->compute_unit->cycle - mod->mshr->cycle));
				best_mshr_size = mod->mshr->size;
			}

		}
	}

	//return the best mshr size
	return best_mshr_size;
}
