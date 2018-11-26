/*
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

#include <stdint.h>
#include <arch/common/arch.h>
#include <lib/esim/esim.h>
#include <lib/esim/trace.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/file.h>
#include <lib/util/list.h>
#include <lib/util/string.h>
#include <network/network.h>

#include "cache.h"
#include "config.h"
#include "command.h"
#include "local-mem-protocol.h"
#include "mem-system.h"
#include "module.h"
#include "nmoesi-protocol.h"
#include "vi-protocol.h"
#include "directory.h"
#include "mshr.h"
//fran
#include <lib/util/estadisticas.h>
#include <stdbool.h>
#include <lib/util/misc.h>
#include <lib/util/linked-list.h>
#include <lib/util/hash-table.h>
#include <dramsim/bindings-c.h>
#include <arch/southern-islands/timing/gpu.h>
//#include "directory.h"

/*
 * Global Variables
 */

int mem_debug_category;
int mem_trace_category;
int mem_peer_transfers;
int fran_category;
int EV_MAIN_MEMORY_TIC;
int mshr_protocol = 0;
int super_stack_enabled = 0;

/* Frequency domain, as returned by function 'esim_new_domain'. */
int mem_frequency = 1000;
enum dir_type_t directory_type = dir_type_nmoesi;
int mem_domain_index;

struct mem_system_t *mem_system;

char *mem_report_file_name = "";



/*
 * Memory System Object
 */

struct mem_system_t *mem_system_create(void)
{
	struct mem_system_t *mem_system;

	/* Initialize */
	mem_system = xcalloc(1, sizeof(struct mem_system_t));
	mem_system->net_list = list_create();
	mem_system->mod_list = list_create();
	mem_system->mm_mod_list = list_create();
	mem_system->dram_systems = hash_table_create(0, 0); /* Dram systems, if any */


	/* Return */
	return mem_system;
}


void mem_system_free(struct mem_system_t *mem_system)
{
	char *key;
	struct dram_system_t *dram_system;

	/* Free memory modules */
	while (list_count(mem_system->mod_list))
		mod_free(list_pop(mem_system->mod_list));
	list_free(mem_system->mod_list);
	list_free(mem_system->mm_mod_list);

	/* Free networks */
	while (list_count(mem_system->net_list))
		net_free(list_pop(mem_system->net_list));
	list_free(mem_system->net_list);

	/* Free dram_systems */
	HASH_TABLE_FOR_EACH(mem_system->dram_systems, key, dram_system)
	{
		free(dram_system->name);
		assert(linked_list_count(dram_system->pending_reads) == 0);
		linked_list_free(dram_system->pending_reads);
                linked_list_free(dram_system->pending_writes);
		dram_system_free(dram_system->handler);
		free(dram_system);
	}
	hash_table_free(mem_system->dram_systems);

	/* Free memory system */
	free(mem_system);
}




/*
 * Public Functions
 */

static char *mem_err_timing =
	"\tA command-line option related with the memory hierarchy ('--mem' prefix)\n"
	"\thas been specified, by no architecture is running a detailed simulation.\n"
	"\tPlease specify at least one detailed simulation (e.g., with option\n"
	"\t'--x86-sim detailed'.\n";

