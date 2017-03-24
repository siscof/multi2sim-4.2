

#ifndef MEM_SYSTEM_MSHR_H
#define MEM_SYSTEM_MSHR_H

#include "mod-stack.h"
#include <stdio.h>
#include <stdbool.h>

extern int flag_mshr_dynamic_enabled;
extern int EV_MSHR_DYNAMIC_SIZE_EVAL;
extern int forzar_mshr_test;
extern int mshr_protocol;

enum mshr_protocol
{
	mshr_protocol_default = 0,
	mshr_protocol_wavefront_fifo,
	mshr_protocol_wavefront_occupancy,
	mshr_protocol_vmb_order
};

struct mshr_t
{
	struct mod_t *mod;
	int size;
	int occupied_entries;
	struct mshr_entry_t *entries;
	struct list_t *waiting_list;
	struct list_t *access_list;
	struct list_t *blocked_list;
	int size_anterior;
	int ipc_anterior;
	int latencia_anterior;

	//test values
	int testing;
	long long cycle;
	long long oper_count;
};

struct mshr_entry_t
{
	struct mod_stack_t *stack;
	struct mshr_t *mshr;
};

void mshr_init(struct mshr_t *mshr, int size);
int mshr_lock(struct mshr_t *mshr, struct mod_stack_t *stack);
void mshr_enqueue(struct mshr_t *mshr, struct mod_stack_t *stack, int event);
void mshr_unlock(struct mod_t *mod, struct mod_stack_t *stack);
//void mshr_wavefront_wakeup(struct mod_t *mod, struct si_wavefront_t *wavefront);
struct mshr_t *mshr_create();
void mshr_free(struct mshr_t *mshr);
void mshr_wakeup(struct mshr_t *mshr, int index);
void mshr_wavefront_wakeup(struct mshr_t *mshr);
void mshr_delay(struct mshr_t *mshr, struct mod_stack_t *stack, int event);
int mshr_pre_lock(struct mshr_t *mshr, struct mod_stack_t *stack);

struct mshr_entry_t * mshr_find_free_entry(struct mshr_t *mshr);
struct list_t * mshr_get_wavefront_list(struct mshr_t *mshr);
void mshr_lock_entry(struct mshr_t *mshr, struct mod_stack_t *stack);
void mshr_unlock_entry(struct mshr_entry_t *entry);
#endif
