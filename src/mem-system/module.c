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

#include <assert.h>
#include <math.h>

#include <lib/esim/esim.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/linked-list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <lib/util/repos.h>
#include <arch/common/arch.h>

#include "cache.h"
#include "directory.h"
#include "local-mem-protocol.h"
#include "mem-system.h"
#include "mod-stack.h"
#include "nmoesi-protocol.h"

//fran
#include <lib/util/estadisticas.h>
#include <arch/southern-islands/timing/gpu.h>
#include <lib/util/class.h>
#include "vi-protocol.h"
#include <mem-system/mshr.h>
#include <arch/southern-islands/timing/compute-unit.h>

/* String map for access type */
struct str_map_t mod_access_kind_map =
{
	3, {
		{ "Load", mod_access_load },
		{ "Store", mod_access_store },
		{ "NCStore", mod_access_nc_store },
		{ "Prefetch", mod_access_prefetch }
	}
};

/*
 * Public Functions
 */

struct mod_t *mod_create(char *name, enum mod_kind_t kind, int num_ports,
	int block_size, int latency)
{
	struct mod_t *mod;

	/* Initialize */
	mod = xcalloc(1, sizeof(struct mod_t));
	mod->name = xstrdup(name);
	mod->kind = kind;
	mod->latency = latency;

	/* MSHR */
	mod->mshr = mshr_create();
	//xcalloc(1,sizeof(struct mshr_t));
	mod->coherence_controller = cc_create();

	/* Ports */
	mod->num_ports = num_ports;
	mod->ports = xcalloc(num_ports, sizeof(struct mod_port_t));

	/* Lists */
	mod->low_mod_list = linked_list_create();
	mod->high_mod_list = linked_list_create();

	/* Block size */
	mod->block_size = block_size;
	assert(!(block_size & (block_size - 1)) && block_size >= 4);
	mod->log_block_size = log_base2(block_size);

	mod->client_info_repos = repos_create(sizeof(struct mod_client_info_t), mod->name);
	return mod;
}


void mod_free(struct mod_t *mod)
{
	linked_list_free(mod->low_mod_list);
	linked_list_free(mod->high_mod_list);
	if (mod->cache)
		cache_free(mod->cache);
	if (mod->dir)
		dir_free(mod->dir);
	if (mod->mshr)
		mshr_free(mod->mshr);

	if(mod->coherence_controller)
		cc_free(mod->coherence_controller);

	free(mod->ports);
	repos_free(mod->client_info_repos);
	free(mod->name);
	free(mod);
}


void mod_dump(struct mod_t *mod, FILE *f)
{
}

long long mod_access_si(struct mod_t *mod, enum mod_access_kind_t access_kind,
	unsigned int addr, int *witness_ptr, int bytes, int wg_id, struct si_wavefront_t *wavefront, struct linked_list_t *event_queue,
	void *event_queue_item, struct mod_client_info_t *client_info)
{
	struct mod_stack_t *stack;
	int event;

	/* Create module stack with new ID */
	mod_stack_id++;
	stack = mod_stack_create(mod_stack_id,
		mod, addr, ESIM_EV_NONE, NULL);

	stack->stack_size = bytes;

	// uop reference
	//stack->uop = uop;

	stack->wavefront = wavefront;

	stack->work_group_id_in_cu = wg_id;

	/* Initialize */
	stack->witness_ptr = witness_ptr;
	stack->event_queue = event_queue;
	stack->event_queue_item = event_queue_item;
	stack->client_info = client_info;

	/* Select initial CPU/GPU event */
	if(directory_type == dir_type_nmoesi)
	{
		if (mod->kind == mod_kind_cache || mod->kind == mod_kind_main_memory)
		{
			if (access_kind == mod_access_load)
			{
				event = EV_MOD_NMOESI_LOAD;
			}
			else if (access_kind == mod_access_store)
			{
				event = EV_MOD_NMOESI_STORE;
			}
			else if (access_kind == mod_access_nc_store)
			{
				event = EV_MOD_NMOESI_NC_STORE;
			}
			else if (access_kind == mod_access_prefetch)
			{
				event = EV_MOD_NMOESI_PREFETCH;
			}
			else
			{
				panic("%s: invalid access kind", __FUNCTION__);
			}
		}
		else if (mod->kind == mod_kind_local_memory)
		{
			if (access_kind == mod_access_load)
			{
				event = EV_MOD_LOCAL_MEM_LOAD;
			}
			else if (access_kind == mod_access_store)
			{
				event = EV_MOD_LOCAL_MEM_STORE;
			}
			else
			{
				panic("%s: invalid access kind", __FUNCTION__);
			}
		}
		else
		{
			panic("%s: invalid mod kind", __FUNCTION__);
		}
	}
	else if(directory_type == dir_type_vi)
	{
		if (mod->kind == mod_kind_cache || mod->kind == mod_kind_main_memory)
		{
			if (access_kind == mod_access_load)
			{
				event = EV_MOD_VI_LOAD;
				mod_stack_add_word(stack, addr, 0);
			}
	        else if (access_kind == mod_access_nc_load)
            {
				event = EV_MOD_VI_NC_LOAD;
				mod_stack_add_word(stack, addr, 0);
            }
			else if (access_kind == mod_access_store)
			{
				event = EV_MOD_VI_STORE;
				mod_stack_add_word_dirty(stack, addr, 0);
			}
			else if (access_kind == mod_access_nc_store)
			{
				event = EV_MOD_VI_NC_STORE;
				mod_stack_add_word_dirty(stack, addr, 0);
			}
			else
			{
				panic("%s: invalid access kind", __FUNCTION__);
			}
		}
		else if (mod->kind == mod_kind_local_memory)
		{
			if (access_kind == mod_access_load)
			{
				event = EV_MOD_LOCAL_MEM_LOAD;
			}
			else if (access_kind == mod_access_store)
			{
				event = EV_MOD_LOCAL_MEM_STORE;
			}
			else
			{
				panic("%s: invalid access kind", __FUNCTION__);
			}
		}
		else
		{
			panic("%s: invalid mod kind", __FUNCTION__);
		}
	}
	else
	{
		panic("%s: invalid mod kind", __FUNCTION__);
	}
	/* Schedule */
	esim_execute_event(event, stack);

	/* Return access ID */
	return stack->id;
}