void mem_system_init(void)
{
	int count;

	/* If any file name was specific for a command-line option related with the
	 * memory hierarchy, make sure that at least one architecture is running
	 * timing simulation. */
	count = arch_get_sim_kind_detailed_count();
	if (mem_report_file_name && *mem_report_file_name && !count)
		fatal("memory report file given, but no timing simulation.\n%s",
				mem_err_timing);
	if (mem_config_file_name && *mem_config_file_name && !count)
		fatal("memory configuration file given, but no timing simulation.\n%s",
				mem_err_timing);

	/* Create trace category. This needs to be done before reading the
	 * memory configuration file with 'mem_config_read', since the latter
	 * function generates the trace headers. */
	mem_trace_category = trace_new_category();

	/* Create global memory system. This needs to be done before reading the
	 * memory configuration file with 'mem_config_read', since the latter
	 * function inserts caches and networks in 'mem_system', and relies on
	 * these lists to have been created. */
	mem_system = mem_system_create();

	/* Read memory configuration file */
	mem_config_read();

	/* Try to open report file */
	if (*mem_report_file_name && !file_can_open_for_write(mem_report_file_name))
		fatal("%s: cannot open GPU cache report file",
			mem_report_file_name);

	/* Event handler for memory hierarchy commands */
	EV_MEM_SYSTEM_COMMAND = esim_register_event_with_name(mem_system_command_handler,
			mem_domain_index, "mem_system_command");
	EV_MEM_SYSTEM_END_COMMAND = esim_register_event_with_name(mem_system_end_command_handler,
			mem_domain_index, "mem_system_end_command");


	/* VI memory event-driven simulation*/

	EV_MOD_VI_LOAD = esim_register_event_with_name(mod_handler_vi_load,
			mem_domain_index, "mod_vi_load");
	EV_MOD_VI_LOAD_SEND = esim_register_event_with_name(mod_handler_vi_load,
                        mem_domain_index, "mod_vi_load_send");
	EV_MOD_VI_LOAD_RECEIVE = esim_register_event_with_name(mod_handler_vi_load,
                        mem_domain_index, "mod_vi_load_receive");
	EV_MOD_VI_LOAD_LOCK = esim_register_event_with_name(mod_handler_vi_load,
			mem_domain_index, "mod_vi_load_lock");
	EV_MOD_VI_LOAD_LOCK2 =
	esim_register_event_with_name(mod_handler_vi_load,
			mem_domain_index, "mod_vi_load_lock2");
	EV_MOD_VI_LOAD_ACTION = esim_register_event_with_name(mod_handler_vi_load,
			mem_domain_index, "mod_vi_load_action");
	EV_MOD_VI_LOAD_MISS = esim_register_event_with_name(mod_handler_vi_load,
			mem_domain_index, "mod_vi_load_miss");
	EV_MOD_VI_LOAD_UNLOCK = esim_register_event_with_name(mod_handler_vi_load,
			mem_domain_index, "mod_vi_load_unlock");
	EV_MOD_VI_LOAD_FINISH = esim_register_event_with_name(mod_handler_vi_load,
			mem_domain_index, "mod_vi_load_finish");

	EV_MOD_VI_NC_LOAD = esim_register_event_with_name(mod_handler_vi_load,
			mem_domain_index, "mod_vi_nc_load");
	EV_MOD_VI_NC_STORE = esim_register_event_with_name(mod_handler_vi_store,
			mem_domain_index, "mod_vi_nc_store");
	EV_MOD_VI_STORE = esim_register_event_with_name(mod_handler_vi_store,
			mem_domain_index, "mod_vi_store");
    EV_MOD_VI_STORE_SEND = esim_register_event_with_name(mod_handler_vi_store,
                        mem_domain_index, "mod_vi_store_send");
    EV_MOD_VI_STORE_RECEIVE = esim_register_event_with_name(mod_handler_vi_store,
                        mem_domain_index, "mod_vi_store_receive");
	EV_MOD_VI_STORE_LOCK = esim_register_event_with_name(mod_handler_vi_store,
			mem_domain_index, "mod_vi_store_lock");
	EV_MOD_VI_STORE_ACTION = esim_register_event_with_name(mod_handler_vi_store,
			mem_domain_index, "mod_vi_store_action");
	EV_MOD_VI_STORE_UNLOCK = esim_register_event_with_name(mod_handler_vi_store,
			mem_domain_index, "mod_vi_store_unlock");
	EV_MOD_VI_STORE_FINISH = esim_register_event_with_name(mod_handler_vi_store,
			mem_domain_index, "mod_vi_store_finish");

	EV_MOD_VI_FIND_AND_LOCK = esim_register_event_with_name(mod_handler_vi_find_and_lock,
			mem_domain_index, "mod_vi_find_and_lock");
	EV_MOD_VI_FIND_AND_LOCK_PORT = esim_register_event_with_name(mod_handler_vi_find_and_lock,
			mem_domain_index, "mod_vi_find_and_lock_port");
	EV_MOD_VI_FIND_AND_LOCK_ACTION = esim_register_event_with_name(mod_handler_vi_find_and_lock,
			mem_domain_index, "mod_vi_find_and_lock_action");
	EV_MOD_VI_FIND_AND_LOCK_FINISH = esim_register_event_with_name(mod_handler_vi_find_and_lock,
			mem_domain_index, "mod_vi_find_and_lock_finish");

	/* NMOESI memory event-driven simulation */

	EV_MOD_NMOESI_LOAD = esim_register_event_with_name(mod_handler_nmoesi_load,
			mem_domain_index, "mod_nmoesi_load");
	EV_MOD_NMOESI_LOAD_SEND = esim_register_event_with_name(mod_handler_nmoesi_load,
      mem_domain_index, "mod_nmoesi_load_send");
	EV_MOD_NMOESI_LOAD_RECEIVE = esim_register_event_with_name(mod_handler_nmoesi_load,
      mem_domain_index, "mod_nmoesi_load_receive");
	EV_MOD_NMOESI_LOAD_LOCK = esim_register_event_with_name(mod_handler_nmoesi_load,
			mem_domain_index, "mod_nmoesi_load_lock");
	EV_MOD_NMOESI_LOAD_LOCK2 = esim_register_event_with_name(mod_handler_nmoesi_load,
			mem_domain_index, "mod_nmoesi_load_lock2");
	EV_MOD_NMOESI_LOAD_ACTION = esim_register_event_with_name(mod_handler_nmoesi_load,
			mem_domain_index, "mod_nmoesi_load_action");
	EV_MOD_NMOESI_LOAD_MISS = esim_register_event_with_name(mod_handler_nmoesi_load,
			mem_domain_index, "mod_nmoesi_load_miss");
	EV_MOD_NMOESI_LOAD_UNLOCK = esim_register_event_with_name(mod_handler_nmoesi_load,
			mem_domain_index, "mod_nmoesi_load_unlock");
	EV_MOD_NMOESI_LOAD_FINISH = esim_register_event_with_name(mod_handler_nmoesi_load,
			mem_domain_index, "mod_nmoesi_load_finish");

	EV_MOD_NMOESI_STORE = esim_register_event_with_name(mod_handler_nmoesi_store,
			mem_domain_index, "mod_nmoesi_store");
	EV_MOD_NMOESI_STORE_SEND = esim_register_event_with_name(mod_handler_nmoesi_store,
      mem_domain_index, "mod_nmoesi_store_send");
  EV_MOD_NMOESI_STORE_RECEIVE = esim_register_event_with_name(mod_handler_nmoesi_store,
    	mem_domain_index, "mod_nmoesi_store_receive");
	EV_MOD_NMOESI_STORE_LOCK = esim_register_event_with_name(mod_handler_nmoesi_store,
			mem_domain_index, "mod_nmoesi_store_lock");
	EV_MOD_NMOESI_STORE_ACTION = esim_register_event_with_name(mod_handler_nmoesi_store,
			mem_domain_index, "mod_nmoesi_store_action");
	EV_MOD_NMOESI_STORE_UNLOCK = esim_register_event_with_name(mod_handler_nmoesi_store,
			mem_domain_index, "mod_nmoesi_store_unlock");
	EV_MOD_NMOESI_STORE_FINISH = esim_register_event_with_name(mod_handler_nmoesi_store,
			mem_domain_index, "mod_nmoesi_store_finish");

	EV_MOD_NMOESI_NC_STORE = esim_register_event_with_name(mod_handler_nmoesi_nc_store,
			mem_domain_index, "mod_nmoesi_nc_store");
	EV_MOD_NMOESI_NC_STORE_SEND = esim_register_event_with_name(mod_handler_nmoesi_nc_store,
      mem_domain_index, "mod_nmoesi_nc_store_send");
  EV_MOD_NMOESI_NC_STORE_RECEIVE = esim_register_event_with_name(mod_handler_nmoesi_nc_store,
      mem_domain_index, "mod_nmoesi_nc_store_receive");
	EV_MOD_NMOESI_NC_STORE_LOCK = esim_register_event_with_name(mod_handler_nmoesi_nc_store,
			mem_domain_index, "mod_nmoesi_nc_store_lock");
	EV_MOD_NMOESI_NC_STORE_LOCK2 = esim_register_event_with_name(mod_handler_nmoesi_nc_store,
			mem_domain_index, "mod_nmoesi_nc_store_lock2");
	EV_MOD_NMOESI_NC_STORE_WRITEBACK = esim_register_event_with_name(mod_handler_nmoesi_nc_store,
			mem_domain_index, "mod_nmoesi_nc_store_writeback");
	EV_MOD_NMOESI_NC_STORE_ACTION = esim_register_event_with_name(mod_handler_nmoesi_nc_store,
			mem_domain_index, "mod_nmoesi_nc_store_action");
	EV_MOD_NMOESI_NC_STORE_MISS= esim_register_event_with_name(mod_handler_nmoesi_nc_store,
			mem_domain_index, "mod_nmoesi_nc_store_miss");
	EV_MOD_NMOESI_NC_STORE_UNLOCK = esim_register_event_with_name(mod_handler_nmoesi_nc_store,
			mem_domain_index, "mod_nmoesi_nc_store_unlock");
	EV_MOD_NMOESI_NC_STORE_FINISH = esim_register_event_with_name(mod_handler_nmoesi_nc_store,
			mem_domain_index, "mod_nmoesi_nc_store_finish");

	EV_MOD_NMOESI_PREFETCH = esim_register_event_with_name(mod_handler_nmoesi_prefetch,
			mem_domain_index, "mod_nmoesi_prefetch");
	EV_MOD_NMOESI_PREFETCH_LOCK = esim_register_event_with_name(mod_handler_nmoesi_prefetch,
			mem_domain_index, "mod_nmoesi_prefetch_lock");
	EV_MOD_NMOESI_PREFETCH_ACTION = esim_register_event_with_name(mod_handler_nmoesi_prefetch,
			mem_domain_index, "mod_nmoesi_prefetch_action");
	EV_MOD_NMOESI_PREFETCH_MISS = esim_register_event_with_name(mod_handler_nmoesi_prefetch,
			mem_domain_index, "mod_nmoesi_prefetch_miss");
	EV_MOD_NMOESI_PREFETCH_UNLOCK = esim_register_event_with_name(mod_handler_nmoesi_prefetch,
			mem_domain_index, "mod_nmoesi_prefetch_unlock");
	EV_MOD_NMOESI_PREFETCH_FINISH = esim_register_event_with_name(mod_handler_nmoesi_prefetch,
			mem_domain_index, "mod_nmoesi_prefetch_finish");

	EV_MOD_NMOESI_FIND_AND_LOCK = esim_register_event_with_name(mod_handler_nmoesi_find_and_lock,
			mem_domain_index, "mod_nmoesi_find_and_lock");
	EV_MOD_NMOESI_FIND_AND_LOCK_PORT = esim_register_event_with_name(mod_handler_nmoesi_find_and_lock,
			mem_domain_index, "mod_nmoesi_find_and_lock_port");
	EV_MOD_NMOESI_FIND_AND_LOCK_ACTION = esim_register_event_with_name(mod_handler_nmoesi_find_and_lock,
			mem_domain_index, "mod_nmoesi_find_and_lock_action");
	EV_MOD_NMOESI_FIND_AND_LOCK_FINISH = esim_register_event_with_name(mod_handler_nmoesi_find_and_lock,
			mem_domain_index, "mod_nmoesi_find_and_lock_finish");

        EV_MOD_NMOESI_EVICT_CHECK = esim_register_event_with_name(mod_handler_nmoesi_evict,
			mem_domain_index, "mod_nmoesi_evict_check");
        EV_MOD_NMOESI_EVICT_LOCK_DIR = esim_register_event_with_name(mod_handler_nmoesi_evict,
			mem_domain_index, "mod_nmoesi_evict_lock_dir");
	EV_MOD_NMOESI_EVICT = esim_register_event_with_name(mod_handler_nmoesi_evict,
			mem_domain_index, "mod_nmoesi_evict");
	EV_MOD_NMOESI_EVICT_INVALID = esim_register_event_with_name(mod_handler_nmoesi_evict,
			mem_domain_index, "mod_nmoesi_evict_invalid");
	EV_MOD_NMOESI_EVICT_ACTION = esim_register_event_with_name(mod_handler_nmoesi_evict,
			mem_domain_index, "mod_nmoesi_evict_action");
	EV_MOD_NMOESI_EVICT_RECEIVE = esim_register_event_with_name(mod_handler_nmoesi_evict,
			mem_domain_index, "mod_nmoesi_evict_receive");
	EV_MOD_NMOESI_EVICT_PROCESS = esim_register_event_with_name(mod_handler_nmoesi_evict,
			mem_domain_index, "mod_nmoesi_evict_process");
	EV_MOD_NMOESI_EVICT_PROCESS_NONCOHERENT = esim_register_event_with_name(mod_handler_nmoesi_evict,
			mem_domain_index, "mod_nmoesi_evict_process_noncoherent");
	EV_MOD_NMOESI_EVICT_REPLY = esim_register_event_with_name(mod_handler_nmoesi_evict,
			mem_domain_index, "mod_nmoesi_evict_reply");
	EV_MOD_NMOESI_EVICT_REPLY = esim_register_event_with_name(mod_handler_nmoesi_evict,
			mem_domain_index, "mod_nmoesi_evict_reply");
	EV_MOD_NMOESI_EVICT_REPLY_RECEIVE = esim_register_event_with_name(mod_handler_nmoesi_evict,
			mem_domain_index, "mod_nmoesi_evict_reply_receive");
	EV_MOD_NMOESI_EVICT_FINISH = esim_register_event_with_name(mod_handler_nmoesi_evict,
			mem_domain_index, "mod_nmoesi_evict_finish");

	EV_MOD_NMOESI_WRITE_REQUEST = esim_register_event_with_name(mod_handler_nmoesi_write_request,
			mem_domain_index, "mod_nmoesi_write_request");
	EV_MOD_NMOESI_WRITE_REQUEST_RECEIVE = esim_register_event_with_name(mod_handler_nmoesi_write_request,
			mem_domain_index, "mod_nmoesi_write_request_receive");
	EV_MOD_NMOESI_WRITE_REQUEST_ACTION = esim_register_event_with_name(mod_handler_nmoesi_write_request,
			mem_domain_index, "mod_nmoesi_write_request_action");
	EV_MOD_NMOESI_WRITE_REQUEST_EXCLUSIVE = esim_register_event_with_name(mod_handler_nmoesi_write_request,
			mem_domain_index, "mod_nmoesi_write_request_exclusive");
	EV_MOD_NMOESI_WRITE_REQUEST_UPDOWN = esim_register_event_with_name(mod_handler_nmoesi_write_request,
			mem_domain_index, "mod_nmoesi_write_request_updown");
	EV_MOD_NMOESI_WRITE_REQUEST_UPDOWN_FINISH = esim_register_event_with_name(mod_handler_nmoesi_write_request,
			mem_domain_index, "mod_nmoesi_write_request_updown_finish");
	EV_MOD_NMOESI_WRITE_REQUEST_DOWNUP = esim_register_event_with_name(mod_handler_nmoesi_write_request,
			mem_domain_index, "mod_nmoesi_write_request_downup");
	EV_MOD_NMOESI_WRITE_REQUEST_DOWNUP_FINISH = esim_register_event_with_name(mod_handler_nmoesi_write_request,
			mem_domain_index, "mod_nmoesi_write_request_downup_finish");
	EV_MOD_NMOESI_WRITE_REQUEST_REPLY = esim_register_event_with_name(mod_handler_nmoesi_write_request,
			mem_domain_index, "mod_nmoesi_write_request_reply");
	EV_MOD_NMOESI_WRITE_REQUEST_FINISH = esim_register_event_with_name(mod_handler_nmoesi_write_request,
			mem_domain_index, "mod_nmoesi_write_request_finish");

	EV_MOD_NMOESI_READ_REQUEST = esim_register_event_with_name(mod_handler_nmoesi_read_request,
			mem_domain_index, "mod_nmoesi_read_request");
	EV_MOD_NMOESI_READ_REQUEST_RECEIVE = esim_register_event_with_name(mod_handler_nmoesi_read_request,
			mem_domain_index, "mod_nmoesi_read_request_receive");
	EV_MOD_NMOESI_READ_REQUEST_ACTION = esim_register_event_with_name(mod_handler_nmoesi_read_request,
			mem_domain_index, "mod_nmoesi_read_request_action");
	EV_MOD_NMOESI_READ_REQUEST_UPDOWN = esim_register_event_with_name(mod_handler_nmoesi_read_request,
			mem_domain_index, "mod_nmoesi_read_request_updown");
	EV_MOD_NMOESI_READ_REQUEST_UPDOWN_MISS = esim_register_event_with_name(mod_handler_nmoesi_read_request,
			mem_domain_index, "mod_nmoesi_read_request_updown_miss");
	EV_MOD_NMOESI_READ_REQUEST_UPDOWN_FINISH = esim_register_event_with_name(mod_handler_nmoesi_read_request,
			mem_domain_index, "mod_nmoesi_read_request_updown_finish");
	EV_MOD_NMOESI_READ_REQUEST_DOWNUP = esim_register_event_with_name(mod_handler_nmoesi_read_request,
			mem_domain_index, "mod_nmoesi_read_request_downup");
	EV_MOD_NMOESI_READ_REQUEST_DOWNUP_WAIT_FOR_REQS = esim_register_event_with_name(mod_handler_nmoesi_read_request,
			mem_domain_index, "mod_nmoesi_read_request_downup_wait_for_reqs");
	EV_MOD_NMOESI_READ_REQUEST_DOWNUP_FINISH = esim_register_event_with_name(mod_handler_nmoesi_read_request,
			mem_domain_index, "mod_nmoesi_read_request_downup_finish");
	EV_MOD_NMOESI_READ_REQUEST_REPLY = esim_register_event_with_name(mod_handler_nmoesi_read_request,
			mem_domain_index, "mod_nmoesi_read_request_reply");
	EV_MOD_NMOESI_READ_REQUEST_FINISH = esim_register_event_with_name(mod_handler_nmoesi_read_request,
			mem_domain_index, "mod_nmoesi_read_request_finish");

	EV_MOD_NMOESI_INVALIDATE = esim_register_event_with_name(mod_handler_nmoesi_invalidate,
			mem_domain_index, "mod_nmoesi_invalidate");
	EV_MOD_NMOESI_INVALIDATE_FINISH = esim_register_event_with_name(mod_handler_nmoesi_invalidate,
			mem_domain_index, "mod_nmoesi_invalidate_finish");

	EV_MOD_NMOESI_PEER_SEND = esim_register_event_with_name(mod_handler_nmoesi_peer,
			mem_domain_index, "mod_nmoesi_peer_send");
	EV_MOD_NMOESI_PEER_RECEIVE = esim_register_event_with_name(mod_handler_nmoesi_peer,
			mem_domain_index, "mod_nmoesi_peer_receive");
	EV_MOD_NMOESI_PEER_REPLY = esim_register_event_with_name(mod_handler_nmoesi_peer,
			mem_domain_index, "mod_nmoesi_peer_reply");
	EV_MOD_NMOESI_PEER_FINISH = esim_register_event_with_name(mod_handler_nmoesi_peer,
			mem_domain_index, "mod_nmoesi_peer_finish");

	EV_MOD_NMOESI_MESSAGE = esim_register_event_with_name(mod_handler_nmoesi_message,
			mem_domain_index, "mod_nmoesi_message");
	EV_MOD_NMOESI_MESSAGE_RECEIVE = esim_register_event_with_name(mod_handler_nmoesi_message,
			mem_domain_index, "mod_nmoesi_message_receive");
	EV_MOD_NMOESI_MESSAGE_ACTION = esim_register_event_with_name(mod_handler_nmoesi_message,
			mem_domain_index, "mod_nmoesi_message_action");
	EV_MOD_NMOESI_MESSAGE_REPLY = esim_register_event_with_name(mod_handler_nmoesi_message,
			mem_domain_index, "mod_nmoesi_message_reply");
	EV_MOD_NMOESI_MESSAGE_FINISH = esim_register_event_with_name(mod_handler_nmoesi_message,
			mem_domain_index, "mod_nmoesi_message_finish");

	/* Local memory event driven simulation */

	EV_MOD_LOCAL_MEM_LOAD = esim_register_event_with_name(mod_handler_local_mem_load,
			mem_domain_index, "mod_local_mem_load");
	EV_MOD_LOCAL_MEM_LOAD_LOCK = esim_register_event_with_name(mod_handler_local_mem_load,
			mem_domain_index, "mod_local_mem_load_lock");
	EV_MOD_LOCAL_MEM_LOAD_FINISH = esim_register_event_with_name(mod_handler_local_mem_load,
			mem_domain_index, "mod_local_mem_load_finish");

	EV_MOD_LOCAL_MEM_STORE = esim_register_event_with_name(mod_handler_local_mem_store,
			mem_domain_index, "mod_local_mem_store");
	EV_MOD_LOCAL_MEM_STORE_LOCK = esim_register_event_with_name(mod_handler_local_mem_store,
			mem_domain_index, "mod_local_mem_store_lock");
	EV_MOD_LOCAL_MEM_STORE_FINISH = esim_register_event_with_name(mod_handler_local_mem_store,
			mem_domain_index, "mod_local_mem_store_finish");

	EV_MOD_LOCAL_MEM_FIND_AND_LOCK = esim_register_event_with_name(mod_handler_local_mem_find_and_lock,
			mem_domain_index, "mod_local_mem_find_and_lock");
	EV_MOD_LOCAL_MEM_FIND_AND_LOCK_PORT = esim_register_event_with_name(mod_handler_local_mem_find_and_lock,
			mem_domain_index, "mod_local_mem_find_and_lock_port");
	EV_MOD_LOCAL_MEM_FIND_AND_LOCK_ACTION = esim_register_event_with_name(mod_handler_local_mem_find_and_lock,
			mem_domain_index, "mod_local_mem_find_and_lock_action");
	EV_MOD_LOCAL_MEM_FIND_AND_LOCK_FINISH = esim_register_event_with_name(mod_handler_local_mem_find_and_lock,
			mem_domain_index, "mod_local_mem_find_and_lock_finish");
        
        //FRAN
	ini_estadisticas();
}


