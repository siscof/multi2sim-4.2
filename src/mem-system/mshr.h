

#ifndef MEM_SYSTEM_MSHR_H
#define MEM_SYSTEM_MSHR_H

#include "mod-stack.h"
#include <lib/util/list.h>

struct mshr_t
{
	int size;
	int entradasOcupadas;
	struct list_t *waiting_list;	
};

void mshr_init(struct mshr_t *mshr, int size);
int mshr_lock(struct mshr_t *mshr, struct mod_stack_t *stack);
void mshr_enqueue(struct mshr_t *mshr, struct mod_stack_t *stack, int event);
void mshr_unlock(struct mshr_t *mshr);

#endif

