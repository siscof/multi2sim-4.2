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

#ifndef ARCH_SOUTHERN_ISLANDS_SPATIAL_REPORT_H
#define ARCH_SOUTHERN_ISLANDS_SPATIAL_REPORT_H


#include <arch/southern-islands/emu/work-group.h>
#include "gpu.h"

/*
 * Public variable
 */
extern int si_spatial_report_active  ;
extern int si_cu_spatial_report_active  ;
extern int si_device_spatial_report_active  ;


void si_scalar_alu_report_new_inst(struct si_compute_unit_t *compute_unit, struct si_uop_t *uop);
void si_branch_report_new_inst(struct si_compute_unit_t *compute_unit,struct si_uop_t *uop);
void si_vector_memory_report_new_inst(struct si_compute_unit_t *compute_unit, struct si_uop_t *uop);
void si_simd_alu_report_new_inst(struct si_compute_unit_t *compute_unit, struct si_uop_t *uop);

void si_lds_report_new_inst(struct si_compute_unit_t *compute_unit, struct si_uop_t *uop);

void si_report_mapped_work_group(struct si_compute_unit_t *compute_unit);

void si_report_unmapped_work_group(struct si_compute_unit_t *compute_unit);

/* Used in vector unit to keep track of num of mem accesses in flight */
void si_report_global_mem_inflight( struct si_compute_unit_t *compute_unit, struct si_uop_t *uop);

void si_report_global_mem_finish( struct si_compute_unit_t *compute_unit, struct si_uop_t *uop);

struct config_t;

void si_spatial_report_config_read(struct config_t *config);

void si_cu_interval_update(struct si_compute_unit_t *compute_unit);
void si_device_interval_update(SIGpu *device);

void si_cu_spatial_report_dump(struct si_compute_unit_t *compute_unit);
void si_device_spatial_report_dump(SIGpu *device);

void si_work_group_report_dump(struct si_work_group_t *wg);
void si_wavefront_report_dump(struct si_wavefront_t *wavefront);

void si_add_block_life_cycles(int level, long long cycles);


void si_spatial_report_done();
/*void si_cu_spatial_report_done();
void si_device_spatial_report_done();*/
void si_stalls_spatial_report(struct si_wavefront_t * wf);

void si_spatial_report_init();
void si_cu_spatial_report_init();
void si_device_spatial_report_init();
void si_wf_spatial_report_init();

void add_wait_for_mem_latency(struct si_compute_unit_t *compute_unit, long long cycles);

void si_report_gpu_idle(SIGpu *device);
void si_device_interval_update_force(SIGpu *device);


#endif
