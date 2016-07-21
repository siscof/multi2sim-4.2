

#ifndef MEM_SYSTEM_MSHR_H
#define MEM_SYSTEM_MSHR_H

#include "mod-stack.h"
#include <stdio.h>
#include <arch/southern-islands/emu/wavefront.h>

extern int flag_mshr_dynamic_enabled;
extern int EV_MSHR_DYNAMIC_SIZE_EVAL;
extern int forzar_mshr_test;
extern int mshr_protocol;

/* Access type */
enum mshr_protocol
{
	mshr_protocol_default = 0,
	mshr_protocol_wavefront
};

struct mshr_t
{
	struct mod_t *mod;
	int size;
	int entradasOcupadas;
	struct list_t *waiting_list;
	struct list_t *accesses_list;
	int size_anterior;
	int ipc_anterior;
	int latencia_anterior;
	struct list_t *wavefront_list;

	struct list_t *wavefront_waiting_list;
	int entradas_ocupadas_wf;

	//test values
	int testing;
	long long cycle;
	long long oper_count;
};

void mshr_init(struct mshr_t *mshr, int size);
int mshr_lock(struct mshr_t *mshr, struct mod_stack_t *stack);
void mshr_enqueue(struct mshr_t *mshr, struct mod_stack_t *stack, int event);
void mshr_unlock(struct mod_t *mod);
//void mshr_unlock(struct mshr_t *mshr);
//void mshr_unlock2(struct mod_t *mod);
struct mshr_t *mshr_create();
void mshr_free(struct mshr_t *mshr);
void mshr_control(int event, void *data);
void mshr_control2();
void mshr_test_sizes();
int mshr_evaluar_test();
void mshr_wakeup(struct mshr_t *mshr, int id);
int mshr_wavefront_misses_count(struct mshr_t *mshr, int id);
int mshr_wavefront_lock(struct mod_t *mod, struct si_wavefront_t *wavefront);
void mshr_wavefront_unlock(struct mod_t *mod, struct si_wavefront_t *wavefront);
void mshr_access(struct mshr_t *mshr, struct mod_stack_t *stack);
void mshr_leave(struct mshr_t *mshr, struct mod_stack_t *stack);
void mshr_wakeup_id(struct mshr_t *mshr, int id);

#endif