/* Access a memory module.
 * Variable 'witness', if specified, will be increased when the access completes.
 * The function returns a unique access ID.
*/
long long mod_access(struct mod_t *mod, enum mod_access_kind_t access_kind,
	unsigned int addr, int *witness_ptr, struct linked_list_t *event_queue,
	void *event_queue_item, struct mod_client_info_t *client_info)
{
	struct mod_stack_t *stack;
	int event;

	/* Create module stack with new ID */
	mod_stack_id++;
	stack = mod_stack_create(mod_stack_id,
		mod, addr, ESIM_EV_NONE, NULL);

	/* Initialize */
	stack->witness_ptr = witness_ptr;
	stack->event_queue = event_queue;
	stack->event_queue_item = event_queue_item;
	stack->client_info = client_info;

	/* Select initial CPU/GPU event */
	if(directory_type == dir_type_nmoesi)
	{
		if (mod->kind == mod_kind_cache || mod->kind == mod_kind_main_memory)
		{
			if (access_kind == mod_access_load)
			{
				event = EV_MOD_NMOESI_LOAD;
			}
			else if (access_kind == mod_access_store)
			{
				event = EV_MOD_NMOESI_STORE;
			}
			else if (access_kind == mod_access_nc_store)
			{
				event = EV_MOD_NMOESI_NC_STORE;
			}
			else if (access_kind == mod_access_prefetch)
			{
				event = EV_MOD_NMOESI_PREFETCH;
			}
			else
			{
				panic("%s: invalid access kind", __FUNCTION__);
			}
		}
		else if (mod->kind == mod_kind_local_memory)
		{
			if (access_kind == mod_access_load)
			{
				event = EV_MOD_LOCAL_MEM_LOAD;
			}
			else if (access_kind == mod_access_store)
			{
				event = EV_MOD_LOCAL_MEM_STORE;
			}
			else
			{
				panic("%s: invalid access kind", __FUNCTION__);
			}
		}
		else
		{
			panic("%s: invalid mod kind", __FUNCTION__);
		}
	}
	else if(directory_type == dir_type_vi)
	{
		if (mod->kind == mod_kind_cache || mod->kind == mod_kind_main_memory)
		{
			if (access_kind == mod_access_load)
			{
				event = EV_MOD_VI_LOAD;
				mod_stack_add_word(stack, addr, 0);
			}
	        else if (access_kind == mod_access_nc_load)
            {
				event = EV_MOD_VI_NC_LOAD;
				mod_stack_add_word(stack, addr, 0);
            }
			else if (access_kind == mod_access_store)
			{
				event = EV_MOD_VI_STORE;
				mod_stack_add_word_dirty(stack, addr, 0);
			}
			else if (access_kind == mod_access_nc_store)
			{
				event = EV_MOD_VI_NC_STORE;
				mod_stack_add_word_dirty(stack, addr, 0);
			}
			else
			{
				panic("%s: invalid access kind", __FUNCTION__);
			}
		}
		else if (mod->kind == mod_kind_local_memory)
		{
			if (access_kind == mod_access_load)
			{
				event = EV_MOD_LOCAL_MEM_LOAD;
			}
			else if (access_kind == mod_access_store)
			{
				event = EV_MOD_LOCAL_MEM_STORE;
			}
			else
			{
				panic("%s: invalid access kind", __FUNCTION__);
			}
		}
		else
		{
			panic("%s: invalid mod kind", __FUNCTION__);
		}
	}
	else
	{
		panic("%s: invalid mod kind", __FUNCTION__);
	}
	/* Schedule */
	esim_execute_event(event, stack);

	/* Return access ID */
	return stack->id;
}


