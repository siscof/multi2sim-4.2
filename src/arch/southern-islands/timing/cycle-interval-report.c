/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <lib/util/config.h>
#include <lib/util/debug.h>
#include <lib/util/file.h>
#include <lib/util/string.h>
#include <string.h>

#include "cycle-interval-report.h"
#include "uop.h"

#include "compute-unit.h"
#include <lib/esim/esim.h>

#include <lib/util/estadisticas.h>
#include <mem-system/mshr.h>

int si_spatial_report_active = 0 ;
int si_cu_spatial_report_active = 0 ;
int si_device_spatial_report_active = 0 ;
static int spatial_profiling_interval = 10000;
static int spatial_profiling_format = 0;
static char *si_spatial_report_section_name = "SISpatialReport";
static FILE *spatial_report_file;
static char *spatial_report_filename = "report-cu-spatial";
static char *device_spatial_report_filename = "report-device-spatial";
static FILE *device_spatial_report_file;
static FILE *device_spatial_report_file_wg;


void si_spatial_report_config_read(struct config_t *config)
{
	char *section;
	char *cu_file_name;
	char *device_file_name;

	/*Nothing if section or config is not present */
	section = si_spatial_report_section_name;
	if (!config_section_exists(config, section))
	{
		/*no spatial profiling */
		return;
	}

	/* Spatial reports are active */
	si_spatial_report_active = 1;

	/* output format */
	config_var_enforce(config, section, "Format");
	spatial_profiling_format = config_read_int(config, section,
		"Format", spatial_profiling_format);


	/* Interval */
	config_var_enforce(config, section, "Interval");
	spatial_profiling_interval = config_read_int(config, section,
		"Interval", spatial_profiling_interval);

	/* Compute Unit File name */
	//config_var_enforce(config, section, "cu_File");
	cu_file_name = config_read_string(config, section, "cu_File", NULL);
	if (cu_file_name && *cu_file_name){
		si_cu_spatial_report_active = 1;

		spatial_report_filename = str_set(NULL, cu_file_name);

		spatial_report_file = file_open_for_write(spatial_report_filename);
		if (!spatial_report_file)
			fatal("%s: could not open spatial report file",
					spatial_report_filename);
	}
		//fatal("%s: %s: invalid or missing value for 'cu_File'",
		//	si_spatial_report_section_name, section);

	/* Device File name */
	//config_var_enforce(config, section, "device_File");
	device_file_name = config_read_string(config, section, "device_File", NULL);
	if (device_file_name && *device_file_name)
	{
		si_device_spatial_report_active = 1;

		device_spatial_report_filename = str_set(NULL, device_file_name);
		device_spatial_report_file = file_open_for_write(device_spatial_report_filename);
		device_spatial_report_file_wg = file_open_for_write(str_concat(device_spatial_report_filename,"_wg"));
		if (!device_spatial_report_file || !device_spatial_report_file_wg)
			fatal("%s or %s: could not open spatial report file",
					device_spatial_report_filename, str_concat(device_spatial_report_filename,"_wg"));
	}

	if(!si_device_spatial_report_active && !si_cu_spatial_report_active)
		fatal("%s: %s: invalid or missing value for 'device_File' and %s: %s: invalid or missing value for 'cu_File'",
			device_file_name, section,cu_file_name, section);
}

void si_spatial_report_init()
{
	if(si_device_spatial_report_active)
		si_device_spatial_report_init();

	if(si_cu_spatial_report_active)
		si_cu_spatial_report_init();
}

void si_cu_spatial_report_init()
{
	fprintf(spatial_report_file, "cycle, esim_time\n");
}

