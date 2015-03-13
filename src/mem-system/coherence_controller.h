/*
 * coherence_controller.h
 *
 *  Created on: 18/2/2015
 *      Author: sisco
 */

#ifndef MEM_SYSTEM_COHERENCE_CONTROLLER_H
#define MEM_SYSTEM_COHERENCE_CONTROLLER_H

#include <lib/util/list.h>
#include <mem-system/mod-stack.h>
#include <mem-system/module.h>

struct coherence_controller_t{

	struct list_t *transaction_queue;

};

struct coherence_controller_t *cc_create();
void cc_free(struct coherence_controller_t *cc);
int cc_add_transaction(struct coherence_controller_t *cc, struct mod_stack_t *stack, int event);
int cc_finish_transaction(struct coherence_controller_t *cc, struct mod_stack_t *stack);
void cc_search_colisions(struct coherence_controller_t *cc, struct mod_stack_t *stack);
void cc_launch_next_transaction(struct coherence_controller_t *cc, struct mod_stack_t *stack);
int cc_search_next_transaction(struct coherence_controller_t *cc, int set, int way);
struct mod_stack_t *cc_search_transaction(struct coherence_controller_t *cc,int stack_id);
void cc_resume_transaction();
void cc_pause_transaction();
void cc_remove_stack(struct coherence_controller_t *cc, struct mod_stack_t *stack);

#endif /* MEM_SYSTEM_COHERENCE_CONTROLLER_H */
