#include "./estadisticas.h"


long long intervalo_anterior = 0;
long long ipc_anterior = 0;

void estadisticas_por_intervalos(long long intervalo){

double latencia;
int z, x, y, k, i;
long long ipc;

if((intervalo_anterior + 10000) > intervalo )
	return;

intervalo_anterior = intervalo;

//IPC
ipc = asEmu(si_emu)->instructions;



if(estadis[0].media_latencia_contador == 0)
	latencia = -1.0;
        //fran_debug_latencia("%.1f\n",-1.0);
else
{
      	latencia = (double) estadis[0].media_latencia/estadis[0].media_latencia_contador;
      	//fran_debug_latencia("%.1f\n",latencia);
        estadis[0].media_latencia=0;
	estadis[0].media_latencia_contador=0;
}

   // calculo de hit ratio
double hitratio0 = estadis[0].accesses ? estadis[0].hits/(double)estadis[0].accesses : -1.0;
double hitratio1 = estadis[1].accesses ? estadis[1].hits/(double)estadis[1].accesses : -1.0;
double hitratio2 = estadis[2].accesses ? estadis[2].hits/(double)estadis[2].accesses : -1.0;
double hitratio2_real = estadis[5].accesses ? estadis[5].hits/(double)estadis[5].accesses : -1.0;

fran_debug_general("%lld ",ipc - ipc_anterior);
fran_debug_general("%lld ",estadis[2].coalesce);
fran_debug_general("%.1f ",latencia);
fran_debug_general("%.3f ",hitratio2_real);
fran_debug_general("%lld ",estadis[5].hits);
fran_debug_general("%lld ",estadis[5].accesses);
fran_debug_general("%.3f ",hitratio0);
fran_debug_general("%lld ",estadis[0].hits);
fran_debug_general("%lld ",estadis[0].accesses);
fran_debug_general("%lld ",estadis[1].hits);
fran_debug_general("%lld ",estadis[1].loads);
fran_debug_general("%lld ",estadis[1].accesses);
fran_debug_general("%.3f ",hitratio1);
fran_debug_general("%lld ",estadis[1].busy_cicles_in);
fran_debug_general("%lld ",estadis[1].busy_cicles_out);
fran_debug_general("%lld ",estadis[1].invalidations);
fran_debug_general("%lld ",estadis[2].hits);
fran_debug_general("%lld ",estadis[2].accesses);
fran_debug_general("%.3f ",hitratio2);
fran_debug_general("%lld ",estadis[2].busy_cicles_in);
fran_debug_general("%lld ",estadis[2].busy_cicles_out);
fran_debug_general("%lld ",estadis[1].latencia_red_cont ? estadis[1].latencia_red_acc/estadis[1].latencia_red_cont : 0);
fran_debug_general("%lld ",estadis[2].latencia_red_cont ? estadis[2].latencia_red_acc/estadis[2].latencia_red_cont : 0);

int tag_ptr;
int state_ptr;
int contador = 0;
struct mod_t *mod;
struct cache_t *cache;
struct dir_t *dir;

for (k = 0; k < list_count(mem_system->mod_list); k++)
{
	mod = list_get(mem_system->mod_list, k);
        if(mod->level != 2)
        	continue;

        dir = mod->dir;
        cache = mod->cache;

        for (x = 0; x < dir->xsize; x++)
        {
	        for (y = 0; y < dir->ysize; y++)
        	{
                	cache_get_block(cache, x, y, &tag_ptr, &state_ptr);
                        if(state_ptr)
                        {
                        	for (z = 0; z < dir->zsize; z++)
                                {
                                	contador = 0;
                                        for (i = 0; i < dir->num_nodes; i++)
                                        {
                                        	if (dir_entry_is_sharer(dir, x, y, z, i))
                                                {
                                                	contador++;
                                                        if(contador == 1)
                                                        {
                                                        	estadis[mod->level].blk_compartidos++;
                                                        }
							else
                                                        {
                                                        	estadis[mod->level].replicas_en_l1++;
							}
                                                }
                                        }
                                }
                        }
                }
        }
}

fran_debug_general("%lld ",estadis[2].blk_compartidos);
fran_debug_general("%lld\n",estadis[2].replicas_en_l1);

estadis[2].coalesce = 0;
estadis[0].accesses = 0;
estadis[0].hits = 0;
estadis[5].accesses = 0;
estadis[5].hits = 0;
estadis[1].loads = 0;
estadis[1].accesses = 0;
estadis[1].hits = 0;
estadis[1].busy_cicles_in = 0;
estadis[1].busy_cicles_out = 0;
estadis[2].accesses = 0;
estadis[2].hits = 0;
estadis[2].busy_cicles_in = 0;
estadis[2].busy_cicles_out = 0;
estadis[1].invalidations = 0;
estadis[1].latencia_red_acc = 0;
estadis[1].latencia_red_cont = 0;
estadis[2].latencia_red_acc = 0;
estadis[2].latencia_red_cont = 0;
estadis[2].blk_compartidos = 0;
estadis[2].replicas_en_l1 = 0;
ipc_anterior = ipc;

}
                       

void hrl2(int hit , struct mod_t *mod, int from_load){

        if((mod->level == 2) && from_load)
        {
                mod->loads++;
                if(hit)
                {
                        estadisticas(1, 5);
                }
                else
                {
                        estadisticas(0, 5);
                }
        }
}

void estadisticas(int hit, int lvl){

        estadis[lvl].accesses++;

        estadis[lvl].hits += hit;
}


void ini_estadisticas(){
//estadis = xcalloc(10,sizeof(struct esta_t));/*  
        for(int i = 0; i < 10; i++){
                estadis[i].coalesce = 0;
                estadis[i].accesses = 0;
                estadis[i].hits= 0;
                estadis[i].busy_cicles_in = 0;
                estadis[i].busy_cicles_in = 0;
                estadis[i].invalidations = 0;
                estadis[i].delayed_read_hit= 0;
                estadis[i].esim_cycle_anterior= 0;
                estadis[i].media_latencia = 0;
                estadis[i].media_latencia = 0;
                estadis[i].tiempo_acceso_latencia = 0;
                estadis[i].latencia_red_acc = 0;
                estadis[i].latencia_red_cont = 0;
                estadis[i].blk_compartidos = 0;
                estadis[i].replicas_en_l1 = 0;
        }
}