/* Return true if module can be accessed. */
int mod_can_access(struct mod_t *mod, unsigned int addr)
{
	int non_coalesced_accesses;

	/* There must be a free port */
	assert(mod->num_locked_ports <= mod->num_ports);
	if (mod->num_locked_ports == mod->num_ports)
		return 0;

	/* If no MSHR is given, module can be accessed */
	if (!mod->mshr_size)
		return 1;

	/* Module can be accessed if number of non-coalesced in-flight
	 * accesses is smaller than the MSHR size. */
	non_coalesced_accesses = mod->access_list_count -
		mod->access_list_coalesced_count;
	return non_coalesced_accesses < mod->mshr_size;
}


/* Return {set, way, tag, state} for an address.
 * The function returns TRUE on hit, FALSE on miss. */
int mod_find_block(struct mod_t *mod, unsigned int addr, int *set_ptr,
	int *way_ptr, int *tag_ptr, int *state_ptr)
{
	struct cache_t *cache = mod->cache;
	struct cache_block_t *blk;
	struct dir_lock_t *dir_lock;

	int set;
	int way;
	int tag;

	/* A transient tag is considered a hit if the block is
	 * locked in the corresponding directory. */
	tag = addr & ~cache->block_mask;
	if (mod->range_kind == mod_range_interleaved)
	{
		unsigned int num_mods = mod->range.interleaved.mod;
		set = ((tag >> cache->log_block_size) / num_mods) % cache->num_sets;
	}
	else if (mod->range_kind == mod_range_bounds)
	{
		set = (tag >> cache->log_block_size) % cache->num_sets;
	}
	else
	{
		panic("%s: invalid range kind (%d)", __FUNCTION__, mod->range_kind);
	}

	for (way = 0; way < cache->assoc; way++)
	{
		blk = &cache->sets[set].blocks[way];
		if (blk->tag == tag && blk->state)
			break;
		if (blk->transient_tag == tag)
		{
			dir_lock = dir_lock_get(mod->dir, set, way);
			if (dir_lock->lock)
				break;
		}
	}

	PTR_ASSIGN(set_ptr, set);
	PTR_ASSIGN(tag_ptr, tag);

	/* Miss */
	if (way == cache->assoc)
	{
	/*
		PTR_ASSIGN(way_ptr, 0);
		PTR_ASSIGN(state_ptr, 0);
	*/
		return 0;
	}

	/* Hit */
	PTR_ASSIGN(way_ptr, way);
	PTR_ASSIGN(state_ptr, cache->sets[set].blocks[way].state);
	return 1;
}

int mod_find_block_fran(struct mod_t *mod, unsigned int addr, unsigned int valid_mask, int *set_ptr,
	int *way_ptr, int *tag_ptr, int *state_ptr)
{
	struct cache_t *cache = mod->cache;
	struct cache_block_t *blk;
	struct dir_lock_t *dir_lock;

	int set;
	int way;
	int tag;

	/* A transient tag is considered a hit if the block is
	 * locked in the corresponding directory. */
	tag = addr & ~cache->block_mask;
	if (mod->range_kind == mod_range_interleaved)
	{
		unsigned int num_mods = mod->range.interleaved.mod;
		set = ((tag >> cache->log_block_size) / num_mods) % cache->num_sets;
	}
	else if (mod->range_kind == mod_range_bounds)
	{
		set = (tag >> cache->log_block_size) % cache->num_sets;
	}
	else
	{
		panic("%s: invalid range kind (%d)", __FUNCTION__, mod->range_kind);
	}

	for (way = 0; way < cache->assoc; way++)
	{
		blk = &cache->sets[set].blocks[way];
		if (blk->tag == tag && blk->state)
			break;



/*		if (blk->transient_tag == tag)
		{
			dir_lock = dir_lock_get(mod->dir, set, way);
			if (dir_lock->lock)
				break;
		}*/
	}

	PTR_ASSIGN(set_ptr, set);
	PTR_ASSIGN(tag_ptr, tag);

	/* Miss */
	if (way == cache->assoc || (valid_mask & cache->sets[set].blocks[way].valid_mask))
	{
	/*
		PTR_ASSIGN(way_ptr, 0);
		PTR_ASSIGN(state_ptr, 0);
	*/
		return 0;
	}

	/* Hit */
	PTR_ASSIGN(way_ptr, way);
	PTR_ASSIGN(state_ptr, cache->sets[set].blocks[way].state);
	return 1;
}