void mem_system_done(void)
{
	/* Dump report */
	mem_system_dump_report();

	/* Free memory system */
	mem_system_free(mem_system);
}

void fran_report_general(){

	struct mod_t *mod;
	int i;

	long long accesses = 0;
	long long effective_reads = 0;
	long long effective_writes = 0;
	long long effective_nc_writes = 0;
	long long hits = 0;
	//mod_contador.misses += mod->accesses - mod->hits);
	long long delayed_read_hit = 0;
	long long reads = 0;
	long long read_retries = 0;
	long long writes = 0;
	long long write_retries = 0;

	/* Report for each cache */
	for (i = 0; i < list_count(mem_system->mod_list); i++)
	{
		mod = list_get(mem_system->mod_list, i);

		if(mod->level == 2){
			accesses += mod->accesses;
			effective_reads += mod->effective_reads;
			effective_writes += mod->effective_writes;
			effective_nc_writes += mod->effective_nc_writes;
			hits += mod->hits;
			//mod_contador.misses += mod->accesses - mod->hits);
			delayed_read_hit += mod->delayed_read_hit;
			reads += mod->reads;
			read_retries += mod->read_retries;
			writes += mod->writes;
			write_retries += mod->write_retries;
			effective_reads += mod->effective_reads;
       		}
	}
//fran_debug_general("%lld,%lld,%lld,%lld,%lld,%lld,%.4g,%lld,%lld,%lld\n", accesses, effective_reads, effective_writes, effective_nc_writes, hits, accesses - hits, accesses ? ((double) hits) / accesses : 0.0, reads, delayed_read_hit, writes);


}