void si_device_spatial_report_init(SIGpu *device)
{
	fprintf(device_spatial_report_file_wg,"op_counter,interval_cycles,esim_time\n");

	device->interval_statistics = calloc(1, sizeof(struct si_gpu_unit_stats));

	fprintf(device_spatial_report_file, "wait_for_mem_time,wait_for_mem_counter,gpu_idle,predicted_opc_op,predicted_opc_cyckes,MSHR_size,");
	fprintf(device_spatial_report_file, "mem_acc_start,mem_acc_end,mem_acc_lat,load_start,load_end,load_lat,write_start,write_end,write_lat,");
	fprintf(device_spatial_report_file, "vcache_load_start,vcache_load_finish,scache_start,scache_finish,vcache_write_start,vcache_write_finish,cache_retry_lat,cache_retry_cont,");
	fprintf(device_spatial_report_file, "active_wavefronts,wavefronts_waiting_mem,");
	fprintf(device_spatial_report_file, "total_i,simd_i,simd_op,scalar_i,v_mem_i,v_mem_op,s_mem_i,lds_i,lds_op,branch_i,");
	fprintf(device_spatial_report_file, "mappedWG,unmappedWG,cycle,esim_time\n");
}

void si_spatial_report_done()
{
	if (si_cu_spatial_report_active)
		si_cu_spatial_report_done();

	if (si_device_spatial_report_active)
		si_device_spatial_report_done();
}

void si_cu_spatial_report_done()
{

	fclose(spatial_report_file);
	spatial_report_file = NULL;
	str_free(spatial_report_filename);
}

void si_device_spatial_report_done()
{
	fclose(device_spatial_report_file);
	device_spatial_report_file = NULL;
	str_free(device_spatial_report_filename);
}

void add_wait_for_mem_latency(struct si_compute_unit_t *compute_unit, long long cycles)
{
	compute_unit->compute_device->interval_statistics->wait_for_mem_time += cycles;
	compute_unit->compute_device->interval_statistics->wait_for_mem_counter++;
}

void si_cu_spatial_report_dump(struct si_compute_unit_t *compute_unit)
{
	FILE *f = spatial_report_file;

	if(spatial_profiling_format == 0){
		fprintf(f,
			"CU,%d,MemAcc,%lld,MappedWGs,%lld,UnmappedWGs,%lld,ALUIssued,%lld,LDSIssued,%lld,Cycles,%lld\n",
			compute_unit->id,
			compute_unit->vector_mem_unit.inflight_mem_accesses,
			compute_unit->interval_mapped_work_groups,
			compute_unit->interval_unmapped_work_groups,
			compute_unit->interval_alu_issued,
			compute_unit->interval_lds_issued,
			compute_unit->interval_cycle);
	}else if(spatial_profiling_format == 1){
		fprintf(f,
			"%d,%lld,%lld,%d,%lld,%lld\n",
			compute_unit->id,
			compute_unit->interval_mapped_work_groups,
			compute_unit->interval_unmapped_work_groups,
			compute_unit->work_group_count,
			asTiming(si_gpu)->cycle,
			esim_time);
	}

}

void si_vector_memory_report_new_inst(struct si_compute_unit_t *compute_unit, struct si_uop_t *uop)
{
	compute_unit->compute_device->interval_statistics->macroinst[v_mem_u]++;
	compute_unit->compute_device->interval_statistics->instructions_counter++;

	compute_unit->compute_device->interval_statistics->op_counter[v_mem_u] += si_wavefront_count_active_work_items(uop->wavefront);

	uop->wavefront->work_group->op_counter += si_wavefront_count_active_work_items(uop->wavefront);
}

void si_branch_report_new_inst(struct si_compute_unit_t *compute_unit, struct si_uop_t *uop)
{
	compute_unit->compute_device->interval_statistics->macroinst[branch_u]++;
	compute_unit->compute_device->interval_statistics->instructions_counter++;

	compute_unit->compute_device->interval_statistics->op_counter[branch_u]++;

	uop->wavefront->work_group->op_counter++;
}

void si_lds_report_new_inst(struct si_compute_unit_t *compute_unit, struct si_uop_t *uop)
{
	compute_unit->interval_lds_issued++;
	compute_unit->compute_device->interval_statistics->macroinst[lds_u]++;
	compute_unit->compute_device->interval_statistics->instructions_counter++;

	compute_unit->compute_device->interval_statistics->op_counter[
	lds_u] += si_wavefront_count_active_work_items(uop->wavefront);

	uop->wavefront->work_group->op_counter += si_wavefront_count_active_work_items(uop->wavefront);
}


