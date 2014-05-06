
#define cache_hit 1
#define cache_accesses 0


static char *fran_file_latencia = "";
static char *fran_file_general = "";
static char *fran_file_t1000k = "";
static char *fran_file_hitRatio = "";
static char *fran_file_red = "";
int SALTAR_L1;
static int replace; // cache = 0; mod = 1
#define fran_debug_latencia(...) debug(fran_latencia, __VA_ARGS__)
int fran_latencia;

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
} estadis[10];

static long long ciclo = 0;
static int resolucion = 0;
void ini_estadisticas();
void estadisticas(int hit, int lvl);
void hrl2(int hit , struct mod_t *mod, int from_load);


