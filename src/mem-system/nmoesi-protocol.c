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
 *  You should have received stack copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <assert.h>
#include <stdbool.h>

#include <lib/esim/esim.h>
#include <lib/esim/trace.h>
#include <lib/util/debug.h>
#include <lib/util/linked-list.h>
#include <lib/util/list.h>
#include <lib/util/string.h>
#include <network/network.h>
#include <network/node.h>
#include <arch/common/arch.h>

#include "cache.h"
#include "directory.h"
#include "mem-system.h"
#include "mod-stack.h"
#include "prefetcher.h"
#include "mshr.h"
//fran
#include <lib/util/misc.h>
#include <lib/util/estadisticas.h>
#include <arch/southern-islands/timing/gpu.h>
#include <arch/southern-islands/timing/compute-unit.h>
#include <lib/util/class.h>
#include <string.h>
#include <dramsim/bindings-c.h>
/* Events */

int EV_MOD_NMOESI_LOAD;
int EV_MOD_NMOESI_LOAD_SEND;
int EV_MOD_NMOESI_LOAD_RECEIVE;
int EV_MOD_NMOESI_LOAD_LOCK;
int EV_MOD_NMOESI_LOAD_LOCK2;
int EV_MOD_NMOESI_LOAD_ACTION;
int EV_MOD_NMOESI_LOAD_MISS;
int EV_MOD_NMOESI_LOAD_UNLOCK;
int EV_MOD_NMOESI_LOAD_FINISH;

int EV_MOD_NMOESI_STORE;
int EV_MOD_NMOESI_STORE_SEND;
int EV_MOD_NMOESI_STORE_RECEIVE;
int EV_MOD_NMOESI_STORE_LOCK;
int EV_MOD_NMOESI_STORE_ACTION;
int EV_MOD_NMOESI_STORE_UNLOCK;
int EV_MOD_NMOESI_STORE_FINISH;

int EV_MOD_NMOESI_PREFETCH;
int EV_MOD_NMOESI_PREFETCH_LOCK;
int EV_MOD_NMOESI_PREFETCH_ACTION;
int EV_MOD_NMOESI_PREFETCH_MISS;
int EV_MOD_NMOESI_PREFETCH_UNLOCK;
int EV_MOD_NMOESI_PREFETCH_FINISH;

int EV_MOD_NMOESI_NC_STORE;
int EV_MOD_NMOESI_NC_STORE_SEND;
int EV_MOD_NMOESI_NC_STORE_RECEIVE;
int EV_MOD_NMOESI_NC_STORE_LOCK;
int EV_MOD_NMOESI_NC_STORE_LOCK2;
int EV_MOD_NMOESI_NC_STORE_WRITEBACK;
int EV_MOD_NMOESI_NC_STORE_ACTION;
int EV_MOD_NMOESI_NC_STORE_MISS;
int EV_MOD_NMOESI_NC_STORE_UNLOCK;
int EV_MOD_NMOESI_NC_STORE_FINISH;

int EV_MOD_NMOESI_FIND_AND_LOCK;
int EV_MOD_NMOESI_FIND_AND_LOCK_PORT;
int EV_MOD_NMOESI_FIND_AND_LOCK_ACTION;
int EV_MOD_NMOESI_FIND_AND_LOCK_FINISH;

int EV_MOD_NMOESI_EVICT_CHECK;
int EV_MOD_NMOESI_EVICT_LOCK_DIR;
int EV_MOD_NMOESI_EVICT;
int EV_MOD_NMOESI_EVICT_INVALID;
int EV_MOD_NMOESI_EVICT_ACTION;
int EV_MOD_NMOESI_EVICT_RECEIVE;
int EV_MOD_NMOESI_EVICT_PROCESS;
int EV_MOD_NMOESI_EVICT_PROCESS_NONCOHERENT;
int EV_MOD_NMOESI_EVICT_REPLY;
int EV_MOD_NMOESI_EVICT_REPLY_RECEIVE;
int EV_MOD_NMOESI_EVICT_FINISH;

int EV_MOD_NMOESI_WRITE_REQUEST;
int EV_MOD_NMOESI_WRITE_REQUEST_RECEIVE;
int EV_MOD_NMOESI_WRITE_REQUEST_ACTION;
int EV_MOD_NMOESI_WRITE_REQUEST_EXCLUSIVE;
int EV_MOD_NMOESI_WRITE_REQUEST_UPDOWN;
int EV_MOD_NMOESI_WRITE_REQUEST_UPDOWN_FINISH;
int EV_MOD_NMOESI_WRITE_REQUEST_DOWNUP;
int EV_MOD_NMOESI_WRITE_REQUEST_DOWNUP_FINISH;
int EV_MOD_NMOESI_WRITE_REQUEST_REPLY;
int EV_MOD_NMOESI_WRITE_REQUEST_FINISH;

int EV_MOD_NMOESI_READ_REQUEST;
int EV_MOD_NMOESI_READ_REQUEST_RECEIVE;
int EV_MOD_NMOESI_READ_REQUEST_ACTION;
int EV_MOD_NMOESI_READ_REQUEST_UPDOWN;
int EV_MOD_NMOESI_READ_REQUEST_UPDOWN_MISS;
int EV_MOD_NMOESI_READ_REQUEST_UPDOWN_FINISH;
int EV_MOD_NMOESI_READ_REQUEST_DOWNUP;
int EV_MOD_NMOESI_READ_REQUEST_DOWNUP_WAIT_FOR_REQS;
int EV_MOD_NMOESI_READ_REQUEST_DOWNUP_FINISH;
int EV_MOD_NMOESI_READ_REQUEST_REPLY;
int EV_MOD_NMOESI_READ_REQUEST_FINISH;

int EV_MOD_NMOESI_INVALIDATE;
int EV_MOD_NMOESI_INVALIDATE_FINISH;

int EV_MOD_NMOESI_PEER_SEND;
int EV_MOD_NMOESI_PEER_RECEIVE;
int EV_MOD_NMOESI_PEER_REPLY;
int EV_MOD_NMOESI_PEER_FINISH;

int EV_MOD_NMOESI_MESSAGE;
int EV_MOD_NMOESI_MESSAGE_RECEIVE;
int EV_MOD_NMOESI_MESSAGE_ACTION;
int EV_MOD_NMOESI_MESSAGE_REPLY;
int EV_MOD_NMOESI_MESSAGE_FINISH;


//long long ciclo = 0;
long long t1000k_ciclo = 0;
long long t1000k_primer_acceso = 0;
int t1000k = 0;
long long tiempo_medio = 0;
long long ciclo_acceso = 0;
int acumulado = 0;
bool AVOID_RETRIES = false;

long long ret_ciclo = 0;

/* NMOESI Protocol */

void mod_handler_nmoesi_load(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *new_stack;

	struct mod_t *target_mod = stack->target_mod;
	struct net_t *net;
	struct net_node_t *src_node;
	struct net_node_t *dst_node;
	int return_event;

	if (event == EV_MOD_NMOESI_LOAD)
	{
		struct mod_stack_t *master_stack;

		mem_debug("%lld %lld 0x%x %s load\n", esim_time, stack->id,
			stack->addr, target_mod->name);
		mem_trace("mem.new_access name=\"A-%lld\" type=\"load\" "
			"state=\"%s:load\" addr=0x%x\n",
			stack->id, target_mod->name, stack->addr);

		/* Record access */
		mod_access_start(target_mod, stack, mod_access_load);
		//add_access(mod->level);
		/* Coalesce access */
		stack->origin = 1;
		master_stack = mod_can_coalesce(target_mod, mod_access_load, stack->addr, stack);
		if ((!flag_coalesce_gpu_enabled && master_stack) || ((stack->target_mod->compute_unit->scalar_cache == target_mod) && master_stack))
		{
			target_mod->hits_aux++;
			target_mod->delayed_read_hit++;

			target_mod->reads++;
			mod_coalesce(target_mod, master_stack, stack);
			mod_stack_wait_in_stack(stack, master_stack, EV_MOD_NMOESI_LOAD_FINISH);

			add_coalesce(target_mod->level);
			add_coalesce_load(target_mod->level);

			return;
		}

		if(stack->client_info && stack->client_info->arch)
			stack->latencias.start = stack->client_info->arch->timing->cycle;

		add_access(target_mod->level);
	 	target_mod->loads++;
		if(target_mod == target_mod->compute_unit->vector_cache)
			target_mod->compute_unit->compute_device->interval_statistics->vcache_load_start++;
  	else
			target_mod->compute_unit->compute_device->interval_statistics->scache_load_start++;
		/* Next event */
		stack->event = EV_MOD_NMOESI_LOAD_LOCK;
		esim_schedule_mod_stack_event(stack, 0);
		//esim_schedule_event(EV_MOD_NMOESI_LOAD_LOCK, stack, 0);
		return;
	}

	if(event == EV_MOD_NMOESI_LOAD_SEND)
	{
		int msg_size;
		if(stack->request_dir == mod_request_up_down)
		{

			//new_stack = mod_stack_create(stack->id, mod_get_low_mod(mod, stack->addr), stack->addr, EV_MOD_NMOESI_LOAD_FINISH, stack);
			stack->request_dir = mod_request_up_down;
			msg_size = 8;
			net = target_mod->high_net;
			src_node = stack->ret_stack->target_mod->low_net_node;
			dst_node = target_mod->high_net_node;
			return_event = EV_MOD_NMOESI_LOAD_RECEIVE;
			stack->msg = net_try_send_ev(net, src_node, dst_node, msg_size,
			return_event, stack, event, stack);
		}
		else if(stack->request_dir == mod_request_down_up)
		{
			msg_size = 72;
			net = target_mod->low_net;
            src_node = mod_get_low_mod(target_mod, stack->addr)->high_net_node;
            dst_node = target_mod->low_net_node;
			return_event = EV_MOD_NMOESI_LOAD_RECEIVE;
			stack->msg = net_try_send_ev(net, src_node, dst_node, msg_size,
			return_event, stack, event, stack);

		}
		else
		{
			fatal("Invalid request_dir in EV_MOD_NMOESI_LOAD_SEND");
		}
		return;
	}
	if(event == EV_MOD_NMOESI_LOAD_RECEIVE)
	{
		if(stack->request_dir == mod_request_up_down)
    {
			net_receive(target_mod->high_net, target_mod->high_net_node, stack->msg);
      esim_schedule_event(EV_MOD_NMOESI_LOAD, stack, 0);
    }
    else if(stack->request_dir == mod_request_down_up)
    {
			net_receive(target_mod->low_net, target_mod->low_net_node, stack->msg);
			esim_schedule_event(EV_MOD_NMOESI_LOAD_FINISH, stack, 0);
    }
		else
    {
      fatal("Invalid request_dir in EV_MOD_NMOESI_LOAD_RECEIVE");
    }

		return;

	}

	if (event == EV_MOD_NMOESI_LOAD_LOCK)
	{
		struct mod_stack_t *older_stack;

		mem_debug("  %lld %lld 0x%x %s load lock\n", esim_time, stack->id,
			stack->addr, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:load_lock\"\n",
			stack->id, target_mod->name);


		if(stack->latencias.start == 0)
			if(stack->client_info && stack->client_info->arch)
				stack->latencias.start = stack->client_info->arch->timing->cycle;

		/* If there is any older write, wait for it */
		older_stack = mod_in_flight_write(target_mod, stack);
		if (older_stack)
		{
			mem_debug("    %lld wait for write %lld\n",
				stack->id, older_stack->id);
			mod_stack_wait_in_stack(stack, older_stack, EV_MOD_NMOESI_LOAD_LOCK);
			return;
		}

		/* If there is any older access to the same address that this access could not
		 * be coalesced with, wait for it. */
		older_stack = mod_in_flight_address(target_mod, stack->addr, stack);
		if (older_stack)
		{
			mem_debug("    %lld wait for access %lld\n",
				stack->id, older_stack->id);
			mod_stack_wait_in_stack(stack, older_stack, EV_MOD_NMOESI_LOAD_LOCK);
			return;
		}
                
                /*older_stack = mod_in_flight_address2(target_mod, stack->addr);
		if (older_stack)
		{
			mem_debug("    %lld wait for access %lld\n",
				stack->id, older_stack->id);
			mod_stack_wait_in_stack(stack, older_stack, EV_MOD_NMOESI_LOAD_UNLOCK);
			return;
		}*/
                
                stack->inflight = true;
		stack->event = EV_MOD_NMOESI_LOAD_LOCK2;
		esim_schedule_mod_stack_event(stack, 0);
		//esim_schedule_event(EV_MOD_NMOESI_FIND_AND_LOCK2, new_stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_LOAD_LOCK2)
	{
		//struct mod_stack_t *older_stack;

		mem_debug("  %lld %lld 0x%x %s load lock2\n", esim_time, stack->id,
			stack->addr, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:load_lock2\"\n",
			stack->id, target_mod->name);


		/*if(AVOID_RETRIES)
		{
			older_stack = mod_global_in_flight_address(target_mod, stack);
			if (older_stack)
			{
				mem_debug("    %lld wait for avoid retry %lld\n",
					stack->id, older_stack->id);
				mod_stack_wait_in_stack(stack, older_stack, EV_MOD_NMOESI_LOAD_LOCK);
				return;
			}
		}*/

		if(stack->client_info && stack->client_info->arch){
			stack->latencias.queue = stack->client_info->arch->timing->cycle - stack->latencias.start;
		}
		/* Call find and lock */

		//new_stack->uop = stack->uop;
		stack->blocking = 1;
                stack->uncacheable = false;
                stack->allow_cache_by_passing = true;
		stack->read = 1;
                stack->eviction = 0;
		stack->event = EV_MOD_NMOESI_FIND_AND_LOCK;
		stack->find_and_lock_return_event = EV_MOD_NMOESI_LOAD_ACTION;
                esim_schedule_mod_stack_event(stack, 0);
                   
		return;
	}

	if (event == EV_MOD_NMOESI_LOAD_ACTION)
	{
		int retry_lat;
                    
                if(stack->waiting_list_master)
                {
                    fatal("???");
                    return;
                }
		mem_debug("  %lld %lld 0x%x %s load action\n", esim_time, stack->id,
			stack->addr, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:load_action\"\n",
			stack->id, target_mod->name);

		/* Error locking */
		if (stack->err)
		{
			target_mod->read_retries++;
			add_retry(stack,load_action_retry);
			retry_lat = mod_get_retry_latency(target_mod);
			if(stack->client_info && stack->client_info->arch){
				stack->latencias.retry += stack->client_info->arch->timing->cycle - stack->latencias.start + retry_lat;
			}
			stack->latencias.start = 0;
			if (stack->mshr_locked != 0)
			{
				mshr_unlock(target_mod, stack);
				stack->mshr_locked = 0;
			}

			mem_debug("    lock error, retrying in %d cycles\n", retry_lat);
			stack->retry = 1;
			stack->err = 0;
			stack->event = EV_MOD_NMOESI_LOAD_LOCK2;
			esim_schedule_mod_stack_event(stack, retry_lat);
			//esim_schedule_event(EV_MOD_NMOESI_LOAD_LOCK, stack, retry_lat);
			return;
		}
		assert(stack->mshr_entry || stack->dir_entry->state);

		/* Hit */
		if (stack->dir_entry->state)
		{
			stack->event = EV_MOD_NMOESI_LOAD_UNLOCK;
			esim_schedule_mod_stack_event(stack, 0);
			//esim_schedule_event(EV_MOD_NMOESI_LOAD_UNLOCK, stack, 0);
			estadisticas(1, 0);
			target_mod->hits_aux++;
			/* The prefetcher may have prefetched this earlier and hence
			 * this is a hit now. Let the prefetcher know of this hit
			 * since without the prefetcher, this may have been a miss. */
			//prefetcher_access_hit(stack, mod);

			return;
		}

		/* Miss */

		estadisticas(0, 0);
                if(super_stack_enabled == 1){
                    //cuantos accesos debo generar?
                    struct mod_stack_t *super_stack = mod_stack_create_super_stack(mod_get_low_mod(target_mod, stack->tag), EV_MOD_NMOESI_LOAD_MISS, stack);
                    //falta acabar esto
                    //super_stack->stack_size = 8;
                    super_stack->return_mod = target_mod;
                    super_stack->addr = stack->addr;
                    super_stack->request_dir = mod_request_up_down;
                    super_stack->wavefront = stack->wavefront;
                    super_stack->uop = stack->uop;
                    super_stack->retry = stack->retry;
                    super_stack->event = EV_MOD_NMOESI_READ_REQUEST;
                    esim_schedule_mod_stack_event(super_stack, 0);
                    
                }else{
                    new_stack = mod_stack_create(stack->id, mod_get_low_mod(target_mod, stack->tag), stack->tag,
                            EV_MOD_NMOESI_LOAD_MISS, stack);
                    //new_stack->peer = mod_stack_set_peer(mod, stack->state);
                    //new_stack->target_mod = mod_get_low_mod(target_mod, stack->tag);
                    new_stack->return_mod = target_mod;
                    new_stack->request_dir = mod_request_up_down;
                    new_stack->wavefront = stack->wavefront;
                    new_stack->uop = stack->uop;
                    new_stack->retry = stack->retry;
                    new_stack->event = EV_MOD_NMOESI_READ_REQUEST;
                    new_stack->allow_cache_by_passing = true;
                    new_stack->stack_size = 8;
                    esim_schedule_mod_stack_event(new_stack, 0);
                    //esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST, new_stack, 0);

                    /* The prefetcher may be interested in this miss */
                    //prefetcher_access_miss(stack, mod);
                }
		return;
	}

	if (event == EV_MOD_NMOESI_LOAD_MISS)
	{
		int retry_lat;

		mem_debug("  %lld %lld 0x%x %s load miss\n", esim_time, stack->id,
			stack->addr, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:load_miss\"\n",
			stack->id, target_mod->name);

		/* Error on read request. Unlock block and retry load. */
		if (stack->err)
		{
			target_mod->read_retries++;
			retry_lat = mod_get_retry_latency(target_mod);

			add_retry(stack,load_miss_retry);
                        
			if(stack->client_info && stack->client_info->arch){
                            stack->latencias.retry += stack->client_info->arch->timing->cycle - stack->latencias.start + retry_lat;
			}
			stack->latencias.start = 0;

			if (stack->mshr_locked != 0)
			{
				mshr_unlock(target_mod, stack);
				stack->mshr_locked = 0;
			}

                        if(stack->dir_entry->dir_lock->stack == stack)
                            dir_entry_unlock(stack->dir_entry);
                        else if(target_mod->cache->extra_dir_structure_type == extra_dir_per_cache_line)
                            assert(stack->uncacheable);
                        
                        mem_debug("    lock error, retrying in %d cycles\n", retry_lat);
		
                        stack->retry = 1;
                        stack->err = 0;

			stack->event = EV_MOD_NMOESI_LOAD_LOCK2;
			esim_schedule_mod_stack_event(stack, retry_lat);
			//esim_schedule_event(EV_MOD_NMOESI_LOAD_LOCK, stack, retry_lat);
			return;
		}
                
		if(stack->client_info && stack->client_info->arch)
                {
			stack->latencias.miss = stack->client_info->arch->timing->cycle - stack->latencias.start - stack->latencias.queue - stack->latencias.lock_mshr - stack->latencias.lock_dir - stack->latencias.eviction;
		}


		/* Set block state to excl/shared depending on return var 'shared'.
		 * Also set the tag of the block. */
                if(!stack->uncacheable)
                {
                    //cache_set_block(target_mod->cache, stack->set, stack->way, stack->tag,
		//	stack->shared ? cache_block_shared : cache_block_exclusive);
                    cache_set_block_new(target_mod->cache, stack, stack->shared ? cache_block_shared : cache_block_exclusive);
                }
		/* Continue */
		stack->event = EV_MOD_NMOESI_LOAD_UNLOCK;
		esim_schedule_mod_stack_event(stack, 0);
		//esim_schedule_event(EV_MOD_NMOESI_LOAD_UNLOCK, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_LOAD_UNLOCK)
	{
		mem_debug("  %lld %lld 0x%x %s load unlock\n", esim_time, stack->id,
			stack->addr, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:load_unlock\"\n",
			stack->id, target_mod->name);

		if (stack->mshr_locked != 0)
		{
			mshr_unlock(target_mod,stack);
			stack->mshr_locked = 0;
		}

		/* Unlock directory entry */
                if(stack->dir_entry->dir_lock->stack == stack)
                    dir_entry_unlock(stack->dir_entry);
                else if(target_mod->cache->extra_dir_structure_type == extra_dir_per_cache_line)
                    assert(stack->uncacheable);

		/* Impose the access latency before continuing */
                stack->reply_size += target_mod->block_size;
                        
		stack->event = EV_MOD_NMOESI_LOAD_FINISH;
		esim_schedule_mod_stack_event(stack, target_mod->latency);
		//esim_schedule_event(EV_MOD_NMOESI_LOAD_FINISH, stack, mod->latency);
		return;
	}

	if (event == EV_MOD_NMOESI_LOAD_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s load finish\n", esim_time, stack->id,
			stack->addr, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:load_finish\"\n",
			stack->id, target_mod->name);
		mem_trace("mem.end_access name=\"A-%lld\"\n",
			stack->id);

		long long ciclo = 0;

		if(target_mod->level == 1)
		{
			if(stack->client_info && stack->client_info->arch)
				ciclo = stack->client_info->arch->timing->cycle;

			if(esim_time > stack->tiempo_acceso)
			{
				mem_load_finish(ciclo - stack->tiempo_acceso);
				estadis[0].media_latencia += (long long) (ciclo - stack->tiempo_acceso);
				estadis[0].media_latencia_contador++;
			}

			//load_finished++;
			/*if( (load_finished % 10000) == 0 )
			{
				load_finished = 0;
				fran_debug_t1000k("%lld\n", ciclo);
				fran_debug_hitRatio("%lld\n",ciclo - ret_ciclo);
				ret_ciclo = ciclo;
			}*/
		}

		if(!stack->coalesced)
		{

			if(target_mod == target_mod->compute_unit->vector_cache)
				target_mod->compute_unit->compute_device->interval_statistics->vcache_load_finish++;
			else
				target_mod->compute_unit->compute_device->interval_statistics->scache_load_finish++;

			if(stack->retry)
			{
				target_mod->compute_unit->compute_device->interval_statistics->cache_retry_lat += stack->latencias.retry;
				target_mod->compute_unit->compute_device->interval_statistics->cache_retry_cont++;
			}

			accu_retry_time_lost(stack);
			if(stack->client_info && stack->client_info->arch){
				stack->latencias.finish = stack->client_info->arch->timing->cycle - stack->latencias.start - stack->latencias.queue - stack->latencias.lock_mshr - stack->latencias.lock_dir - stack->latencias.eviction - stack->latencias.miss;
			}

			add_latencias_load(stack);

			if(stack->client_info && stack->client_info->arch && strcmp(stack->client_info->arch->name, "SouthernIslands") == 0)
			{
				copy_latencies_to_wavefront(&(stack->latencias),stack->wavefront);
			}
		}

		if(stack->hit && !(stack->latencias.queue + stack->latencias.lock_mshr + stack->latencias.lock_dir + stack->latencias.eviction + stack->latencias.miss))
		{
			add_CoalesceHit(target_mod->level);
		}
		else
		{
			add_CoalesceMiss(target_mod->level);
		}

		//if(stack->ret_stack)
		//	stack->ret_stack->state = stack->state;

		/* Increment witness variable */
		if (stack->witness_ptr)
			(*stack->witness_ptr)++;

		if((*stack->witness_ptr) == 0 && stack->client_info)
			stack->uop->mem_access_finish_cycle = stack->client_info->arch->timing->cycle;

		/* Return event queue element into event queue */
		if (stack->event_queue && stack->event_queue_item)
			linked_list_add(stack->event_queue, stack->event_queue_item);

		/* Free the mod_client_info object, if any */
		if (stack->client_info)
			mod_client_info_free(target_mod, stack->client_info);

		/* Finish access */
		mod_access_finish(target_mod, stack);

		/* Return */
		mod_stack_return(stack);
		return;
	}

	abort();
}


void mod_handler_nmoesi_store(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *new_stack;

	struct mod_t *target_mod = stack->target_mod;


	if (event == EV_MOD_NMOESI_STORE)
	{
		struct mod_stack_t *master_stack;

		mem_debug("%lld %lld 0x%x %s store\n", esim_time, stack->id,
			stack->addr, target_mod->name);
		mem_trace("mem.new_access name=\"A-%lld\" type=\"store\" "
			"state=\"%s:store\" addr=0x%x\n",
			stack->id, target_mod->name, stack->addr);
		abort();
		/* Record access */
		mod_access_start(target_mod, stack, mod_access_store);

		add_access(1);

		/* Coalesce access */
		master_stack = mod_can_coalesce(target_mod, mod_access_store, stack->addr, stack);
		if (!flag_coalesce_gpu_enabled && master_stack)
		{
			target_mod->writes++;
			mod_coalesce(target_mod, master_stack, stack);
			mod_stack_wait_in_stack(stack, master_stack, EV_MOD_NMOESI_STORE_FINISH);

			add_coalesce(target_mod->level);
			add_coalesce_store(target_mod->level);

			/* Increment witness variable */
			if (stack->witness_ptr)
				(*stack->witness_ptr)++;

			return;
		}

		target_mod->compute_unit->compute_device->interval_statistics->vcache_write_start++;

		/* Continue */
		esim_schedule_event(EV_MOD_NMOESI_STORE_LOCK, stack, 0);
		return;
	}


	if (event == EV_MOD_NMOESI_STORE_LOCK)
	{
		struct mod_stack_t *older_stack;

		mem_debug("  %lld %lld 0x%x %s store lock\n", esim_time, stack->id,
			stack->addr, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:store_lock\"\n",
			stack->id, target_mod->name);

		/* If there is any older access, wait for it */
		older_stack = stack->access_list_prev;
		if (older_stack)
		{
			mem_debug("    %lld wait for access %lld\n",
				stack->id, older_stack->id);
			mod_stack_wait_in_stack(stack, older_stack, EV_MOD_NMOESI_STORE_LOCK);
			return;
		}

		/* Call find and lock */
		/*
		new_stack = mod_stack_create(stack->id, mod, stack->addr,
			EV_MOD_NMOESI_STORE_ACTION, stack);
		new_stack->wavefront = stack->wavefront;
		new_stack->uop = stack->uop;
		new_stack->blocking = 1;
		new_stack->write = 1;
		new_stack->retry = stack->retry;
		new_stack->witness_ptr = stack->witness_ptr;
		*/
		stack->blocking = 1;
		stack->write = 1;
		stack->event = EV_MOD_NMOESI_FIND_AND_LOCK;
		stack->find_and_lock_return_event = EV_MOD_NMOESI_STORE_ACTION;
		esim_schedule_mod_stack_event(stack, 0);
		//esim_schedule_event(EV_MOD_NMOESI_FIND_AND_LOCK, new_stack, 0);

		/* Set witness variable to NULL so that retries from the same
		 * stack do not increment it multiple times */
		stack->witness_ptr = NULL;

		return;
	}

	if (event == EV_MOD_NMOESI_STORE_ACTION)
	{
		int retry_lat;

		mem_debug("  %lld %lld 0x%x %s store action\n", esim_time, stack->id,
			stack->addr, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:store_action\"\n",
			stack->id, target_mod->name);

		/* Error locking */
		if (stack->err)
		{
			target_mod->write_retries++;
			retry_lat = mod_get_retry_latency(target_mod);
			mem_debug("    lock error, retrying in %d cycles\n", retry_lat);
			stack->retry = 1;
			esim_schedule_event(EV_MOD_NMOESI_STORE_LOCK, stack, retry_lat);
			return;
		}

		/* Hit - state=M/E */
		if (stack->dir_entry->state == cache_block_modified ||
			stack->dir_entry->state == cache_block_exclusive)
		{
			esim_schedule_event(EV_MOD_NMOESI_STORE_UNLOCK, stack, 0);

			/* The prefetcher may have prefetched this earlier and hence
			 * this is a hit now. Let the prefetcher know of this hit
			 * since without the prefetcher, this may have been a miss. */
			//prefetcher_access_hit(stack, mod);

			return;
		}

		/* Miss - state=O/S/I/N */
		new_stack = mod_stack_create(stack->id, mod_get_low_mod(target_mod, stack->tag), stack->tag,
			EV_MOD_NMOESI_STORE_UNLOCK, stack);
		new_stack->wavefront = stack->wavefront;
		new_stack->retry = stack->retry;
		new_stack->uop = stack->uop;
		//new_stack->peer = mod_stack_set_peer(mod, stack->state);
		new_stack->return_mod = target_mod;
		new_stack->request_dir = mod_request_up_down;
		esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST, new_stack, 0);

		/* The prefetcher may be interested in this miss */
		//prefetcher_access_miss(stack, mod);

		return;
	}

	if (event == EV_MOD_NMOESI_STORE_UNLOCK)
	{
		int retry_lat;

		mem_debug("  %lld %lld 0x%x %s store unlock\n", esim_time, stack->id,
			stack->addr, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:store_unlock\"\n",
			stack->id, target_mod->name);

		/* Error in write request, unlock block and retry store. */
		if (stack->err)
		{
			target_mod->write_retries++;
			retry_lat = mod_get_retry_latency(target_mod);
			dir_entry_unlock(stack->dir_entry);
			mem_debug("    lock error, retrying in %d cycles\n", retry_lat);
			stack->retry = 1;
			esim_schedule_event(EV_MOD_NMOESI_STORE_LOCK, stack, retry_lat);
			return;
		}

		if (stack->mshr_locked != 0)
		{
			mshr_unlock(target_mod,stack);
			stack->mshr_locked = 0;
		}

		/* Update tag/state and unlock */
		//cache_set_block(target_mod->cache, stack->set, stack->way,
		//	stack->tag, cache_block_modified);
                cache_set_block_new(target_mod->cache, stack, cache_block_modified);
                
		dir_entry_unlock(stack->dir_entry);

		/* Impose the access latency before continuing */
		esim_schedule_event(EV_MOD_NMOESI_STORE_FINISH, stack,
			target_mod->latency);
		return;
	}

	if (event == EV_MOD_NMOESI_STORE_FINISH)
	{
		mem_debug("%lld %lld 0x%x %s store finish\n", esim_time, stack->id,
			stack->addr, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:store_finish\"\n",
			stack->id, target_mod->name);
		mem_trace("mem.end_access name=\"A-%lld\"\n",
			stack->id);

		/* Return event queue element into event queue */
		if (stack->event_queue && stack->event_queue_item)
			linked_list_add(stack->event_queue, stack->event_queue_item);

		/* Free the mod_client_info object, if any */
		if (stack->client_info)
			mod_client_info_free(target_mod, stack->client_info);

		/* Finish access */
		mod_access_finish(target_mod, stack);

		if(stack->coalesced == 0)
			target_mod->compute_unit->compute_device->interval_statistics->vcache_write_start++;

		/* Return */
		mod_stack_return(stack);
		return;
	}

	abort();
}

