/*
 * coherence_controler.c
 *
 *  Created on: 18/2/2015
 *      Author: sisco
 */
#include <mem-system/module.h>
#include <mem-system/mod_stack.h>
#include <lib/mhandle/mhandle.h>

struct coherence_controller_t cc_init()
{
	struct coherence_controller_t *cc;
	cc = xcalloc(1,sizeof(struct coherence_controller_t));
	cc->transaction_queue = list_create_with_size(32);
	return cc;
}

int cc_add_transaction(struct coherence_controller *cc, struct mod_stack *stack)
{
	struct mod_t *mod = stack->mod;


	return dir_entry_lock(mod->dir, stack->set, stack->way, EV_MOD_NMOESI_FIND_AND_LOCK, stack);
}
