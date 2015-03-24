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

	struct mod_port_t *port = stack->find_and_lock_stack->port;

	//int index = cc_search_transaction_index(cc,stack->id);
	int index = list_index_of(cc->transaction_queue,(void *)stack);


	if(index == -1)
		list_add(cc->transaction_queue,(void *)stack);


	assert(stack->id == stack->find_and_lock_stack->id);

	struct dir_lock_t *dir_lock = dir_lock_get(mod->dir, stack->find_and_lock_stack->set, stack->find_and_lock_stack->way);

	assert(stack->id != dir_lock->stack_id);

	if(dir_lock->lock)
	{

		stack->transaction_idle = 1;
		stack->find_and_lock_stack->transaction_idle = 1;
		struct mod_stack_t *stack_locked = NULL;
		if(!stack->find_and_lock_stack->blocking)
		{
			//up-down
			stack_locked = stack->ret_stack;
			while(stack_locked->ret_stack)
				stack_locked = stack_locked->ret_stack;
			//transaction is blocked
			stack_locked->transaction_blocked = 1;
			stack_locked->stack_superior == stack;

		}else if(stack->mod->level != 1){
			//else down-up

			stack_locked = cc_search_transaction(cc,dir_lock->stack_id);
			//the transaction is blocking other transaction
			stack_locked->transaction_blocking = 1;
			//stack->high_priority_transaction = 1;
			dir_entry_lock(mod->dir, stack->find_and_lock_stack->set, stack->find_and_lock_stack->way, event, stack->find_and_lock_stack);
			printf("stack_id = %d  |  mod_level = %d  |  mod = %d  |  blocking = %d\n",stack->id,stack->mod->level, mod->level, stack->find_and_lock_stack->blocking);
			stack->transaction_idle = 0;
			stack->find_and_lock_stack->transaction_idle = 0;
		}

		if(stack->find_and_lock_stack->mshr_locked)
		{
			mshr_unlock2(mod);
			stack->find_and_lock_stack->mshr_locked = 0;
		}

		mod_unlock_port(mod, port, stack->find_and_lock_stack);
		stack->port_locked = 0;

		//cc_search_colisions(cc, stack)
		//puedo procesar la peticion??
		stack->find_and_lock_stack->dir_lock_event = event;
		stack->way = stack->find_and_lock_stack->way;


		//con el funcionamiento normal si que funciona correctamente
		if(stack_locked && stack_locked->transaction_blocked && stack_locked->transaction_blocking)
		//if(!stack->find_and_lock_stack->blocking)
		{
			printf("REMOVING -> stack_id = %d  |  mod_level = %d  |  mod = %d",stack_locked->id,stack_locked->mod->level, mod->level);
			if(stack_locked->find_and_lock_stack)
				printf("  |  blocking = %d", stack_locked->find_and_lock_stack->blocking);
			printf("\n");
			if(stack_locked->stack_superior->target_mod)
				cc_remove_stack(stack_locked->stack_superior->target_mod->coherence_controller,
					stack_locked->stack_superior);
			else
				cc_remove_stack(stack_locked->stack_superior->mod->coherence_controller,
					stack_locked->stack_superior);
		}

		return 0;
	}

	//prueba
	if(stack->ret_stack && stack->ret_stack->transaction_blocked == 1)
			stack->ret_stack->transaction_blocked = 0;
	//hasta aqui

	int locked = dir_entry_lock(mod->dir, stack->find_and_lock_stack->set, stack->find_and_lock_stack->way, event, stack->find_and_lock_stack);
	assert(locked);

	struct mod_stack_t *aux = NULL;
	if(stack->find_and_lock_stack)
		aux = stack->find_and_lock_stack;
	else
		aux = stack;

	aux->transaction_idle = 0;
	aux->transaction_blocked = 0;
	while(aux->ret_stack)
	{
		aux->transaction_idle = 0;
		aux->transaction_blocked = 0;
		aux = aux->ret_stack;
	}
	//stack->transaction_idle = 0;
	//stack->find_and_lock_stack->transaction_idle = 0;
	return 1;
}

