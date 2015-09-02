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

#include "gpu.h"
#include "cycle-interval-report.h"

#include "compute-unit.h"
#include <lib/esim/esim.h>


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

		spatial_report_filename = str_set(NULL, device_file_name);
		device_spatial_report_file = file_open_for_write(spatial_report_filename);
		if (!device_spatial_report_file)
			fatal("%s: could not open spatial report file",
					spatial_report_filename);
	}

	if(!si_device_spatial_report_active && !si_cu_spatial_report_active)
		fatal("%s: %s: invalid or missing value for 'device_File' and %s: %s: invalid or missing value for 'cu_File'",
			device_file_name, section,cu_file_name, section);
}

void si_cu_spatial_report_init()
{

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
	return;
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
			"%lld,%lld,%lld,%lld,%lld\n",
			compute_unit->interval_mapped_work_groups,
			compute_unit->interval_unmapped_work_groups,
			compute_unit->work_group_count,
			asTiming(si_gpu)->cycle,
			esim_time);
	}

}

void si_lds_report_new_inst(struct si_compute_unit_t *compute_unit)
{
	compute_unit->interval_lds_issued = compute_unit->interval_lds_issued + 1;
}

void si_alu_report_new_inst(struct si_compute_unit_t *compute_unit)
{
	compute_unit->interval_alu_issued ++ ;

}


void si_report_global_mem_inflight( struct si_compute_unit_t *compute_unit, int long long pending_accesses)
{
	/* Read stage adds a negative number for accesses added
	 * Write stage adds a positive number for accesses finished
	 */
	compute_unit->vector_mem_unit.inflight_mem_accesses += pending_accesses;


}

void si_report_global_mem_finish( struct si_compute_unit_t *compute_unit, int long long completed_accesses)
{
	/* Read stage adds a negative number for accesses added */
	/* Write stage adds a positive number for accesses finished */
	compute_unit->vector_mem_unit.inflight_mem_accesses -= completed_accesses;

}

void si_report_mapped_work_group(struct si_compute_unit_t *compute_unit)
{
	/*TODO Add calculation here to change this to wavefront pool entries used */
	compute_unit->interval_mapped_work_groups++;
}

void si_report_unmapped_work_group(struct si_compute_unit_t *compute_unit)
{
	/*TODO Add calculation here to change this to wavefront pool entries used */
	compute_unit->interval_unmapped_work_groups++;
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

void si_device_interval_update(SIGpu *device)
{
	/* If interval - reset the counters in all the engines */
	//device->interval_cycle ++;

	if (si_device_spatial_report_active && !(asTiming(device)->cycle % spatial_profiling_interval))
	{
		si_device_spatial_report_dump(device);

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
	}
}

void si_device_spatial_report_dump(SIGpu *device)
{
	FILE *f = device_spatial_report_file;

	fprintf(f,
		"%lld,%lld\n",
		asTiming(device)->cycle,
		esim_time);

}