void mem_system_dump_report(void)
{
	struct net_t *net;
	struct mod_t *mod;
	struct cache_t *cache;
	bool report_access_lists = false;

	FILE *f;

	int i;

	/* Open file */
	f = file_open_for_write(mem_report_file_name);
	if (!f)
		return;

	/* Intro */
	fprintf(f, "; Report for caches, TLBs, and main memory\n");
	fprintf(f, ";    Accesses - Total number of accesses\n");
	fprintf(f, ";    Hits, Misses - Accesses resulting in hits/misses\n");
	fprintf(f, ";    HitRatio - Hits divided by accesses\n");
	fprintf(f, ";    Evictions - Invalidated or replaced cache blocks\n");
	fprintf(f, ";    Retries - For L1 caches, accesses that were retried\n");
	fprintf(f, ";    ReadRetries, WriteRetries, NCWriteRetries - Read/Write retried accesses\n");
	fprintf(f, ";    NoRetryAccesses - Number of accesses that were not retried\n");
	fprintf(f, ";    NoRetryHits, NoRetryMisses - Hits and misses for not retried accesses\n");
	fprintf(f, ";    NoRetryHitRatio - NoRetryHits divided by NoRetryAccesses\n");
	fprintf(f, ";    NoRetryReads, NoRetryWrites - Not retried reads and writes\n");
	fprintf(f, ";    Reads, Writes, NCWrites - Total read/write accesses\n");
	fprintf(f, ";    BlockingReads, BlockingWrites, BlockingNCWrites - Reads/writes coming from lower-level cache\n");
	fprintf(f, ";    NonBlockingReads, NonBlockingWrites, NonBlockingNCWrites - Coming from upper-level cache\n");
	fprintf(f, "\n\n");

	fprintf(f, "Directory type = %d\n\n",directory_type);

	/* Report for each cache */
	for (i = 0; i < list_count(mem_system->mod_list); i++)
	{
		mod = list_get(mem_system->mod_list, i);

		if(mod->access_list_count != 0)
			report_access_lists = true;

		cache = mod->cache;
		fprintf(f, "[ %s ]\n", mod->name);
		fprintf(f, "\n");

		/* Configuration */
		if (cache) {
			fprintf(f, "Level = %d\n",mod->level);
			fprintf(f, "Sets = %d\n", cache->num_sets);
			fprintf(f, "Assoc = %d\n", cache->assoc);
			fprintf(f, "Policy = %s\n", str_map_value(&cache_policy_map, cache->policy));
		}
		fprintf(f, "BlockSize = %d\n", mod->block_size);
		fprintf(f, "Latency = %d\n", mod->latency);
		fprintf(f, "DirectoryLatency = %d\n", mod->dir_latency);
		fprintf(f, "Ports = %d\n", mod->num_ports);
		fprintf(f, "\n");

		/* Statistics */
		fprintf(f, "Accesses = %lld\n", mod->accesses);
                fprintf(f, "Loads = %lld\n", mod->loads);
		fprintf(f, "Hits = %lld\n", mod->hits);
		fprintf(f, "Hits_aux = %lld\n", mod->hits_aux);
		fprintf(f, "Misses = %lld\n", mod->accesses - mod->hits);
		fprintf(f, "HitRatio = %.4g\n", mod->accesses ?
			(double) mod->hits / mod->accesses : 0.0);
                fprintf(f, "HitRatio_aux = %.4g\n", mod->loads ?
                        (double) mod->hits_aux / mod->loads : 0.0);
		fprintf(f, "Evictions = %lld\n", mod->evictions);
		//FRAN
		fprintf(f, "EvictionsWithSharers = %lld\n", mod->evictions_with_sharers);
		fprintf(f, "EvictionsSharersInvalidation = %lld\n", mod->evictions_sharers_invalidation);
		fprintf(f, "Retries = %lld\n", mod->read_retries + mod->write_retries +
			mod->nc_write_retries);
		fprintf(f,"\n");
		fprintf(f,"Scalar_Load = %lld\n",mod->scalar_load);
		fprintf(f,"Scalar_Load_nc = %lld\n",mod->scalar_load_nc);
		fprintf(f,"Vector_Load = %lld\n",mod->vector_load);
		fprintf(f,"Vector_Load_nc = %lld\n",mod->vector_load_nc);
		fprintf(f,"Vector_Write = %lld\n",mod->vector_write);
		fprintf(f,"Vector_Write_ncb= %lld\n",mod->vector_write_nc);

		fprintf(f, "\n");
		fprintf(f, "Reads = %lld\n", mod->reads);
		fprintf(f, "CoalescedRead = %lld\n",mod->delayed_read_hit);
		fprintf(f, "ReadRetries = %lld\n", mod->read_retries);
		fprintf(f, "BlockingReads = %lld\n", mod->blocking_reads);
		fprintf(f, "NonBlockingReads = %lld\n", mod->non_blocking_reads);
		fprintf(f, "ReadHits = %lld\n", mod->read_hits);
		fprintf(f, "ReadMisses = %lld\n", mod->reads - mod->read_hits);
		fprintf(f, "\n");
		fprintf(f, "Writes = %lld\n", mod->writes);
		fprintf(f, "WriteRetries = %lld\n", mod->write_retries);
		fprintf(f, "BlockingWrites = %lld\n", mod->blocking_writes);
		fprintf(f, "NonBlockingWrites = %lld\n", mod->non_blocking_writes);
		fprintf(f, "WriteHits = %lld\n", mod->write_hits);
		fprintf(f, "WriteMisses = %lld\n", mod->writes - mod->write_hits);
		fprintf(f, "\n");
		fprintf(f, "NCWrites = %lld\n", mod->nc_writes);
		fprintf(f, "NCWriteRetries = %lld\n", mod->nc_write_retries);
		fprintf(f, "NCBlockingWrites = %lld\n", mod->blocking_nc_writes);
		fprintf(f, "NCNonBlockingWrites = %lld\n", mod->non_blocking_nc_writes);
		fprintf(f, "NCWriteHits = %lld\n", mod->nc_write_hits);
		fprintf(f, "NCWriteMisses = %lld\n", mod->nc_writes - mod->nc_write_hits);
		fprintf(f, "Prefetches = %lld\n", mod->prefetches);
		fprintf(f, "PrefetchAborts = %lld\n", mod->prefetch_aborts);
		fprintf(f, "UselessPrefetches = %lld\n", mod->useless_prefetches);
		fprintf(f, "\n");
		fprintf(f, "NoRetryAccesses = %lld\n", mod->no_retry_accesses);
		fprintf(f, "NoRetryHits = %lld\n", mod->no_retry_hits);
		fprintf(f, "NoRetryMisses = %lld\n", mod->no_retry_accesses - mod->no_retry_hits);
		fprintf(f, "NoRetryHitRatio = %.4g\n", mod->no_retry_accesses ?
			(double) mod->no_retry_hits / mod->no_retry_accesses : 0.0);
		fprintf(f, "NoRetryReads = %lld\n", mod->no_retry_reads);
		fprintf(f, "NoRetryReadHits = %lld\n", mod->no_retry_read_hits);
		fprintf(f, "NoRetryReadMisses = %lld\n", (mod->no_retry_reads -
			mod->no_retry_read_hits));
		fprintf(f, "NoRetryWrites = %lld\n", mod->no_retry_writes);
		fprintf(f, "NoRetryWriteHits = %lld\n", mod->no_retry_write_hits);
		fprintf(f, "NoRetryWriteMisses = %lld\n", mod->no_retry_writes
			- mod->no_retry_write_hits);
		fprintf(f, "NoRetryNCWrites = %lld\n", mod->no_retry_nc_writes);
		fprintf(f, "NoRetryNCWriteHits = %lld\n", mod->no_retry_nc_write_hits);
		fprintf(f, "NoRetryNCWriteMisses = %lld\n", mod->no_retry_nc_writes
			- mod->no_retry_nc_write_hits);
		fprintf(f, "\n\n");
	}

	if(report_access_lists)
	{
		fprintf(f,"STALLS REPORTS\n\n");
		for (i = 0; i < list_count(mem_system->mod_list); i++)
		{
			mod = list_get(mem_system->mod_list, i);
			while(mod->access_list_count)
			{
				struct mod_stack_t *stack = mod->access_list_head;
				struct mod_stack_t *aux_stack;
				fprintf(f,"Stack %lld : \n",stack->id);

				if(stack->master_stack){
					fprintf(f,"Master_stack = %lld \n",stack->master_stack->id);
				}

				fprintf(f,"waiting_list ->");

				while (stack->waiting_list_count)
				{
					fprintf(f," %lld",stack->waiting_list_head->id);
					aux_stack = stack->waiting_list_head;
					DOUBLE_LINKED_LIST_REMOVE(stack, waiting, aux_stack);
				}
				fprintf(f,"\n\n");
				DOUBLE_LINKED_LIST_REMOVE(mod, access, stack);
			}
		}
	}
	/* Dump report for networks */
	for (i = 0; i < list_count(mem_system->net_list); i++)
	{
		net = list_get(mem_system->net_list, i);
		net_dump_report(net, f);
	}

	/* Done */
	fclose(f);
}

