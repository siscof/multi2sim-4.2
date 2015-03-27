
#ifndef LIB_UTIL_ESTADISTICAS_H
#define LIB_UTIL_ESTADISTICAS_H

#include <mem-system/mem-system.h>
#include <mem-system/directory.h>
#include <mem-system/module.h>
#include <lib/util/debug.h>
#include <arch/southern-islands/emu/emu.h>
#include <lib/util/list.h>
#include <arch/southern-islands/timing/uop.h>
#include <arch/southern-islands/timing/wavefront-pool.h>
#include <arch/southern-islands/emu/wavefront.h>
#include <mem-system/cache.h>

#define cache_hit 1
#define cache_accesses 0

typedef enum
{
	scalar_u = 0,
	simd_u,
	s_mem_u,
	v_mem_u,
	branch_u,
	lds_u
}si_units;

typedef enum
{
	si_uop_start = 0,
	si_uop_fetch,
	si_uop_issue,
	si_uop_finish
}latencies;

typedef enum
{
	load_action_retry = 0,
	load_miss_retry,
	nc_store_writeback_retry,
	nc_store_action_retry,
	nc_store_miss_retry,
	num_retries_kinds
}retries_kinds_t;

struct retry_stats_t
{
	long long counter;
	long long cycles;
};

extern long long ventana_muestreo;

extern char *fran_file_ipc;
extern char *fran_file_general;
extern char *fran_file_t1000k;
extern char *fran_file_hitRatio;
extern char *fran_file_red;
extern char *report_cache_states;
extern int SALTAR_L1;
extern int replace; // cache = 0; mod = 1


#define fran_debug_ipc(...) debug(fran_ipc, __VA_ARGS__)
int fran_ipc;

#define fran_debug_general(...) debug(fran_general, __VA_ARGS__)
int fran_general;

#define fran_debug_t1000k(...) debug(fran_t1000k, __VA_ARGS__)
int fran_t1000k;

#define fran_debug_hitRatio(...) debug(fran_hitRatio, __VA_ARGS__)
int fran_hitRatio;

#define fran_debug_red(...) debug(fran_red, __VA_ARGS__)
int fran_red;

#define report_cache_states(...) debug(report_cache_states_category, __VA_ARGS__)
int report_cache_states_category;

struct esta_t
{
        /* Statistics */
        long long coalesce;
        long long accesses;
        long long hits;
	long long misses;
        long long loads;
        long long invalidations;
	long long evictions;

	long long cache_state[cache_block_state_size];

	long long load_action_retry;
	long long load_miss_retry;
	long long nc_store_writeback_retry;
	long long nc_store_action_retry;
	long long nc_store_miss_retry;

	long long busy_cicles_in;
        long long busy_cicles_out;
        long long delayed_read_hit;
        long long esim_cycle_anterior;
        long long media_latencia;
        long long media_latencia_contador;
        long long tiempo_acceso_latencia;
	long long retry_time_lost;

	long long latencia_red_acc;
        long long latencia_red_cont;
        long long blk_compartidos;
        long long replicas_en_l1;
	long long entradas_bloqueadas;
	long long coalesceHits;
	long long coalesceMisses;
};

struct si_gpu_unit_stats
{
	long long unit[6];
	//sustituir cuando se pueda "unit[6]" por "inst[6]" para que tenga mas sentido junto con macroinst[6]
	long long macroinst[6];
	long long total;
	long long loads_latency;
	long long loads_count;
	long long simd_idle[4];
	long long v_mem_full;
	
	//latecias de las instrucciones de acceso a memoria
	struct latenciometro *latencias_load;
	struct latenciometro *latencias_nc_write;

	//instruction latency
	long long start2fetch;		
	long long fetch2complete;  
	
	//stall causes in dispatch stage 
	long long cycles_simd_running;
	long long dispatch_no_stall;
	long long dispatch_stall_instruction_infly;
	long long dispatch_stall_barrier;
	long long dispatch_stall_mem_access;
	long long dispatch_stall_no_wavefront;
	long long dispatch_stall_others;
	
	//type instruction in fly on dispatch
	long long dispatch_branch_instruction_infly;
	long long dispatch_scalar_instruction_infly;
	long long dispatch_mem_scalar_instruction_infly;
	long long dispatch_simd_instruction_infly;
	long long dispatch_v_mem_instruction_infly;
	long long dispatch_lds_instruction_infly;
	
	//stall causes in fetch stage
	long long no_stall;
	long long stall_instruction_infly;
	long long stall_barrier;
	long long stall_mem_access;
	long long stall_fetch_buffer_full;
	long long stall_no_wavefront;
	long long stall_others;

	// MSHR
	long long superintervalo_latencia;
	long long superintervalo_contador;
	long long superintervalo_operacion;
	long long superintervalo_ciclos;
}; 

struct mem_system_stats
{
	struct esta_t mod_level[5];
	long long load_latency;
	long long load_latency_count;
	struct latenciometro *latencias_load;
	struct latenciometro *latencias_nc_write;

	struct retry_stats_t retries[num_retries_kinds];
	long long stacks_with_retries;
	// MSHR
	long long superintervalo_latencia;
	long long superintervalo_contador;
	long long superintervalo_operacion;
	long long superintervalo_ciclos;
};

struct latenciometro
{
	long long start;
	long long queue;
	long long lock_mshr;
	long long lock_dir;
	long long eviction;
	long long retry;
	long long miss;
	long long finish;
	long long access;
	long long invalidar;
	long long total;
};

struct mem_system_stats mem_stats, instrucciones_mem_stats_anterior, ciclos_mem_stats_anterior;

struct si_gpu_unit_stats *gpu_inst;
struct esta_t estadis[10];
struct si_gpu_unit_stats gpu_stats, instrucciones_gpu_stats_anterior; 

struct esta_t *estadisticas_ipc;

void estadisticas_por_intervalos(long long intervalo);
void ipc_instructions(long long cycle, si_units unit);

void mem_load_finish(long long lat);
void hrl2(int hit , struct mod_t *mod, int from_load);
void estadisticas(int hit, int lvl);
void ini_estadisticas();
void add_coalesce(int level);
void add_access(int level);
void add_hit(int level);
void add_miss(int level);
long long add_si_inst(si_units unit);
long long add_si_macroinst(si_units unit, struct si_uop_t *uop);
void add_CoalesceHit(int level);
void add_CoalesceMiss(int level);
void load_finish(long long latencia, long long cantidad);
void gpu_load_finish(long long latencia, long long cantidad);
void add_latencias(struct latenciometro *latencias);
void add_simd_idle_cycle(int simd_id);
void add_cu_mem_full();
void add_latencias_load(struct latenciometro *latencias);
void add_latencias_nc_write(struct latenciometro *latencias);
void add_simd_running_cycle();
void analizarCausaBloqueo(struct si_wavefront_pool_t *wavefront_pool, int active_fb);
void analizeTypeInstructionInFly(struct si_wavefront_t *wf);
void add_uop_latencies(struct si_uop_t *uop);
void add_invalidation(int level);
void add_load_action_retry(int level);
void add_load_miss_retry(int level);
void add_nc_store_writeback_retry(int level);
void add_nc_store_action_retry(int level);
void add_nc_store_miss_retry(int level);
void add_retry_time_lost(struct mod_stack_t *stack);
void accu_retry_time_lost(struct mod_stack_t *stack);
void add_eviction(int level);
void print_cache_states(long long *results);

#endif