void mod_handler_nmoesi_nc_store(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *new_stack;

	struct mod_t *target_mod = stack->target_mod;

  struct net_t *net;
  struct net_node_t *src_node;
  struct net_node_t *dst_node;
  int return_event;

	if (event == EV_MOD_NMOESI_NC_STORE)
	{
		struct mod_stack_t *master_stack;

		mem_debug("%lld %lld 0x%x %s nc store\n", esim_time, stack->id,
			stack->addr, target_mod->name);
		mem_trace("mem.new_access name=\"A-%lld\" type=\"nc_store\" "
			"state=\"%s:nc store\" addr=0x%x\n", stack->id, target_mod->name, stack->addr);

		/* Record access */
		mod_access_start(target_mod, stack, mod_access_nc_store);

		/* Increment witness variable */
		if (flag_no_blocking_store && stack->witness_ptr)
		{
			(*stack->witness_ptr)++;
			stack->witness_ptr = NULL;
		}
		if(stack->client_info && stack->client_info->arch)
			stack->latencias.start = stack->client_info->arch->timing->cycle;

		stack->origin = 1;

		/* Coalesce access */
		master_stack = mod_can_coalesce(target_mod, mod_access_nc_store, stack->addr, stack);
		if (!flag_coalesce_gpu_enabled && master_stack)
		{
			target_mod->nc_writes++;
			mod_coalesce(target_mod, master_stack, stack);
			mod_stack_wait_in_stack(stack, master_stack, EV_MOD_NMOESI_NC_STORE_FINISH);

			add_coalesce(target_mod->level);
			add_coalesce_store(target_mod->level);
			return;
		}

		add_access(target_mod->level);

		/* Next event */
		stack->event = EV_MOD_NMOESI_NC_STORE_LOCK;
		esim_schedule_mod_stack_event(stack, 0);
		//esim_schedule_event(EV_MOD_NMOESI_NC_STORE_LOCK, stack, 0);
		return;
	}

	if(event == EV_MOD_NMOESI_NC_STORE_SEND)
    {
    	int msg_size;
        if(stack->request_dir == mod_request_up_down)
        {
        	stack->request_dir = mod_request_up_down;
            msg_size = 72;
            net = target_mod->high_net;
            src_node = stack->ret_stack->target_mod->low_net_node;
            dst_node = target_mod->high_net_node;
            return_event = EV_MOD_NMOESI_NC_STORE_RECEIVE;
            stack->msg = net_try_send_ev(net, src_node, dst_node, msg_size, return_event, stack, event, stack);
        }
        else if(stack->request_dir == mod_request_down_up)
        {
        	msg_size = 8;
            net = target_mod->low_net;
            src_node = mod_get_low_mod(target_mod, stack->addr)->high_net_node;
            dst_node = target_mod->low_net_node;
            return_event = EV_MOD_NMOESI_NC_STORE_RECEIVE;
            stack->msg = net_try_send_ev(net, src_node, dst_node, msg_size, return_event, stack, event, stack);
        }
        else
        {
	        fatal("Invalid request_dir in EV_MOD_NMOESI_NC_STORE_SEND");
        }
        return;
    }

    if(event == EV_MOD_NMOESI_NC_STORE_RECEIVE)
    {
    	if(stack->request_dir == mod_request_up_down)
    	{
        	net_receive(target_mod->high_net, target_mod->high_net_node, stack->msg);
            esim_schedule_event(EV_MOD_NMOESI_NC_STORE, stack, 0);
        }
        else if(stack->request_dir == mod_request_down_up)
        {
            net_receive(target_mod->low_net, target_mod->low_net_node, stack->msg);
        	esim_schedule_event(EV_MOD_NMOESI_NC_STORE_FINISH, stack, 0);
        }
        else
        {
        	fatal("Invalid request_dir in EV_MOD_NMOESI_NC_STORE_RECEIVE");
        }
    	return;
    }


	if (event == EV_MOD_NMOESI_NC_STORE_LOCK)
	{
		struct mod_stack_t *older_stack;

		mem_debug("  %lld %lld 0x%x %s nc store lock\n", esim_time, stack->id,
			stack->addr, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:nc_store_lock\"\n",
			stack->id, target_mod->name);

		if(stack->latencias.start == 0)
			if(stack->client_info && stack->client_info->arch)
				stack->latencias.start = stack->client_info->arch->timing->cycle;

		/* If there is any older write, wait for it */
		older_stack = mod_in_flight_write(target_mod, stack);
		if (older_stack)
		{
			mem_debug("    %lld wait for write %lld\n", stack->id, older_stack->id);
			mod_stack_wait_in_stack(stack, older_stack, EV_MOD_NMOESI_NC_STORE_LOCK);
			return;
		}

		/* If there is any older access to the same address that this access could not
		 * be coalesced with, wait for it. */
		older_stack = mod_in_flight_address(target_mod, stack->addr, stack);
		if (older_stack)
		{
			mem_debug("    %lld wait for write %lld\n", stack->id, older_stack->id);
			mod_stack_wait_in_stack(stack, older_stack, EV_MOD_NMOESI_NC_STORE_LOCK);
			return;
		}
                
                /*older_stack = mod_in_flight_address2(target_mod, stack->addr);
		if (older_stack)
		{
			mem_debug("    %lld wait for access %lld\n",
				stack->id, older_stack->id);
			mod_stack_wait_in_stack(stack, older_stack, EV_MOD_NMOESI_LOAD_UNLOCK);
			return;
		}*/
                
                
                stack->inflight = true;
		stack->event = EV_MOD_NMOESI_NC_STORE_LOCK2;
		esim_schedule_mod_stack_event(stack, 0);
		//esim_schedule_event(EV_MOD_NMOESI_NC_STORE_LOCK2, stack, 0);
		return;
		}

		if (event == EV_MOD_NMOESI_NC_STORE_LOCK2)
		{
		//	struct mod_stack_t *older_stack;

			mem_debug("  %lld %lld 0x%x %s nc store lock2\n", esim_time, stack->id,
				stack->addr, target_mod->name);
			mem_trace("mem.access name=\"A-%lld\" state=\"%s:nc_store_lock2\"\n",
				stack->id, target_mod->name);

		/*if(AVOID_RETRIES)
		{
			older_stack = mod_global_in_flight_address(target_mod, stack);
			if (older_stack)
			{
				mem_debug("    %lld wait for avoid retry %lld\n",
					stack->id, older_stack->id);
				mod_stack_wait_in_stack(stack, older_stack, EV_MOD_NMOESI_NC_STORE_LOCK);
				return;
			}
		}*/

		if(stack->client_info && stack->client_info->arch){
			stack->latencias.queue = stack->client_info->arch->timing->cycle - stack->latencias.start;
		}

		/* Call find and lock */
		/*
		new_stack = mod_stack_create(stack->id, mod, stack->addr,
			EV_MOD_NMOESI_NC_STORE_WRITEBACK, stack);
		new_stack->wavefront = stack->wavefront;
		new_stack->uop = stack->uop;
		new_stack->blocking = 1;
		new_stack->nc_write = 1;
		new_stack->retry = stack->retry;
		*/
		stack->blocking = 1;
		stack->nc_write = 1;
                stack->eviction = 0;
                stack->uncacheable = false;
                stack->allow_cache_by_passing = false;
		stack->err = 0;
		stack->event = EV_MOD_NMOESI_FIND_AND_LOCK;
		stack->find_and_lock_return_event = EV_MOD_NMOESI_NC_STORE_WRITEBACK;
                //if(!uop_cache_port || stack->uop->accesses_in_dir == 0)
                //{
                    esim_schedule_mod_stack_event(stack, 0);
                /*    stack->waiting_dir_access = 0;
                    stack->uop->accesses_in_dir++;
                }else{
                    stack->waiting_dir_access = 1; 
                }*/
                
                
		//esim_schedule_event(EV_MOD_NMOESI_FIND_AND_LOCK, new_stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_NC_STORE_WRITEBACK)
	{
		int retry_lat;

		mem_debug("  %lld %lld 0x%x %s nc store writeback\n", esim_time, stack->id,
			stack->addr, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:nc_store_writeback\"\n",
			stack->id, target_mod->name);

		/* Error locking */
		if (stack->err)
		{
			target_mod->nc_write_retries++;
			add_retry(stack,nc_store_writeback_retry);

			retry_lat = mod_get_retry_latency(target_mod);

			stack->latencias.retry += stack->client_info->arch->timing->cycle - stack->latencias.start + retry_lat;
			stack->latencias.start = 0;

			if (stack->mshr_locked != 0)
			{
				mshr_unlock(target_mod, stack);
				stack->mshr_locked = 0;
			}

			mem_debug("    lock error, retrying in %d cycles\n", retry_lat);
			stack->retry = 1;

			stack->event = EV_MOD_NMOESI_NC_STORE_LOCK;
			esim_schedule_mod_stack_event(stack, retry_lat);
			//esim_schedule_event(EV_MOD_NMOESI_NC_STORE_LOCK, stack, retry_lat);
			return;
		}

		/* If the block has modified data, evict it so that the lower-level cache will
		 * have the latest copy */
		if (stack->dir_entry->state == cache_block_modified || stack->dir_entry->state == cache_block_owned)
		{
			stack->eviction = 1;
			new_stack = mod_stack_create(stack->id, mod_get_low_mod(target_mod, stack->tag), 0,
				EV_MOD_NMOESI_NC_STORE_ACTION, stack);
			new_stack->return_mod = target_mod;
			new_stack->wavefront = stack->wavefront;
			new_stack->retry = stack->retry;
			new_stack->uop = stack->uop;
			new_stack->set = stack->set;
			new_stack->way = stack->way;
                        new_stack->dir_entry = stack->dir_entry;
                        assert(new_stack->set == stack->dir_entry->set && new_stack->way == stack->dir_entry->way);
			new_stack->event = EV_MOD_NMOESI_EVICT;
			esim_schedule_mod_stack_event(new_stack, 0);
			//esim_schedule_event(EV_MOD_NMOESI_EVICT, new_stack, 0);
			return;
		}

		stack->event = EV_MOD_NMOESI_NC_STORE_ACTION;
		esim_schedule_mod_stack_event(stack, 0);
		//esim_schedule_event(EV_MOD_NMOESI_NC_STORE_ACTION, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_NC_STORE_ACTION)
	{
		int retry_lat;

		mem_debug("  %lld %lld 0x%x %s nc store action\n", esim_time, stack->id,
			stack->addr, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:nc_store_action\"\n",
			stack->id, target_mod->name);

		/* Error locking */
		if (stack->err)
		{
			target_mod->nc_write_retries++;
			add_retry(stack,nc_store_action_retry);

			retry_lat = mod_get_retry_latency(target_mod);

			if(stack->client_info && stack->client_info->arch->timing){
				stack->latencias.retry += stack->client_info->arch->timing->cycle - stack->latencias.start + retry_lat;
			}
			stack->latencias.start = 0;

			if (stack->mshr_locked != 0)
			{
				mshr_unlock(target_mod, stack);
				stack->mshr_locked = 0;
			}

			mem_debug("    lock error, retrying in %d cycles\n", retry_lat);
			stack->retry = 1;

			stack->event = EV_MOD_NMOESI_NC_STORE_LOCK2;
			esim_schedule_mod_stack_event(stack, retry_lat);
			//esim_schedule_event(EV_MOD_NMOESI_NC_STORE_LOCK, stack, retry_lat);
			return;
		}

		stack->latencias.eviction = stack->client_info->arch->timing->cycle - stack->latencias.start - stack->latencias.queue - stack->latencias.lock_mshr - stack->latencias.lock_dir;


		/* Main memory modules are a special case */
		if (target_mod->kind == mod_kind_main_memory)
		{
			/* For non-coherent stores, finding an E or M for the state of
			 * a cache block in the directory still requires a message to
			 * the lower-level module so it can update its owner field.
			 * These messages should not be sent if the module is a main
			 * memory module. */

			stack->event = EV_MOD_NMOESI_NC_STORE_UNLOCK;
 			esim_schedule_mod_stack_event(stack, 0);
			//esim_schedule_event(EV_MOD_NMOESI_NC_STORE_UNLOCK, stack, 0);
			return;
		}

		/* N/S are hit */
		if (stack->dir_entry->state == cache_block_shared || stack->dir_entry->state == cache_block_noncoherent)
		{
			stack->event = EV_MOD_NMOESI_NC_STORE_UNLOCK;
 			esim_schedule_mod_stack_event(stack, 0);
			//esim_schedule_event(EV_MOD_NMOESI_NC_STORE_UNLOCK, stack, 0);
		}
		/* E state must tell the lower-level module to remove this module as an owner */
		else if (stack->dir_entry->state == cache_block_exclusive)
		{
			new_stack = mod_stack_create(stack->id, mod_get_low_mod(target_mod, stack->tag), stack->tag,
				EV_MOD_NMOESI_NC_STORE_MISS, stack);
			new_stack->wavefront = stack->wavefront;
			new_stack->retry = stack->retry;
			new_stack->uop = stack->uop;
			new_stack->message = message_clear_owner;
			new_stack->return_mod = target_mod;
			new_stack->event = EV_MOD_NMOESI_MESSAGE;
                        new_stack->stack_size = 8;
			esim_schedule_mod_stack_event(new_stack, 0);
			//esim_schedule_event(EV_MOD_NMOESI_MESSAGE, new_stack, 0);
		}
		/* Modified and Owned states need to call read request because we've already
		 * evicted the block so that the lower-level cache will have the latest value
		 * before it becomes non-coherent */
		else
		{
                        if(stack->dir_entry->state == cache_block_invalid && stack->uncacheable){
                            new_stack = mod_stack_create(stack->id, mod_get_low_mod(target_mod, stack->tag), stack->tag,
                                    EV_MOD_NMOESI_NC_STORE_MISS, stack);
                            new_stack->wavefront = stack->wavefront;
                            new_stack->retry = stack->retry;
                            new_stack->uop = stack->uop;
                            new_stack->allow_cache_by_passing = true;
                            new_stack->nc_write = 1;
                            new_stack->return_mod = target_mod;
                            new_stack->request_dir = mod_request_up_down;
                            new_stack->stack_size = 72;
                            new_stack->event = EV_MOD_NMOESI_NC_STORE_SEND;
                            esim_schedule_mod_stack_event(new_stack, 0);
                        }else{
                            new_stack = mod_stack_create(stack->id, mod_get_low_mod(target_mod, stack->tag), stack->tag,
                                    EV_MOD_NMOESI_NC_STORE_MISS, stack);
                            new_stack->wavefront = stack->wavefront;
                            new_stack->retry = stack->retry;
                            new_stack->uop = stack->uop;
                            new_stack->allow_cache_by_passing = false;
                            //new_stack->peer = mod_stack_set_peer(target_mod, stack->state);
                            new_stack->nc_write = 1;
                            new_stack->return_mod = target_mod;
                            new_stack->request_dir = mod_request_up_down;
                            new_stack->stack_size = 8;
                            new_stack->event = EV_MOD_NMOESI_READ_REQUEST;
                            esim_schedule_mod_stack_event(new_stack, 0);
                            //esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST, new_stack, 0);
                        }
		}

		return;
	}

	if (event == EV_MOD_NMOESI_NC_STORE_MISS)
	{
		int retry_lat;

		mem_debug("  %lld %lld 0x%x %s nc store miss\n", esim_time, stack->id,
			stack->addr, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:nc_store_miss\"\n",
			stack->id, target_mod->name);

		/* Error on read request. Unlock block and retry nc store. */
		if (stack->err)
		{
			target_mod->nc_write_retries++;
			add_retry(stack,nc_store_miss_retry);

			retry_lat = mod_get_retry_latency(target_mod);

			if(stack->client_info && stack->client_info->arch){
				stack->latencias.retry += stack->client_info->arch->timing->cycle - stack->latencias.start + retry_lat;
			}
			stack->latencias.start = 0;

			if (stack->mshr_locked != 0)
			{
				mshr_unlock(target_mod, stack);
				stack->mshr_locked = 0;
			}

                        if(stack->dir_entry->dir_lock->stack == stack)
                            dir_entry_unlock(stack->dir_entry);
                        else if(target_mod->cache->extra_dir_structure_type == extra_dir_per_cache_line)
                            assert(stack->uncacheable);
                        
                        mem_debug("    lock error, retrying in %d cycles\n", retry_lat);
			stack->retry = 1;

			stack->event = EV_MOD_NMOESI_NC_STORE_LOCK2;
			esim_schedule_mod_stack_event(stack, retry_lat);
			//esim_schedule_event(EV_MOD_NMOESI_NC_STORE_LOCK, stack, retry_lat);
			return;
		}

		stack->latencias.miss = stack->client_info->arch->timing->cycle - stack->latencias.start - stack->latencias.queue - stack->latencias.lock_mshr - stack->latencias.lock_dir - stack->latencias.eviction;

		/* Continue */
		stack->event = EV_MOD_NMOESI_NC_STORE_UNLOCK;
		esim_schedule_mod_stack_event(stack, 0);
		//esim_schedule_event(EV_MOD_NMOESI_NC_STORE_UNLOCK, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_NC_STORE_UNLOCK)
	{
		mem_debug("  %lld %lld 0x%x %s nc store unlock\n", esim_time, stack->id,
			stack->addr, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:nc_store_unlock\"\n",
			stack->id, target_mod->name);

		/* Set block state to excl/shared depending on return var 'shared'.
		 * Also set the tag of the block. */
		if(!stack->uncacheable)
                {
                    cache_set_block_new(target_mod->cache, stack, cache_block_noncoherent);
                   
                }

		if (stack->mshr_locked != 0)
		{
			mshr_unlock(target_mod,stack);
			stack->mshr_locked = 0;
		}

		/* Unlock directory entry */
                if(stack->dir_entry->dir_lock->stack == stack)
                    dir_entry_unlock(stack->dir_entry);
                else if(target_mod->cache->extra_dir_structure_type == extra_dir_per_cache_line)
                    assert(stack->uncacheable);

                stack->reply_size = 8;
                
		/* Impose the access latency before continuing */
		stack->event = EV_MOD_NMOESI_NC_STORE_FINISH;
		esim_schedule_mod_stack_event(stack, target_mod->latency);
		//esim_schedule_event(EV_MOD_NMOESI_NC_STORE_FINISH, stack,	mod->latency);
		return;
	}

	if (event == EV_MOD_NMOESI_NC_STORE_FINISH)
	{
		mem_debug("%lld %lld 0x%x %s nc store finish\n", esim_time, stack->id,
			stack->addr, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:nc_store_finish\"\n",
			stack->id, target_mod->name);
		mem_trace("mem.end_access name=\"A-%lld\"\n",
			stack->id);

		if (!stack->coalesced)
		{
			accu_retry_time_lost(stack);
			if(stack->client_info && stack->client_info->arch){
				long long ciclo = stack->client_info->arch->timing->cycle;
				mem_load_finish(ciclo - stack->tiempo_acceso);

				stack->latencias.finish = stack->client_info->arch->timing->cycle - stack->latencias.start - stack->latencias.queue - stack->latencias.lock_mshr - stack->latencias.lock_dir - stack->latencias.eviction - stack->latencias.miss;
			}
			if(stack->retry && !stack->hit)
				add_latencias_nc_write(&(stack->latencias));
		}

		/* Increment witness variable */
		if (stack->witness_ptr)
			(*stack->witness_ptr)++;

		/* Return event queue element into event queue */
		if (stack->event_queue && stack->event_queue_item)
			linked_list_add(stack->event_queue, stack->event_queue_item);

		/* Free the mod_client_info object, if any */
		if (stack->client_info)
			mod_client_info_free(target_mod, stack->client_info);

		/* Finish access */
		mod_access_finish(target_mod, stack);

		/* Return */
		mod_stack_return(stack);
		return;
	}

	abort();
}

void mod_handler_nmoesi_prefetch(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *new_stack;

	struct mod_t *target_mod = stack->target_mod;


	if (event == EV_MOD_NMOESI_PREFETCH)
	{
		struct mod_stack_t *master_stack;

		fatal("mod_handler_nmoesi_prefetch_event");

		mem_debug("%lld %lld 0x%x %s prefetch\n", esim_time, stack->id,
			stack->addr, target_mod->name);
		mem_trace("mem.new_access name=\"A-%lld\" type=\"store\" "
			"state=\"%s:prefetch\" addr=0x%x\n",
			stack->id, target_mod->name, stack->addr);

		/* Record access */
		mod_access_start(target_mod, stack, mod_access_prefetch);

		/* Coalesce access */
		master_stack = mod_can_coalesce(target_mod, mod_access_prefetch, stack->addr, stack);
		if (master_stack)
		{
			/* Doesn't make sense to prefetch as the block is already being fetched */
			mem_debug("  %lld %lld 0x%x %s useless prefetch - already being fetched\n",
				  esim_time, stack->id, stack->addr, target_mod->name);

			target_mod->useless_prefetches++;
			esim_schedule_event(EV_MOD_NMOESI_PREFETCH_FINISH, stack, 0);

			/* Increment witness variable */
			if (stack->witness_ptr)
				(*stack->witness_ptr)++;

			return;
		}

		/* Continue */
		esim_schedule_event(EV_MOD_NMOESI_PREFETCH_LOCK, stack, 0);
		return;
	}


	if (event == EV_MOD_NMOESI_PREFETCH_LOCK)
	{
		struct mod_stack_t *older_stack;

		mem_debug("  %lld %lld 0x%x %s prefetch lock\n", esim_time, stack->id,
			stack->addr, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:prefetch_lock\"\n",
			stack->id, target_mod->name);

		/* If there is any older write, wait for it */
		older_stack = mod_in_flight_write(target_mod, stack);
		if (older_stack)
		{
			mem_debug("    %lld wait for write %lld\n",
				stack->id, older_stack->id);
			mod_stack_wait_in_stack(stack, older_stack, EV_MOD_NMOESI_PREFETCH_LOCK);
			return;
		}

		/* Call find and lock */
		/*
		new_stack = mod_stack_create(stack->id, mod, stack->addr,
			EV_MOD_NMOESI_PREFETCH_ACTION, stack);
		new_stack->wavefront = stack->wavefront;
		new_stack->retry = stack->retry;
		new_stack->uop = stack->uop;
		new_stack->blocking = 0;
		new_stack->prefetch = 1;
		new_stack->retry = 0;
		new_stack->witness_ptr = stack->witness_ptr;
		*/
		stack->blocking = 0;
		stack->prefetch = 1;
		stack->event = EV_MOD_NMOESI_FIND_AND_LOCK;
		stack->find_and_lock_return_event = EV_MOD_NMOESI_PREFETCH_ACTION;
		esim_schedule_mod_stack_event(stack, 0);
		//esim_schedule_event(EV_MOD_NMOESI_FIND_AND_LOCK, new_stack, 0);

		/* Set witness variable to NULL so that retries from the same
		 * stack do not increment it multiple times */
		stack->witness_ptr = NULL;

		return;
	}

	if (event == EV_MOD_NMOESI_PREFETCH_ACTION)
	{
		mem_debug("  %lld %lld 0x%x %s prefetch action\n", esim_time, stack->id,
			stack->addr, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:prefetch_action\"\n",
			stack->id, target_mod->name);

		/* Error locking */
		if (stack->err)
		{
			/* Don't want to ever retry prefetches if getting a lock failed.
			Effectively this means that prefetches are of low priority.
			This can be improved to not retry only when the current lock
			holder is writing to the block. */
			target_mod->prefetch_aborts++;

			if (stack->mshr_locked != 0)
			{
				mshr_unlock(target_mod, stack);
				stack->mshr_locked = 0;
			}

			mem_debug("    lock error, aborting prefetch\n");
			esim_schedule_event(EV_MOD_NMOESI_PREFETCH_FINISH, stack, 0);
			return;
		}

		/* Hit */
		if (stack->dir_entry->state)
		{
			/* block already in the cache */
			mem_debug("  %lld %lld 0x%x %s useless prefetch - cache hit\n",
				  esim_time, stack->id, stack->addr, target_mod->name);

			target_mod->useless_prefetches++;
			esim_schedule_event(EV_MOD_NMOESI_PREFETCH_UNLOCK, stack, 0);
			return;
		}

		/* Miss */
		new_stack = mod_stack_create(stack->id, mod_get_low_mod(target_mod, stack->tag), stack->tag,
			EV_MOD_NMOESI_PREFETCH_MISS, stack);
		new_stack->wavefront = stack->wavefront;
		new_stack->retry = stack->retry;
		new_stack->uop = stack->uop;
		//new_stack->peer = mod_stack_set_peer(mod, stack->state);
		new_stack->return_mod = target_mod;
		new_stack->request_dir = mod_request_up_down;
		new_stack->prefetch = 1;
		esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST, new_stack, 0);
		return;
	}
	if (event == EV_MOD_NMOESI_PREFETCH_MISS)
	{
		mem_debug("  %lld %lld 0x%x %s prefetch miss\n", esim_time, stack->id,
			stack->addr, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:prefetch_miss\"\n",
			stack->id, target_mod->name);

		/* Error on read request. Unlock block and abort. */
		if (stack->err)
		{
			/* Don't want to ever retry prefetches if read request failed.
			 * Effectively this means that prefetches are of low priority.
			 * This can be improved depending on the reason for read request fail */
			target_mod->prefetch_aborts++;

			if (stack->mshr_locked != 0)
			{
				mshr_unlock(target_mod, stack);
				stack->mshr_locked = 0;
			}

			dir_entry_unlock(stack->dir_entry);
			mem_debug("    lock error, aborting prefetch\n");
			esim_schedule_event(EV_MOD_NMOESI_PREFETCH_FINISH, stack, 0);
			return;
		}

		/* Set block state to excl/shared depending on return var 'shared'.
		 * Also set the tag of the block. */
		//cache_set_block(target_mod->cache, stack->set, stack->way, stack->tag,
		//	stack->shared ? cache_block_shared : cache_block_exclusive);
                    cache_set_block_new(target_mod->cache, stack,
                            stack->shared ? cache_block_shared : cache_block_exclusive);

		/* Mark the prefetched block as prefetched. This is needed to let the
		 * prefetcher know about an actual access to this block so that it
		 * is aware of all misses as they would be without the prefetcher.
		 * TODO: The lower caches that will be filled because of this prefetch
		 * do not know if it was a prefetch or not. Need to have a way to mark
		 * them as prefetched too. */
		mod_block_set_prefetched(target_mod, stack->addr, 1);

		/* Continue */
		esim_schedule_event(EV_MOD_NMOESI_PREFETCH_UNLOCK, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_PREFETCH_UNLOCK)
	{
		mem_debug("  %lld %lld 0x%x %s prefetch unlock\n", esim_time, stack->id,
			stack->addr, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:prefetch_unlock\"\n",
			stack->id, target_mod->name);

		if (stack->mshr_locked != 0)
		{
			mshr_unlock(target_mod, stack);
			stack->mshr_locked = 0;
		}

		/* Unlock directory entry */
		dir_entry_unlock(stack->dir_entry);

		/* Continue */
		esim_schedule_event(EV_MOD_NMOESI_PREFETCH_FINISH, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_PREFETCH_FINISH)
	{
		mem_debug("%lld %lld 0x%x %s prefetch\n", esim_time, stack->id,
			stack->addr, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:prefetch_finish\"\n",
			stack->id, target_mod->name);
		mem_trace("mem.end_access name=\"A-%lld\"\n",
			stack->id);

		/* Increment witness variable */
		if (stack->witness_ptr)
			(*stack->witness_ptr)++;

		/* Return event queue element into event queue */
		if (stack->event_queue && stack->event_queue_item)
			linked_list_add(stack->event_queue, stack->event_queue_item);

		/* Free the mod_client_info object, if any */
		if (stack->client_info)
			mod_client_info_free(target_mod, stack->client_info);

		/* Finish access */
		mod_access_finish(target_mod, stack);

		/* Return */
		mod_stack_return(stack);
		return;
	}

	abort();
}

void mod_handler_nmoesi_find_and_lock(int event, void *data)
{
	struct mod_stack_t *stack = data;
	//struct mod_stack_t *ret = stack->ret_stack;
	//struct mod_stack_t *ret = stack;
	struct mod_stack_t *new_stack;

	struct mod_t *target_mod = stack->target_mod;


	if (event == EV_MOD_NMOESI_FIND_AND_LOCK)
	{
		mem_debug("  %lld %lld 0x%x %s find and lock (blocking=%d)\n",
			esim_time, stack->id, stack->addr, target_mod->name, stack->blocking);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:find_and_lock\"\n",
			stack->id, target_mod->name);

                if(stack->is_super_stack)
                {
                    mod_stack_wakeup_super_stack(stack);
                    return;
                }
                
		/* Default return values */
		stack->err = 0;

		/* If this stack has already been assigned a way, keep using it */
		//stack->way = ret->way;

		/* Get a port */
		mod_lock_port(target_mod, stack, EV_MOD_NMOESI_FIND_AND_LOCK_PORT);
		return;
	}

	if (event == EV_MOD_NMOESI_FIND_AND_LOCK_PORT)
	{
		struct mod_port_t *port = stack->port;
		struct dir_lock_t *dir_lock;
                //struct mod_stack_t *iter_stack = stack;
                //if(/*iter_stack->waiting_dir_access == 0 &&*/ (stack->find_and_lock_return_event == EV_MOD_NMOESI_LOAD_ACTION || stack->find_and_lock_return_event == EV_MOD_NMOESI_NC_STORE_WRITEBACK) && target_mod->level == 1 && stack->request_dir == mod_request_up_down)
                //    stack->uop->accesses_in_dir--;
                //assert(stack->uop->accesses_in_dir >= 0);

		assert(stack->port);
                //assert(stack->uop);
		//assert(list_count(stack->uop->mem_accesses_list));
           /* for(int i= 0;i < list_count(stack->uop->mem_accesses_list);i++)
            {
                if(!uop_cache_port || target_mod->level > 1 || stack->request_dir == mod_request_down_up)
                {
                    iter_stack = stack;
                    i = list_count(stack->uop->mem_accesses_list);
                }else{
                    iter_stack = list_get(stack->uop->mem_accesses_list,i);
                    if(iter_stack->waiting_dir_access || iter_stack == stack)
                    {
                        iter_stack->waiting_dir_access = 0;
                    }else{
                        continue;
                    }
                }*/
                assert(stack->find_and_lock_return_event);
                
                mem_debug("  %lld %lld 0x%x %s find and lock port\n", esim_time, stack->id,
			stack->addr, target_mod->name);
                mem_trace("mem.access name=\"A-%lld\" state=\"%s:find_and_lock_port\"\n",
			stack->id, target_mod->name);

		/* Set parent stack flag expressing that port has already been locked.
		 * This flag is checked by new writes to find out if it is already too
		 * late to coalesce. */
                //if(iter_stack == stack)
                    //stack->port_locked = 1;

		/* Look for block. */
		//stack->hit = mod_find_block(target_mod, stack->addr, &stack->set,
		//	&stack->way, &stack->tag, &stack->state);
                stack->hit = mod_find_block_new(target_mod, stack);
		//fran
                if(stack->hit)
                    add_cache_states(stack->dir_entry->state, target_mod->level);
                else
                    add_cache_states(cache_block_invalid, target_mod->level);
                
		/* Debug */
		if (stack->hit)
			mem_debug("    %lld 0x%x %s hit: set=%d, way=%d, state=%s\n", stack->id,
				stack->tag, target_mod->name, stack->set, stack->way,
				str_map_value(&cache_block_state_map, stack->dir_entry->state));

		/* Statistics */
		target_mod->accesses++;

		if (stack->hit){
			target_mod->hits++;
			//sumamos acceso y hit
			estadisticas(1, target_mod->level);
		}else{
			//sumamos acceso
			estadisticas(0, target_mod->level);
		}
		if (stack->read)
		{
			target_mod->reads++;
			target_mod->effective_reads++;
			stack->blocking ? target_mod->blocking_reads++ : target_mod->non_blocking_reads++;
			if (stack->hit)
				target_mod->read_hits++;
		}
		else if (stack->prefetch)
		{
			target_mod->prefetches++;
		}
		else if (stack->nc_write)  /* Must go after read */
		{
			target_mod->nc_writes++;
			target_mod->effective_nc_writes++;
			stack->blocking ? target_mod->blocking_nc_writes++ : target_mod->non_blocking_nc_writes++;
			if (stack->hit)
				target_mod->nc_write_hits++;
		}
		else if (stack->write)
		{
			target_mod->writes++;
			target_mod->effective_writes++;
			stack->blocking ? target_mod->blocking_writes++ : target_mod->non_blocking_writes++;

			/* Increment witness variable when port is locked */
			if (stack->witness_ptr)
			{
				(*stack->witness_ptr)++;
				stack->witness_ptr = NULL;
			}

			if (stack->hit)
				target_mod->write_hits++;
		}
		else if (stack->message)
		{
			/* FIXME */
		}
		else
		{
			fatal("Unknown memory operation type");
		}

		if (!stack->retry)
		{
			target_mod->no_retry_accesses++;
			if (stack->hit)
				target_mod->no_retry_hits++;

			if (stack->read)
			{
				target_mod->no_retry_reads++;
				if (stack->hit)
					target_mod->no_retry_read_hits++;
			}
			else if (stack->nc_write)  /* Must go after read */
			{
				target_mod->no_retry_nc_writes++;
				if (stack->hit)
					target_mod->no_retry_nc_write_hits++;
			}
			else if (stack->write)
			{
				target_mod->no_retry_writes++;
				if (stack->hit)
					target_mod->no_retry_write_hits++;
			}
			else if (stack->prefetch)
			{
				/* No retries currently for prefetches */
			}
			else if (stack->message)
			{
				/* FIXME */
			}
			else
			{
				fatal("Unknown memory operation type");
			}
		}

		if (!stack->hit)
		{
			/* Find victim */
			if (stack->way < 0 )
			{
				stack->way = cache_replace_block(target_mod->cache, stack->set);
			}
			if(flag_mshr_enabled && stack->request_dir != mod_request_down_up && stack->read && stack->mshr_locked == 0 && target_mod->kind != mod_kind_main_memory)
			{
				if(!mshr_lock(target_mod->mshr, stack))
				{
                                    mem_debug("  %lld %lld 0x%x %s find and lock port mshr full(blocking=%d)\n",
			esim_time, stack->id, stack->addr, target_mod->name, stack->blocking);
                                    if(stack->port != 0){
                                        mod_unlock_port(target_mod, port, stack);
					//stack->port_locked = 0;
                                    }
					stack->mshr_locked = 0;
					
                                        if(stack->dir_entry->dir_lock->stack == stack /* && stack->dir_lock->lock_queue && stack->dir_lock->lock == 0 */)
						dir_entry_unlock(stack->dir_entry);

					if(!stack->blocking)
					{
                                                stack->err = 1;
						stack->event = stack->find_and_lock_return_event;
						esim_schedule_mod_stack_event(stack, 0);
						//mod_stack_return(stack);
					}else{
						mshr_enqueue(target_mod->mshr,stack, EV_MOD_NMOESI_FIND_AND_LOCK);
					}
					return;
				}

				if(stack->client_info && stack->client_info->arch){
					stack->latencias.lock_mshr = stack->client_info->arch->timing->cycle - stack->latencias.start - stack->latencias.queue;
				}
				stack->mshr_locked = 1;
			}
		}
		assert(stack->way >= 0  || target_mod->dir->extra_dir_structure_type == extra_dir_per_cache);
         
                
                //if(stack->set != -1 /*&& stack->way != -1*/)
                    //stack->cache_block = cache_get_block_new(target_mod->cache, stack->set, stack->way);
		/* If directory entry is locked and the call to FIND_AND_LOCK is not
		 * blocking, release port and return error. */
                //struct dir_entry_t *dir_entry;
                if(!stack->hit)
                {
                    stack->cache_block = cache_get_block_new(target_mod->cache, stack->set, stack->way);
                    
                    stack->dir_entry = dir_entry_find_free_entry(target_mod->dir, stack->cache_block->dir_entry_selected);
                    /*if(dir_entry != NULL)
                    {
                        stack->dir_entry = dir_entry;
                    }else{
                        stack->dir_entry = stack->cache_block->dir_entry_selected;
                    }*/
                }else{
                    /*dir_entry = stack->cache_block->dir_entry_selected;
                    int w;
                    for(w = 0; w < target_mod->dir->wsize ;w++)
                    {
                        dir_entry = dir_entry_get(target_mod->dir, dir_entry->x, dir_entry->y, dir_entry->z, w);
                        if(dir_entry->tag == stack->tag || dir_entry->transient_tag == stack->tag)
                        {
                            break;
                        }
                    }*/
                    assert(stack->dir_entry != NULL /*|| target_mod->dir->extra_dir_structure_type == extra_dir_per_cache */);
                    //assert(w < target_mod->dir->wsize);
                }
                
                //stack->dir_entry = dir_entry;
                //stack->cache_block = cache_get_block_new(target_mod->cache, stack->set, stack->way);
                dir_lock = stack->dir_entry->dir_lock;
		if (dir_lock->lock && !stack->blocking && stack->hit)
		{
                        mem_debug("    %lld 0x%x %s block locked at set=%d, way=%d by A-%lld - aborting\n",
				stack->id, stack->tag, target_mod->name, stack->set, stack->way, dir_lock->stack->id);
			stack->err = 1;
			if (stack->mshr_locked != 0)
			{
				mshr_unlock(target_mod, stack);
				stack->mshr_locked = 0;
			}
			
			stack->event = stack->find_and_lock_return_event;
			esim_schedule_mod_stack_event(stack, 0);
                 
                        if(stack->port != 0)
                        {
                            mod_unlock_port(target_mod, port, stack);
                        }
			return;
		}  
                //bool avoid_dir_entry_lock = false;
                //buscar en extra_dir_entries
                /*if(dir_lock->lock && !stack->hit && multidir_enabled)
                {
                    struct dir_entry_t *dir_entry;
                    
                    int i = 0;
                    assert(stack->set >=0 && stack->way >= 0);
                    for(; i < target_mod->cache->extra_dir_entry_size;i++)
                    {
                        if(!target_mod->cache->sets[stack->set].blocks[stack->way].extra_dir_entry[i].dir_lock->lock)
                        {
                            dir_lock = target_mod->cache->sets[stack->set].blocks[stack->way].extra_dir_entry[i].dir_lock->lock;
                            //stack->dir_lock = dir_lock;
                            stack->state = cache_block_invalid;
                            avoid_dir_entry_lock = true;
                        }
                    }*/
                    
                    //no hay entradas libres
                    /*if(i >= target_mod->cache->extra_dir_entry_size)
                    {
                        
                    }*/
                    
                //}

                if(stack->allow_cache_by_passing && dir_lock->lock && !stack->hit && target_mod->allow_cache_by_passing)
                {
                    stack->uncacheable = true;
                    stack->event = EV_MOD_NMOESI_FIND_AND_LOCK_ACTION;
                    stack->way = -1;
                    //stack->state = cache_block_invalid;
                    esim_schedule_mod_stack_event(stack, target_mod->dir_latency);    
                    return;
                }

                /*if (stack->hit 
                        && target_mod->cache->sets[stack->set].blocks[stack->way].transient_tag == 
                        target_mod->cache->sets[stack->set].blocks[stack->way].tag 
                        && stack->blocking)
                {
                    if(stack->find_and_lock_return_event == EV_MOD_NMOESI_LOAD_ACTION)
                    {
                        mod_unlock_port(target_mod, port, stack);
                        mod_stack_wait_in_stack(stack, dir_lock->stack, EV_MOD_NMOESI_LOAD_FINISH);
                        return;
                    }else if(stack->find_and_lock_return_event == EV_MOD_NMOESI_NC_STORE_WRITEBACK){
                        mod_unlock_port(target_mod, port, stack);
                        mod_stack_wait_in_stack(stack, dir_lock->stack, EV_MOD_NMOESI_NC_STORE_FINISH);
                        return;
                    }
                }*/

                /* Lock directory entry. If lock fails, port needs to be released to prevent
                 * deadlock.  When the directory entry is released, locking port and
                 * directory entry will be retried. */
         
                if (!dir_entry_lock(stack->dir_entry, EV_MOD_NMOESI_FIND_AND_LOCK, stack))
                {
                        mem_debug("    %lld 0x%x %s block locked at set=%d, way=%d by A-%lld - waiting\n",
                                stack->id, stack->tag, target_mod->name, stack->set, stack->way, dir_lock->stack->id);
                        /*if (stack->mshr_locked != 0)
                        {
                                mshr_unlock2(mod);
                                stack->mshr_locked = 0;
                        }*/ 
                        // enviar mensaje para abortar el acceso anterior
                        bool allow_abort_accesses = false;
                        if(allow_abort_accesses && stack->request_dir == mod_request_down_up)
                        {
                            struct mod_stack_t *retry_stack;
                            retry_stack = dir_lock->stack;
                            new_stack = mod_stack_create(retry_stack->id, mod_get_low_mod(target_mod, retry_stack->tag), retry_stack->tag,
                                0, NULL);

                            new_stack->return_mod = target_mod;
                            new_stack->request_dir = mod_request_up_down;
                            new_stack->wavefront = retry_stack->wavefront;
                            new_stack->uop = retry_stack->uop;
                            new_stack->event = EV_MOD_NMOESI_MESSAGE;
                            new_stack->message = message_abort_access;
                            new_stack->stack_size = 8;
                            esim_schedule_mod_stack_event(new_stack, 0);
                        }

                        //iter_stack->uop->accesses_in_dir--;
                        if(stack->port != 0)
                        {
                            mod_unlock_port(target_mod, port, stack);
                            //stack->stack = 0;
                        }
                        return;
                }
                
                
                //lock directory entries for all the stacks belonging to stack->uop

		/* Miss */
		if (!stack->hit)
		{
                    
			/* Find victim */
			//cache_get_block(target_mod->cache, stack->set, stack->way, NULL, &stack->state);
                        if(stack->dir_entry->state)
			//if(stack->state)
			{
                            /*for(int i = 0; i < target_mod->cache->extra_dir_entry_size; i++)
                            {                
                                assert(stack->dir_lock != target_mod->cache->sets[stack->set].blocks[stack->way].extra_dir_entry[i].dir_lock);
                            }*/
                            
                            if(stack->read)
                                    add_load_invalidation(target_mod->level);
                            else
                                    add_store_invalidation(target_mod->level);
			}

                        if(target_mod->dir->extra_dir_structure_type == extra_dir_per_cache_line)
                        {
                            assert(stack->dir_entry->state || !dir_entry_group_shared_or_owned(target_mod->dir,
				stack->dir_entry));
                        }else if(target_mod->dir->extra_dir_structure_type == extra_dir_per_cache){
                            assert(!stack->dir_entry->state);
                        }
			mem_debug("    %lld 0x%x %s miss -> lru: set=%d, way=%d, state=%s\n",
				stack->id, stack->tag, target_mod->name, stack->set, stack->way,
				str_map_value(&cache_block_state_map, stack->dir_entry->state));
		}


		/* Entry is locked. Record the transient tag so that a subsequent lookup
		 * detects that the block is being brought.
		 * Also, update LRU counters here. */
		//cache_set_transient_tag(target_mod->cache, stack->set, stack->way, stack->tag);
                stack->dir_entry->transient_tag = stack->tag;
                
                //actualizar way solo con hit?
		cache_access_block(target_mod->cache, stack->set, stack->way);

		/* Access latency */
		stack->event = EV_MOD_NMOESI_FIND_AND_LOCK_ACTION;
		esim_schedule_mod_stack_event(stack, target_mod->dir_latency);
                //stack->dir_lock = dir_lock;
                 //stack->uop->accesses_in_dir--;
		//esim_schedule_event(EV_MOD_NMOESI_FIND_AND_LOCK_ACTION, stack, mod->dir_latency);
		
            //}
            return;
	}

	if (event == EV_MOD_NMOESI_FIND_AND_LOCK_ACTION)
	{
		struct mod_port_t *port = stack->port;

		//assert(port);
		mem_debug("  %lld %lld 0x%x %s find and lock action\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:find_and_lock_action\"\n",
			stack->id, target_mod->name);

		/* Release port */
                        assert(stack->port != 0);
                      
                            mod_unlock_port(target_mod, port, stack);
                      
		//mod_unlock_port(target_mod, port, stack);
		//stack->port_locked = 0;
		if(stack->client_info && stack->client_info->arch){
			stack->latencias.lock_dir = stack->client_info->arch->timing->cycle - stack->latencias.start - stack->latencias.queue - stack->latencias.lock_mshr;
		}
		/* On miss, evict if victim is a valid block. */
		if (!stack->uncacheable && !stack->hit && stack->dir_entry->state)
		{
			stack->eviction = 1;
			//mod_get_low_mod(target_mod, stack->tag)
			new_stack = mod_stack_create(stack->id,	mod_get_low_mod(target_mod,stack->dir_entry->tag), 0,
				EV_MOD_NMOESI_FIND_AND_LOCK_FINISH, stack);
			new_stack->wavefront = stack->wavefront;
			new_stack->retry = stack->retry;
			new_stack->uop = stack->uop;
			new_stack->set = stack->set;
			new_stack->way = stack->way;
			new_stack->event = EV_MOD_NMOESI_EVICT;
                        new_stack->dir_entry = stack->dir_entry;
			new_stack->return_mod = target_mod;
			esim_schedule_mod_stack_event(new_stack, 0);
			//esim_schedule_event(EV_MOD_NMOESI_EVICT, new_stack, 0);
			return;
		}

		/* Continue */
		stack->event = EV_MOD_NMOESI_FIND_AND_LOCK_FINISH;
		esim_schedule_mod_stack_event(stack, 0);
		//esim_schedule_event(EV_MOD_NMOESI_FIND_AND_LOCK_FINISH, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_FIND_AND_LOCK_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s find and lock finish (err=%d)\n", esim_time, stack->id,
			stack->tag, target_mod->name, stack->err);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:find_and_lock_finish\"\n",
			stack->id, target_mod->name);

		/* If evict produced err, return err */
		if (stack->err)
		{
			if (stack->mshr_locked != 0)
			{
				mshr_unlock(target_mod, stack);
				stack->mshr_locked = 0;
			}

			//cache_get_block(target_mod->cache, stack->set, stack->way, NULL, &stack->dir_entry->state);
			assert(stack->dir_entry->state);
			assert(stack->eviction);
			stack->err = 1;
			
                        if(stack->dir_entry->dir_lock->stack == stack)
                            dir_entry_unlock(stack->dir_entry);
                        else if(target_mod->cache->extra_dir_structure_type == extra_dir_per_cache_line)
                            assert(stack->uncacheable);
                        
                        //stack->dir_lock = NULL;
			stack->find_and_lock_stack = NULL;
			stack->event = stack->find_and_lock_return_event;
			stack->find_and_lock_return_event = 0;
			esim_schedule_mod_stack_event(stack, 0);
			//mod_stack_return(stack);
			return;
		}
		if(stack->client_info && stack->client_info->arch){
			stack->latencias.eviction = stack->client_info->arch->timing->cycle - stack->latencias.start - stack->latencias.queue - stack->latencias.lock_mshr - stack->latencias.lock_dir;
		}

		/* Eviction */
		if (stack->eviction)
		{
			target_mod->evictions++;
			add_eviction(target_mod->level);
			//cache_get_block(target_mod->cache, stack->set, stack->way, NULL, &stack->dir_entry->state);
			assert(!stack->dir_entry->state);
		}

		/* If this is a main memory, the block is here. A previous miss was just a miss
		 * in the directory. */
		if (target_mod->kind == mod_kind_main_memory && !stack->dir_entry->state)
		{
                    assert(!stack->uncacheable);
			//stack->state = cache_block_exclusive;
			//cache_set_block(target_mod->cache, stack->set, stack->way,
			//	stack->tag, stack->state);
                        cache_set_block_new(target_mod->cache, stack, cache_block_exclusive);
		}

		if(!stack->retry && (stack->origin || !stack->blocking))
		{
			if(stack->hit)
			{
				add_hit(target_mod->level);
			}else{
				add_miss(target_mod->level);
			}
		}

		/* Return */
		stack->err = 0;
		//stack->hit = stack->hit;
		//ret->dir_lock = stack->dir_lock;
		//stack->set = stack->set;
		//stack->way = stack->way;
		//stack->state = stack->state;
		//stack->tag = stack->tag;
		//stack->mshr_locked = stack->mshr_locked;
		//stack->find_and_lock_stack = NULL;
		stack->event = stack->find_and_lock_return_event;
		stack->find_and_lock_return_event = 0;
		esim_schedule_mod_stack_event(stack, 0);
		//mod_stack_return(stack);
		return;
	}

	abort();
}


void mod_handler_nmoesi_evict(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *ret = stack->ret_stack;
	struct mod_stack_t *new_stack;

	struct mod_t *return_mod = stack->return_mod;
	struct mod_t *target_mod = stack->target_mod;

	struct dir_t *dir;
	struct dir_entry_t *dir_entry;

	uint32_t dir_entry_tag, z;

        if (event == EV_MOD_NMOESI_EVICT_CHECK)
	{
            mem_debug("  %lld %lld 0x%x %s evict check (set=%d, way=%d, state=%s) err=%d\n", esim_time, stack->id,
			stack->addr, target_mod->name, stack->dir_entry->set, stack->dir_entry->way,
			str_map_value(&cache_block_state_map, stack->dir_entry->state), stack->err);
            if(stack->err)
            {        
                stack->err = 0;
                struct mod_stack_t *new_stack = mod_stack_create(stack->id, mod_get_low_mod(stack->target_mod,stack->addr), stack->addr, EV_MOD_NMOESI_EVICT_CHECK, stack);
                new_stack->set = stack->set;
                new_stack->way = stack->way;
                new_stack->return_mod = stack->target_mod;
                new_stack->dir_entry = stack->dir_entry;
               
                new_stack->event = EV_MOD_NMOESI_EVICT_LOCK_DIR;
                esim_schedule_mod_stack_event(new_stack, 0);
                return;
            }   
            assert(stack->dir_entry->dir_lock->stack == stack);
            dir_entry_unlock(stack->dir_entry);
            mod_stack_return(stack);
            return;
        }
        if(event == EV_MOD_NMOESI_EVICT_LOCK_DIR)
        {
            mem_debug("  %lld %lld 0x%x %s evict lock dir (set=%d, way=%d, state=%s)\n", esim_time, stack->id,
			stack->tag, return_mod->name, stack->dir_entry->set, stack->dir_entry->way,
			str_map_value(&cache_block_state_map, stack->dir_entry->state));
            //assert(!stack->dir_entry->dir_lock->lock);
            if(stack->dir_entry->tag != stack->tag)
            {
                    mod_stack_return(stack);
                    return;
            }
            
            if(dir_entry_lock(stack->dir_entry, EV_MOD_NMOESI_EVICT_LOCK_DIR, stack))
            {
                assert(stack->dir_entry->dir_lock->stack == stack);
                stack->event = EV_MOD_NMOESI_EVICT;
                esim_schedule_mod_stack_event(stack, 0);
            }
            return;
            
        }
	if (event == EV_MOD_NMOESI_EVICT)
	{
		/* Default return value */
		ret->err = 0;

		/* Get block info */
		//cache_get_block(return_mod->cache, stack->set, stack->way, &stack->tag, &stack->dir_entry->state);
                stack->tag = stack->dir_entry->tag;
		assert(stack->dir_entry->state || !dir_entry_group_shared_or_owned(target_mod->dir, stack->dir_entry));
		//assert(stack->dir_entry->set == stack->set && stack->dir_entry->way == stack->way);
                mem_debug("  %lld %lld 0x%x %s evict (set=%d, way=%d, state=%s)\n", esim_time, stack->id,
			stack->tag, return_mod->name, stack->dir_entry->set, stack->dir_entry->way,
			str_map_value(&cache_block_state_map, stack->dir_entry->state));
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:evict\"\n",
			stack->id, return_mod->name);

		/* Save some data */
		//stack->src_set = stack->set;
		//stack->src_way = stack->way;
		stack->src_tag = stack->tag;
                stack->src_stack = mod_stack_create(stack->id, stack->target_mod, stack->dir_entry->tag, 0, NULL);
                stack->src_stack->dir_entry = stack->dir_entry; 
                stack->src_stack->set = stack->dir_entry->set;
                stack->src_stack->way = stack->dir_entry->way;
		assert(stack->target_mod == mod_get_low_mod(return_mod, stack->tag));

		/* Send write request to all sharers */
		new_stack = mod_stack_create(stack->id, return_mod, stack->addr, EV_MOD_NMOESI_EVICT_INVALID, stack);
		new_stack->wavefront = stack->wavefront;
		new_stack->retry = stack->retry;
		new_stack->uop = stack->uop;
		new_stack->except_mod = NULL;
                new_stack->tag = stack->dir_entry->tag;
		new_stack->set = stack->dir_entry->set;
		new_stack->way = stack->dir_entry->way;
                new_stack->dir_entry = stack->dir_entry;

		new_stack->event = EV_MOD_NMOESI_INVALIDATE;
		esim_schedule_mod_stack_event(new_stack, 0);
		//esim_schedule_event(EV_MOD_NMOESI_INVALIDATE, new_stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_EVICT_INVALID)
	{
		mem_debug("  %lld %lld 0x%x %s evict invalid\n", esim_time, stack->id,
			stack->tag, return_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:evict_invalid\"\n",
			stack->id, return_mod->name);

		/* If module is main memory, we just need to set the block as invalid,
		 * and finish. */
		if (return_mod->kind == mod_kind_main_memory)
		{
			//cache_set_block(return_mod->cache, stack->src_set, stack->src_way,
			//	0, cache_block_invalid);
                        cache_set_block_new(return_mod->cache, stack->src_stack, cache_block_invalid);
			stack->event = EV_MOD_NMOESI_EVICT_FINISH;
			esim_schedule_mod_stack_event(stack, 0);
			//esim_schedule_event(EV_MOD_NMOESI_EVICT_FINISH, stack, 0);
			return;
		}

		/* Continue */
		stack->event = EV_MOD_NMOESI_EVICT_ACTION;
		esim_schedule_mod_stack_event(stack, 0);
		//esim_schedule_event(EV_MOD_NMOESI_EVICT_ACTION, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_EVICT_ACTION)
	{
		struct mod_t *low_mod;
		struct net_node_t *low_node;
		int msg_size;

		mem_debug("  %lld %lld 0x%x %s evict action\n", esim_time, stack->id,
			stack->tag, return_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:evict_action\"\n",
			stack->id, return_mod->name);

		/* Get low node */
		low_mod = stack->target_mod;
		low_node = low_mod->high_net_node;
		assert(low_mod != return_mod);
		assert(low_mod == mod_get_low_mod(return_mod, stack->tag));
		assert(low_node && low_node->user_data == low_mod);

		/* Update the cache state since it may have changed after its
		 * higher-level modules were invalidated */
		//cache_get_block(return_mod->cache, stack->set, stack->way, NULL, &stack->state);

		/* State = I */
		if (stack->dir_entry->state == cache_block_invalid)
		{
			stack->event = EV_MOD_NMOESI_EVICT_FINISH;
			esim_schedule_mod_stack_event(stack, 0);
			//esim_schedule_event(EV_MOD_NMOESI_EVICT_FINISH, stack, 0);
			return;
		}

		/* If state is M/O/N, data must be sent to lower level mod */
		if (stack->dir_entry->state == cache_block_modified || stack->dir_entry->state == cache_block_owned ||
			stack->dir_entry->state == cache_block_noncoherent)
		{
			/* Need to transmit data to low module */
			msg_size = 8 + return_mod->block_size;
			stack->reply = reply_ack_data;
		}
		/* If state is E/S, just an ack needs to be sent */
		else
		{
			msg_size = 8;
			stack->reply = reply_ack;
		}

		/* Send message */
		stack->msg = net_try_send_ev(return_mod->low_net, return_mod->low_net_node,
			low_node, msg_size, EV_MOD_NMOESI_EVICT_RECEIVE, stack, event, stack);
		return;
	}

	if (event == EV_MOD_NMOESI_EVICT_RECEIVE)
	{
		mem_debug("  %lld %lld 0x%x %s evict receive\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:evict_receive\"\n",
			stack->id, target_mod->name);

		/* Receive message */
		net_receive(target_mod->high_net, target_mod->high_net_node, stack->msg);

		/* Find and lock */
		if (stack->dir_entry->state == cache_block_noncoherent)
		{
			/*
			new_stack = mod_stack_create(stack->id, target_mod, stack->src_tag,
				EV_MOD_NMOESI_EVICT_PROCESS_NONCOHERENT, stack);
			new_stack->wavefront = stack->wavefront;
			new_stack->retry = stack->retry;
			new_stack->uop = stack->uop;
			new_stack->nc_write = 1;
			*/
			stack->addr = stack->src_tag;
			stack->nc_write = 1;
			stack->find_and_lock_return_event = EV_MOD_NMOESI_EVICT_PROCESS_NONCOHERENT;
		}
		else
		{
			/*
			new_stack = mod_stack_create(stack->id, target_mod, stack->src_tag,
				EV_MOD_NMOESI_EVICT_PROCESS, stack);
			new_stack->wavefront = stack->wavefront;
			new_stack->uop = stack->uop;
			new_stack->write = 1;
			*/
			stack->addr = stack->src_tag;
			stack->write = 1;
			stack->find_and_lock_return_event = EV_MOD_NMOESI_EVICT_PROCESS;
		}

		/* FIXME It's not guaranteed to be a write */
		/*new_stack->blocking = 0;
		new_stack->retry = 0;
		new_stack->event = EV_MOD_NMOESI_FIND_AND_LOCK;
		*/
		stack->blocking = 0;
		stack->retry = 0;
		stack->event = EV_MOD_NMOESI_FIND_AND_LOCK;
		//stack->find_and_lock_return_event = EV_MOD_NMOESI_PREFETCH_ACTION;
		esim_schedule_mod_stack_event(stack, 0);
		//esim_schedule_mod_stack_event(new_stack, 0);
		//esim_schedule_event(EV_MOD_NMOESI_FIND_AND_LOCK, new_stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_EVICT_PROCESS)
	{
		mem_debug("  %lld %lld 0x%x %s evict process\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:evict_process\"\n",
			stack->id, target_mod->name);
                assert(stack->hit);
		/* Error locking block */
		if (stack->err)
		{
			if (stack->mshr_locked != 0)
			{
				mshr_unlock(target_mod, stack);
				stack->mshr_locked = 0;
			}

			ret->err = 1;
			stack->event = EV_MOD_NMOESI_EVICT_REPLY;
			esim_schedule_mod_stack_event(stack, 0);
			//esim_schedule_event(EV_MOD_NMOESI_EVICT_REPLY, stack, 0);
			return;
		}

		/* If data was received, set the block to modified */
		if (stack->reply == reply_ack)
		{
			/* Nothing to do */
		}
		else if (stack->reply == reply_ack_data)
		{
			if(target_mod->kind == mod_kind_main_memory &&
                            target_mod->dram_system)
                        {
                                struct dram_system_t *ds = target_mod->dram_system;
                                //struct x86_ctx_t *ctx = stack->client_info->ctx;
                                assert(ds);
                                //assert(ctx);

				if (stack->mshr_locked != 0)
				{
					mshr_unlock(target_mod, stack);
					stack->mshr_locked = 0;
				}

                                /* Retry if memory controller cannot accept transaction */
                                //if (!dram_system_will_accept_trans(ds->handler, stack->tag >> 2))
                                if (!dram_system_will_accept_trans(ds->handler, stack->addr))
                                {
                                        stack->err = 1;
                                        ret->err = 1;
                                        ret->retry |= 1 << target_mod->level;

                                        dir = target_mod->dir;

                                        if(stack->dir_entry->dir_lock->stack == stack)
                                            dir_entry_unlock(stack->dir_entry);
                                        else if(target_mod->cache->extra_dir_structure_type == extra_dir_per_cache_line)
                                            assert(stack->uncacheable);

                                        mem_debug("    %lld 0x%x %s mc queue full, retrying write...\n", stack->id, stack->tag, target_mod->name);

                                        esim_schedule_event(EV_MOD_NMOESI_EVICT_REPLY, stack, 0);
                                        return;
                                }

                                /* Access main memory system */
                                //dram_system_add_write_trans(ds->handler, stack->tag >> 2, stack->wavefront->wavefront_pool_entry->wavefront_pool->compute_unit->id, stack->wavefront->id);
				if(stack->wavefront == NULL){
					dram_system_add_write_trans(ds->handler, stack->addr, 0, 0);
				}else{
					dram_system_add_write_trans(ds->handler, stack->addr, stack->wavefront->wavefront_pool_entry->wavefront_pool->compute_unit->id, stack->wavefront->id);
				}
                                /*mem_debug("  %lld %lld 0x%x %s dram access enqueued\n", esim_time, stack->id, stack->tag, stack->target_mod->dram_system->name);
          
                                linked_list_add(ds->pending_writes, stack);
                                return;*/
                                /* Ctx main memory stats */
                        //ctx->mm_write_accesses++;
                        }
			if (stack->dir_entry->state == cache_block_exclusive)
			{
				//cache_set_block(target_mod->cache, stack->set, stack->way,
				//	stack->tag, cache_block_modified);
                                cache_set_block_new(target_mod->cache, stack, cache_block_modified);
			}
			else if (stack->dir_entry->state == cache_block_modified)
			{
				/* Nothing to do */
			}
			else
			{
				fatal("%s: Invalid cache block state: %d\n", __FUNCTION__,
					stack->dir_entry->state);
			}
		}
		else
		{
			fatal("%s: Invalid cache block state: %d\n", __FUNCTION__,
				stack->dir_entry->state);
		}

		/* Remove sharer and owner */
		dir = target_mod->dir;
		for (z = 0; z < dir->zsize; z++)
		{
			/* Skip other subblocks */
			dir_entry_tag = stack->tag + z * target_mod->sub_block_size;
			assert(dir_entry_tag < stack->tag + target_mod->block_size);
			if (dir_entry_tag < stack->src_tag ||
				dir_entry_tag >= stack->src_tag + return_mod->block_size)
			{
				continue;
			}

			dir_entry = dir_entry_get(dir, stack->dir_entry->set,stack->dir_entry->way, z, stack->dir_entry->w);
			dir_entry_clear_sharer(dir_entry, return_mod->low_net_node->index);
			if (dir_entry->owner == return_mod->low_net_node->index)
			{
				dir_entry_set_owner(dir_entry, DIR_ENTRY_OWNER_NONE);
			}
		}

		/* Unlock the directory entry */
		dir = target_mod->dir;

		if (stack->mshr_locked != 0)
		{
			mshr_unlock(target_mod, stack);
			stack->mshr_locked = 0;
		}

                 if(stack->dir_entry->dir_lock->stack == stack)
                    dir_entry_unlock(stack->dir_entry);
                else if(target_mod->cache->extra_dir_structure_type == extra_dir_per_cache_line)
                    assert(stack->uncacheable);
                
		

		stack->event = EV_MOD_NMOESI_EVICT_REPLY;
		/* If the access is to main memory then return inmediately, because the transaction is already
		 * inserted and will be processed in background if using a memory controller or ignored if using a fixed latency main memory */
		esim_schedule_mod_stack_event(stack, target_mod->kind == mod_kind_main_memory ? 0 : target_mod->latency);
		//esim_schedule_event(EV_MOD_NMOESI_EVICT_REPLY, stack, target_mod->latency);

		return;
	}

	if (event == EV_MOD_NMOESI_EVICT_PROCESS_NONCOHERENT)
	{
		mem_debug("  %lld %lld 0x%x %s evict process noncoherent\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:evict_process_noncoherent\"\n",
			stack->id, target_mod->name);
                assert(stack->hit);

		/* Error locking block */
		if (stack->err)
		{
			if (stack->mshr_locked != 0)
			{
				mshr_unlock(target_mod, stack);
				stack->mshr_locked = 0;
			}

			ret->err = 1;

			stack->event = EV_MOD_NMOESI_EVICT_REPLY;
			esim_schedule_mod_stack_event(stack, 0);
			//esim_schedule_event(EV_MOD_NMOESI_EVICT_REPLY, stack, 0);
			return;
		}

		/* If data was received, set the block to modified */
		if (stack->reply == reply_ack_data)
		{

			if(target_mod->kind == mod_kind_main_memory &&
				target_mod->dram_system)
			{
				struct dram_system_t *ds = target_mod->dram_system;
				//struct x86_ctx_t *ctx = stack->client_info->ctx;
				assert(ds);
				//assert(ctx);

				if (stack->mshr_locked != 0)
				{
					mshr_unlock(target_mod, stack);
					stack->mshr_locked = 0;
				}

				/* Retry if memory controller cannot accept transaction */
				//if (!dram_system_will_accept_trans(ds->handler, stack->tag >> 2))
				if (!dram_system_will_accept_trans(ds->handler, stack->addr))
				{
					stack->err = 1;
					ret->err = 1;
					ret->retry |= 1 << target_mod->level;

					dir = target_mod->dir;
                                        
                                        if(stack->dir_entry->dir_lock->stack == stack)
                                            dir_entry_unlock(stack->dir_entry);
                                        else if(target_mod->cache->extra_dir_structure_type == extra_dir_per_cache_line)
                                            assert(stack->uncacheable);
					

					mem_debug("    %lld 0x%x %s mc queue full, retrying write...\n", stack->id, stack->tag, target_mod->name);

					esim_schedule_event(EV_MOD_NMOESI_EVICT_REPLY, stack, 0);
					return;
				}
				/* Access main memory system */
				//dram_system_add_write_trans(ds->handler, stack->tag >> 2, stack->wavefront->wavefront_pool_entry->wavefront_pool->compute_unit->id, stack->wavefront->id);
				dram_system_add_write_trans(ds->handler, stack->addr, stack->wavefront->wavefront_pool_entry->wavefront_pool->compute_unit->id, stack->wavefront->id);

				/* Ctx main memory stats */
				//ctx->mm_write_accesses++;
			}

			if (stack->dir_entry->state == cache_block_exclusive)
			{
				//cache_set_block(target_mod->cache, stack->set, stack->way,
				//	stack->tag, cache_block_modified);
                                cache_set_block_new(target_mod->cache, stack, cache_block_modified);
			}
			else if (stack->dir_entry->state == cache_block_owned ||
				stack->dir_entry->state == cache_block_modified)
			{
				/* Nothing to do */
			}
			else if (stack->dir_entry->state == cache_block_shared ||
				stack->dir_entry->state == cache_block_noncoherent)
			{
				//cache_set_block(target_mod->cache, stack->set, stack->way,
				//	stack->tag, cache_block_noncoherent);
                                cache_set_block_new(target_mod->cache, stack, cache_block_noncoherent);
			}
			else
			{
				fatal("%s: Invalid cache block state: %d\n", __FUNCTION__,
					stack->dir_entry->state);
			}
		}
		else
		{
			fatal("%s: Invalid cache block state: %d\n", __FUNCTION__,
				stack->dir_entry->state);
		}

		/* Remove sharer and owner */
		dir = target_mod->dir;
		for (z = 0; z < dir->zsize; z++)
		{
			/* Skip other subblocks */
			dir_entry_tag = stack->tag + z * target_mod->sub_block_size;
			assert(dir_entry_tag < stack->tag + target_mod->block_size);
			if (dir_entry_tag < stack->src_tag ||
				dir_entry_tag >= stack->src_tag + return_mod->block_size)
			{
				continue;
			}

			dir_entry = dir_entry_get(dir, stack->dir_entry->set, stack->dir_entry->way, z, stack->dir_entry->w);
			dir_entry_clear_sharer(dir_entry, return_mod->low_net_node->index);
			if (dir_entry->owner == return_mod->low_net_node->index)
			{
				dir_entry_set_owner(dir_entry, DIR_ENTRY_OWNER_NONE);
			}
		}

		/* Unlock the directory entry */
		dir = target_mod->dir;

		if (stack->mshr_locked != 0)
		{
			mshr_unlock(target_mod, stack);
			stack->mshr_locked = 0;
		}

                if(stack->dir_entry->dir_lock->stack == stack)
                    dir_entry_unlock(stack->dir_entry);
                else if(target_mod->cache->extra_dir_structure_type == extra_dir_per_cache_line)
                    assert(stack->uncacheable);
		

		stack->event = EV_MOD_NMOESI_EVICT_REPLY;
		esim_schedule_mod_stack_event(stack, target_mod->kind == mod_kind_main_memory ? 0 : target_mod->latency);
		//esim_schedule_event(EV_MOD_NMOESI_EVICT_REPLY, stack, target_mod->latency);
		return;
	}

	if (event == EV_MOD_NMOESI_EVICT_REPLY)
	{
		mem_debug("  %lld %lld 0x%x %s evict reply\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:evict_reply\"\n",
			stack->id, target_mod->name);

		/* Send message */
		stack->msg = net_try_send_ev(target_mod->high_net, target_mod->high_net_node,
			return_mod->low_net_node, 8, EV_MOD_NMOESI_EVICT_REPLY_RECEIVE, stack,
			event, stack);
		return;

	}

	if (event == EV_MOD_NMOESI_EVICT_REPLY_RECEIVE)
	{
		mem_debug("  %lld %lld 0x%x %s evict reply receive\n", esim_time, stack->id,
			stack->tag, return_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:evict_reply_receive\"\n",
			stack->id, return_mod->name);

		/* Receive message */
		net_receive(return_mod->low_net, return_mod->low_net_node, stack->msg);

		/* Invalidate block if there was no error. */
		if (!stack->err)
                {
			//cache_set_block(return_mod->cache, stack->src_set, stack->src_way,
			//	0, cache_block_invalid);
                        cache_set_block_new(return_mod->cache, stack->src_stack, cache_block_invalid);
                }
                
		assert(!dir_entry_group_shared_or_owned(return_mod->dir, stack->ret_stack->dir_entry));
		stack->event = EV_MOD_NMOESI_EVICT_FINISH;
		esim_schedule_mod_stack_event(stack, 0);
		//esim_schedule_event(EV_MOD_NMOESI_EVICT_FINISH, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_EVICT_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s evict finish\n", esim_time, stack->id,
			stack->tag, return_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:evict_finish\"\n",
			stack->id, return_mod->name);
                mod_stack_return(stack->src_stack);
                stack->src_stack = NULL;
		mod_stack_return(stack);
		return;
	}

	abort();
}


void mod_handler_nmoesi_read_request(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *ret = stack->ret_stack;
	struct mod_stack_t *new_stack;

	struct mod_t *return_mod = stack->return_mod;
	struct mod_t *target_mod = stack->target_mod;

	struct dir_t *dir;
	struct dir_entry_t *dir_entry;

	uint32_t dir_entry_tag, z;

	if (event == EV_MOD_NMOESI_READ_REQUEST)
	{
		struct net_t *net;
		struct net_node_t *src_node;
		struct net_node_t *dst_node;

		mem_debug("  %lld %lld 0x%x %s read request\n", esim_time, stack->id,
			stack->addr, return_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:read_request\"\n",
			stack->id, return_mod->name);

		/* Default return values*/
		if(!stack->is_super_stack)
                {
                    ret->shared = 0;
                    ret->err = 0;
                }

		/* Checks */
		assert(stack->request_dir);
		assert(mod_get_low_mod(return_mod, stack->addr) == target_mod ||
			stack->request_dir == mod_request_down_up);
		assert(mod_get_low_mod(target_mod, stack->addr) == return_mod ||
			stack->request_dir == mod_request_up_down);

		/* Get source and destination nodes */
		if (stack->request_dir == mod_request_up_down)
		{
			net = return_mod->low_net;
			src_node = return_mod->low_net_node;
			dst_node = target_mod->high_net_node;
		}
		else
		{
			net = return_mod->high_net;
			src_node = return_mod->high_net_node;
			dst_node = target_mod->low_net_node;
		}
                assert(stack->stack_size > 7);
		/* Send message */
		stack->msg = net_try_send_ev(net, src_node, dst_node, stack->stack_size,
			EV_MOD_NMOESI_READ_REQUEST_RECEIVE, stack, event, stack);
		return;
	}

	if (event == EV_MOD_NMOESI_READ_REQUEST_RECEIVE)
	{
		mem_debug("  %lld %lld 0x%x %s read request receive\n", esim_time, stack->id,
			stack->addr, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:read_request_receive\"\n",
			stack->id, target_mod->name);

		/* Receive message */
		if (stack->request_dir == mod_request_up_down)
		{
			net_receive(target_mod->high_net, target_mod->high_net_node, stack->msg);
			add_access(target_mod->level);
		}else{
			net_receive(target_mod->low_net, target_mod->low_net_node, stack->msg);
		}
		/* Find and lock */
		/*
		new_stack = mod_stack_create(stack->id, target_mod, stack->addr,
			EV_MOD_NMOESI_READ_REQUEST_ACTION, stack);
		new_stack->wavefront = stack->wavefront;
		new_stack->retry = stack->retry;
		new_stack->uop = stack->uop;
		new_stack->blocking = stack->request_dir == mod_request_down_up;
		new_stack->read = 1;
		new_stack->retry = 0;
		new_stack->event = EV_MOD_NMOESI_FIND_AND_LOCK;
		*/
		stack->read = 1;
                if(stack->ret_stack->uncacheable)
                {   
                    assert(stack->request_dir == mod_request_up_down);
                    stack->blocking = 1;
                }else
                    stack->blocking = stack->request_dir == mod_request_down_up;
		stack->event = EV_MOD_NMOESI_FIND_AND_LOCK;
		stack->find_and_lock_return_event = EV_MOD_NMOESI_READ_REQUEST_ACTION;
		esim_schedule_mod_stack_event(stack, 0);
		//esim_schedule_mod_stack_event(new_stack, 0);
		//esim_schedule_event(EV_MOD_NMOESI_FIND_AND_LOCK, new_stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_READ_REQUEST_ACTION)
	{
		mem_debug("  %lld %lld 0x%x %s read request action\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:read_request_action\"\n",
			stack->id, target_mod->name);

		/* Check block locking error. If read request is down-up, there should not
		 * have been any error while locking. */
		if (stack->err)
		{
			assert(stack->request_dir == mod_request_up_down);
			ret->err = 1;

			if (stack->mshr_locked != 0)
			{
				mshr_unlock(target_mod, stack);
				stack->mshr_locked = 0;
			}

			mod_stack_set_reply(ret, reply_ack_error);
			stack->reply_size = 8;
			stack->event = EV_MOD_NMOESI_READ_REQUEST_REPLY;
			esim_schedule_mod_stack_event(stack, 0);
			//esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST_REPLY, stack, 0);
			return;
		}
		stack->event = stack->request_dir == mod_request_up_down ?
			EV_MOD_NMOESI_READ_REQUEST_UPDOWN : EV_MOD_NMOESI_READ_REQUEST_DOWNUP;
		esim_schedule_mod_stack_event(stack, 0);
		//esim_schedule_event(stack->request_dir == mod_request_up_down ?	EV_MOD_NMOESI_READ_REQUEST_UPDOWN : EV_MOD_NMOESI_READ_REQUEST_DOWNUP, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_READ_REQUEST_UPDOWN)
	{
		struct mod_t *owner;

		mem_debug("  %lld %lld 0x%x %s read request updown\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:read_request_updown\"\n",
			stack->id, target_mod->name);

		stack->pending = 1;

		/* Set the initial reply message and size.  This will be adjusted later if
		 * a transfer occur between peers. */
		stack->reply_size = return_mod->block_size + 8;
		mod_stack_set_reply(stack, reply_ack_data);

		if (stack->dir_entry->state)
		{
			/*estadisticas*/
                        if(stack->ret_event == EV_MOD_NMOESI_LOAD_MISS)
                        {
				add_CoalesceHit(target_mod->level);
			}
			/* Status = M/O/E/S/N
			 * Check: address is a multiple of requester's block_size
			 * Check: no sub-block requested by mod is already owned by mod */
			assert(stack->addr % return_mod->block_size == 0);
			dir = target_mod->dir;
			for (z = 0; z < dir->zsize; z++)
			{
				dir_entry_tag = stack->tag + z * target_mod->sub_block_size;
				assert(dir_entry_tag < stack->tag + target_mod->block_size);
				if (dir_entry_tag < stack->addr || dir_entry_tag >= stack->addr + return_mod->block_size)
					continue;
				dir_entry = dir_entry_get(dir, stack->dir_entry->set, stack->dir_entry->way, z, stack->dir_entry->w);
				assert(dir_entry->owner != return_mod->low_net_node->index);
			}
                        
                        assert(!stack->uncacheable);
                        if(!ret->uncacheable)
                        {

                            /* TODO If there is only sharers, should one of them
                             *      send the data to mod instead of having target_mod do it? */

                            /* Send read request to owners other than mod for all sub-blocks. */
                            for (z = 0; z < dir->zsize; z++)
                            {
                                    struct net_node_t *node;

                                    dir_entry = dir_entry_get(dir, stack->dir_entry->set, stack->dir_entry->way, z, stack->dir_entry->w);
                                    dir_entry_tag = stack->tag + z * target_mod->sub_block_size;

                                    /* No owner */
                                    if (!DIR_ENTRY_VALID_OWNER(dir_entry))
                                            continue;

                                    /* Owner is mod */

                                    //if (dir_entry->owner == return_mod->low_net_node->index)
                                    //	continue;

                                    /* Get owner mod */
                                    node = list_get(target_mod->high_net->node_list, dir_entry->owner);
                                    assert(node->kind == net_node_end);
                                    owner = node->user_data;
                                    assert(owner);

                                    /* Not the first sub-block */
                                    if (dir_entry_tag % owner->block_size)
                                            continue;

                                    /* Send read request */
                                    stack->pending++;
                                    new_stack = mod_stack_create(stack->id, owner, dir_entry_tag,
                                            EV_MOD_NMOESI_READ_REQUEST_UPDOWN_FINISH, stack);
                                    new_stack->wavefront = stack->wavefront;
                                    new_stack->retry = stack->retry;
                                    new_stack->uop = stack->uop;
                                    /* Only set peer if its a subblock that was requested */
                                    /*if (dir_entry_tag >= stack->addr &&
                                            dir_entry_tag < stack->addr + mod->block_size)
                                    {
                                            new_stack->peer = mod_stack_set_peer(mod, stack->state);
                                    }*/
                                    new_stack->return_mod = target_mod;
                                    new_stack->request_dir = mod_request_down_up;
                                    new_stack->stack_size = 8;
                                       new_stack->event = EV_MOD_NMOESI_READ_REQUEST;
                                    esim_schedule_mod_stack_event(new_stack, 0);
                                    //esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST, new_stack, 0);
                            }
                        }
			stack->event = EV_MOD_NMOESI_READ_REQUEST_UPDOWN_FINISH;
			esim_schedule_mod_stack_event(stack, 0);
			//esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST_UPDOWN_FINISH, stack, 0);

			/* The prefetcher may have prefetched this earlier and hence
			 * this is a hit now. Let the prefetcher know of this hit
			 * since without the prefetcher, this may have been a miss.
			 * TODO: I'm not sure how relavant this is here for all states. */
			//prefetcher_access_hit(stack, target_mod);
		}
		else
		{
			/* State = I */

                        if(stack->ret_event == EV_MOD_NMOESI_LOAD_MISS)
                        {
                            add_CoalesceMiss(target_mod->level);
			}

			assert(stack->uncacheable || !dir_entry_group_shared_or_owned(target_mod->dir, stack->dir_entry));
			new_stack = mod_stack_create(stack->id, mod_get_low_mod(target_mod, stack->tag), stack->tag,
				EV_MOD_NMOESI_READ_REQUEST_UPDOWN_MISS, stack);
			new_stack->wavefront = stack->wavefront;
			new_stack->return_mod = target_mod;
			new_stack->retry = stack->retry;
			new_stack->uop = stack->uop;
                        new_stack->allow_cache_by_passing = stack->allow_cache_by_passing;
			/* Peer is NULL since we keep going up-down */
			new_stack->request_dir = mod_request_up_down;
                        new_stack->stack_size = 8;
			new_stack->event = EV_MOD_NMOESI_READ_REQUEST;
			esim_schedule_mod_stack_event(new_stack, 0);
			//esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST, new_stack, 0);

			/* The prefetcher may be interested in this miss */
			//prefetcher_access_miss(stack, target_mod);

		}
		return;
	}

	if (event == EV_MOD_NMOESI_READ_REQUEST_UPDOWN_MISS)
	{
		mem_debug("  %lld %lld 0x%x %s read request updown miss\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:read_request_updown_miss\"\n",
			stack->id, target_mod->name);

		/* Check error */
		if (stack->err)
		{
			if (stack->mshr_locked != 0)
			{
				mshr_unlock(target_mod, stack);
				stack->mshr_locked = 0;
			}

                        if(stack->dir_entry->dir_lock->stack == stack)
                            dir_entry_unlock(stack->dir_entry);
                        else if(target_mod->cache->extra_dir_structure_type == extra_dir_per_cache_line)
                            assert(stack->uncacheable);
			
			ret->err = 1;
			mod_stack_set_reply(ret, reply_ack_error);
			stack->reply_size = 8;

			stack->event = EV_MOD_NMOESI_READ_REQUEST_REPLY;
			esim_schedule_mod_stack_event(stack, 0);
			//esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST_REPLY, stack, 0);
			return;
		}

		/* Set block state to excl/shared depending on the return value 'shared'
		 * that comes from a read request into the next cache level.
		 * Also set the tag of the block. */
                if(!stack->uncacheable)
                {
                    //cache_set_block(target_mod->cache, stack->set, stack->way, stack->tag,
		//	stack->shared ? cache_block_shared : cache_block_exclusive);
                    cache_set_block_new(target_mod->cache, stack, stack->shared ? cache_block_shared : cache_block_exclusive);
                }
                
		stack->event = EV_MOD_NMOESI_READ_REQUEST_UPDOWN_FINISH;
		esim_schedule_mod_stack_event(stack, 0);
		//esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST_UPDOWN_FINISH, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_READ_REQUEST_UPDOWN_FINISH)
	{
		int shared;

		/* Ensure that a reply was received */
		assert(stack->reply);

		/* Ignore while pending requests */
		assert(stack->pending > 0);
		stack->pending--;
		if (stack->pending)
			return;

		/* Trace */
		mem_debug("  %lld %lld 0x%x %s read request updown finish\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:read_request_updown_finish\"\n",
			stack->id, target_mod->name);

		/* If blocks were sent directly to the peer, the reply size would
		 * have been decreased.  Based on the final size, we can tell whether
		 * to send more data or simply ack */
		if (stack->reply_size == 8)
		{
			mod_stack_set_reply(ret, reply_ack);
		}
		else if (stack->reply_size > 8)
		{
			mod_stack_set_reply(ret, reply_ack_data);
		}
		else
		{
			fatal("Invalid reply size: %d", stack->reply_size);
		}

		dir = target_mod->dir;

		/* If another module has not given the block, access main memory */
    if (target_mod->kind == mod_kind_main_memory &&
        target_mod->dram_system &&
        stack->request_dir == mod_request_up_down &&
        !stack->main_memory_accessed &&
        stack->reply != reply_ack_data_sent_to_peer)
    {
        struct dram_system_t *ds = target_mod->dram_system;
        //struct x86_ctx_t *ctx = stack->client_info->ctx;
        //assert(ctx);
        assert(ds);

	if (stack->mshr_locked != 0)
	{
            mshr_unlock(target_mod, stack);
            stack->mshr_locked = 0;
	}

        //if (!dram_system_will_accept_trans(ds->handler, stack->tag >> 2))
        if (!dram_system_will_accept_trans(ds->handler, stack->addr))
        {
            stack->err = 1;
            ret->err = 1;
            //ret->retry |= 1 << target_mod->level;
            mod_stack_set_reply(ret, reply_ack_error);
            stack->reply_size = 8;
        
            if(stack->dir_entry->dir_lock->stack == stack)
                dir_entry_unlock(stack->dir_entry);
            else if(target_mod->cache->extra_dir_structure_type == extra_dir_per_cache_line)
                assert(stack->uncacheable);
        
            mem_debug("    %lld 0x%x %s mc queue full, retrying...\n", stack->id, stack->tag, target_mod->name);
            esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST_REPLY, stack, 0);
            return;
      }

      /* Access main memory system */
      mem_debug("  %lld %lld 0x%x %s dram access enqueued\n", esim_time, stack->id, stack->tag, stack->target_mod->dram_system->name);
      linked_list_add(ds->pending_reads, stack);
      //dram_system_add_read_trans(ds->handler, stack->tag >> 2, stack->wavefront->wavefront_pool_entry->wavefront_pool->compute_unit->id, stack->wavefront->id);
			if(stack->wavefront == NULL){
				dram_system_add_read_trans(ds->handler, stack->addr, 0, 0);
			}else{
				dram_system_add_read_trans(ds->handler, stack->addr, stack->wavefront->wavefront_pool_entry->wavefront_pool->compute_unit->id, stack->wavefront->id);
			}
			stack->dramsim_mm_start = asTiming(si_gpu)->cycle;

      /* Ctx main memory stats */
      //ctx->mm_read_accesses++;
      //if (stack->prefetch)
      //    ctx->mm_pref_accesses++;
      return;
    }

                if(stack->uncacheable)
                    ret->uncacheable = true;
                
		shared = 0; 
                if(!ret->uncacheable)
                {
                    /* With the Owned state, the directory entry may remain owned by the sender */
                    if (!stack->retain_owner)
                    {
                            /* Set owner to 0 for all directory entries not owned by mod. */
                            for (z = 0; z < dir->zsize; z++)
                            {
                                    dir_entry = dir_entry_get(dir, stack->dir_entry->set, stack->dir_entry->way, z, stack->dir_entry->w);
                                    if (dir_entry->owner != return_mod->low_net_node->index)
                                            dir_entry_set_owner(dir_entry, DIR_ENTRY_OWNER_NONE);
                            }
                    }

                    /* For each sub-block requested by mod, set mod as sharer, and
                     * check whether there is other cache sharing it. */
               
                    for (z = 0; z < dir->zsize; z++)
                    {
                            dir_entry_tag = stack->tag + z * target_mod->sub_block_size;
                            if (dir_entry_tag < stack->addr || dir_entry_tag >= stack->addr + return_mod->block_size)
                                    continue;
                            dir_entry = dir_entry_get(dir, stack->dir_entry->set, stack->dir_entry->way, z, stack->dir_entry->w);
                            dir_entry_set_sharer(dir_entry, return_mod->low_net_node->index);
                            if (dir_entry->num_sharers > 1 || stack->nc_write || stack->shared)
                                    shared = 1;

                            /* If the block is owned, non-coherent, or shared,
                             * mod (the higher-level cache) should never be exclusive */
                            if (stack->dir_entry->state == cache_block_owned ||
                                    stack->dir_entry->state == cache_block_noncoherent ||
                                    stack->dir_entry->state == cache_block_shared )
                                    shared = 1;
                    }
                

                    /* If no sub-block requested by mod is shared by other cache, set mod
                     * as owner of all of them. Otherwise, notify requester that the block is
                     * shared by setting the 'shared' return value to true. */
                    ret->shared = shared;
                    if (!shared)
                    {
                            for (z = 0; z < dir->zsize; z++)
                            {
                                    dir_entry_tag = stack->tag + z * target_mod->sub_block_size;
                                    if (dir_entry_tag < stack->addr || dir_entry_tag >= stack->addr + return_mod->block_size)
                                            continue;
                                    dir_entry = dir_entry_get(dir, stack->dir_entry->set, stack->dir_entry->way, z, stack->dir_entry->w);
                                    dir_entry_set_owner(dir_entry, return_mod->low_net_node->index);
                            }
                    }
                }

		if (stack->mshr_locked != 0)
		{
			mshr_unlock(target_mod, stack);
			stack->mshr_locked = 0;
		}
                
                if(stack->dir_entry->dir_lock->stack == stack)
                    dir_entry_unlock(stack->dir_entry);
                else if(target_mod->cache->extra_dir_structure_type == extra_dir_per_cache_line)
                    assert(stack->uncacheable);
                
		
		stack->dramsim_mm_start = asTiming(si_gpu)->cycle;
		//int latency = stack->reply == reply_ack_data_sent_to_peer ? 0 : target_mod->latency;
		int latency = target_mod->latency;
		stack->event = EV_MOD_NMOESI_READ_REQUEST_REPLY;
		esim_schedule_mod_stack_event(stack, latency);
		//esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST_REPLY, stack, latency);
		return;
	}

	if (event == EV_MOD_NMOESI_READ_REQUEST_DOWNUP)
	{
		struct mod_t *owner;

		mem_debug("  %lld %lld 0x%x %s read request downup\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:read_request_downup\"\n",
			stack->id, target_mod->name);

		/* Check: state must not be invalid or shared.
		 * By default, only one pending request.
		 * Response depends on state */
		assert(stack->dir_entry->state != cache_block_invalid);
		assert(stack->dir_entry->state != cache_block_shared);
		assert(stack->dir_entry->state != cache_block_noncoherent);
		stack->pending = 1;

		/* Send a read request to the owner of each subblock. */
		dir = target_mod->dir;
		for (z = 0; z < dir->zsize; z++)
		{
			struct net_node_t *node;

			dir_entry_tag = stack->tag + z * target_mod->sub_block_size;
			assert(dir_entry_tag < stack->tag + target_mod->block_size);
			dir_entry = dir_entry_get(dir, stack->dir_entry->set, stack->dir_entry->way, z, stack->dir_entry->w);

			/* No owner */
			if (!DIR_ENTRY_VALID_OWNER(dir_entry))
				continue;

			/* Get owner mod */
			node = list_get(target_mod->high_net->node_list, dir_entry->owner);
			assert(node && node->kind == net_node_end);
			owner = node->user_data;

			/* Not the first sub-block */
			if (dir_entry_tag % owner->block_size)
				continue;

			stack->pending++;
			new_stack = mod_stack_create(stack->id, owner, dir_entry_tag,
				EV_MOD_NMOESI_READ_REQUEST_DOWNUP_WAIT_FOR_REQS, stack);
			new_stack->wavefront = stack->wavefront;
			new_stack->retry = stack->retry;
			new_stack->uop = stack->uop;
			new_stack->return_mod = target_mod;
			new_stack->request_dir = mod_request_down_up;
                        new_stack->stack_size = 8;
			new_stack->event = EV_MOD_NMOESI_READ_REQUEST;
			esim_schedule_mod_stack_event(new_stack, 0);
			//esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST, new_stack, 0);
		}
		stack->event = EV_MOD_NMOESI_READ_REQUEST_DOWNUP_WAIT_FOR_REQS;
		esim_schedule_mod_stack_event(stack, 0);
		//esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST_DOWNUP_WAIT_FOR_REQS, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_READ_REQUEST_DOWNUP_WAIT_FOR_REQS)
	{
		/* Ignore while pending requests */
		assert(stack->pending > 0);
		stack->pending--;
		if (stack->pending)
			return;

		mem_debug("  %lld %lld 0x%x %s read request downup wait for reqs\n",
			esim_time, stack->id, stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:read_request_downup_wait_for_reqs\"\n",
			stack->id, target_mod->name);

		/*if (stack->peer)
		{*/
			/* Send this block (or subblock) to the peer */
		/*	new_stack = mod_stack_create(stack->id, target_mod, stack->tag,
				EV_MOD_NMOESI_READ_REQUEST_DOWNUP_FINISH, stack);
			new_stack->peer = mod_stack_set_peer(stack->peer, stack->state);
			new_stack->target_mod = stack->target_mod;
			esim_schedule_event(EV_MOD_NMOESI_PEER_SEND, new_stack, 0);
		}
		else
		{*/
			/* No data to send to peer, so finish */
			stack->event = EV_MOD_NMOESI_READ_REQUEST_DOWNUP_FINISH;
			esim_schedule_mod_stack_event(stack, 0);
			//esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST_DOWNUP_FINISH, stack, 0);
		//}

		return;
	}

	if (event == EV_MOD_NMOESI_READ_REQUEST_DOWNUP_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s read request downup finish\n",
			esim_time, stack->id, stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:read_request_downup_finish\"\n",
			stack->id, target_mod->name);

		if (stack->reply == reply_ack_data)
		{
			/* If data was received, it was owned or modified by a higher level cache.
			 * We need to continue to propagate it up until a peer is found */

			/*if (stack->peer)
			{*/
				/* Peer was found, so this directory entry should be changed
				 * to owned */
				/*cache_set_block(target_mod->cache, stack->set, stack->way,
					stack->tag, cache_block_owned);
					*/
				/* Higher-level cache changed to shared, set owner of
				 * sub-blocks to NONE. */
				/*dir = target_mod->dir;
				for (z = 0; z < dir->zsize; z++)
				{
					dir_entry_tag = stack->tag + z * target_mod->sub_block_size;
					assert(dir_entry_tag < stack->tag + target_mod->block_size);
					dir_entry = dir_entry_get(dir, stack->set, stack->way, z);
					dir_entry_set_owner(dir, stack->set, stack->way, z,
						DIR_ENTRY_OWNER_NONE);
				}

				stack->reply_size = 8;
				mod_stack_set_reply(ret, reply_ack_data_sent_to_peer);
				*/
				/* Decrease the amount of data that mod will have to send back
				 * to its higher level cache */
				/*ret->reply_size -= target_mod->block_size;
				assert(ret->reply_size >= 8);
				*/
				/* Let the lower-level cache know not to delete the owner */
				/*ret->retain_owner = 1;
			}
			else
			{*/
				/* Set state to shared */
				//cache_set_block(target_mod->cache, stack->set, stack->way,
				//	stack->tag, cache_block_shared);
                                cache_set_block_new(target_mod->cache, stack, cache_block_shared);

				/* State is changed to shared, set owner of sub-blocks to 0. */
				dir = target_mod->dir;
				for (z = 0; z < dir->zsize; z++)
				{
					dir_entry_tag = stack->tag + z * target_mod->sub_block_size;
					assert(dir_entry_tag < stack->tag + target_mod->block_size);
					dir_entry = dir_entry_get(dir, stack->dir_entry->set, stack->dir_entry->way, z, stack->dir_entry->w);
					dir_entry_set_owner(dir_entry, DIR_ENTRY_OWNER_NONE);
				}

				stack->reply_size = target_mod->block_size + 8;
				mod_stack_set_reply(ret, reply_ack_data);
			//}
		}
		else if (stack->reply == reply_ack)
		{
			/* Higher-level cache was exclusive with no modifications above it */
			stack->reply_size = 8;

			/* Set state to shared */
			//cache_set_block(target_mod->cache, stack->set, stack->way,
			//	stack->tag, cache_block_shared);
                        cache_set_block_new(target_mod->cache, stack, cache_block_shared);

			/* State is changed to shared, set owner of sub-blocks to 0. */
			dir = target_mod->dir;
			for (z = 0; z < dir->zsize; z++)
			{
				dir_entry_tag = stack->tag + z * target_mod->sub_block_size;
				assert(dir_entry_tag < stack->tag + target_mod->block_size);
				dir_entry = dir_entry_get(dir, stack->dir_entry->set, stack->dir_entry->way, z, stack->dir_entry->w);
				dir_entry_set_owner(dir_entry, DIR_ENTRY_OWNER_NONE);
			}

			/*if (stack->peer)
			{
				stack->reply_size = 8;
				mod_stack_set_reply(ret, reply_ack_data_sent_to_peer);
				*/
				/* Decrease the amount of data that mod will have to send back
				 * to its higher level cache */
				/*ret->reply_size -= target_mod->block_size;
				assert(ret->reply_size >= 8);
			}
			else
			{*/
				mod_stack_set_reply(ret, reply_ack);
				stack->reply_size = 8;
			//}
		}
		else if (stack->reply == reply_none)
		{
			/* This block is not present in any higher level caches */

			/*if (stack->peer)
			{
				stack->reply_size = 8;
				mod_stack_set_reply(ret, reply_ack_data_sent_to_peer);
				*/
				/* Decrease the amount of data that mod will have to send back
				 * to its higher level cache */
				/*ret->reply_size -= target_mod->sub_block_size;
				assert(ret->reply_size >= 8);

				if (stack->state == cache_block_modified ||
					stack->state == cache_block_owned)
				{*/
					/* Let the lower-level cache know not to delete the owner */
					//ret->retain_owner = 1;

					/* Set block to owned */
					/*cache_set_block(target_mod->cache, stack->set, stack->way,
						stack->tag, cache_block_owned);
				}
				else
				{*/
					/* Set block to shared */
				/*	cache_set_block(target_mod->cache, stack->set, stack->way,
						stack->tag, cache_block_shared);
				}
			}
			else
			{*/
				if (stack->dir_entry->state == cache_block_exclusive ||
					stack->dir_entry->state == cache_block_shared)
				{
					stack->reply_size = 8;
					mod_stack_set_reply(ret, reply_ack);

				}
				else if (stack->dir_entry->state == cache_block_owned ||
					stack->dir_entry->state == cache_block_modified ||
					stack->dir_entry->state == cache_block_noncoherent)
				{
					/* No peer exists, so data is returned to mod */
					stack->reply_size = target_mod->sub_block_size + 8;
					mod_stack_set_reply(ret, reply_ack_data);
				}
				else
				{
					fatal("Invalid cache block state: %d\n", stack->dir_entry->state);
				}

				/* Set block to shared */
				//cache_set_block(target_mod->cache, stack->set, stack->way,
				//	stack->tag, cache_block_shared);
                                cache_set_block_new(target_mod->cache, stack, cache_block_shared);
			//}
		}
		else
		{
			fatal("Unexpected reply type: %d\n", stack->reply);
		}

		if (stack->mshr_locked != 0)
		{
			mshr_unlock(target_mod, stack);
			stack->mshr_locked = 0;
		}

                if(stack->dir_entry->dir_lock->stack == stack)
                    dir_entry_unlock(stack->dir_entry);
                else if(target_mod->cache->extra_dir_structure_type == extra_dir_per_cache_line)
                    assert(stack->uncacheable);

		//int latency = stack->reply == reply_ack_data_sent_to_peer ? 0 : target_mod->latency;
		int latency = target_mod->latency;

		stack->event = EV_MOD_NMOESI_READ_REQUEST_REPLY;
		esim_schedule_mod_stack_event(stack, latency);
		//esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST_REPLY, stack, latency);
		return;
	}

	if (event == EV_MOD_NMOESI_READ_REQUEST_REPLY)
	{
		struct net_t *net;
		struct net_node_t *src_node;
		struct net_node_t *dst_node;

		mem_debug("  %lld %lld 0x%x %s read request reply\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:read_request_reply\"\n",
			stack->id, target_mod->name);

		/* Checks */
		assert(stack->reply_size);
		assert(stack->request_dir);
		assert(mod_get_low_mod(return_mod, stack->addr) == target_mod ||
			mod_get_low_mod(target_mod, stack->addr) == return_mod);

		if(stack->main_memory_accessed != 1)
		{
			stack->main_memory_accessed = 1;
			stack->uop->mem_mm_latency += asTiming(si_gpu)->cycle  - stack->dramsim_mm_start;
			stack->uop->mem_mm_accesses++;
		}

		/* Get network and nodes */
		if (stack->request_dir == mod_request_up_down)
		{
			net = return_mod->low_net;
			src_node = target_mod->high_net_node;
			dst_node = return_mod->low_net_node;
		}
		else
		{
			net = return_mod->high_net;
			src_node = target_mod->low_net_node;
			dst_node = return_mod->high_net_node;
		}

		/* Send message */
		stack->msg = net_try_send_ev(net, src_node, dst_node, stack->reply_size,
			EV_MOD_NMOESI_READ_REQUEST_FINISH, stack, event, stack);
		return;
	}

	if (event == EV_MOD_NMOESI_READ_REQUEST_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s read request finish\n", esim_time, stack->id,
			stack->tag, return_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:read_request_finish\"\n",
			stack->id, return_mod->name);

		/* Receive message */
		if (stack->request_dir == mod_request_up_down)
			net_receive(return_mod->low_net, return_mod->low_net_node, stack->msg);
		else
			net_receive(return_mod->high_net, return_mod->high_net_node, stack->msg);

		/* Return */
		mod_stack_return(stack);
		return;
	}

	abort();
}


void mod_handler_nmoesi_write_request(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *ret = stack->ret_stack;
	struct mod_stack_t *new_stack;

	struct mod_t *return_mod = stack->return_mod;
	struct mod_t *target_mod = stack->target_mod;

	struct dir_t *dir;
	struct dir_entry_t *dir_entry;

	uint32_t dir_entry_tag, z;


	if (event == EV_MOD_NMOESI_WRITE_REQUEST)
	{
		struct net_t *net;
		struct net_node_t *src_node;
		struct net_node_t *dst_node;

		mem_debug("  %lld %lld 0x%x %s write request\n", esim_time, stack->id,
			stack->addr, return_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:write_request\"\n",
			stack->id, return_mod->name);

		/* Default return values */
		ret->err = 0;

		/* For write requests, we need to set the initial reply size because
		 * in updown, peer transfers must be allowed to decrease this value
		 * (during invalidate). If the request turns out to be downup, then
		 * these values will get overwritten. */
		stack->reply_size = return_mod->block_size + 8;
		mod_stack_set_reply(stack, reply_ack_data);

		/* Checks */
		assert(stack->request_dir);
		assert(mod_get_low_mod(return_mod, stack->addr) == target_mod ||
			stack->request_dir == mod_request_down_up);
		assert(mod_get_low_mod(target_mod, stack->addr) == return_mod ||
			stack->request_dir == mod_request_up_down);

		/* Get source and destination nodes */
		if (stack->request_dir == mod_request_up_down)
		{
			net = return_mod->low_net;
			src_node = return_mod->low_net_node;
			dst_node = target_mod->high_net_node;
		}
		else
		{
			net = return_mod->high_net;
			src_node = return_mod->high_net_node;
			dst_node = target_mod->low_net_node;
		}

		/* Send message */
		stack->msg = net_try_send_ev(net, src_node, dst_node, 8,
			EV_MOD_NMOESI_WRITE_REQUEST_RECEIVE, stack, event, stack);
		return;
	}

	if (event == EV_MOD_NMOESI_WRITE_REQUEST_RECEIVE)
	{
		mem_debug("  %lld %lld 0x%x %s write request receive\n", esim_time, stack->id,
			stack->addr, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:write_request_receive\"\n",
			stack->id, target_mod->name);

		/* Receive message */
		if (stack->request_dir == mod_request_up_down)
			net_receive(target_mod->high_net, target_mod->high_net_node, stack->msg);
		else
			net_receive(target_mod->low_net, target_mod->low_net_node, stack->msg);

		/* Find and lock */
		/*
		new_stack = mod_stack_create(stack->id, target_mod, stack->addr,
			EV_MOD_NMOESI_WRITE_REQUEST_ACTION, stack);
		new_stack->wavefront = stack->wavefront;
		new_stack->retry = stack->retry;
		new_stack->uop = stack->uop;
		new_stack->blocking = stack->request_dir == mod_request_down_up;
		new_stack->write = 1;
		new_stack->event = EV_MOD_NMOESI_FIND_AND_LOCK;
		*/

		stack->write = 1;
		stack->blocking = stack->request_dir == mod_request_down_up;
		stack->event = EV_MOD_NMOESI_FIND_AND_LOCK;
		stack->find_and_lock_return_event = EV_MOD_NMOESI_WRITE_REQUEST_ACTION;
		esim_schedule_mod_stack_event(stack, 0);
		//esim_schedule_mod_stack_event(new_stack, 0);
		//esim_schedule_event(EV_MOD_NMOESI_FIND_AND_LOCK, new_stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_WRITE_REQUEST_ACTION)
	{
		mem_debug("  %lld %lld 0x%x %s write request action\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:write_request_action\"\n",
			stack->id, target_mod->name);

		/* Check lock error. If write request is down-up, there should
		 * have been no error. */
		if (stack->err)
		{
			if (stack->mshr_locked != 0)
			{
				mshr_unlock(target_mod, stack);
				stack->mshr_locked = 0;
			}

			assert(stack->request_dir == mod_request_up_down);
			ret->err = 1;
			stack->reply_size = 8;
			stack->event = EV_MOD_NMOESI_WRITE_REQUEST_REPLY;
			esim_schedule_mod_stack_event(stack, 0);
			//esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST_REPLY, stack, 0);
			return;
		}

                if(stack->dir_entry->state == cache_block_invalid && stack->uncacheable)
                {
                    stack->event = EV_MOD_NMOESI_WRITE_REQUEST_EXCLUSIVE;
                    esim_schedule_mod_stack_event(stack, 0);
                }else{
                    /* Invalidate the rest of upper level sharers */
                    new_stack = mod_stack_create(stack->id, target_mod, 0,
                            EV_MOD_NMOESI_WRITE_REQUEST_EXCLUSIVE, stack);
                    new_stack->wavefront = stack->wavefront;
                    new_stack->retry = stack->retry;
                    new_stack->uop = stack->uop;
                    new_stack->except_mod = return_mod;
                    new_stack->set = stack->set;
                    new_stack->way = stack->way;
                    new_stack->dir_entry = stack->dir_entry;
                    //ew_stack->peer = mod_stack_set_peer(stack->peer, stack->state);
                    new_stack->event = EV_MOD_NMOESI_INVALIDATE;
                    esim_schedule_mod_stack_event(new_stack, 0);
                    //esim_schedule_event(EV_MOD_NMOESI_INVALIDATE, new_stack, 0);
                }
		return;
	}

	if (event == EV_MOD_NMOESI_WRITE_REQUEST_EXCLUSIVE)
	{
		mem_debug("  %lld %lld 0x%x %s write request exclusive\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:write_request_exclusive\"\n",
			stack->id, target_mod->name);


		int event = stack->request_dir == mod_request_up_down ? EV_MOD_NMOESI_WRITE_REQUEST_UPDOWN : EV_MOD_NMOESI_WRITE_REQUEST_DOWNUP;
		stack->event = event;
		esim_schedule_mod_stack_event(stack, 0);
		return;
		/*if (stack->request_dir == mod_request_up_down)
			esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST_UPDOWN, stack, 0);
		else
			esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST_DOWNUP, stack, 0);
		return;*/
	}

	if (event == EV_MOD_NMOESI_WRITE_REQUEST_UPDOWN)
	{
		mem_debug("  %lld %lld 0x%x %s write request updown\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:write_request_updown\"\n",
			stack->id, target_mod->name);

		/* state = M/E */
		if (stack->dir_entry->state == cache_block_modified ||
			stack->dir_entry->state == cache_block_exclusive)
		{
			stack->event = EV_MOD_NMOESI_WRITE_REQUEST_UPDOWN_FINISH;
			esim_schedule_mod_stack_event(stack, 0);
			//esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST_UPDOWN_FINISH, stack, 0);
		}
		/* state = O/S/I/N */
		else if (stack->dir_entry->state == cache_block_owned || stack->dir_entry->state == cache_block_shared ||
			stack->dir_entry->state == cache_block_invalid || stack->dir_entry->state == cache_block_noncoherent)
		{
			new_stack = mod_stack_create(stack->id, mod_get_low_mod(target_mod, stack->tag), stack->tag,
				EV_MOD_NMOESI_WRITE_REQUEST_UPDOWN_FINISH, stack);
			new_stack->wavefront = stack->wavefront;
			new_stack->retry = stack->retry;
			new_stack->uop = stack->uop;
                        
                        if(stack->allow_cache_by_passing && stack->uncacheable)
                            new_stack->allow_cache_by_passing = true;
                        else
                            new_stack->allow_cache_by_passing = false;
                        
			//new_stack->peer = mod_stack_set_peer(mod, stack->state);
			new_stack->return_mod = target_mod;
			new_stack->request_dir = mod_request_up_down;
			new_stack->event = EV_MOD_NMOESI_WRITE_REQUEST;
			esim_schedule_mod_stack_event(new_stack, 0);
			//esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST, new_stack, 0);

			if (stack->dir_entry->state == cache_block_invalid)
			{
				/* The prefetcher may be interested in this miss */
				//prefetcher_access_miss(stack, target_mod);
			}
		}
		else
		{
			fatal("Invalid cache block state: %d\n", stack->dir_entry->state);
		}

		if (stack->dir_entry->state != cache_block_invalid)
		{
			/* The prefetcher may have prefetched this earlier and hence
			 * this is a hit now. Let the prefetcher know of this hit
			 * since without the prefetcher, this may been a miss.
			 * TODO: I'm not sure how relavant this is here for all states. */
			//prefetcher_access_hit(stack, target_mod);
		}

		return;
	}

	if (event == EV_MOD_NMOESI_WRITE_REQUEST_UPDOWN_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s write request updown finish\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:write_request_updown_finish\"\n",
			stack->id, target_mod->name);

		/* Ensure that a reply was received */
		assert(stack->reply);

		/* Error in write request to next cache level */
		if (stack->err)
		{
			if (stack->mshr_locked != 0)
			{
				mshr_unlock(target_mod, stack);
				stack->mshr_locked = 0;
			}

			ret->err = 1;
			mod_stack_set_reply(ret, reply_ack_error);
			stack->reply_size = 8;
                        
                        if(stack->dir_entry->dir_lock->stack == stack)
                            dir_entry_unlock(stack->dir_entry);
                        else if(target_mod->cache->extra_dir_structure_type == extra_dir_per_cache_line)
                            assert(stack->uncacheable);
			
			stack->event = EV_MOD_NMOESI_WRITE_REQUEST_REPLY;
			esim_schedule_mod_stack_event(stack, 0);
			//esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST_REPLY, stack, 0);
			return;
		}

		/* If another module has not given the block,
			 access main memory */
		if (target_mod->kind == mod_kind_main_memory &&
		 	target_mod->dram_system &&
			stack->request_dir == mod_request_up_down &&
			!stack->main_memory_accessed &&
			stack->reply != reply_ack_data_sent_to_peer)
		{
			struct dram_system_t *ds = target_mod->dram_system;
			//struct x86_ctx_t *ctx = stack->client_info->ctx;
			assert(ds);
			//assert(ctx);

			if (stack->mshr_locked != 0)
			{
				mshr_unlock(target_mod, stack);
				stack->mshr_locked = 0;
			}

			//if (!dram_system_will_accept_trans(ds->handler, stack->tag >> 2))
			if (!dram_system_will_accept_trans(ds->handler, stack->addr))
			{
				stack->err = 1;
				ret->err = 1;
				ret->retry |= 1 << target_mod->level;

				mod_stack_set_reply(ret, reply_ack_error);
				stack->reply_size = 8;

                                if(stack->dir_entry->dir_lock->stack == stack)
                                    dir_entry_unlock(stack->dir_entry);
                                else if(target_mod->cache->extra_dir_structure_type == extra_dir_per_cache_line)
                                    assert(stack->uncacheable);

				mem_debug("    %lld 0x%x %s mc queue full, retrying...\n", stack->id, stack->tag, target_mod->name);

				esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST_REPLY, stack, 0);
				return;
			}

			/* Access main memory system */
			mem_debug("  %lld %lld 0x%x %s dram access enqueued\n", esim_time, stack->id, stack->tag, 	stack->target_mod->dram_system->name);
			linked_list_add(ds->pending_reads, stack);
			//dram_system_add_read_trans(ds->handler, stack->tag >> 2, stack->wavefront->wavefront_pool_entry->wavefront_pool->compute_unit->id, stack->wavefront->id);
			dram_system_add_write_trans(ds->handler, stack->addr, stack->wavefront->wavefront_pool_entry->wavefront_pool->compute_unit->id, stack->wavefront->id);

			stack->dramsim_mm_start = asTiming(si_gpu)->cycle ;
			/* Ctx main memory stats */
			assert(!stack->prefetch);
			//ctx->mm_read_accesses++;

			return;
		 }

		/* Check that addr is a multiple of mod.block_size.
		 * Set mod as sharer and owner. */
                if(!stack->uncacheable)
                {
                    dir = target_mod->dir;
                    for (z = 0; z < dir->zsize; z++)
                    {
                            assert(stack->addr % return_mod->block_size == 0);
                            dir_entry_tag = stack->tag + z * target_mod->sub_block_size;
                            assert(dir_entry_tag < stack->tag + target_mod->block_size);
                            if (dir_entry_tag < stack->addr || dir_entry_tag >= stack->addr + return_mod->block_size)
                                    continue;
                            dir_entry = dir_entry_get(dir, stack->set, stack->way, z, stack->dir_entry->w);
                            dir_entry_set_sharer(dir_entry, return_mod->low_net_node->index);
                            dir_entry_set_owner(dir_entry, return_mod->low_net_node->index);
                            assert(dir_entry->num_sharers == 1);
                    }

                    /* Set state to exclusive */
                    //cache_set_block(target_mod->cache, stack->set, stack->way,
                    //        stack->tag, cache_block_exclusive);
                    cache_set_block_new(target_mod->cache, stack, cache_block_exclusive);

                    /* If blocks were sent directly to the peer, the reply size would
                     * have been decreased.  Based on the final size, we can tell whether
                     * to send more data up or simply ack */
                    if (stack->reply_size == 8)
                    {
                            mod_stack_set_reply(ret, reply_ack);
                    }
                    else if (stack->reply_size > 8)
                    {
                            mod_stack_set_reply(ret, reply_ack_data);
                    }
                    else
                    {
                            fatal("Invalid reply size: %d", stack->reply_size);
                    }
                }
                
		if (stack->mshr_locked != 0)
		{
			mshr_unlock(target_mod, stack);
			stack->mshr_locked = 0;
		}

		/* Unlock, reply_size is the data of the size of the requester's block. */
                if(stack->dir_entry->dir_lock->stack == stack)
                    dir_entry_unlock(stack->dir_entry);
                else if(target_mod->cache->extra_dir_structure_type == extra_dir_per_cache_line)
                    assert(stack->uncacheable);

		stack->dramsim_mm_start = asTiming(si_gpu)->cycle ;

		//int latency = stack->reply == reply_ack_data_sent_to_peer ? 0 : target_mod->latency;
		int latency = target_mod->latency;
		stack->event = EV_MOD_NMOESI_WRITE_REQUEST_REPLY;
		esim_schedule_mod_stack_event(stack, latency);
		//esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST_REPLY, stack, latency);
		return;
	}

	if (event == EV_MOD_NMOESI_WRITE_REQUEST_DOWNUP)
	{
		mem_debug("  %lld %lld 0x%x %s write request downup\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:write_request_downup\"\n",
			stack->id, target_mod->name);

		assert(stack->dir_entry->state != cache_block_invalid);
		assert(!dir_entry_group_shared_or_owned(target_mod->dir, stack->dir_entry));

		/* Compute reply size */
		if (stack->dir_entry->state == cache_block_exclusive ||
			stack->dir_entry->state == cache_block_shared)
		{
			/* Exclusive and shared states send an ack */
			stack->reply_size = 8;
			mod_stack_set_reply(ret, reply_ack);
		}
		else if (stack->dir_entry->state == cache_block_noncoherent)
		{
			/* Non-coherent state sends data */
			stack->reply_size = target_mod->block_size + 8;
			mod_stack_set_reply(ret, reply_ack_data);
		}
		else if (stack->dir_entry->state == cache_block_modified ||
			stack->dir_entry->state == cache_block_owned)
		{
			/*if (stack->peer)
			{*/
				/* Modified or owned entries send data directly to peer
				 * if it exists */
				/*mod_stack_set_reply(ret, reply_ack_data_sent_to_peer);
				stack->reply_size = 8;
				*/
				/* This control path uses an intermediate stack that disappears, so
				 * we have to update the return stack of the return stack */
				/*ret->ret_stack->reply_size -= target_mod->block_size;
				assert(ret->ret_stack->reply_size >= 8);
				*/
				/* Send data to the peer */
				/*new_stack = mod_stack_create(stack->id, target_mod, stack->tag,
					EV_MOD_NMOESI_WRITE_REQUEST_DOWNUP_FINISH, stack);
				new_stack->peer = mod_stack_set_peer(stack->peer, stack->state);
				new_stack->target_mod = stack->target_mod;

				esim_schedule_event(EV_MOD_NMOESI_PEER_SEND, new_stack, 0);
				return;
			}
			else
			{*/
				/* If peer does not exist, data is returned to mod */
				mod_stack_set_reply(ret, reply_ack_data);
				stack->reply_size = target_mod->block_size + 8;
			//}
		}
		else
		{
			fatal("Invalid cache block state: %d\n", stack->dir_entry->state);
		}
		stack->event = EV_MOD_NMOESI_WRITE_REQUEST_DOWNUP_FINISH;
		esim_schedule_mod_stack_event(stack, 0);
		//esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST_DOWNUP_FINISH, stack, 0);

		return;
	}

	if (event == EV_MOD_NMOESI_WRITE_REQUEST_DOWNUP_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s write request downup complete\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:write_request_downup_finish\"\n",
			stack->id, target_mod->name);

		if (stack->mshr_locked != 0)
		{
			mshr_unlock(target_mod, stack);
			stack->mshr_locked = 0;
		}

		/* Set state to I, unlock*/
		//cache_set_block(target_mod->cache, stack->set, stack->way, 0, cache_block_invalid);
		cache_set_block_new(target_mod->cache, stack, cache_block_invalid);
                if(stack->dir_entry->dir_lock->stack == stack)
                    dir_entry_unlock(stack->dir_entry);
                else if(target_mod->cache->extra_dir_structure_type == extra_dir_per_cache_line)
                    assert(stack->uncacheable);

		//int latency = ret->reply == reply_ack_data_sent_to_peer ? 0 : target_mod->latency;
		int latency = target_mod->latency;
		stack->event = EV_MOD_NMOESI_WRITE_REQUEST_REPLY;
		esim_schedule_mod_stack_event(stack, latency);
		//esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST_REPLY, stack, latency);
		return;
	}

	if (event == EV_MOD_NMOESI_WRITE_REQUEST_REPLY)
	{
		struct net_t *net;
		struct net_node_t *src_node;
		struct net_node_t *dst_node;

		mem_debug("  %lld %lld 0x%x %s write request reply\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:write_request_reply\"\n",
			stack->id, target_mod->name);

		/* Checks */
		assert(stack->reply_size);
		assert(mod_get_low_mod(return_mod, stack->addr) == target_mod ||
			mod_get_low_mod(target_mod, stack->addr) == return_mod);


		if(stack->main_memory_accessed != 1)
		{
			stack->main_memory_accessed = 1;
			if(stack->uop)
                        {
                            stack->uop->mem_mm_latency += asTiming(si_gpu)->cycle  - stack->dramsim_mm_start;
                            stack->uop->mem_mm_accesses++;
                        }
		}

		/* Get network and nodes */
		if (stack->request_dir == mod_request_up_down)
		{
			net = return_mod->low_net;
			src_node = target_mod->high_net_node;
			dst_node = return_mod->low_net_node;
		}
		else
		{
			net = return_mod->high_net;
			src_node = target_mod->low_net_node;
			dst_node = return_mod->high_net_node;
		}

		stack->msg = net_try_send_ev(net, src_node, dst_node, stack->reply_size,
			EV_MOD_NMOESI_WRITE_REQUEST_FINISH, stack, event, stack);
		return;
	}

	if (event == EV_MOD_NMOESI_WRITE_REQUEST_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s write request finish\n", esim_time, stack->id,
			stack->tag, return_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:write_request_finish\"\n",
			stack->id, return_mod->name);

		/* Receive message */
		if (stack->request_dir == mod_request_up_down)
		{
			net_receive(return_mod->low_net, return_mod->low_net_node, stack->msg);
		}
		else
		{
			net_receive(return_mod->high_net, return_mod->high_net_node, stack->msg);
		}


		/* Return */
		mod_stack_return(stack);
		return;
	}

	abort();
}


