#include "./estadisticas.h"


void estadisticas_por_intervalos(long long intervalo){

double latencia;
int z, x, y, k, i;
//long long ipc;

if((intervalo_anterior + ventana_muestreo) > intervalo )
	return;

intervalo_anterior = intervalo;

//IPC
//ipc = asEmu(si_emu)->instructions;



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

//fran_debug_general("%lld ",ipc - ipc_anterior);
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
//ipc_anterior = ipc;

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
estadisticas_ipc = (struct esta_t *) calloc(10, sizeof(struct esta_t));

gpu_inst = (struct si_gpu_unit_stats *) calloc(1, sizeof(struct si_gpu_unit_stats));

//imprimir columnas
fran_debug_general("IPC Coa_L2 Lat HR_L2 Hits_L2 accesos_L2 HR_L1 Hits_L1 Accesos_L1 X X X X L1->L2_busy_in L1<-L2_busy_out invalidaciones X X X L2<-MM_busy_in L2->MM_busy_out Lat_L1-L2 Lat_L2-MM blk_comp_L2 Replicas_L1");

fran_debug_ipc("Coalesce_L1 Coalesce_L2 accesos_L1 accesos_L2 efectivos_L1 efectivos_L2 MPKI_L1 MPKI_L2 HR_L1 HR_L2 Lat_L1-L2 Lat_L2-MM campo1 campo2 campo ....3 \n");

        for(int i = 0; i < 10; i++){
                estadis[i].coalesce = 0;
                estadis[i].accesses = 0;
                estadis[i].hits= 0;
                estadis[i].misses = 0;
		estadis[i].busy_cicles_in = 0;
                estadis[i].busy_cicles_in = 0;
                estadis[i].invalidations = 0;
                estadis[i].delayed_read_hit= 0;
                estadis[i].esim_cycle_anterior= 0;
                estadis[i].media_latencia = 0; 
                estadis[i].tiempo_acceso_latencia = 0;
                estadis[i].latencia_red_acc = 0;
                estadis[i].latencia_red_cont = 0;
                estadis[i].blk_compartidos = 0;
                estadis[i].replicas_en_l1 = 0;
        }
}

void add_coalesce(int level)
{
	(estadisticas_ipc + level)->coalesce++;
}

void add_access(int level)
{
	(estadisticas_ipc + level)->accesses++;
}

void add_hit(int level)
{
	(estadisticas_ipc + level)->hits++;
}

void add_miss(int level)
{
        (estadisticas_ipc + level)->misses++;
}

long long add_si_inst(si_units unit)
{
	gpu_inst->unit[unit]++;
	gpu_inst->total++;
	return gpu_inst->total;
}

void load_finish(long long latencia, long long cantidad)
{
	gpu_stats.loads_latency = latencia + cantidad;
	gpu_stats.loads_count = cantidad;
}

void ipc_instructions(long long cycle, si_units unit)
{
	long long efectivosL1, efectivosL2;
	
	long long intervalo_instrucciones = 100000;

	if(!(add_si_inst(unit) % intervalo_instrucciones))
	{
		efectivosL1 = (estadisticas_ipc + 1)->accesses - (estadisticas_ipc + 1)->coalesce;
                efectivosL2 = (estadisticas_ipc + 2)->accesses - (estadisticas_ipc + 2)->coalesce;
		
		fran_debug_ipc("%d %d ",(estadisticas_ipc + 1)->coalesce, (estadisticas_ipc + 2)->coalesce);
		fran_debug_ipc("%d %d ",(estadisticas_ipc + 1)->accesses, (estadisticas_ipc + 2)->accesses);
		fran_debug_ipc("%d %d ",efectivosL1, efectivosL2);

		// MPKI
		fran_debug_ipc("%.2f %.2f ",(estadisticas_ipc + 1)->misses / (intervalo_instrucciones / 1000.0), (estadisticas_ipc + 2)->misses / (intervalo_instrucciones/1000.0));
		
		if(efectivosL1 != 0 )
		{
			fran_debug_ipc("%.2f ", ((double) (estadisticas_ipc + 1)->hits) / efectivosL1);
		}
		else
		{
			fran_debug_ipc("nan ");
		}

                if(efectivosL2 != 0 )
                {
                        fran_debug_ipc("%.2f ", ((double) (estadisticas_ipc + 2)->hits) / efectivosL2);
                }
		else
                {
                        fran_debug_ipc("nan ");
                }

		if( (estadisticas_ipc + 1)->latencia_red_cont != 0 )
		{	
     			fran_debug_ipc("%lld ",(estadisticas_ipc + 1)->latencia_red_acc/(estadisticas_ipc + 1)->latencia_red_cont);
		}
		else
		{
			fran_debug_ipc("nan ");
		}

                if( (estadisticas_ipc + 2)->latencia_red_cont != 0 )
                { 
                        fran_debug_ipc("%lld ",(estadisticas_ipc + 2)->latencia_red_acc/(estadisticas_ipc + 2)->latencia_red_cont);
                }
                else
                {
                        fran_debug_ipc("nan ");
                }
		cycle -= ipc_last_cycle;
		//fran_debug_ipc("%.2f ",   ipc_inst / (double)cycle);

		if(gpu_stats.loads_count - instruciones_gpu_stats_anterior.loads_count)
		{
			long long load = (gpu_stats.loads_latency - instruciones_gpu_stats_anterior.loads_latency)/(gpu_stats.loads_count - instruciones_gpu_stats_anterior.loads_count);
			fran_debug_ipc("%lld ",load);
			
			instruciones_gpu_stats_anterior.loads_latency = gpu_stats.loads_latency;
			instruciones_gpu_stats_anterior.loads_count = gpu_stats.loads_count;
		}
		else
		{		
			fran_debug_ipc("nan ");
		}
	
		fran_debug_ipc("%.2f %.2f %.2f %.2f %.2f %.2f %.2f\n", gpu_inst->unit[scalar_u] / (double)cycle, gpu_inst->unit[simd_u] / (double)cycle, gpu_inst->unit[s_mem_u] / (double)cycle, gpu_inst->unit[v_mem_u] / (double)cycle, gpu_inst->unit[branch_u] / (double)cycle, gpu_inst->unit[lds_u] / (double)cycle, gpu_inst->total / (double)cycle);



		ipc_inst = 0;
		ipc_last_cycle = cycle;

		
		free(estadisticas_ipc);
		free(gpu_inst);

		estadisticas_ipc = (struct esta_t *) calloc(10, sizeof(struct esta_t));
		gpu_inst = (struct si_gpu_unit_stats *) calloc(1, sizeof(struct si_gpu_unit_stats));

	}
}