void si_scalar_alu_report_new_inst(struct si_compute_unit_t *compute_unit, struct si_uop_t *uop)
{
	compute_unit->interval_alu_issued++;
	compute_unit->compute_device->interval_statistics->macroinst[scalar_u]++;
	compute_unit->compute_device->interval_statistics->instructions_counter++;

	compute_unit->compute_device->interval_statistics->op_counter[scalar_u]++;

	uop->wavefront->work_group->op_counter++;
}

void si_simd_alu_report_new_inst(struct si_compute_unit_t *compute_unit, struct si_uop_t *uop)
{
	compute_unit->compute_device->interval_statistics->macroinst[simd_u]++;
	compute_unit->compute_device->interval_statistics->instructions_counter++;

	compute_unit->compute_device->interval_statistics->op_counter[simd_u] += si_wavefront_count_active_work_items(uop->wavefront);

	uop->wavefront->work_group->op_counter += si_wavefront_count_active_work_items(uop->wavefront);
}

void si_work_group_report(struct si_work_group_t *wg)
{
	fprintf(device_spatial_report_file_wg,"%lld,",wg->op_counter);

	fprintf(device_spatial_report_file_wg,"%lld,%lld",
	asTiming(wg->wavefront_pool->compute_unit->compute_device)->cycle - wg->start_cycle, esim_time);
	fprintf(device_spatial_report_file_wg,"\n");
	//wg->wavefront_pool->compute_unit->compute_device->cycle_last_wg_op_counter_report = asTiming(wg->wavefront_pool->compute_unit->compute_device)->cycle;
}

void si_report_global_mem_inflight( struct si_compute_unit_t *compute_unit, struct si_uop_t *uop)
{
	/* Read stage adds a negative number for accesses added
	 * Write stage adds a positive number for accesses finished
	 */
	compute_unit->vector_mem_unit.inflight_mem_accesses += uop->active_work_items;
	compute_unit->compute_device->interval_statistics->memory.accesses_started += uop->active_work_items;

	//latency
	if(uop->vector_mem_read)
	{
		compute_unit->compute_device->interval_statistics->memory.load_start += uop->active_work_items;
	}
	if(uop->vector_mem_write)
	{
		compute_unit->compute_device->interval_statistics->memory.write_start += uop->active_work_items;
	}


}

void si_report_global_mem_finish( struct si_compute_unit_t *compute_unit, struct si_uop_t *uop)
{
	/* Read stage adds a negative number for accesses added */
	/* Write stage adds a positive number for accesses finished */
	compute_unit->vector_mem_unit.inflight_mem_accesses -= uop->active_work_items;
	compute_unit->compute_device->interval_statistics->memory.accesses_finished += uop->active_work_items;
	compute_unit->compute_device->interval_statistics->memory.accesses_latency += (asTiming(si_gpu)->cycle - uop->send_cycle) * uop->active_work_items;
	//latency
	if(uop->vector_mem_read)
	{
		compute_unit->compute_device->interval_statistics->memory.load_finish += uop->active_work_items;
		compute_unit->compute_device->interval_statistics->memory.load_latency += (asTiming(si_gpu)->cycle - uop->send_cycle) * uop->active_work_items;
	}
	if(uop->vector_mem_write)
	{
		compute_unit->compute_device->interval_statistics->memory.write_finish += uop->active_work_items;
		compute_unit->compute_device->interval_statistics->memory.write_latency += (asTiming(si_gpu)->cycle - uop->send_cycle) * uop->active_work_items;
	}

}

void si_report_gpu_idle(SIGpu *device)
{
	device->interval_statistics->gpu_idle = 1;
}

void si_report_mapped_work_group(struct si_compute_unit_t *compute_unit)
{
	/*TODO Add calculation here to change this to wavefront pool entries used */
	compute_unit->interval_mapped_work_groups++;
	compute_unit->compute_device->interval_statistics->interval_mapped_work_groups++;
}


