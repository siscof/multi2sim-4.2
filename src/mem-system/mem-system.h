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

#ifndef MEM_SYSTEM_MEM_SYSTEM_H
#define MEM_SYSTEM_MEM_SYSTEM_H

#include <dramsim/bindings-c.h>
#include <stdbool.h>

/*
 * Memory System Object
 */

enum cache_block_state_t
{
	cache_block_invalid = 0,
	cache_block_noncoherent,
	cache_block_modified,
	cache_block_owned,
	cache_block_exclusive,
	cache_block_shared,
	cache_block_valid,
	cache_block_state_size
};

struct mem_system_t
{
	/* List of modules and networks */
	struct list_t *mod_list;
	struct list_t *net_list;
	/* Main memory modules */
	struct list_t *mm_mod_list;

	/* Main memory systems */
	struct hash_table_t *dram_systems;
};

struct dram_system_t
{
	char *name;
	struct dram_system_handler_t *handler; /* Handler for dramsim */
	int dram_domain_index;
	struct linked_list_t *pending_reads; /* We only track pending read requests because writes are enqueue-and-forget */
        struct linked_list_t *pending_writes; /* We only track pending read requests because writes are enqueue-and-forget */
	int num_mcs; /* Number of memory controllers in this dram system */
};

/*
 * Global Variables
 */


extern char *mem_report_file_name;

#define mem_debugging() debug_status(mem_debug_category)
#define mem_debug(...) debug(mem_debug_category, __VA_ARGS__)
extern int mem_debug_category;

#define mem_tracing() trace_status(mem_trace_category)
#define mem_trace(...) trace(mem_trace_category, __VA_ARGS__)
#define mem_trace_header(...) trace_header(mem_trace_category, __VA_ARGS__)
extern int mem_trace_category;


/* Configuration */
extern int mem_frequency;
extern int mem_peer_transfers;
extern enum dir_type_t directory_type;

/* Frequency and frequency domain */
extern int mem_domain_index;

/* Global memory system */
extern struct mem_system_t *mem_system;

extern int flag_mshr_enabled;
extern int super_stack_enabled;
extern int flag_coalesce_gpu_enabled;
extern int flag_no_blocking_store;
extern bool multidir_enabled;

/* Event for dramsim clock tics */
extern int EV_MAIN_MEMORY_TIC;

/*
 * Public Functions
 */


void mem_system_init(void);
void mem_system_done(void);

void mem_system_dump_report(void);

struct mod_t *mem_system_get_mod(char *mod_name);
struct net_t *mem_system_get_net(char *net_name);

void main_memory_tic_scheduler(struct dram_system_t *ds);
void main_memory_tic_handler(int event, void *data);
void main_memory_read_callback(void *payload, unsigned int id, uint64_t address, uint64_t interthread_penalty);
void main_memory_write_callback(void *payload, unsigned int id, uint64_t address, uint64_t interthread_penalty);
void main_memory_power_callback(double a, double b,	double c, double d);

//void mshr_control(int latencia, int opc);

#endif
