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
#include "local-mem-protocol.h"
#include "mem-system.h"
#include "module.h"
#include "nmoesi-protocol.h"
//fran
#include <lib/util/fran.h>
#include "directory.h"

/*
 * Global Variables
 */

int mem_debug_category;
int mem_trace_category;
int mem_peer_transfers;
int fran_category;

/* Frequency domain, as returned by function 'esim_new_domain'. */
int mem_frequency = 1000;
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

	/* Return */
	return mem_system;
}


void mem_system_free(struct mem_system_t *mem_system)
{
	/* Free memory modules */
	while (list_count(mem_system->mod_list))
		mod_free(list_pop(mem_system->mod_list));
	list_free(mem_system->mod_list);

	/* Free networks */
	while (list_count(mem_system->net_list))
		net_free(list_pop(mem_system->net_list));
	list_free(mem_system->net_list);

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

	/* NMOESI memory event-driven simulation */

	EV_MOD_NMOESI_LOAD = esim_register_event_with_name(mod_handler_nmoesi_load,
			mem_domain_index, "mod_nmoesi_load");
	EV_MOD_NMOESI_LOAD_SEND = esim_register_event_with_name(mod_handler_nmoesi_load,
                        mem_domain_index, "mod_nmoesi_load_send");
	EV_MOD_NMOESI_LOAD_RECEIVE = esim_register_event_with_name(mod_handler_nmoesi_load,
                        mem_domain_index, "mod_nmoesi_load_receive");
	EV_MOD_NMOESI_LOAD_LOCK = esim_register_event_with_name(mod_handler_nmoesi_load,
			mem_domain_index, "mod_nmoesi_load_lock");
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
	EV_MOD_NMOESI_NC_STORE_LOCK = esim_register_event_with_name(mod_handler_nmoesi_nc_store,
			mem_domain_index, "mod_nmoesi_nc_store_lock");
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

	FILE *f;

	int i;

        f = file_open_for_write(mem_report_file_name);
        if (!f)
                return;

        fran_report_general();

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
	
	/* Report for each cache */
	for (i = 0; i < list_count(mem_system->mod_list); i++)
	{
		mod = list_get(mem_system->mod_list, i);
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

	/* Dump report for networks */
	for (i = 0; i < list_count(mem_system->net_list); i++)
	{
		net = list_get(mem_system->net_list, i);
		net_dump_report(net, f);
	}
	
	/* Done */
	fclose(f);
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

void estadisticas(int hit, int lvl){
	 
	estadis[lvl].accesses++;
	 
	estadis[lvl].hits += hit;
	//if(estadis[lvl].accesses == 1){
/*		estadis[lvl].esim_cycle_anterior += esim_cycle();
	//}else 
	if(estadis[lvl].accesses >= 640){
		double hit_ratio = estadis[lvl].hits ? (double) estadis[lvl].hits/ estadis[lvl].accesses : 0.0;
		
		//ciclo = esim_cycle();

		//fran_debug_t1000k("%d %lld %lld\n",lvl,estadis[lvl].esim_cycle_anterior/resolucon,ciclo - estadis[lvl].esim_cycle_anterior/resolucion); //tiempo
		fran_debug_hitRatio("%d %lld %.4f\n",lvl, estadis[lvl].esim_cycle_anterior/640, hit_ratio);
		estadis[lvl].accesses = 0;
		estadis[lvl].hits = 0;
		estadis[lvl].esim_cycle_anterior = 0;
	}*/
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

void estadisticas_1000ciclos(){
// calculo de latencia
double latencia;
int z, x, y, k, i;

if(estadis[0].media_latencia_contador == 0)
	latencia = -1.0;
	//fran_debug_latencia("%.1f\n",-1.0);
else{
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
}