void mod_block_set_prefetched(struct mod_t *mod, unsigned int addr, int val)
{
	int set, way;

	assert(mod->kind == mod_kind_cache && mod->cache != NULL);
	if (mod->cache->prefetcher && mod_find_block(mod, addr, &set, &way, NULL, NULL))
	{
		mod->cache->sets[set].blocks[way].prefetched = val;
	}
}

int mod_block_get_prefetched(struct mod_t *mod, unsigned int addr)
{
	int set, way;

	assert(mod->kind == mod_kind_cache && mod->cache != NULL);
	if (mod->cache->prefetcher && mod_find_block(mod, addr, &set, &way, NULL, NULL))
	{
		return mod->cache->sets[set].blocks[way].prefetched;
	}

	return 0;
}

/* Lock a port, and schedule event when done.
 * If there is no free port, the access is enqueued in the port
 * waiting list, and it will retry once a port becomes available with a
 * call to 'mod_unlock_port'. */
void mod_lock_port(struct mod_t *mod, struct mod_stack_t *stack, int event)
{
	struct mod_port_t *port = NULL;
	int i;

	/* No free port */
	if (mod->num_locked_ports >= mod->num_ports)
	{
		assert(!DOUBLE_LINKED_LIST_MEMBER(mod, port_waiting, stack));

		/* If the request to lock the port is down-up, give it priority since
		 * it is possibly holding up a large portion of the memory hierarchy */
		if (stack->request_dir == mod_request_down_up)
		{
			DOUBLE_LINKED_LIST_INSERT_HEAD(mod, port_waiting, stack);
		}
		else
		{
			DOUBLE_LINKED_LIST_INSERT_TAIL(mod, port_waiting, stack);
		}
		stack->port_waiting_list_event = event;
		return;
	}

	/* Get free port */
	for (i = 0; i < mod->num_ports; i++)
	{
		port = &mod->ports[i];
		if (!port->stack)
			break;
	}

	/* Lock port */
	assert(port && i < mod->num_ports);
	port->stack = stack;
	stack->port = port;
	mod->num_locked_ports++;

	/* Debug */
	mem_debug("  %lld stack %lld %s port %d locked\n", esim_time, stack->id, mod->name, i);

	/* Schedule event */
	esim_schedule_event(event, stack, 0);
}


void mod_unlock_port(struct mod_t *mod, struct mod_port_t *port,
	struct mod_stack_t *stack)
{
	int event;

	/* Checks */
	assert(mod->num_locked_ports > 0);
	assert(stack->port == port && port->stack == stack);
	assert(stack->mod == mod);

	/* Unlock port */
	stack->port = NULL;
	port->stack = NULL;
	mod->num_locked_ports--;

	/* Debug */
	mem_debug("  %lld %lld %s port unlocked\n", esim_time,
		stack->id, mod->name);

	/* Check if there was any access waiting for free port */
	if (!mod->port_waiting_list_count)
		return;

	/* Wake up one access waiting for a free port */
	stack = mod->port_waiting_list_head;
	event = stack->port_waiting_list_event;
	stack->port_waiting_list_event = 0;
	assert(DOUBLE_LINKED_LIST_MEMBER(mod, port_waiting, stack));
	DOUBLE_LINKED_LIST_REMOVE(mod, port_waiting, stack);
	mod_lock_port(mod, stack, event);

}


void mod_access_start(struct mod_t *mod, struct mod_stack_t *stack,
	enum mod_access_kind_t access_kind)
{
	int index;

	/* Record access kind */
	stack->access_kind = access_kind;

	/* Insert in access list */
	DOUBLE_LINKED_LIST_INSERT_TAIL(mod, access, stack);

	/* Insert in write access list */
	if (access_kind == mod_access_store)
		DOUBLE_LINKED_LIST_INSERT_TAIL(mod, write_access, stack);