void main_memory_power_callback(double a, double b, double c, double d)
{
    
}


void main_memory_read_callback(void *payload, unsigned int id, uint64_t address, uint64_t interthread_penalty)
{
	int found = 0;
	//int cpu_freq; /* In MHz */
	//int dram_freq; /* In MHz */
	//struct x86_uop_t *uop;
	struct mod_stack_t *stack = NULL;
	struct dram_system_t *dram_system = (struct dram_system_t *) payload;

	/* You cannnot use LINKED_LIST_FOR_EACH if you plan to remove elements */
	linked_list_head(dram_system->pending_reads);
	while(!linked_list_is_end(dram_system->pending_reads))
	{
		stack = linked_list_get(dram_system->pending_reads);
		if (stack->addr == address)
		{
			mem_debug("  %lld %lld 0x%x %s dram access completed\n", esim_time, stack->id, stack->addr, stack->target_mod->dram_system->name);
			stack->main_memory_accessed = 1;

			if(stack->uop)
			{
				stack->uop->mem_mm_latency += asTiming(si_gpu)->cycle  - stack->dramsim_mm_start;
				stack->uop->mem_mm_accesses++;
			}

			if(directory_type == dir_type_nmoesi)
			{
				dir_entry_unlock(stack->dir_entry);
				esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST_REPLY, stack, 0);
			}else{
				esim_schedule_mod_stack_event(stack, 0);
			}
			linked_list_remove(dram_system->pending_reads);
			found++;
		}
		else
			linked_list_next(dram_system->pending_reads);
	}
	assert(found == 1);