void cc_remove_stack(struct coherence_controller_t *cc, struct mod_stack_t *stack)
{
	//llegar a la ultima stack
	while(stack->stack_superior)
		stack = stack->stack_superior;

	if(stack->target_mod)
		cc = stack->target_mod->coherence_controller;
	else
		cc = stack->mod->coherence_controller;

	int index = cc_search_transaction_index(cc,stack->id);

	//int index = list_index_of(cc->transaction_queue,(void *)stack);

	//assert(index != -1);

	struct mod_stack_t *stack_in_list = list_remove_at(cc->transaction_queue, index);

	if(stack_in_list->find_and_lock_stack)
		stack_in_list = stack_in_list->find_and_lock_stack;

	assert(stack_in_list->transaction_idle);

	stack_in_list->transaction_idle = 0;

	//if(stack_in_list->ret_stack)
	struct mod_stack_t *aux = stack_in_list->ret_stack;
	while(aux)
	{
		aux->transaction_idle = 0;
		aux = aux->ret_stack;
	}

	stack_in_list->ret_stack->err = 1;
	mod_stack_return(stack_in_list);
}

void cc_pause_transaction()
{

}

void cc_resume_transaction()
{

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
	//int index = cc_search_transaction_index(cc,stack->id);

	assert(index != -1);

	//struct mod_stack_t *removed_stack = (struct mod_stack_t *)
	list_remove_at(cc->transaction_queue, index);

	if(stack->find_and_lock_stack)
		stack = stack->find_and_lock_stack;


	/* Unlock directory entry */
	/*if (stack->target_mod)
	{
		dir_entry_unlock_stack(stack->target_mod->dir, stack->set, stack->way, stack);

		if(stack->mshr_locked)
		{
			mshr_unlock2(stack->target_mod);
			stack->mshr_locked = 0;
		}
	}else{
		dir_entry_unlock_stack(stack->mod->dir, stack->set, stack->way, stack);

		if(stack->mshr_locked)
		{
			mshr_unlock2(stack->mod);
			stack->mshr_locked = 0;
		}
	}*/

	stack->transaction_blocked = 0;
	stack->transaction_blocking = 0;
	//cc_launch_next_transaction(cc,stack);
	return 0;
}

void cc_launch_next_transaction(struct coherence_controller_t *cc, struct mod_stack_t *stack)
{
	if(stack->find_and_lock_stack)
		stack = stack->find_and_lock_stack;

	int index =	cc_search_next_transaction(cc, stack->set, stack->way);

	if(index >= 0)
	{
		struct mod_stack_t *launch_stack = (struct mod_stack_t *) list_get(cc->transaction_queue, index);

		if(launch_stack->find_and_lock_stack)
		{
			esim_schedule_event(launch_stack->find_and_lock_stack->dir_lock_event, launch_stack->find_and_lock_stack, 1);
			launch_stack->find_and_lock_stack->dir_lock_event = 0;
		}else{
			esim_schedule_event(launch_stack->dir_lock_event, launch_stack, 1);
			launch_stack->dir_lock_event = 0;
		}
		launch_stack->transaction_idle = 0;
		launch_stack->transaction_blocked = 0;
		while(launch_stack->ret_stack)
		{
			launch_stack->transaction_idle = 0;
			launch_stack->transaction_blocked = 0;
			launch_stack = launch_stack->ret_stack;
		}
		//if(launch_stack->find_and_lock_stack)
		//	launch_stack->find_and_lock_stack->transaction_idle = 0;
	}



}
struct mod_stack_t *cc_search_transaction(struct coherence_controller_t *cc,int stack_id)
{
	struct mod_stack_t *stack_in_list;
	for(int i = 0;i < list_count(cc->transaction_queue); i++)
	{
		stack_in_list = (struct mod_stack_t *) list_get(cc->transaction_queue, i);
		if(stack_in_list->id == stack_id)
			return stack_in_list;
	}
	return NULL;
}

int cc_search_transaction_index(struct coherence_controller_t *cc,int stack_id)
{
	struct mod_stack_t *stack_in_list;
	for(int i = 0;i < list_count(cc->transaction_queue); i++)
	{
		stack_in_list = (struct mod_stack_t *) list_get(cc->transaction_queue, i);
		if(stack_in_list->id == stack_id)
			return i;
	}
	return -1;
}

int cc_search_next_transaction(struct coherence_controller_t *cc, int set, int way)
{
	struct mod_stack_t *stack_in_list;
	for(int i = 0;i < list_count(cc->transaction_queue); i++)
	{
		stack_in_list = (struct mod_stack_t *) list_get(cc->transaction_queue, i);

		if(stack_in_list->id == 0 || stack_in_list->id > mod_stack_id)
		{
					list_remove_at(cc->transaction_queue, i);
					i--;
					continue;
		}

		if(stack_in_list->find_and_lock_stack)
			stack_in_list = stack_in_list->find_and_lock_stack;

		if(stack_in_list->dir_lock_event == 0)
			continue;

		if(stack_in_list->transaction_idle == 0)
			continue;

		if( stack_in_list->set == set && stack_in_list->way == way)
		{
			return i;
		}
	}

	return -1;

}