	/* Insert in access hash table */
	index = (stack->addr >> mod->log_block_size) % MOD_ACCESS_HASH_TABLE_SIZE;
	DOUBLE_LINKED_LIST_INSERT_TAIL(&mod->access_hash_table[index], bucket, stack);

	/* estadisticas */
	//add_access(mod->level);
	if(stack->client_info && stack->client_info->arch)
		stack->tiempo_acceso = stack->client_info->arch->timing->cycle;
}


void mod_access_finish(struct mod_t *mod, struct mod_stack_t *stack)
{
	int index;

	/* Remove from access list */
	DOUBLE_LINKED_LIST_REMOVE(mod, access, stack);

	/* Remove from write access list */
	assert(stack->access_kind);
	if (stack->access_kind == mod_access_store)
		DOUBLE_LINKED_LIST_REMOVE(mod, write_access, stack);

	/* Remove from hash table */
	index = (stack->addr >> mod->log_block_size) % MOD_ACCESS_HASH_TABLE_SIZE;
	DOUBLE_LINKED_LIST_REMOVE(&mod->access_hash_table[index], bucket, stack);

	/* If this was a coalesced access, update counter */
	if (stack->coalesced)
	{
		assert(mod->access_list_coalesced_count > 0);
		mod->access_list_coalesced_count--;
	}
	add_mem_access_finished();
	//fran
	stack->finished = 1;
}


/* Return true if the access with identifier 'id' is in flight.
 * The address of the access is passed as well because this lookup is done on the
 * access truth table, indexed by the access address.
 */
int mod_in_flight_access(struct mod_t *mod, long long id, unsigned int addr)
{
	struct mod_stack_t *stack;
	int index;

	/* Look for access */
	index = (addr >> mod->log_block_size) % MOD_ACCESS_HASH_TABLE_SIZE;
	for (stack = mod->access_hash_table[index].bucket_list_head; stack; stack = stack->bucket_list_next)
		if (stack->id == id)
			return 1;

	/* Not found */
	return 0;
}


/* Return the youngest in-flight access older than 'older_than_stack' to block containing 'addr'.
 * If 'older_than_stack' is NULL, return the youngest in-flight access containing 'addr'.
 * The function returns NULL if there is no in-flight access to block containing 'addr'.
 */
struct mod_stack_t *mod_in_flight_address(struct mod_t *mod, unsigned int addr,
	struct mod_stack_t *older_than_stack)
{
	struct mod_stack_t *stack;
	int index;

	/* Look for address */
	index = (addr >> mod->log_block_size) % MOD_ACCESS_HASH_TABLE_SIZE;
	for (stack = mod->access_hash_table[index].bucket_list_head; stack;
		stack = stack->bucket_list_next)
	{
		/* This stack is not older than 'older_than_stack' */
		if (older_than_stack && stack->id >= older_than_stack->id)
			continue;

		/* Address matches */
		if (stack->addr >> mod->log_block_size == addr >> mod->log_block_size)
			return stack;
	}

	/* Not found */
	return NULL;
}

struct mod_stack_t *mod_global_in_flight_address(struct mod_t *mod,
	struct mod_stack_t *stack)
{
	struct mod_stack_t *ret_stack = NULL;
	struct mod_t *target_mod = mod_get_low_mod(mod, stack->tag),*mod_in_conflict;
	struct mod_stack_t *aux_stack = mod_stack_create(stack->id, target_mod,
		stack->addr, 0, NULL);
	struct dir_lock_t *dir_lock;

	/*if(mod_find_block(mod, stack->addr, &aux_stack->set,
		&aux_stack->way, &aux_stack->tag, &aux_stack->state))
		{
		free(aux_stack);
		return NULL;
	}*/