void mod_handler_nmoesi_peer(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_t *src = stack->target_mod;
	struct mod_t *peer = stack->peer;

	if (event == EV_MOD_NMOESI_PEER_SEND)
	{
		mem_debug("  %lld %lld 0x%x %s %s peer send\n", esim_time, stack->id,
			stack->tag, src->name, peer->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:peer\"\n",
			stack->id, src->name);

		fatal("mod_handler_nmoesi_peer_event");

		/* Send message from src to peer */
		stack->msg = net_try_send_ev(src->low_net, src->low_net_node, peer->low_net_node,
			src->block_size + 8, EV_MOD_NMOESI_PEER_RECEIVE, stack, event, stack);

		return;
	}

	if (event == EV_MOD_NMOESI_PEER_RECEIVE)
	{
		mem_debug("  %lld %lld 0x%x %s %s peer receive\n", esim_time, stack->id,
			stack->tag, src->name, peer->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:peer_receive\"\n",
			stack->id, peer->name);

		/* Receive message from src */
		net_receive(peer->low_net, peer->low_net_node, stack->msg);

		stack->event = EV_MOD_NMOESI_PEER_REPLY;
		esim_schedule_mod_stack_event(stack, 0);
		//esim_schedule_event(EV_MOD_NMOESI_PEER_REPLY, stack, 0);

		return;
	}

	if (event == EV_MOD_NMOESI_PEER_REPLY)
	{
		mem_debug("  %lld %lld 0x%x %s %s peer reply ack\n", esim_time, stack->id,
			stack->tag, src->name, peer->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:peer_reply_ack\"\n",
			stack->id, peer->name);

		/* Send ack from peer to src */
		stack->msg = net_try_send_ev(peer->low_net, peer->low_net_node, src->low_net_node,
				8, EV_MOD_NMOESI_PEER_FINISH, stack, event, stack);

		return;
	}

	if (event == EV_MOD_NMOESI_PEER_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s %s peer finish\n", esim_time, stack->id,
			stack->tag, src->name, peer->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:peer_finish\"\n",
			stack->id, src->name);

		/* Receive message from src */
		net_receive(src->low_net, src->low_net_node, stack->msg);

		mod_stack_return(stack);
		return;
	}

	abort();
}