void si_report_unmapped_work_group(struct si_compute_unit_t *compute_unit)
{
	/*TODO Add calculation here to change this to wavefront pool entries used */
	compute_unit->interval_unmapped_work_groups++;
	compute_unit->compute_device->interval_statistics->interval_unmapped_work_groups++;
}


void si_cu_interval_update(struct si_compute_unit_t *compute_unit)
{
	/* If interval - reset the counters in all the engines */
	compute_unit->interval_cycle ++;

	if (si_cu_spatial_report_active && !(asTiming(si_gpu)->cycle % spatial_profiling_interval))
	{
		si_cu_spatial_report_dump(compute_unit);

		/*
		 * This counter is not reset since memory accesses could still
		 * be in flight in the hierarchy
		 * compute_unit->inflight_mem_accesses = 0;
		 */
		compute_unit->interval_cycle = 0;
		compute_unit->interval_mapped_work_groups = 0;
		compute_unit->interval_unmapped_work_groups = 0;
		compute_unit->interval_alu_issued = 0;
		compute_unit->interval_lds_issued = 0;
	}
}

int contador_mshr = 20;

void si_device_interval_update(SIGpu *device)
{
	/* If interval - reset the counters in all the engines */
	device->interval_statistics->interval_cycles++;

	if (si_device_spatial_report_active && (device->interval_statistics->interval_cycles >= spatial_profiling_interval))
	{
		si_device_spatial_report_dump(device);
		device->op = device->interval_statistics->macroinst[scalar_u] + device->interval_statistics->op_counter[simd_u] + device->interval_statistics->op_counter[v_mem_u] + device->interval_statistics->macroinst[s_mem_u] + device->interval_statistics->op_counter[lds_u];
		device->cycles = device->interval_statistics->interval_cycles;

		/*
		 * This counter is not reset since memory accesses could still
		 * be in flight in the hierarchy
		 * compute_unit->inflight_mem_accesses = 0;
		 */
		/*device->interval_cycle = 0;
		device->interval_mapped_work_groups = 0;
		compute_unit->interval_unmapped_work_groups = 0;
		compute_unit->interval_alu_issued = 0;
		compute_unit->interval_lds_issued = 0;*/
		contador_mshr--;

		if(flag_mshr_dynamic_enabled && contador_mshr == 0)
		{
		  mshr_control2();
			contador_mshr=20;
		}

		memset(device->interval_statistics, 0, sizeof(struct si_gpu_unit_stats));
	}
}

void si_device_interval_update_force(SIGpu *device)
{
	if(device->interval_statistics->interval_cycles > 0)
	{
		si_device_spatial_report_dump(device);
		memset(device->interval_statistics, 0, sizeof(struct si_gpu_unit_stats));
	}
}

void analizar_wavefront(SIGpu *device)
{
	int i,j,w;
	struct si_compute_unit_t *compute_unit;
	struct si_wavefront_t *wavefront;

	for(j = 0; j < si_gpu_num_compute_units; j++)
	{
		compute_unit = device->compute_units[j];
		for(w = 0; w < si_gpu_num_wavefront_pools; w++)
		{
			for (i = 0; i < si_gpu_max_wavefronts_per_wavefront_pool; i++)
			{
				wavefront = compute_unit->wavefront_pools[w]->entries[i]->wavefront;

				/* No wavefront */
				if (!wavefront)
					continue;

				device->interval_statistics->active_wavefronts++;

				/* If the wavefront finishes, there still may be outstanding
				 * memory operations, so if the entry is marked finished
				 * the wavefront must also be finished, but not vice-versa */
				if (wavefront->wavefront_pool_entry->wavefront_finished)
				{
					continue;
				}

				/* Wavefront is finished but other wavefronts from workgroup
				 * remain.  There may still be outstanding memory operations,
				 * but no more instructions should be fetched. */
				if (wavefront->finished)
					continue;

				/* Wavefront is ready but waiting on outstanding
				 * memory instructions */
				if (wavefront->wavefront_pool_entry->wait_for_mem)
				{
					if (wavefront->wavefront_pool_entry->lgkm_cnt ||
						wavefront->wavefront_pool_entry->vm_cnt)
					{
						device->interval_statistics->wavefronts_waiting_mem++;
						continue;
					}
				}

				/* Wavefront is ready but waiting at barrier */
				if (wavefront->wavefront_pool_entry->wait_for_barrier)
				{
					continue;
				}

			}
		}
	}
}