/*
	cpu_freq = arch_x86->frequency;
	dram_freq = esim_domain_frequency(dram_system->dram_domain_index);

	uop = stack->event_queue_item;
	if (uop)
		uop->uinst->dram_interthread_penalty_cycles += interthread_penalty * cpu_freq / dram_freq;
	assert(uop || stack->client_info->instr_fetch || stack->prefetch);
	*//* If there isn't a uop, then it must be a instruction fetch access or a prefetch */
}


void main_memory_write_callback(void *payload, unsigned int id, uint64_t address, uint64_t clock_cycle)
{
    	/*int found = 0;
	struct mod_stack_t *stack = NULL;
	struct dram_system_t *dram_system = (struct dram_system_t *) payload;*/

	/* You cannnot use LINKED_LIST_FOR_EACH if you plan to remove elements */
	/*linked_list_head(dram_system->pending_writes);
	while(!linked_list_is_end(dram_system->pending_writes))
	{
		stack = linked_list_get(dram_system->pending_writes);
		if (stack->addr == address)
		{
			mem_debug("  %lld %lld 0x%x %s dram access completed\n", esim_time, stack->id, stack->addr, stack->target_mod->dram_system->name);
			stack->main_memory_accessed = 1;

			if(stack->uop)
			{
				stack->uop->mem_mm_latency += asTiming(si_gpu)->cycle  - stack->dramsim_mm_start;
				stack->uop->mem_mm_accesses++;
			}

			if(directory_type == dir_type_nmoesi)
			{
				dir_entry_unlock(stack->dir_entry);
				esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST_REPLY, stack, 0);
			}else{
				esim_schedule_mod_stack_event(stack, 0);
			}
			linked_list_remove(dram_system->pending_writes);
			found++;
		}
		else
			linked_list_next(dram_system->pending_writes);
	}
	assert(found == 1);*/
}


