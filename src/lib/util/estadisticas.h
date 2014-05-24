
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

static long long ventana_muestreo = 10000;

static char *fran_file_ipc = "";
static char *fran_file_general = "";
static char *fran_file_t1000k = "";
static char *fran_file_hitRatio = "";
static char *fran_file_red = "";
int SALTAR_L1;
static int replace; // cache = 0; mod = 1

static long long intervalo_anterior = 0;
static long long ipc_anterior = 0;
static long long ipc_inst = 0;
static long long ipc_last_cycle = 0;

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
};

struct si_gpu_unit_stats
{
	long long unit[6];
	long long total;
	long long loads_latency;
	long long loads_count;
}; 

struct si_gpu_unit_stats *gpu_inst;
static struct esta_t estadis[10];
static struct si_gpu_unit_stats gpu_stats, instruciones_gpu_stats_anterior; 

struct esta_t *estadisticas_ipc;
static long long ciclo = 0;
static int resolucion = 0;
void ini_estadisticas();
void estadisticas(int hit, int lvl);
void hrl2(int hit , struct mod_t *mod, int from_load);
void estadisticas_por_intervalos(long long intervalo);
void ipc_instructions(long long cycle, si_units unit);
void load_finish(long long latencia, long long cantidad);