	int index;
	struct mod_stack_t *stack_array[20] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	stack_array[mod->compute_unit->id] = stack;
	for (int k = 0; k < list_count(mem_system->mod_list); k++)
	{
		mod_in_conflict = list_get(mem_system->mod_list, k);
		if(mod_in_conflict->level != 1 || mod_in_conflict == mod || mod_in_conflict->compute_unit->id == stack->mod->compute_unit->id || mod_in_conflict == stack->mod->compute_unit->scalar_cache)
			continue;

		index = (stack->addr >> mod_in_conflict->log_block_size) % MOD_ACCESS_HASH_TABLE_SIZE;
		for (ret_stack = mod_in_conflict->access_hash_table[index].bucket_list_head; ret_stack;
		ret_stack = ret_stack->bucket_list_next)
		{
			/* Address matches */
			if (ret_stack->addr >> mod_in_conflict->log_block_size == stack->addr >> stack->mod->log_block_size && (ret_stack->event == EV_MOD_NMOESI_LOAD_LOCK || ret_stack->event == EV_MOD_NMOESI_NC_STORE_LOCK)&& ret_stack->waiting_list_event == 0 && stack!=ret_stack)
			{

				/*if(ret_stack->waiting_list_event)
				{
					struct mod_stack_t *master = ret_stack->waiting_list_master;

					if(master->addr >> master->mod->log_block_size == stack->addr >> stack->mod->log_block_size && master != stack && master->waiting_list_event)
					{
						free(aux_stack);
						return master;
					}
					continue;
				}*/

				/*if(ret_stack->waiting_list_head)
				{
					free(aux_stack);
					return ret_stack;
				}*/

				if(ret_stack->access_kind != stack->access_kind){
					continue;
				}

				if(ret_stack->latencias.start < stack->latencias.start )
				{
					free(aux_stack);
					return ret_stack;
				}

				if(ret_stack->latencias.start == stack->latencias.start)
				{
					if(mod_in_conflict->compute_unit->scalar_cache == mod_in_conflict)
						stack_array[ret_stack->mod->compute_unit->id+10] = ret_stack;
					else
						stack_array[ret_stack->mod->compute_unit->id] = ret_stack;
					break;
				}
				/*if((ret_stack->find_and_lock_stack != NULL || (ret_stack->dir_lock && ret_stack->dir_lock->lock)))
				{
					free(aux_stack);
					return ret_stack;
				}*/
			}
		}
	}
	long long mod_priority = asTiming(si_gpu)->cycle;
	long long i = 0;
	ret_stack = NULL;

	while(!ret_stack && i < 20)
	{
		if(stack_array[mod_priority%20] != stack)
			ret_stack = stack_array[mod_priority%20];
		mod_priority++;
		i++;
	}

	free(aux_stack);
	return ret_stack;
}


/* Return the youngest in-flight write older than 'older_than_stack'. If 'older_than_stack'
 * is NULL, return the youngest in-flight write. Return NULL if there is no in-flight write.
 */
struct mod_stack_t *mod_in_flight_write(struct mod_t *mod,
	struct mod_stack_t *older_than_stack)
{
	struct mod_stack_t *stack;

	/* No 'older_than_stack' given, return youngest write */
	if (!older_than_stack)
		return mod->write_access_list_tail;

	/* Search */
	for (stack = older_than_stack->access_list_prev; stack;
		stack = stack->access_list_prev)
		if (stack->access_kind == mod_access_store)
			return stack;

	/* Not found */
	return NULL;
}

struct mod_stack_t *mod_in_flight_write_fran(struct mod_t *mod,
	struct mod_stack_t *older_than_stack)
{
	struct mod_stack_t *stack;

	/* No 'older_than_stack' given, return youngest write */
	if (!older_than_stack)
		return mod->write_access_list_tail;

	/* Look for address */
	int index = (older_than_stack->addr >> mod->log_block_size) % MOD_ACCESS_HASH_TABLE_SIZE;
	for (stack = mod->access_hash_table[index].bucket_list_head; stack;
		stack = stack->bucket_list_next)
	{
		/* This stack is not older than 'older_than_stack' */
		if (stack->id == older_than_stack->id)
			continue;

		/* Address matches */
		if ((stack->waiting_list_event == 0) && (stack->access_kind == mod_access_store)
			&& stack->work_group_id_in_cu == older_than_stack->work_group_id_in_cu /*&& (stack->addr >> mod->log_block_size == older_than_stack->addr >> mod->log_block_size)*/)
			return stack;
	}

	/* Not found */
	return NULL;
}

int mod_serves_address(struct mod_t *mod, unsigned int addr)
{
	/* Address bounds */
	if (mod->range_kind == mod_range_bounds)
		return addr >= mod->range.bounds.low &&
			addr <= mod->range.bounds.high;

	/* Interleaved addresses */
	if (mod->range_kind == mod_range_interleaved)
		return (addr / mod->range.interleaved.div) %
			mod->range.interleaved.mod ==
			mod->range.interleaved.eq;

	/* Invalid */
	panic("%s: invalid range kind", __FUNCTION__);
	return 0;
}


/* Return the low module serving a given address. */
struct mod_t *mod_get_low_mod(struct mod_t *mod, unsigned int addr)
{
	struct mod_t *low_mod;
	struct mod_t *server_mod;

	/* Main memory does not have a low module */
	assert(mod_serves_address(mod, addr));
	if (mod->kind == mod_kind_main_memory)
	{
		assert(!linked_list_count(mod->low_mod_list));
		return NULL;
	}

