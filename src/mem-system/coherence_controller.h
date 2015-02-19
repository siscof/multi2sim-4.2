/*
 * coherence_controller.h
 *
 *  Created on: 18/2/2015
 *      Author: sisco
 */

#ifndef MEM_SYSTEM_COHERENCE_CONTROLLER_H
#define MEM_SYSTEM_COHERENCE_CONTROLLER_H


struct coherence_controller_t{

	struct list *transaction_queue;

};

struct coherence_controller_t *cc_create();
int cc_free(struct coherence_controller_t *cc);
int cc_add_transaction(struct coherence_controller *cc, struct mod_stack *stack);
int cc_finish_transaction(struct coherence_controller_t *cc, struct mod_stack_t *stack);

#endif /* MEM_SYSTEM_COHERENCE_CONTROLLER_H */