/* Schedules an event to notify dramsim that a main memory cycle has passed */
void main_memory_tic_scheduler(struct dram_system_t *ds)
{
	//int cpu_freq = arch_x86->frequency; /* In MHz */
	int dram_freq = dram_system_get_dram_freq(ds->handler) / 1000000; /* In MHz */
        printf("\nregister ds event with freq: %d = %lld / 1000000\n", dram_freq, dram_system_get_dram_freq(ds->handler));

	//assert(cpu_freq >= dram_freq);

	/* New domain and event for dramsim clock tics */
	ds->dram_domain_index = esim_new_domain(dram_freq);
	EV_MAIN_MEMORY_TIC = esim_register_event_with_name(main_memory_tic_handler, ds->dram_domain_index, "dram_system_tic");

	esim_schedule_event(EV_MAIN_MEMORY_TIC, ds, 1);
}


/* Notifies dramsim that a main memory cycle has passed */
void main_memory_tic_handler(int event, void *data)
{
	struct dram_system_t *ds = (struct dram_system_t*) data;

	/* If simulation has ended and dram system has no more petitions, no more
	 * events to schedule. */
	if (esim_finish && !linked_list_count(ds->pending_reads) && !esim_event_count())
		return;

	dram_system_dram_tick(ds->handler);
	esim_schedule_event(event, ds, 1);
}