	/* Check which low module serves address */
	server_mod = NULL;
	LINKED_LIST_FOR_EACH(mod->low_mod_list)
	{
		/* Get new low module */
		low_mod = linked_list_get(mod->low_mod_list);
		if (!mod_serves_address(low_mod, addr))
			continue;

		/* Address served by more than one module */
		if (server_mod)
			fatal("%s: low modules %s and %s both serve address 0x%x",
				mod->name, server_mod->name, low_mod->name, addr);

		/* Assign server */
		server_mod = low_mod;
	}

	/* Error if no low module serves address */
	if (!server_mod)
		fatal("module %s: no lower module serves address 0x%x",
			mod->name, addr);

	/* Return server module */
	return server_mod;
}


int mod_get_retry_latency(struct mod_t *mod)
{
	return random() % mod->latency + mod->latency;
}


/* Check if an access to a module can be coalesced with another access older
 * than 'older_than_stack'. If 'older_than_stack' is NULL, check if it can
 * be coalesced with any in-flight access.
 * If it can, return the access that it would be coalesced with. Otherwise,
 * return NULL. */
struct mod_stack_t *mod_can_coalesce(struct mod_t *mod,
	enum mod_access_kind_t access_kind, unsigned int addr,
	struct mod_stack_t *older_than_stack)
{
	struct mod_stack_t *stack;
	struct mod_stack_t *tail;

	/* For efficiency, first check in the hash table of accesses
	 * whether there is an access in flight to the same block. */
	assert(access_kind);
	if (!mod_in_flight_address(mod, addr, older_than_stack))
		return NULL;

	/* Get youngest access older than 'older_than_stack' */
	tail = older_than_stack ? older_than_stack->access_list_prev :
		mod->access_list_tail;

	/* Coalesce depending on access kind */
	switch (access_kind)
	{

	case mod_access_load:
	{
		for (stack = tail; stack; stack = stack->access_list_prev)
		{
			/* Only coalesce with groups of reads or prefetches at the tail */
			if (stack->access_kind != mod_access_load &&
			    stack->access_kind != mod_access_prefetch)
				return NULL;

			if (stack->addr >> mod->log_block_size ==
				addr >> mod->log_block_size)
				return stack->master_stack ? stack->master_stack : stack;
		}
		break;
	}

	case mod_access_store:
	{
		/* Only coalesce with last access */
		stack = tail;
		if (!stack)
			return NULL;

		/* Only if it is a write */
		if (stack->access_kind != mod_access_store)
			return NULL;

		/* Only if it is an access to the same block */
		if (stack->addr >> mod->log_block_size != addr >> mod->log_block_size)
			return NULL;

		/* Only if previous write has not started yet */
		if (stack->port_locked)
			return NULL;

		/* Coalesce */
		return stack->master_stack ? stack->master_stack : stack;
	}

	case mod_access_nc_store:
	{
		/* Only coalesce with last access */
		stack = tail;
		if (!stack)
			return NULL;

		/* Only if it is a non-coherent write */
		if (stack->access_kind != mod_access_nc_store)
			return NULL;

		/* Only if it is an access to the same block */
		if (stack->addr >> mod->log_block_size != addr >> mod->log_block_size)
			return NULL;

		/* Only if previous write has not started yet */
		if (stack->port_locked)
			return NULL;

		/* Coalesce */
		return stack->master_stack ? stack->master_stack : stack;
	}
	case mod_access_prefetch:
		/* At this point, we know that there is another access (load/store)
		 * to the same block already in flight. Just find and return it.
		 * The caller may abort the prefetch since the block is already
		 * being fetched. */
		for (stack = tail; stack; stack = stack->access_list_prev)
		{
			if (stack->addr >> mod->log_block_size ==
				addr >> mod->log_block_size)
				return stack;
		}
		assert(!"Hash table wrongly reported another access to same block.\n");
		break;


	default:
		panic("%s: invalid access type", __FUNCTION__);
		break;
	}

	/* No access found */
	return NULL;
}

