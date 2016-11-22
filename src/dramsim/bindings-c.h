/*
 *  DramSim C bindings
 *  Copyright (C) 2014 Vicent Selfa (viselol [at] disca [dot] upv [dot] es)
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

#ifndef DRAMSIM_C_BINDINGS_H
#define DRAMSIM_C_BINDINGS_H

#include <stdbool.h>
#include <stdint.h>
#define dram_system_handler_t MultiChannelMemorySystem

#ifdef __cplusplus
extern "C" {
#endif

struct dram_system_handler_t* dram_system_create(const char *dev_desc_file, const char *sys_desc_file, unsigned int total_memory_megs, const char *vis_file);
void dram_system_free(struct dram_system_handler_t *ds);

/* Insert transactions */
bool dram_system_add_read_trans(struct dram_system_handler_t *ds, unsigned long long addr, int core, int thread);
bool dram_system_add_write_trans(struct dram_system_handler_t *ds, unsigned long long addr, int core, int thread);

/* Set CPU frequency. Must be in Hz. */
void dram_system_set_cpu_freq(struct dram_system_handler_t *ds, long long freq);

/* Get DRAM frequency in Hz. It depends on the device used. */
long long dram_system_get_dram_freq(struct dram_system_handler_t *ds);

/* Get stats to estimate bandwidth consumption per memory controller and per core */
long long dram_system_get_bwc(struct dram_system_handler_t *ds, int mc, int core);
long long dram_system_get_bwn(struct dram_system_handler_t *ds, int mc, int core);
long long dram_system_get_bwno(struct dram_system_handler_t *ds, int mc, int core);
void dram_system_reset_bwc(struct dram_system_handler_t *ds, int mc, int core);
void dram_system_reset_bwn(struct dram_system_handler_t *ds, int mc, int core);
void dram_system_reset_bwno(struct dram_system_handler_t *ds, int mc, int core);

/* Get number of memory controllers in the dram system */
int dram_system_get_num_mcs(struct dram_system_handler_t *ds);

/* Dramsim needs to know when a cycle has passed.
 * User of the library can call cpu_tick every cycle of CPU *OR* call dram_tick every cycle of DRAM.
 * If CPU clocks are used, dramsim makes internally the appropiate conversion. */
void dram_system_cpu_tick(struct dram_system_handler_t *ds);
void dram_system_dram_tick(struct dram_system_handler_t *ds);

void dram_system_set_epoch_length(struct dram_system_handler_t *ds, unsigned long long epoch_lenght);
void dram_system_print_stats(struct dram_system_handler_t *ds);
void dram_system_print_final_stats(struct dram_system_handler_t *ds);

bool dram_system_will_accept_trans(struct dram_system_handler_t *ds, unsigned long long addr);

void dram_system_register_callbacks(
		struct dram_system_handler_t *ds,
		void (*read_done)(unsigned int, uint64_t, uint64_t),
		void (*write_done)(unsigned int, uint64_t, uint64_t),
		void (*report_power)(double bgpower, double burstpower, double refreshpower, double actprepower));

void dram_system_register_payloaded_callbacks(
		struct dram_system_handler_t *ds,
		void *payload,
		void(*read_done)(void*, unsigned int, uint64_t, uint64_t),
		void(*write_done)(void*, unsigned int, uint64_t, uint64_t),
		void(*report_power)(double bgpower, double burstpower, double refreshpower, double actprepower));

#ifdef __cplusplus
}
#endif

#endif //DRAMSIM_C_BINDINGS_H

