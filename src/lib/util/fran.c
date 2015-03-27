

#include "fran.h"


void ini_estadisticas(){
	estadisticas.accesses = 0;
	estadisticas.hits= 0;
	estadisticas.delayed_read_hit= 0;
	estadisticas.esim_cycle_anterior= 0;
}

long long tiempo;

void estadisticas(int hit){
	 
	estadisticas.accesses++;
	 
	estadisticas.hits += hit;
	if(estadisticas.accesses == 1){
		estadisticas.esim_cycle_anterior = esim_cycle();
	}else if(estadisticas.accesses >= 1000){
		double hit_ratio = estadisticas.hits ? (double) estadisticas.hits/ estadisticas.accesses : 0.0;
		
		ciclo = esim_cycle();

		fran_debug_t10000k("%lld %lld\n",ciclo - estadisticas.esim_cycle_anterior,estadisticas.esim_cycle_anterior); //tiempo
		fran_debug_hitRatio("%.4f %lld\n",hit_ratio,estadisticas.esim_cycle_anterior);
		estadisticas.accesses = 0;
		estadisticas.hits = 0;
		estadisticas.esim_cycle_anterior = ciclo;
		}
	}
}
