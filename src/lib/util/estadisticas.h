
#include <mem-system/mem-system.h>
#include <mem-system/directory.h>
#include <mem-system/module.h>
#include <lib/util/debug.h>
#include <arch/southern-islands/emu/emu.h>
#include <lib/util/list.h>

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

extern long long ventana_muestreo;

extern char *fran_file_ipc;
extern char *fran_file_general;
extern char *fran_file_t1000k;
extern char *fran_file_hitRatio;
extern char *fran_file_red;
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

#define fran_debug_red(...) debug_new_category(fran_red, __VA_ARGS__)
int fran_red;

struct esta_t
{
        /* Statistics */
        long long coalesce;
        long long accesses;
        long long hits;
	long long misses;
        long long loads;
        long long invalidations;
        long long busy_cicles_in;
        long long busy_cicles_out;
        long long delayed_read_hit;
        long long esim_cycle_anterior;
        long long media_latencia;
        long long media_latencia_contador;
        long long tiempo_acceso_latencia;
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
}; 

struct mem_system_stats
{
	struct esta_t mod_level[5];
	long long load_latency;
	long long load_latency_count;
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
long long add_si_macroinst(si_units unit);
void add_CoalesceHit(int level);
void add_CoalesceMiss(int level);
void load_finish(long long latencia, long long cantidad);