void mod_handler_nmoesi_invalidate(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *new_stack;

	struct mod_t *target_mod = stack->target_mod;

	struct dir_t *dir;
	struct dir_entry_t *dir_entry;

	uint32_t dir_entry_tag;
	uint32_t z;
	uint32_t aux = 1;

	if (event == EV_MOD_NMOESI_INVALIDATE)
	{
		struct mod_t *sharer;
		int i;

		/* Get block info */
		//cache_get_block(target_mod->cache, stack->set, stack->way, &stack->tag, NULL);
                stack->addr = stack->dir_entry->tag;
                stack->tag = stack->dir_entry->tag;
		mem_debug("  %lld %lld 0x%x %s invalidate (set=%d, way=%d, state=%s)\n", esim_time, stack->id,
			stack->tag, target_mod->name, stack->set, stack->way,
			str_map_value(&cache_block_state_map, stack->dir_entry->state));
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:invalidate\"\n",
			stack->id, target_mod->name);

		/* At least one pending reply */
		stack->pending = 1;

		/* Send write request to all upper level sharers except 'except_mod' */
		dir = target_mod->dir;
		for (z = 0; z < dir->zsize; z++)
		{
			dir_entry_tag = stack->tag + z * target_mod->sub_block_size;
			assert(dir_entry_tag < stack->tag + target_mod->block_size);
                        dir_entry = dir_entry_get(dir, stack->dir_entry->x, stack->dir_entry->y, z,stack->dir_entry->w);
			for (i = 0; i < dir->num_nodes; i++)
			{
				struct net_node_t *node;

				/* Skip non-sharers and 'except_mod' */
				if (!dir_entry_is_sharer(dir_entry, i))
					continue;

				node = list_get(target_mod->high_net->node_list, i);
				sharer = node->user_data;
				if (sharer == stack->except_mod)
					continue;

				/* Clear sharer and owner */
				dir_entry_clear_sharer(dir_entry, i);
				if (dir_entry->owner == i)
					dir_entry_set_owner(dir_entry, DIR_ENTRY_OWNER_NONE);

				/* Send write request upwards if beginning of block */
				if (dir_entry_tag % sharer->block_size)
					continue;

				if(aux)
				{
					aux=0;
					target_mod->evictions_with_sharers++;
				}
				target_mod->evictions_sharers_invalidation++;

				new_stack = mod_stack_create(stack->id, sharer, dir_entry_tag,
					EV_MOD_NMOESI_INVALIDATE_FINISH, stack);
				new_stack->wavefront = stack->wavefront;
				new_stack->retry = stack->retry;
				new_stack->uop = stack->uop;
				new_stack->return_mod = target_mod;
				new_stack->request_dir = mod_request_down_up;

				//FRAN
				estadis[target_mod->level - 1].invalidations++;
				add_invalidation(target_mod->level - 1);
				new_stack->event = EV_MOD_NMOESI_WRITE_REQUEST;
				esim_schedule_mod_stack_event(new_stack, 0);
				//esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST, new_stack, 0);
				stack->pending++;
			}
		}
			stack->event = EV_MOD_NMOESI_INVALIDATE_FINISH;
			esim_schedule_mod_stack_event(stack, 0);

		//esim_schedule_event(EV_MOD_NMOESI_INVALIDATE_FINISH, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_INVALIDATE_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s invalidate finish\n", esim_time, stack->id,
			stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:invalidate_finish\"\n",
			stack->id, target_mod->name);

		if (stack->reply == reply_ack_data)
                {
			//cache_set_block(target_mod->cache, stack->set, stack->way, stack->tag,
			//	cache_block_modified);
                        cache_set_block_new(target_mod->cache, stack, cache_block_modified);
                }
		/* Ignore while pending */
		assert(stack->pending > 0);
		stack->pending--;
		if (stack->pending)
		{
			return;
		}
		mod_stack_return(stack);
		return;
	}

	abort();
}

