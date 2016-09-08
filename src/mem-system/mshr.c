
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
	mshr->access_list = list_create();
	return mshr;
}

int mshr_lock(struct mshr_t *mshr, struct mod_stack_t *stack)
{
struct mod_t *mod = stack->mod;
	if(mshr->size > mshr->entradasOcupadas)
	{
		if(mshr_protocol == mshr_protocol_wavefront && mod->compute_unit && mod->compute_unit->vector_cache == mod && mod->level == 1)
		{
			struct mod_stack_t *stack_access;
			bool wavefront_permitido = false;
			struct list_t *wavefront_list = list_create();
			for(int j = 0; j < list_count(mshr->access_list); j++)
			{
				stack_access = list_get(mshr->access_list,j);
				assert(stack_access);
				if(stack->wavefront->id == stack_access->wavefront->id)
				{
					wavefront_permitido = true;
					break;
				}

				if(list_index_of(wavefront_list, stack_access->wavefront) == -1)
						list_add(wavefront_list,stack_access->wavefront);
			}
			if(!wavefront_permitido)
			{
				for(int i = 0; i < list_count(wavefront_list); i++)
				{
					struct si_wavefront_t *wavefront_in_mshr = list_get(wavefront_list,i);
					if(wavefront_in_mshr->wavefront_pool_entry->wait_for_mem == 0)
					{
						list_free(wavefront_list);
						return 0;
					}
				}
			}
			list_free(wavefront_list);
		}
		list_add(mshr->access_list,stack);
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

void mshr_unlock_si(struct mod_t *mod, struct mod_stack_t *stack)
{
	struct mshr_t *mshr = mod->mshr;
	struct mod_stack_t *next_stack;

	assert(mshr->entradasOcupadas > 0);
	list_remove(mshr->access_list,stack);
	mshr->entradasOcupadas--;

	if(mshr_protocol == mshr_protocol_wavefront && mod->compute_unit && mod->compute_unit->vector_cache == mod && mod->level == 1)
	{
		assert(mshr->entradasOcupadas < mshr->size);
		for(int i = 0; i < list_count(mshr->waiting_list); i++)
		{
			if(mshr->entradasOcupadas >= mshr->size)
				return;

			struct mod_stack_t *stack_access;
			bool wavefront_permitido = false;
			struct list_t *wavefront_list = list_create();
			next_stack = (struct mod_stack_t *) list_get(mshr->waiting_list,i);

			for(int j = 0; j < list_count(mshr->access_list); j++)
			{
				stack_access = list_get(mshr->access_list,j);
				assert(stack_access);
				if(next_stack->wavefront->id == stack_access->wavefront->id)
				{
					wavefront_permitido = true;
					break;
				}

				if(list_index_of(wavefront_list, stack_access->wavefront) == -1)
						list_add(wavefront_list,stack_access->wavefront);
			}
			if(!wavefront_permitido)
			{
				wavefront_permitido = true;
				for(int i = 0; i < list_count(wavefront_list); i++)
				{
					struct si_wavefront_t *wavefront_in_mshr = list_get(wavefront_list,i);
					if(wavefront_in_mshr->wavefront_pool_entry->wait_for_mem != 0)
					{
						wavefront_permitido = false;
					}
				}
			}
			list_free(wavefront_list);

			if(wavefront_permitido)
			{
				list_remove_at(mshr->waiting_list,i);
				int event = next_stack->waiting_list_event;
				next_stack->mshr_locked = 1;
				assert(next_stack->ret_stack);
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
	}else{

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

int temporizador_reinicio = 3;

void mshr_control(int event, void *data)
{
/* fixme latencia and opc dont do anything */
	int latencia = 0, opc = 0;

	fatal("mshr_control is broken");
	int accion = 0;
  struct mod_t *mod;
	int mshr_size;

	printf("llamada a mshr_control()");

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

void mshr_control2()
{
    int accion = 0;
    struct mod_t *mod;
    int mshr_size;

	if(flag_mshr_dynamic_enabled == 0)
		return;

    for (int k = 0; k < list_count(mem_system->mod_list); k++)
    {
        mod = list_get(mem_system->mod_list, k);
            if(mod->level == 1 && mod_is_vector_cache(mod))
                break;
    }

	//GPU running?

	//finalizar test
	if(mod->mshr->testing == 1){
		mshr_size = mshr_evaluar_test();
		temporizador_reinicio = 3;
		accion = 4;
	}else{

	  //reinicio
		temporizador_reinicio--;

		if(temporizador_reinicio <= 0)
		{
			temporizador_reinicio = 3;
			accion = 3;
		}
  }

	switch(accion)
	{
		case 3: mshr_test_sizes();
						break;

		case 4: for (int k = 0; k < list_count(mem_system->mod_list); k++)
						{
							mod = list_get(mem_system->mod_list, k);
							if(mod->level == 1 && mod_is_vector_cache(mod))
							{
								mod->mshr->size = mshr_size;
							}
						}
						break;

		default: break;
  }
	//if(flag_mshr_dynamic_enabled)
		//esim_schedule_event(EV_MSHR_DYNAMIC_SIZE_EVAL, NULL, INTERVAL);

}

long long desplazamiento = 0;
void mshr_test_sizes(){
	struct mod_t *mod;
	//int testing_cu = 0;
	//int max_mshr_size;
	//int min_mshr_size = 4;
	int size[] = {8, 16, 16, 16, 32, 32, 32,64,128,256};

	for (int k = 0; k < list_count(mem_system->mod_list); k++)
	{
		mod = list_get(mem_system->mod_list, k);

		if(mod_is_vector_cache(mod) && mod->level == 1)
		{
			//mod->compute_unit->compute_device->opc ;
			float opc_actual = mod->compute_unit->compute_device->op / (float)mod->compute_unit->compute_device->cycles;
			if ((mod->compute_unit->compute_device->opc * 0.95) < opc_actual)
			{
				size[(0+desplazamiento)%10] = mod->mshr->size;
				size[(1+desplazamiento)%10] = mod->mshr->size;
				size[(2+desplazamiento)%10] = mod->mshr->size;
				size[(3+desplazamiento)%10] = mod->mshr->size;
				size[(4+desplazamiento)%10] = mod->mshr->size;
				size[(5+desplazamiento)%10] = 8;
				size[(6+desplazamiento)%10] = 16;
				size[(7+desplazamiento)%10] = 32;
				size[(8+desplazamiento)%10] = 64;
				size[(9+desplazamiento)%10] = 256;
			}else{
				size[(0+desplazamiento)%10] = 8;
				size[(1+desplazamiento)%10] = 16;
				size[(2+desplazamiento)%10] = 16;
				size[(3+desplazamiento)%10] = 16;
				size[(4+desplazamiento)%10] = 32;
				size[(5+desplazamiento)%10] = 32;
				size[(6+desplazamiento)%10] = 32;
				size[(7+desplazamiento)%10] = 64;
				size[(8+desplazamiento)%10] = 128;
				size[(9+desplazamiento)%10] = 256;

			}
			desplazamiento++;

			// restart gpu counters
			mod->compute_unit->compute_device->opc = opc_actual;
			mod->compute_unit->compute_device->op = 0;
			mod->compute_unit->compute_device->cycles = 0;
			break;
		}
	}

	for (int k = 0; k < list_count(mem_system->mod_list); k++)
	{
		mod = list_get(mem_system->mod_list, k);

		if(mod_is_vector_cache(mod) && mod->level == 1)
		{


			//max_mshr_size = mod->dir->ysize * mod->dir->xsize;

			//int mshr_interval = max_mshr_size / si_gpu_num_compute_units;

			mod->mshr->testing = 1;
			mod->mshr->size = size[mod->compute_unit->id%10];
			//testing_cu++;
			//mod->mshr->size = mshr_interval * testing_cu;

			mod->mshr->cycle = mod->compute_unit->cycle;
			mod->mshr->oper_count = mod->compute_unit->oper_count;

		}
	}

	//int mshr_interval = max_mshr_size / si_gpu_num_compute_units;

	/*for (int k = 0; k < list_count(mem_system->mod_list); k++)
	{
		mod = list_get(mem_system->mod_list, k);

		if(mod_is_vector_cache(mod) && mod->level == 1)
		{
			testing_cu++;
      mod->mshr->testing = 1;
			mod->mshr->size = size[testing_cu];
			testing_cu++;
			//mod->mshr->size = mshr_interval * testing_cu;

			mod->mshr->cycle = mod->compute_unit->cycle;
			mod->mshr->oper_count = mod->compute_unit->oper_count;
		}
	}*/

}

int mshr_evaluar_test(){

	int opc = 0;
	//int testing_cu = 0;
	int best_mshr_size = 4;
	struct mod_t *mod;

	for (int k = 0; k < list_count(mem_system->mod_list); k++)
	{
		mod = list_get(mem_system->mod_list, k);

		if(mod_is_vector_cache(mod) && mod->mshr->testing == 1)
		{
			mod->mshr->testing = 0;
			long long op = mod->compute_unit->oper_count - mod->mshr->oper_count;
			long long cycles = mod->compute_unit->cycle - mod->mshr->cycle;
			if(op == 0)
				continue;

			if(opc < (op / (float)cycles)){
				opc = op / (float)cycles;
				mod->compute_unit->compute_device->interval_statistics->predicted_opc_cycles = cycles;
				mod->compute_unit->compute_device->interval_statistics->predicted_opc_op = op;

				best_mshr_size = mod->mshr->size;
			}

		}
	}

	if(best_mshr_size < 1)
		best_mshr_size = 2;

	//return the best mshr size
	if(forzar_mshr_test == 0)
		return best_mshr_size;
	else
		return 32;
}