struct mod_stack_t *mod_can_coalesce_fran(struct mod_t *mod,
	enum mod_access_kind_t access_kind, unsigned int addr,
	int *global_mem_witness)
{
	struct mod_stack_t *stack;
	struct mod_stack_t *tail;
	//For efficiency
	int limit = 64;

	/* For efficiency, first check in the hash table of accesses
	 * whether there is an access in flight to the same block. */
	assert(access_kind);
	//if (!mod_in_flight_address(mod, addr, NULL))
	//	return NULL;

	/* Get youngest access older than 'older_than_stack' */
	tail = mod->access_list_tail;

	/* Coalesce depending on access kind */
	if(access_kind == mod_access_load || access_kind == mod_access_nc_load)
	{
		for (stack = tail; stack; stack = stack->access_list_prev)
		{
			/* Only coalesce with groups of reads or prefetches at the tail */
			if (stack->access_kind != mod_access_load)
				continue;

			if (global_mem_witness == stack->witness_ptr && stack->addr >> mod->log_block_size == addr >> mod->log_block_size)
			{
				return stack;
			}

			limit--;
			if(limit <= 0)
				break;
		}
	}

	if(access_kind == mod_access_store || access_kind == mod_access_nc_store)
	{
		for (stack = tail; stack; stack = stack->access_list_prev)
		{
			/* Only coalesce with groups of reads or prefetches at the tail */
			if (stack->access_kind != mod_access_store && stack->access_kind != mod_access_nc_store)
				continue;

			/* Only if previous write has not started yet */
			if (stack->port_locked)
				return NULL;

			if (global_mem_witness == stack->witness_ptr && stack->addr >> mod->log_block_size == addr >> mod->log_block_size)
			{
				return stack;
			}

			limit--;
			if(limit <= 0)
				break;
		}
	}


	/* No access found */
	return NULL;
}


void mod_coalesce(struct mod_t *mod, struct mod_stack_t *master_stack,
	struct mod_stack_t *stack)
{
	/* Debug */
	mem_debug("  %lld %lld 0x%x %s coalesce with %lld\n", esim_time,
		stack->id, stack->addr, mod->name, master_stack->id);

	/* Master stack must not have a parent. We only want one level of
	 * coalesced accesses. */
	assert(!master_stack->master_stack);

	/* Access must have been recorded already, which sets the access
	 * kind to a valid value. */
	assert(stack->access_kind);

	/* Set slave stack as a coalesced access */
	stack->coalesced = 1;
	stack->master_stack = master_stack;
	assert(mod->access_list_coalesced_count <= mod->access_list_count);

	/* Record in-flight coalesced access in module */
	mod->access_list_coalesced_count++;
	//fran
	master_stack->coalesced_count++;
}

struct mod_client_info_t *mod_client_info_create(struct mod_t *mod)
{
	struct mod_client_info_t *client_info;

	/* Create object */
	client_info = repos_create_object(mod->client_info_repos);

	/* Return */
	return client_info;
}

void mod_client_info_free(struct mod_t *mod, struct mod_client_info_t *client_info)
{
	/*FIXME*/
	free(client_info);
	//repos_free_object(mod->client_info_repos, client_info);
}

//FRAN - este metodo sustituye a cache_replace_block()
int mod_replace_block(struct mod_t *mod, int set)
{
	uint8_t is_shared = 0;
	uint32_t way;
	struct dir_entry_t *dir_entry;

	assert(set >= 0 && set < mod->cache->num_sets);

	if (mod->cache->policy == cache_policy_lru ||
		mod->cache->policy == cache_policy_fifo)
	{
		struct cache_block_t *blk = mod->cache->sets[set].way_tail;
		struct dir_t *dir = mod->dir;
		do
		{

			if(!blk->way_prev)
			{
				way = mod->cache->sets[set].way_tail->way;
				break;
			}
			else if(is_shared)
				blk = blk->way_prev;

			is_shared = 0;
			way = blk->way;

			for (int z = 0; z < dir->zsize; z++)
			{
				dir_entry = dir_entry_get(dir, set, way, z);
				for (int i = 0; i < ceil(dir->num_nodes/8.0); i++)
				{
					if (dir_entry->sharer[i] > 0)
					{
						is_shared = 1;
						break;
					}
				}
				if(is_shared)
					break;
			}
		}
		while(is_shared);
		cache_update_waylist(&mod->cache->sets[set], mod->cache->sets[set].way_tail,
			cache_waylist_head);

		return way;
	}

	/* Random replacement */
	assert(mod->cache->policy == cache_policy_random);
	return random() % mod->cache->assoc;
}

unsigned int mod_get_valid_mask(struct mod_t *mod, int set, int way)
{
	if(mod->kind == mod_kind_main_memory)
		return mod->block_size-1;
	else
		return mod->cache->sets[set].blocks[way].valid_mask;
}

bool mod_is_vector_cache(struct mod_t *mod)
{

	if(mod->compute_unit && mod->compute_unit->vector_cache == mod)
		return true;
	else
		return false;
}
