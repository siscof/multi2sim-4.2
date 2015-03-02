/*
 * coherence_controler.c
 *
 *  Created on: 18/2/2015
 *      Author: sisco
 */

#include <mem-system/coherence_controller.h>
#include <lib/mhandle/mhandle.h>
#include <mem-system/directory.h>
#include <mem-system/mshr.h>
#include <mem-system/nmoesi-protocol.h>
#include <lib/esim/esim.h>

struct coherence_controller_t *cc_create()
{
	struct coherence_controller_t *cc;
	cc = xcalloc(1,sizeof(struct coherence_controller_t));
	cc->transaction_queue = list_create_with_size(32);
	return cc;
}

void cc_free(struct coherence_controller_t *cc)
{
	list_free(cc->transaction_queue);
	free(cc);
}

// desde el find and lock se tiene que pasar stack->ret
int cc_add_transaction(struct coherence_controller_t *cc, struct mod_stack_t *stack, int event)
{
	struct mod_t *mod = stack->find_and_lock_stack->mod;

	if(list_index_of(cc->transaction_queue,(void *)stack) == -1)
		list_add(cc->transaction_queue,(void *)stack);

	assert(stack->id == stack->find_and_lock_stack->id);

	struct dir_lock_t *dir_lock = dir_lock_get(mod->dir, stack->find_and_lock_stack->set, stack->find_and_lock_stack->way);

	if(dir_lock->lock)
	{
		//cc_search_colisions(cc, stack)
		//puedo procesar la peticion??
		stack->find_and_lock_stack->dir_lock_event = event;
		stack->way = stack->find_and_lock_stack->way;
		return 0;
	}

	int locked = dir_entry_lock(mod->dir, stack->find_and_lock_stack->set, stack->find_and_lock_stack->way, event, stack->find_and_lock_stack);
	assert(locked);
	return 1;
}

void cc_search_colisions(struct coherence_controller_t *cc, struct mod_stack_t *stack)
{
	int colision_detected = -1;

	for(int i = 0; i < list_count(cc->transaction_queue); i++)
	{
		struct mod_stack_t *stack_in_queue = (struct mod_stack_t *) list_get(cc->transaction_queue, i);

		if(stack_in_queue->find_and_lock_stack)
			stack_in_queue = stack_in_queue->find_and_lock_stack;

		if(stack_in_queue->set == stack->find_and_lock_stack->set
				&& stack_in_queue->way == stack->find_and_lock_stack->way)
		{
			colision_detected = i;
		}
	}
	//añadir tareas a la stack principal. esto deberia incrementar el memory level parallelism
}

// este metodo debe liberar el directorio
int cc_finish_transaction(struct coherence_controller_t *cc, struct mod_stack_t *stack)
{

	int index = list_index_of(cc->transaction_queue,(void *)stack);

	assert(index != -1);

	//struct mod_stack_t *removed_stack = (struct mod_stack_t *)
	list_remove_at(cc->transaction_queue, index);

	if(stack->find_and_lock_stack)
		stack = stack->find_and_lock_stack;


	/* Unlock directory entry */
	if (stack->target_mod)
	{
		dir_entry_unlock(stack->target_mod->dir, stack->set, stack->way);

		if(stack->mshr_locked)
		{
			mshr_unlock2(stack->target_mod);
			stack->mshr_locked = 0;
		}
	}else{
		dir_entry_unlock(stack->mod->dir, stack->set, stack->way);

		if(stack->mshr_locked)
		{
			mshr_unlock2(stack->mod);
			stack->mshr_locked = 0;
		}
	}

	cc_launch_next_transaction(cc,stack);
	return 0;
}

void cc_launch_next_transaction(struct coherence_controller_t *cc, struct mod_stack_t *stack)
{
	int index =	cc_search_next_transaction(cc, stack->set, stack->way);

	if(index >= 0)
	{
		struct mod_stack_t *launch_stack = (struct mod_stack_t *) list_get(cc->transaction_queue, index);
		/*int locked = dir_entry_lock(launch_stack->mod->dir, launch_stack->find_and_lock_stack->set, launch_stack->find_and_lock_stack->way, EV_MOD_NMOESI_FIND_AND_LOCK, stack);
		assert(locked);*/

		if(launch_stack->find_and_lock_stack)
		{
			esim_schedule_event(launch_stack->find_and_lock_stack->dir_lock_event, launch_stack->find_and_lock_stack, 1);
		}else{
			esim_schedule_event(launch_stack->dir_lock_event, launch_stack, 1);
		}
	}



}

int cc_search_next_transaction(struct coherence_controller_t *cc, int set, int way)
{
	struct mod_stack_t *stack_in_list;
	for(int i = 0;i < list_count(cc->transaction_queue); i++)
	{
		stack_in_list = (struct mod_stack_t *) list_get(cc->transaction_queue, i);
		if(stack_in_list->find_and_lock_stack)
		{
			stack_in_list = stack_in_list->find_and_lock_stack;
		}
		if( stack_in_list->set == set && stack_in_list->way == way)
		{
			return i;
		}
	}

	return -1;

}