void si_device_spatial_report_dump(SIGpu *device)
{
	FILE *f = device_spatial_report_file;

	analizar_wavefront(device);
	fprintf(f, "%lld,", device->interval_statistics->wait_for_mem_time);
	fprintf(f, "%lld,", device->interval_statistics->wait_for_mem_counter);

	fprintf(f, "%lld,", device->interval_statistics->gpu_idle);

	fprintf(f, "%lld,", device->interval_statistics->predicted_opc_op);
	fprintf(f, "%lld,", device->interval_statistics->predicted_opc_cycles);

        if(device->compute_units[0]->vector_cache->mshr->testing)
            fprintf(f,"nan,");
        else
            fprintf(f, "%d,", device->compute_units[0]->vector_cache->mshr->size);

	// memory mem_acc_start mem_acc_end mem_acc_lat load_start load_end load_lat write_start write_end write_lat
	fprintf(f, "%lld,", device->interval_statistics->memory.accesses_started);
	fprintf(f, "%lld,", device->interval_statistics->memory.accesses_finished);
	fprintf(f, "%lld,", device->interval_statistics->memory.accesses_latency);
	fprintf(f, "%lld,", device->interval_statistics->memory.load_start);
	fprintf(f, "%lld,", device->interval_statistics->memory.load_finish);
	fprintf(f, "%lld,", device->interval_statistics->memory.load_latency);
	fprintf(f, "%lld,", device->interval_statistics->memory.write_start);
	fprintf(f, "%lld,", device->interval_statistics->memory.write_finish);
	fprintf(f, "%lld,", device->interval_statistics->memory.write_latency);


	fprintf(f, "%lld,", device->interval_statistics->vcache_load_start);
	fprintf(f, "%lld,", device->interval_statistics->vcache_load_finish);
	fprintf(f, "%lld,", device->interval_statistics->scache_load_start);
	fprintf(f, "%lld,", device->interval_statistics->scache_load_finish);
	fprintf(f, "%lld,", device->interval_statistics->vcache_write_start);
	fprintf(f, "%lld,", device->interval_statistics->vcache_write_finish);
	fprintf(f, "%lld,", device->interval_statistics->cache_retry_lat);
	fprintf(f, "%lld,", device->interval_statistics->cache_retry_cont);

	fprintf(f, "%lld,", device->interval_statistics->active_wavefronts);
	fprintf(f, "%lld,", device->interval_statistics->wavefronts_waiting_mem);

	//instruction report total_i, simd_i, scalar_i, v_mem_i, s_mem_i, lds_i
	fprintf(f, "%lld,", device->interval_statistics->instructions_counter);
	fprintf(f, "%lld,", device->interval_statistics->macroinst[simd_u]);
	fprintf(f, "%lld,", device->interval_statistics->op_counter[simd_u]);
	fprintf(f, "%lld,", device->interval_statistics->macroinst[scalar_u]);
	fprintf(f, "%lld,", device->interval_statistics->macroinst[v_mem_u]);
	fprintf(f, "%lld,", device->interval_statistics->op_counter[v_mem_u]);
	fprintf(f, "%lld,", device->interval_statistics->macroinst[s_mem_u]);
	fprintf(f, "%lld,", device->interval_statistics->macroinst[lds_u]);
	fprintf(f, "%lld,", device->interval_statistics->op_counter[lds_u]);
	fprintf(f, "%lld,", device->interval_statistics->macroinst[branch_u]);

	fprintf(f, "%lld,", device->interval_statistics->interval_mapped_work_groups);
	fprintf(f, "%lld,",
	device->interval_statistics->interval_unmapped_work_groups);
  // fixme change spatial_profiling_interval for cycle_counter or device->interval_cycle
	fprintf(f,"%lld,%lld", device->interval_statistics->interval_cycles,	esim_time);

	fprintf(f,"\n");

}