void mod_handler_nmoesi_message(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *ret = stack->ret_stack;
	//struct mod_stack_t *new_stack;

	struct mod_t *return_mod = stack->return_mod;
	struct mod_t *target_mod = stack->target_mod;

	struct dir_t *dir;
	struct dir_entry_t *dir_entry;
	uint32_t z;

	if (event == EV_MOD_NMOESI_MESSAGE)
	{
		struct net_t *net;
		struct net_node_t *src_node;
		struct net_node_t *dst_node;

		mem_debug("  %lld %lld 0x%x %s message\n", esim_time, stack->id,
			stack->addr, return_mod->name);

		stack->reply_size = 8;
		stack->reply = reply_ack;

		/* Default return values*/
		ret->err = 0;

		/* Checks */
		assert(stack->message);

		/* Get source and destination nodes */
		net = return_mod->low_net;
		src_node = return_mod->low_net_node;
		dst_node = target_mod->high_net_node;

		/* Send message */
		stack->msg = net_try_send_ev(net, src_node, dst_node, 8,
			EV_MOD_NMOESI_MESSAGE_RECEIVE, stack, event, stack);
		return;
	}
	if (event == EV_MOD_NMOESI_MESSAGE_RECEIVE)
	{
		mem_debug("  %lld %lld 0x%x %s message receive\n", esim_time, stack->id,
			stack->addr, target_mod->name);

		/* Receive message */
		net_receive(target_mod->high_net, target_mod->high_net_node, stack->msg);

		/* Find and lock */
		/*
		new_stack = mod_stack_create(stack->id, target_mod, stack->addr,
			EV_MOD_NMOESI_MESSAGE_ACTION, stack);
		new_stack->wavefront = stack->wavefront;
		new_stack->retry = stack->retry;
		new_stack->uop = stack->uop;
		new_stack->message = stack->message;
		new_stack->blocking = 0;
		new_stack->event = EV_MOD_NMOESI_FIND_AND_LOCK;
		*/
                if (stack->message == message_abort_access)
		{
                    /*struct dir_lock_t *dir_lock;
                    struct mod_t *mod = stack->target_mod; 
                    int set;
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
                    
                        dir_lock = dir_lock_get(target_mod->dir, set, stack->way);
			
			struct dir_t *dir = target_mod->dir;
			for (z = 0; z < dir->zsize; z++)
			{
                                dir_entry = dir_entry_get(dir, stack->set, stack->way, z);
                                       
                                struct mod_stack_t * stack = dir_lock->lock_queue;
                                stack->event = stack->dir_lock_event;
                                stack->dir_lock_event = 0;
                                dir_lock->lock_queue = stack->dir_lock_next;
                                stack->dir_lock_next = NULL;
                                esim_schedule_mod_stack_event(stack, 1);
			}*/
		}

		stack->blocking = 0;
		stack->event = EV_MOD_NMOESI_FIND_AND_LOCK;
		stack->find_and_lock_return_event = EV_MOD_NMOESI_MESSAGE_ACTION;
		esim_schedule_mod_stack_event(stack, 0);
		//esim_schedule_mod_stack_event(new_stack, 0);
		//esim_schedule_event(EV_MOD_NMOESI_FIND_AND_LOCK, new_stack, 0);
		return;
	}
	if (event == EV_MOD_NMOESI_MESSAGE_ACTION)
	{
		mem_debug("  %lld %lld 0x%x %s clear owner action\n", esim_time, stack->id,
			stack->tag, target_mod->name);

		assert(stack->message);

		/* Check block locking error. */
		mem_debug("stack err = %u\n", stack->err);
		if (stack->err)
		{
			if (stack->mshr_locked != 0)
			{
				mshr_unlock(target_mod, stack);
				stack->mshr_locked = 0;
			}

			ret->err = 1;
			mod_stack_set_reply(ret, reply_ack_error);
			stack->event = EV_MOD_NMOESI_MESSAGE_REPLY;
			esim_schedule_mod_stack_event(stack, 0);
			//esim_schedule_event(EV_MOD_NMOESI_MESSAGE_REPLY, stack, 0);
			return;
		}
                /*if (stack->message == message_evict_block)
                {
                    if(stack->err)
                    {
                        stack->err = 0;
			stack->event = EV_MOD_NMOESI_FIND_AND_LOCK;
			stack->find_and_lock_return_event = EV_MOD_NMOESI_MESSAGE_ACTION;
                        esim_schedule_mod_stack_event(stack, 5);                        
                    }
                    
                    if(stack->hit)
                    {
                        stack->event = EV_MOD_NMOESI_EVICT;
                        esim_schedule_mod_stack_event(new_stack, 0);
                    }else{
                        mod_stack_return(stack);
                    } 
                }*/
		if (stack->message == message_clear_owner)
		{
			/* Remove owner */
			dir = target_mod->dir;
			for (z = 0; z < dir->zsize; z++)
			{
				/* Skip other subblocks */
				if (stack->addr == stack->tag + z * target_mod->sub_block_size)
				{
					/* Clear the owner */
					dir_entry = dir_entry_get(dir, stack->dir_entry->set, stack->dir_entry->way, z, stack->dir_entry->w);
					assert(dir_entry->owner == return_mod->low_net_node->index);
					dir_entry_set_owner(dir_entry, DIR_ENTRY_OWNER_NONE);
				}
			}
		}
		else
		{
			fatal("Unexpected message");
		}

		if (stack->mshr_locked != 0)
		{
			mshr_unlock(target_mod, stack);
			stack->mshr_locked = 0;
		}

		/* Unlock the directory entry */
                if(stack->dir_entry->dir_lock->stack == stack)
                    dir_entry_unlock(stack->dir_entry);
                else if(target_mod->cache->extra_dir_structure_type == extra_dir_per_cache_line)
                    assert(stack->uncacheable);

		stack->event = EV_MOD_NMOESI_MESSAGE_REPLY;
		esim_schedule_mod_stack_event(stack, 0);
		//esim_schedule_event(EV_MOD_NMOESI_MESSAGE_REPLY, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_MESSAGE_REPLY)
	{
		struct net_t *net;
		struct net_node_t *src_node;
		struct net_node_t *dst_node;

		mem_debug("  %lld %lld 0x%x %s message reply\n", esim_time, stack->id,
			stack->tag, target_mod->name);

		/* Checks */
		assert(mod_get_low_mod(return_mod, stack->addr) == target_mod ||
			mod_get_low_mod(target_mod, stack->addr) == return_mod);

		/* Get network and nodes */
		net = return_mod->low_net;
		src_node = target_mod->high_net_node;
		dst_node = return_mod->low_net_node;

		/* Send message */
		stack->msg = net_try_send_ev(net, src_node, dst_node, stack->reply_size,
			EV_MOD_NMOESI_MESSAGE_FINISH, stack, event, stack);
		return;
	}

	if (event == EV_MOD_NMOESI_MESSAGE_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s message finish\n", esim_time, stack->id,
			stack->tag, return_mod->name);

		/* Receive message */
		net_receive(return_mod->low_net, return_mod->low_net_node, stack->msg);

		/* Return */
		mod_stack_return(stack);
		return;
	}

	abort();
}