struct mod_t *mem_system_get_mod(char *mod_name)
{
	struct mod_t *mod;

	int mod_id;

	/* Look for module */
	LIST_FOR_EACH(mem_system->mod_list, mod_id)
	{
		mod = list_get(mem_system->mod_list, mod_id);
		if (!strcasecmp(mod->name, mod_name))
			return mod;
	}

	/* Not found */
	return NULL;
}


struct net_t *mem_system_get_net(char *net_name)
{
	struct net_t *net;

	int net_id;

	/* Look for network */
	LIST_FOR_EACH(mem_system->net_list, net_id)
	{
		net = list_get(mem_system->net_list, net_id);
		if (!strcasecmp(net->name, net_name))
			return net;
	}

	/* Not found */
	return NULL;
}


/*
int temporizador_reinicio = 50;

void mshr_control(int latencia, int opc)
{
        int flag = 1;
	int accion = 0;
        struct mod_t *mod;

        for (int k = 0; k < list_count(mem_system->mod_list); k++)
        {
                mod = list_get(mem_system->mod_list, k);
		if(mod->level == 1)
         		break;
	}

        //reinicio
	temporizador_reinicio--;

	if(temporizador_reinicio <= 0)
	{
		temporizador_reinicio = 50;
		accion = 3;
	}

	// primera decision
	if(!mod->mshr->size_anterior)
	{
		if(latencia > 1000)
			accion = 2;
		else
			accion = 1;
	}

	// dependiendo del OPC
	if(accion == 0 && opc < (mod->mshr->ipc_anterior * 0.95))
	{
		if((mod->mshr->latencia_anterior*1.05) < latencia)
		{
			if(mod->mshr->size > mod->mshr->size_anterior)
				accion = 2;
			else if(mod->mshr->size < mod->mshr->size_anterior)
				accion = 1;
			else
			{
				if(mod->mshr->size == (mod->dir->ysize * mod->dir->xsize))
					accion = 2;
				else
					accion = 1;
			}

		}
	}else if(accion == 0 && opc > (mod->mshr->ipc_anterior * 1.05)){
                if(mod->mshr->size > mod->mshr->size_anterior)
                        accion = 1;
                else if(mod->mshr->size < mod->mshr->size_anterior)
                        accion = 2;
	}

	//dependiendo de la latencia

	//(latencia > 1000)
  //      {
	//accion = 1;
	//}else if(latencia <= 1000){
	//	accion = 2;
	//}

	//if(accion)
	//{
        	mod->mshr->size_anterior = mod->mshr->size;
		mod->mshr->ipc_anterior = opc;
		mod->mshr->latencia_anterior = latencia;
	//}

        for (int k = 0; k < list_count(mem_system->mod_list); k++)
        {
                mod = list_get(mem_system->mod_list, k);

		if(mod->level == 1)
		{
			switch(accion)
			{
				case 1:	if(mod->mshr->size < (mod->dir->ysize * mod->dir->xsize))
					{
						if(mod->mshr->size >= 64)
						{
							mod->mshr->size += 32;
							mod->mshr_size += 32;
						}else{
							mod->mshr->size *= 2;
							mod->mshr_size *= 2;
						}
					}
					break;

				case 2:	if(mod->mshr->size > 8)
					{
						if(mod->mshr->size >= 64)
						{
							mod->mshr->size -= 32;
							mod->mshr_size -= 32;
						}else{
							mod->mshr->size /= 2;
							mod->mshr_size /= 2;
						}
					}
					break;

				case 3: mod->mshr->size_anterior = 0;
					mod->mshr->size = 32;
					mod->mshr_size = 32;
					break;

				default : break;
			}
		}
        }
}*/
