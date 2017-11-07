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

int EV_MOD_VI_LOAD;
int EV_MOD_VI_LOAD_SEND;
int EV_MOD_VI_LOAD_RECEIVE;
int EV_MOD_VI_LOAD_LOCK;
int EV_MOD_VI_LOAD_LOCK2;
int EV_MOD_VI_LOAD_ACTION;
int EV_MOD_VI_LOAD_MISS;
int EV_MOD_VI_LOAD_UNLOCK;
int EV_MOD_VI_LOAD_FINISH;

int EV_MOD_VI_NC_LOAD;

int EV_MOD_VI_STORE;
int EV_MOD_VI_NC_STORE;
int EV_MOD_VI_STORE_SEND;
int EV_MOD_VI_STORE_RECEIVE;
int EV_MOD_VI_STORE_LOCK;
int EV_MOD_VI_STORE_ACTION;
int EV_MOD_VI_STORE_UNLOCK;
int EV_MOD_VI_STORE_FINISH;

int EV_MOD_VI_FIND_AND_LOCK;
int EV_MOD_VI_FIND_AND_LOCK_PORT;
int EV_MOD_VI_FIND_AND_LOCK_ACTION;
int EV_MOD_VI_FIND_AND_LOCK_WAIT_MSHR;
int EV_MOD_VI_FIND_AND_LOCK_FINISH;

/* VI Protocol */


void mod_handler_vi_load(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *new_stack;
	//, *older_stack;

	struct mod_t *mod = stack->target_mod;
	struct net_t *net;
	struct net_node_t *src_node;
	struct net_node_t *dst_node;
	int return_event;
	//int retry_lat;


	if (event == EV_MOD_VI_LOAD || event == EV_MOD_VI_NC_LOAD)
	{
		struct mod_stack_t *master_stack;

		mem_debug("%lld %lld 0x%x %s load\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.new_access name=\"A-%lld\" type=\"load\" "
			"state=\"%s:load\" addr=0x%x\n",
			stack->id, mod->name, stack->addr);

		/* proba para accesosos*/
		/*
		if(mod->level == 1 && stack->wavefront->wavefront_pool_entry->id_in_wavefront_pool != (asTiming(si_gpu)->cycle/ 10)%10){
			long long wait = ((asTiming(si_gpu)->cycle - asTiming(si_gpu)->cycle%100)  +(stack->wavefront->wavefront_pool_entry->id_in_wavefront_pool * 10)) - asTiming(si_gpu)->cycle;
			if(wait < 0)
				wait += 100;

			stack->event = event;
			esim_schedule_mod_stack_event(stack, wait);
			return;
		}*/

		//if(event == EV_MOD_VI_LOAD)
		//	stack->glc = 1;

		/* Record access */
		//mod_access_start(mod, stack, mod_access_load);
		//add_access(mod->level);
		//FRAN
		mod->loads++;

		/* Coalesce access */
		stack->origin = 1;
		master_stack = mod_can_coalesce(mod, mod_access_load, stack->addr, NULL);

		if ((stack->target_mod->level == 1 && ((!flag_coalesce_gpu_enabled && master_stack) || ((stack->target_mod->compute_unit->scalar_cache == mod) && master_stack))) || (stack->target_mod->level != 1 && master_stack))
		{
			mod_access_start(mod, stack, mod_access_load);
			mod->hits_aux++;
			mod->delayed_read_hit++;
			//add_access(mod->level);
			add_coalesce(mod->level);
			add_coalesce_load(mod->level);
			mod->reads++;

			mod_coalesce(mod, master_stack, stack);
			mod_stack_wait_in_stack(stack, master_stack, EV_MOD_VI_LOAD_FINISH);
			return;
		}

		mod_access_start(mod, stack, mod_access_load);
		add_access(mod->level);

		/* Next event */
		stack->event = EV_MOD_VI_LOAD_LOCK;
		esim_schedule_mod_stack_event(stack, 0);
		//esim_schedule_event(EV_MOD_VI_LOAD_LOCK, stack, 0);
		return;
	}

	if(event == EV_MOD_VI_LOAD_SEND)
	{
		int msg_size;
		if(stack->request_dir == mod_request_up_down)
		{
			//new_stack = mod_stack_create(stack->id, mod_get_low_mod(mod, stack->addr), stack->addr, EV_MOD_VI_LOAD_FINISH, stack);

			if(stack->reply_size)
				msg_size = stack->reply_size;
			else
				msg_size = 8;

			//stack->request_dir = mod_request_up_down;

			net = mod->high_net;
			src_node = stack->ret_stack->target_mod->low_net_node;
			dst_node = mod->high_net_node;
			return_event = EV_MOD_VI_LOAD_RECEIVE;
			stack->msg = net_try_send_ev(net, src_node, dst_node, msg_size,
				return_event, stack, event, stack);
		}
		else if(stack->request_dir == mod_request_down_up)
		{
			//if(stack->reply_size)
			//	msg_size = stack->reply_size;
			//else
			msg_size = 72;

			net = mod->low_net;
      src_node = mod_get_low_mod(mod, stack->addr)->high_net_node;
      dst_node = mod->low_net_node;
			return_event = EV_MOD_VI_LOAD_RECEIVE;
			stack->msg = net_try_send_ev(net, src_node, dst_node, msg_size,
				return_event, stack, event, stack);
		}
		else
		{
			fatal("Invalid request_dir in EV_MOD_VI_LOAD_SEND");
		}
		return;
	}
	if(event == EV_MOD_VI_LOAD_RECEIVE)
	{
		if(stack->request_dir == mod_request_up_down)
    {
			net_receive(mod->high_net, mod->high_net_node, stack->msg);
			stack->event = EV_MOD_VI_LOAD;
			esim_schedule_mod_stack_event(stack, 0);
			//esim_schedule_event(EV_MOD_VI_LOAD, stack, 0);
		}
    else if(stack->request_dir == mod_request_down_up)
    {
			net_receive(mod->low_net, mod->low_net_node, stack->msg);
			if(SALTAR_L1 && mod->level == 1)
			{
				stack->event = EV_MOD_VI_LOAD_UNLOCK;
				esim_schedule_mod_stack_event(stack, 0);
				//esim_schedule_event(EV_MOD_VI_LOAD_UNLOCK, stack, 0);
			}else{
				stack->event = EV_MOD_VI_LOAD_MISS;
				esim_schedule_mod_stack_event(stack, 0);
				//esim_schedule_event(EV_MOD_VI_LOAD_MISS, stack, 0);
			}
		}
		else
    {
			fatal("Invalid request_dir in EV_MOD_VI_LOAD_RECEIVE");
    }
		return;
	}
if (event == EV_MOD_VI_LOAD_LOCK)
{
	struct mod_stack_t *older_stack;

	mem_debug("  %lld %lld 0x%x %s load lock\n", esim_time, stack->id,
		stack->addr, mod->name);
	mem_trace("mem.access name=\"A-%lld\" state=\"%s:load_lock\"\n",
		stack->id, mod->name);

	if(stack->latencias.start == 0)
		if(mod->level == 1 && stack->client_info && stack->client_info->arch)
			stack->latencias.start = stack->client_info->arch->timing->cycle;

	/* If there is any older access to the same address that this access could not
	 * be coalesced with, wait for it. */
	//older_stack = mod_in_flight_write_fran(mod, stack);

	older_stack = mod_in_flight_write(mod, stack);
	if (mod->level == 1 && older_stack)
	{
		mem_debug("    %lld wait for access %lld\n",
			stack->id, older_stack->id);
		mod_stack_wait_in_stack(stack, older_stack, EV_MOD_VI_LOAD_LOCK);
		return;
	}

	older_stack = mod_in_flight_address(mod, stack->addr, stack);
	if (mod->level == 1 && older_stack)
	{
		mem_debug("    %lld wait for access %lld\n",
			stack->id, older_stack->id);
			//assert(!older_stack->waiting_list_event);
		mod_stack_wait_in_stack(stack, older_stack, EV_MOD_VI_LOAD_LOCK);
		return;
	}

  if(SALTAR_L1 && mod->level == 1)
	{
		stack->request_dir = mod_request_down_up;
		new_stack = mod_stack_create(stack->id, mod_get_low_mod(mod, stack->addr), stack->addr, EV_MOD_VI_LOAD_SEND, stack);
       	new_stack->reply_size = 8;
       	new_stack->request_dir= mod_request_up_down;
       	new_stack->valid_mask = stack->valid_mask;

		stack->reply_size = 8 + mod->block_size;
		new_stack->event = EV_MOD_VI_LOAD_SEND;
		esim_schedule_mod_stack_event(new_stack, 0);
    //   	esim_schedule_event(EV_MOD_VI_LOAD_SEND, new_stack, 0);
		return;
	}

	if(mod->level == 1 && stack->client_info && stack->client_info->arch)
	{
		stack->latencias.queue = stack->client_info->arch->timing->cycle - stack->latencias.start;
	}

	/* Call find and lock */
	/*new_stack = mod_stack_create(stack->id, mod, stack->addr,
		EV_MOD_VI_LOAD_ACTION, stack);
	new_stack->wavefront = stack->wavefront;
	new_stack->uop = stack->uop;
	new_stack->blocking = 1;
	new_stack->read = 1;
	new_stack->valid_mask = stack->valid_mask;
	new_stack->tiempo_acceso = stack->tiempo_acceso;
	new_stack->retry = stack->retry;
	stack->find_and_lock_stack = new_stack;
	new_stack->event = EV_MOD_VI_FIND_AND_LOCK;
	esim_schedule_mod_stack_event(new_stack, 0);*/
	//esim_schedule_event(EV_MOD_VI_FIND_AND_LOCK, new_stack, 0);
	stack->event = EV_MOD_VI_LOAD_LOCK2;
	esim_schedule_mod_stack_event(stack, 0);
	return;
}

if (event == EV_MOD_VI_LOAD_LOCK2)
{

	mem_debug("  %lld %lld 0x%x %s load lock2\n", esim_time, stack->id,
		stack->addr, mod->name);
	mem_trace("mem.access name=\"A-%lld\" state=\"%s:load_lock2\"\n",
		stack->id, mod->name);

	if(stack->client_info && stack->client_info->arch){
		stack->latencias.queue = stack->client_info->arch->timing->cycle - stack->latencias.start;
	}
	/* Call find and lock */

	/*new_stack = mod_stack_create(stack->id, mod, stack->addr,
		EV_MOD_VI_LOAD_ACTION, stack);
	new_stack->wavefront = stack->wavefront;
	new_stack->uop = stack->uop;
	new_stack->blocking = 1;
	new_stack->read = 1;
	new_stack->valid_mask = stack->valid_mask;
	new_stack->tiempo_acceso = stack->tiempo_acceso;
	new_stack->retry = stack->retry;
	stack->find_and_lock_stack = new_stack;
	new_stack->event = EV_MOD_VI_FIND_AND_LOCK;*/

	stack->blocking = 1;
	stack->read = 1;
	stack->find_and_lock_return_event = EV_MOD_VI_LOAD_ACTION;
	stack->event = EV_MOD_VI_FIND_AND_LOCK;
	esim_schedule_mod_stack_event(stack, 0);
	return;
}

if (event == EV_MOD_VI_LOAD_ACTION)
{
	int retry_lat;

	mem_debug("  %lld %lld 0x%x %s load action\n", esim_time, stack->id,
		stack->addr, mod->name);
	mem_trace("mem.access name=\"A-%lld\" state=\"%s:load_action\"\n",
		stack->id, mod->name);

	/* Error locking */
	if (stack->err)
	{
		mod->read_retries++;
		add_retry(stack,load_action_retry);
		retry_lat = mod_get_retry_latency(mod);

		if(mod->level == 1 && stack->client_info && stack->client_info->arch){
			stack->latencias.retry += stack->client_info->arch->timing->cycle - stack->latencias.start + retry_lat;
		}

		stack->latencias.start = 0;

		if (stack->mshr_locked != 0)
		{
			mshr_unlock(mod, stack);
			stack->mshr_locked = 0;
		}

		mem_debug("    lock error, retrying in %d cycles\n", retry_lat);
		stack->retry = 1;
		stack->event = EV_MOD_VI_LOAD_LOCK2;
		esim_schedule_mod_stack_event(stack, retry_lat);
		//esim_schedule_event(EV_MOD_VI_LOAD_LOCK, stack, retry_lat);
		return;
	}

	//mem_stats.mod_level[mod->level].entradas_bloqueadas++;

	if(mod->level == 1)
	{
			/* Hit */
		if (stack->hit && (stack->glc == 0))
		{
			esim_schedule_event(EV_MOD_VI_LOAD_UNLOCK, stack, 0);
			//estadisticas(1, 0);

			//add_hit(mod->level);
			mod->hits_aux++;

			return;
		}

		/* MISS */
//		cache_set_block(mod->cache, stack->set, stack->way, stack->tag, cache_block_invalid);
		stack->dir_entry->state = 0;

		//add_miss(mod->level);
		//estadisticas(0, 0);

		new_stack = mod_stack_create(stack->id, mod_get_low_mod(mod, stack->addr), stack->addr, EV_MOD_VI_LOAD_SEND, stack);
		new_stack->reply_size = 8;
		new_stack->valid_mask = stack->valid_mask;
		stack->reply_size = 8 + mod->block_size;
		stack->request_dir = mod_request_down_up;
		new_stack->request_dir = mod_request_up_down;
		new_stack->wavefront = stack->wavefront;
		new_stack->uop = stack->uop;
		new_stack->retry = stack->retry;
		new_stack->event = EV_MOD_VI_LOAD_SEND;
		esim_schedule_mod_stack_event(new_stack, 0);
		//esim_schedule_event(EV_MOD_VI_LOAD_SEND, new_stack, 0);

		return;
	}
	else
	{
			/* Hit */
		if (stack->hit || mod->kind == mod_kind_main_memory)
		{
			/*
			estadisticas(1, 0);
			stack->ret_stack->valid_mask = mod_get_valid_mask(mod, stack->set, stack->way);
			//add_hit(mod->level);
			mod->hits_aux++;
			*/

			/* If another module has not given the block,
				 access main memory */
			if (mod->kind == mod_kind_main_memory
				&& mod->dram_system)
			{
				struct dram_system_t *ds = mod->dram_system;
				assert(ds);

				assert(stack->mshr_locked == 0);

				if (!dram_system_will_accept_trans(ds->handler, stack->addr))
				{
					//stack->err = 1;
					//ret->err = 1;
					//ret->retry |= 1 << mod->level;

					//mod_stack_set_reply(ret, reply_ack_error);
					//stack->reply_size = 8;

					//dir_entry_unlock(mod->dir, stack->set, stack->way);

					mem_debug("    %lld 0x%x %s mc queue full, retrying...\n", stack->id, stack->tag, mod->name);


					/*
					new_stack = mod_stack_create(stack->id, mod, stack->addr,
						EV_MOD_VI_LOAD_ACTION, stack);
					new_stack->wavefront = stack->wavefront;
					new_stack->uop = stack->uop;
					new_stack->blocking = 1;
					new_stack->read = 1;
					new_stack->valid_mask = stack->valid_mask;
					new_stack->tiempo_acceso = stack->tiempo_acceso;
					new_stack->retry = stack->retry;
					stack->find_and_lock_stack = new_stack;
					new_stack->event = EV_MOD_VI_FIND_AND_LOCK;
					*/
					stack->event = EV_MOD_VI_LOAD_ACTION;
					esim_schedule_mod_stack_event(stack, 10);

					//esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST_REPLY, stack, 5);
					return;
				}

				//estadisticas(1, 0);
				stack->ret_stack->valid_mask = mod_get_valid_mask(mod, stack->set, stack->way);
				//add_hit(mod->level);
				mod->hits_aux++;
				stack->event = EV_MOD_VI_LOAD_UNLOCK;

				/* Access main memory system */
				mem_debug("  %lld %lld 0x%x %s dram access enqueued\n", esim_time, stack->id, stack->tag, 	stack->target_mod->dram_system->name);
				linked_list_add(ds->pending_reads, stack);
				//dram_system_add_read_trans(ds->handler, stack->tag >> 2, stack->wavefront->wavefront_pool_entry->wavefront_pool->compute_unit->id, stack->wavefront->id);
				dram_system_add_read_trans(ds->handler, stack->addr, stack->wavefront->wavefront_pool_entry->wavefront_pool->compute_unit->id, stack->wavefront->id);

				stack->dramsim_mm_start = asTiming(si_gpu)->cycle ;
				/* Ctx main memory stats */
				assert(!stack->prefetch);
				//ctx->mm_read_accesses++;
				return;
			}

			//estadisticas(1, 0);
 			stack->ret_stack->valid_mask = mod_get_valid_mask(mod, stack->set, stack->way);
 			//add_hit(mod->level);
 			mod->hits_aux++;

			stack->event = EV_MOD_VI_LOAD_UNLOCK;
 			esim_schedule_mod_stack_event(stack, 0);
 			//esim_schedule_event(EV_MOD_VI_LOAD_UNLOCK, stack, 0);
			return;
		}

		/* MISS */
		//add_miss(mod->level);
		//estadisticas(0, 0);

		if(stack->eviction)
		{
			int tag = 0;
			cache_get_block(mod->cache, stack->set, stack->way, &tag, NULL);
			cache_set_block(mod->cache, stack->set, stack->way, 0, cache_block_invalid);

			new_stack = mod_stack_create(stack->id, mod_get_low_mod(mod, tag), tag,
			ESIM_EV_NONE, NULL);
			new_stack->src_mod = mod;
			new_stack->reply_size = 8 + mod->block_size;
			new_stack->request_dir = mod_request_up_down;
			new_stack->wavefront = stack->wavefront;
			new_stack->uop = stack->uop;
			new_stack->retry = stack->retry;
			new_stack->event = EV_MOD_VI_STORE_SEND;
			esim_schedule_mod_stack_event(new_stack, 0);
			//esim_schedule_event(EV_MOD_VI_STORE_SEND, new_stack, 0);
		}

		new_stack = mod_stack_create(stack->id, mod_get_low_mod(mod, stack->addr), stack->addr,
			EV_MOD_VI_LOAD_SEND, stack);
		new_stack->reply_size = 8;
		stack->reply_size = 8 + mod->block_size;
		stack->request_dir = mod_request_down_up;
		new_stack->request_dir = mod_request_up_down;
		new_stack->wavefront = stack->wavefront;
		new_stack->uop = stack->uop;
		new_stack->retry = stack->retry;
		new_stack->event = EV_MOD_VI_LOAD_SEND;
		esim_schedule_mod_stack_event(new_stack, 0);
		//esim_schedule_event(EV_MOD_VI_LOAD_SEND, new_stack, 0);

		return;
	}


		/* Miss */
/*	        if (mod->mshr_count >= mod->mshr_size)
                {
                        mod->read_retries++;
                        retry_lat = mod_get_retry_latency(mod);
                        dir_entry_unlock(mod->dir, stack->set, stack->way);
                        mem_debug("mshr full, retrying in %d cycles\n", retry_lat);
                        stack->retry = 1;
                        esim_schedule_event(EV_MOD_VI_LOAD_LOCK, stack, retry_lat);
                        return;
                }*/
		//mod->mshr_count++;


	}

	if (event == EV_MOD_VI_LOAD_MISS)
	{
		//int retry_lat;

		mem_debug("  %lld %lld 0x%x %s load miss\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:load_miss\"\n",
			stack->id, mod->name);

		//Fran
		//stack->mod->mshr_count--;

		/* Error on read request. Unlock block and retry load. */
		assert(!stack->err);

		if(mod->level == 1 && stack->client_info && stack->client_info->arch)
		{
			stack->latencias.miss = stack->client_info->arch->timing->cycle - stack->latencias.start - stack->latencias.queue - stack->latencias.lock_mshr - stack->latencias.lock_dir - stack->latencias.eviction;
		}
		/* Set block state to excl/shared depending on return var 'shared'.
		 * Also set the tag of the block. */
		cache_set_block(mod->cache, stack->set, stack->way, stack->tag, cache_block_valid);
		cache_write_block_valid_mask(mod->cache, stack->set, stack->way, stack->valid_mask);
		/* Continue */
		stack->event = EV_MOD_VI_LOAD_UNLOCK;
		esim_schedule_mod_stack_event(stack, 0);
		//esim_schedule_event(EV_MOD_VI_LOAD_UNLOCK, stack, 0);
		return;
	}

	if (event == EV_MOD_VI_LOAD_UNLOCK)
	{
		mem_debug("  %lld %lld 0x%x %s load unlock\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:load_unlock\"\n",
			stack->id, mod->name);
		assert(!stack->err);
		//mem_stats.mod_level[mod->level].entradas_bloqueadas--;
		if (stack->mshr_locked != 0)
		{
			mshr_unlock(mod, stack);
			stack->mshr_locked = 0;
		}

		/* Unlock directory entry */
		dir_entry_unlock(stack->dir_entry);

		/* Impose the access latency before continuing */

		stack->event = EV_MOD_VI_LOAD_FINISH;

                stack->reply_size = mod->block_size + 8;
		if(mod->kind == mod_kind_main_memory
			&& mod->dram_system)
		{
			esim_schedule_mod_stack_event(stack, 0);
		}else{
			esim_schedule_mod_stack_event(stack, mod->latency);
		}
		//esim_schedule_event(EV_MOD_VI_LOAD_FINISH, stack,	mod->latency);

		return;
	}

	if (event == EV_MOD_VI_LOAD_FINISH)
	{

		mem_debug("%lld %lld 0x%x %s load finish\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:load_finish\"\n",
			stack->id, mod->name);
		mem_trace("mem.end_access name=\"A-%lld\"\n",
			stack->id);
		long long ciclo = 0;
		//fran_debug("%lld %lld\n",esim_time, stack->id);
	 	if(mod->level == 1)
		{
			if(stack->client_info && stack->client_info->arch)
				ciclo = stack->client_info->arch->timing->cycle;

			if(esim_time > stack->tiempo_acceso){

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

		if(!stack->coalesced && mod->level == 1)
		{
	    //mod->mshr_count--;
			//mod_stack_wakeup_mod_head(mod);

			if(mod == mod->compute_unit->vector_cache)
				mod->compute_unit->compute_device->interval_statistics->vcache_load_finish++;
			else
				mod->compute_unit->compute_device->interval_statistics->scache_load_finish++;

			if(stack->retry)
			{
				mod->compute_unit->compute_device->interval_statistics->cache_retry_lat += stack->latencias.retry;
				mod->compute_unit->compute_device->interval_statistics->cache_retry_cont++;
			}

			accu_retry_time_lost(stack);
			if(mod->level == 1 && stack->client_info && stack->client_info->arch){
				stack->latencias.finish = stack->client_info->arch->timing->cycle - stack->latencias.start - stack->latencias.queue - stack->latencias.lock_mshr - stack->latencias.lock_dir - stack->latencias.eviction - stack->latencias.miss;
			}

			add_latencias_load(stack);

			if(mod->level == 1 && stack->client_info && stack->client_info->arch && strcmp(stack->client_info->arch->name, "SouthernIslands") == 0)
			{
				copy_latencies_to_wavefront(&(stack->latencias),stack->wavefront);
			}
		}


		if(stack->hit && !(stack->latencias.queue + stack->latencias.lock_mshr + stack->latencias.lock_dir + stack->latencias.eviction + stack->latencias.miss))
		{
			add_CoalesceHit(mod->level);
		}
		else
		{
			add_CoalesceMiss(mod->level);
		}

		//if(stack->ret_stack)
		//	stack->ret_stack->reply_size = 72;
		/* Increment witness variable */
		if(mod->level == 1)
		{
			if (stack->witness_ptr)
				(*stack->witness_ptr)++;

			if((*stack->witness_ptr) == 0 && stack->client_info)
				stack->uop->mem_access_finish_cycle = stack->client_info->arch->timing->cycle;
		}
		/* Return event queue element into event queue */
		if (mod->level == 1 && stack->event_queue && stack->event_queue_item)
			linked_list_add(stack->event_queue, stack->event_queue_item);

		/* Free the mod_client_info object, if any */
		if (mod->level == 1 && stack->client_info){
			mod_client_info_free(mod, stack->client_info);
		}
		/* Finish access */
		mod_access_finish(mod, stack);

		/* Return */
		mod_stack_return(stack);

		return;
	}

	abort();
}



void mod_handler_vi_store(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *new_stack, *master_stack;
	//*older_stack, *master_stack;

	struct mod_t *mod = stack->target_mod;

  struct net_t *net;
  struct net_node_t *src_node;
  struct net_node_t *dst_node;
  int return_event;
	//int retry_lat;

	if (event == EV_MOD_VI_STORE || event == EV_MOD_VI_NC_STORE)
	{

		mem_debug("%lld %lld 0x%x %s store\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.new_access name=\"A-%lld\" type=\"store\" "
			"state=\"%s:store\" addr=0x%x\n",
			stack->id, mod->name, stack->addr);

		if(event == EV_MOD_VI_STORE)
			stack->glc = 1;

		/* Record access */
		//mod_access_start(mod, stack, mod_access_nc_store);

		/* Increment witness variable */
		if (stack->target_mod->level == 1 && stack->witness_ptr)
		{
			(*stack->witness_ptr)++;
			stack->witness_ptr = NULL;
		}
		if(mod->level == 1 && stack->client_info && stack->client_info->arch)
			stack->latencias.start = stack->client_info->arch->timing->cycle;

		if (stack->target_mod->level == 1)
			stack->origin = 1;

		master_stack = mod_can_coalesce(mod, mod_access_nc_store, stack->addr, stack);
		//mod_access_start(mod, stack, mod_access_store);
		if ((stack->target_mod->level == 1 && !flag_coalesce_gpu_enabled && master_stack) || (stack->target_mod->level != 1 && master_stack))
		{
			mod_access_start(mod, stack, mod_access_store);
			assert(master_stack->addr == stack->addr);
			mod->nc_writes++;
			mod_stack_merge_dirty_mask(master_stack, stack->dirty_mask);
			mod_coalesce(mod, master_stack, stack);
			mod_stack_wait_in_stack(stack, master_stack, EV_MOD_VI_STORE_FINISH);
			add_coalesce(mod->level);
			add_coalesce_store(mod->level);

			// Increment witness variable
			/*if (stack->witness_ptr)
				(*stack->witness_ptr)++;
			stack->witness_ptr = NULL;*/
			return;
		}

		/*if (mod->mshr_size && mod->mshr_count >= mod->mshr_size)
		{
			mod_stack_wait_in_mod(stack, mod, EV_MOD_VI_STORE);
			return;
		}
		mod->mshr_count++;*/
		add_access(mod->level);
		mod_access_start(mod, stack, mod_access_store);
		/* Continue */
		stack->event = EV_MOD_VI_STORE_LOCK;
		esim_schedule_mod_stack_event(stack, 0);
		//esim_schedule_event(EV_MOD_VI_STORE_LOCK, stack, 0);
		return;
	}
    if(event == EV_MOD_VI_STORE_SEND)
    {
		int msg_size;
        if(stack->request_dir == mod_request_up_down)
        {
			//stack->request_dir = mod_request_up_down;
            msg_size = 72;
            net = mod->high_net;
            src_node = stack->src_mod->low_net_node;
            dst_node = mod->high_net_node;
            return_event = EV_MOD_VI_STORE_RECEIVE;
            stack->msg = net_try_send_ev(net, src_node, dst_node, msg_size, return_event, stack, event, stack);
        }
        else if(stack->request_dir == mod_request_down_up)
        {
			msg_size = 8;
			net = mod->low_net;
			src_node = mod_get_low_mod(mod, stack->addr)->high_net_node;
			dst_node = mod->low_net_node;
			return_event = EV_MOD_VI_STORE_RECEIVE;
			stack->msg = net_try_send_ev(net, src_node, dst_node, msg_size, return_event, stack, event, stack);
        }
        else
        {
			fatal("Invalid request_dir in EV_MOD_VI_STORE_SEND");
        }
        return;
    }
    if(event == EV_MOD_VI_STORE_RECEIVE)
    {
        if(stack->request_dir == mod_request_up_down)
        {
			net_receive(mod->high_net, mod->high_net_node, stack->msg);
			stack->event = EV_MOD_VI_STORE;
			esim_schedule_mod_stack_event(stack, 0);
			//esim_schedule_event(EV_MOD_VI_STORE, stack, 0);
        }
        else if(stack->request_dir == mod_request_down_up)
        {
			net_receive(mod->low_net, mod->low_net_node, stack->msg);
			stack->event = EV_MOD_VI_STORE_UNLOCK;
			esim_schedule_mod_stack_event(stack, 0);
			//esim_schedule_event(EV_MOD_VI_STORE_UNLOCK, stack, 0);
        }
        else
        {
			fatal("Invalid request_dir in EV_MOD_VI_STORE_RECEIVE");
        }

        return;
    }

	if (event == EV_MOD_VI_STORE_LOCK)
	{
		struct mod_stack_t *older_stack;


		mem_debug("  %lld %lld 0x%x %s store lock\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:store_lock\"\n",
			stack->id, mod->name);

		if(stack->latencias.start == 0)
			if(mod->level == 1 && stack->client_info && stack->client_info->arch)
				stack->latencias.start = stack->client_info->arch->timing->cycle;

		//older_stack = mod_in_flight_write_fran(mod, stack);
		older_stack = mod_in_flight_write(mod, stack);
    if (mod->level == 1 && older_stack)
    {
			//assert(!older_stack->waiting_list_event);
      mem_debug("    %lld wait for write %lld\n", stack->id, older_stack->id);
      mod_stack_wait_in_stack(stack, older_stack, EV_MOD_VI_STORE_LOCK);
			return;
    }

		//mod->mshr_count++;

		if(SALTAR_L1 && mod->level == 1)
		{
			stack->request_dir = mod_request_down_up;
			new_stack = mod_stack_create(stack->id, mod_get_low_mod(mod, stack->addr), stack->addr, EV_MOD_VI_STORE_SEND, stack);
			new_stack->request_dir= mod_request_up_down;
			new_stack->src_mod = mod;
			new_stack->dirty_mask = stack->dirty_mask;

			// Increment witness variable
			/*if (stack->witness_ptr)
				(*stack->witness_ptr)++;
			stack->witness_ptr = NULL;*/
			stack->event = EV_MOD_VI_STORE_SEND;
			esim_schedule_mod_stack_event(new_stack, 0);
			//esim_schedule_event(EV_MOD_VI_STORE_SEND, new_stack, 0);
			return;

		}

		if(mod->level == 1 && stack->client_info && stack->client_info->arch){
			stack->latencias.queue = stack->client_info->arch->timing->cycle - stack->latencias.start;
		}

		/* Call find and lock */
		/*new_stack = mod_stack_create(stack->id, mod, stack->addr,
			EV_MOD_VI_STORE_ACTION, stack);
		new_stack->blocking = 1;
		new_stack->nc_write = 1;
		new_stack->retry = stack->retry;
		new_stack->wavefront = stack->wavefront;
		new_stack->uop = stack->uop;
		new_stack->witness_ptr = stack->witness_ptr;
		stack->witness_ptr = NULL;
		new_stack->event = EV_MOD_VI_FIND_AND_LOCK;*/

		stack->blocking = 1;
		stack->nc_write = 1;
		stack->find_and_lock_return_event = EV_MOD_VI_STORE_ACTION;
		stack->event = EV_MOD_VI_FIND_AND_LOCK;
		esim_schedule_mod_stack_event(stack, 0);
		//esim_schedule_event(EV_MOD_VI_FIND_AND_LOCK, new_stack, 0);

		return;
	}

	if (event == EV_MOD_VI_STORE_ACTION)
	{

		mem_debug("  %lld %lld 0x%x %s store action\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:store_action\"\n",
			stack->id, mod->name);

		/* Error locking */
		assert(!stack->err);

		if(mod->level == 1)
		{
			// write-through
			struct mod_t *target_mod = mod_get_low_mod(mod, stack->tag);
			new_stack = mod_stack_create(stack->id, target_mod, stack->tag, ESIM_EV_NONE, NULL);
			new_stack->src_mod = mod;
			new_stack->dirty_mask = stack->dirty_mask;
			new_stack->request_dir = mod_request_up_down;
			new_stack->event = EV_MOD_VI_STORE_SEND;
			esim_schedule_mod_stack_event(new_stack, 0);
			//esim_schedule_event(EV_MOD_VI_STORE_SEND, new_stack, 0);


			//da igual si es un hit o un miss

			//cache_clean_block_dirty(mod->cache, stack->set, stack->way);

			if(stack->glc == 0 && (~stack->dirty_mask) == 0)
			{
				if(stack->hit && stack->dir_entry->state)
					add_store_invalidation(1);

				//cache_set_block(mod->cache, stack->set, stack->way, stack->tag, cache_block_valid);
				//cache_write_block_valid_mask(mod->cache, stack->set, stack->way, stack->dirty_mask);
			}
			else if(stack->hit)
			{
				add_store_invalidation(1);
				//cache_set_block(mod->cache, stack->set, stack->way, stack->tag, cache_block_invalid);
			}

			stack->event = EV_MOD_VI_STORE_UNLOCK;
			esim_schedule_mod_stack_event(stack, 0);
			//esim_schedule_event(EV_MOD_VI_STORE_UNLOCK, stack, 0);
		}
		else
		{
			/* Hit - state=V*/
			/*if (stack->hit)
			{
				if(mod->kind != mod_kind_main_memory)
				{
					cache_write_block_dirty_mask(mod->cache, stack->set, stack->way, stack->dirty_mask);
					cache_write_block_valid_mask(mod->cache, stack->set, stack->way, stack->dirty_mask);
				}
				//esim_schedule_event(EV_MOD_VI_STORE_UNLOCK, stack, 0);
			}
			else
			{*/
			/* Miss - state=I */



			/* If another module has not given the block,
				 access main memory */
			/*
			if (mod->kind == mod_kind_main_memory
				&& mod->dram_system)
			{
				struct dram_system_t *ds = mod->dram_system;
				assert(ds);

				assert(stack->mshr_locked == 0);

				if (!dram_system_will_accept_trans(ds->handler, stack->addr))
				{
					//stack->err = 1;
					//ret->err = 1;
					//ret->retry |= 1 << mod->level;

					//mod_stack_set_reply(ret, reply_ack_error);
					//stack->reply_size = 8;

					dir_entry_unlock(mod->dir, stack->set, stack->way);

					mem_debug("    %lld 0x%x %s mc queue full, retrying...\n", stack->id, stack->tag, mod->name);
					new_stack = mod_stack_create(stack->id, mod, stack->addr,
						EV_MOD_VI_STORE_ACTION, stack);
					new_stack->blocking = 1;
					new_stack->nc_write = 1;
					new_stack->retry = stack->retry;
					new_stack->wavefront = stack->wavefront;
					new_stack->uop = stack->uop;
					new_stack->witness_ptr = stack->witness_ptr;
					stack->witness_ptr = NULL;
					new_stack->event = EV_MOD_VI_FIND_AND_LOCK;
					esim_schedule_mod_stack_event(new_stack, 10);
					return;
				}
				stack->event = EV_MOD_VI_STORE_UNLOCK;

				mem_debug("  %lld %lld 0x%x %s dram access enqueued\n", esim_time, stack->id, stack->tag, 	stack->target_mod->dram_system->name);
				linked_list_add(ds->pending_reads, stack);
				dram_system_add_read_trans(ds->handler, stack->addr, 99, 0);

				stack->dramsim_mm_start = asTiming(si_gpu)->cycle ;
				assert(!stack->prefetch);
				//ctx->mm_read_accesses++;
				return;
			}
			*/
			/* If another module has not given the block,
				 access main memory */
			if (mod->kind == mod_kind_main_memory
				&& mod->dram_system)
			{
				struct dram_system_t *ds = mod->dram_system;
				assert(ds);
				assert(stack->mshr_locked == 0);

				//if (!dram_system_will_accept_trans(ds->handler, stack->tag))
				if (!dram_system_will_accept_trans(ds->handler, stack->addr))
				{
					//stack->err = 1;
					//ret->err = 1;
					//ret->retry |= 1 << mod->level;

					//mod_stack_set_reply(ret, reply_ack_error);
					//stack->reply_size = 8;

					//dir_entry_unlock(mod->dir, stack->set, stack->way);

					mem_debug("    %lld 0x%x %s mc queue full, retrying...\n", stack->id, stack->tag, mod->name);


					/*
					new_stack = mod_stack_create(stack->id, mod, stack->addr,
						EV_MOD_VI_LOAD_ACTION, stack);
					new_stack->wavefront = stack->wavefront;
					new_stack->uop = stack->uop;
					new_stack->blocking = 1;
					new_stack->read = 1;
					new_stack->valid_mask = stack->valid_mask;
					new_stack->tiempo_acceso = stack->tiempo_acceso;
					new_stack->retry = stack->retry;
					stack->find_and_lock_stack = new_stack;
					new_stack->event = EV_MOD_VI_FIND_AND_LOCK;
					*/
					stack->event = EV_MOD_VI_LOAD_ACTION;
					esim_schedule_mod_stack_event(stack, 10);

					//esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST_REPLY, stack, 5);
					return;
				}

				//estadisticas(1, 0);
				//stack->ret_stack->valid_mask = mod_get_valid_mask(mod, stack->set, stack->way);
				//add_hit(mod->level);
				mod->hits_aux++;
				stack->event = EV_MOD_VI_STORE_UNLOCK;

				/* Access main memory system */
				mem_debug("  %lld %lld 0x%x %s dram access enqueued\n", esim_time, stack->id, stack->tag, 	stack->target_mod->dram_system->name);
				linked_list_add(ds->pending_reads, stack);
				//dram_system_add_read_trans(ds->handler, stack->tag >> 2, stack->wavefront->wavefront_pool_entry->wavefront_pool->compute_unit->id, stack->wavefront->id);
				dram_system_add_read_trans(ds->handler, stack->addr, stack->wavefront->wavefront_pool_entry->wavefront_pool->compute_unit->id, stack->wavefront->id);

				stack->dramsim_mm_start = asTiming(si_gpu)->cycle ;
				/* Ctx main memory stats */
				assert(!stack->prefetch);
				//ctx->mm_read_accesses++;
				return;
			}

			if (stack->hit)
			{
				/*if(mod->kind != mod_kind_main_memory)
				{
					cache_write_block_dirty_mask(mod->cache, stack->set, stack->way, stack->dirty_mask);
					cache_write_block_valid_mask(mod->cache, stack->set, stack->way, stack->dirty_mask);
				}*/
				//esim_schedule_event(EV_MOD_VI_STORE_UNLOCK, stack, 0);
			}
			else
			{

				if(stack->eviction)
				{
					int tag = 0;
					cache_get_block(mod->cache, stack->set, stack->way, &tag, NULL);
					new_stack = mod_stack_create(stack->id, mod_get_low_mod(mod, tag), tag, ESIM_EV_NONE, NULL);
					new_stack->src_mod = mod;
					new_stack->dirty_mask = cache_get_block_dirty_mask(mod->cache, stack->set, stack->way);
					new_stack->reply_size = 8 + mod->block_size;
					new_stack->wavefront = stack->wavefront;
					new_stack->uop = stack->uop;
					new_stack->request_dir = mod_request_up_down;
					new_stack->event = EV_MOD_VI_STORE_SEND;
					esim_schedule_mod_stack_event(new_stack, 0);
					//esim_schedule_event(EV_MOD_VI_STORE_SEND, new_stack, 0);
					//cache_set_block(mod->cache, stack->set, stack->way, 0, cache_block_invalid);
				}

				/*new_stack = mod_stack_create(stack->id, mod_get_low_mod(mod, stack->addr), stack->addr,  EV_MOD_VI_STORE_SEND, stack);
				new_stack->src_mod = mod;
				new_stack->dirty_mask = cache_get_block_dirty_mask(mod->cache, stack->set, stack->way);
				new_stack->reply_size = 8 + mod->block_size;
				new_stack->request_dir = mod_request_up_down;
				stack->request_dir = mod_request_down_up;
				new_stack->wavefront = stack->wavefront;
				new_stack->uop = stack->uop;
				stack->reply_size = 8 + mod->block_size;
				esim_schedule_event(EV_MOD_VI_STORE_SEND, new_stack, 0);*/

				//resetearla mascara de dirty y añadir bit
				cache_set_block(mod->cache, stack->set, stack->way, stack->tag, cache_block_invalid);
				//cache_clean_block_dirty(mod->cache, stack->set, stack->way);
				//cache_write_block_dirty_mask(mod->cache, stack->set, stack->way, stack->dirty_mask);
				//cache_write_block_valid_mask(mod->cache, stack->set, stack->way, stack->dirty_mask);

				//return;
			}
			stack->event = EV_MOD_VI_STORE_UNLOCK;
			esim_schedule_mod_stack_event(stack, 0);
			//esim_schedule_event(EV_MOD_VI_STORE_UNLOCK, stack, 0);
		}
		return;
	}

	if (event == EV_MOD_VI_STORE_UNLOCK)
	{

		mem_debug("  %lld %lld 0x%x %s store unlock\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:store_unlock\"\n",
			stack->id, mod->name);

		/* Error in write request, unlock block and retry store. */
		assert(!stack->err);
		if(stack->request_dir == mod_request_down_up)
		{
			cache_set_block(mod->cache, stack->set, stack->way, stack->tag, cache_block_valid);
			cache_write_block_dirty_mask(mod->cache, stack->set, stack->way, stack->dirty_mask);
			cache_write_block_valid_mask(mod->cache, stack->set, stack->way, stack->dirty_mask);
		}

		if (stack->mshr_locked != 0)
		{
			mshr_unlock(mod, stack);
			stack->mshr_locked = 0;
		}

		dir_entry_unlock(stack->dir_entry);

                stack->reply_size = 8;
                
		int latency = mod->latency;
		if (mod->kind == mod_kind_main_memory
			&& mod->dram_system)
			{
				latency = 0;
			}

		/* Impose the access latency before continuing */
		stack->event = EV_MOD_VI_STORE_FINISH;
		esim_schedule_mod_stack_event(stack, latency);
		//esim_schedule_event(EV_MOD_VI_STORE_FINISH, stack, latency);
		return;
	}

	if (event == EV_MOD_VI_STORE_FINISH)
	{
		mem_debug("%lld %lld 0x%x %s store finish\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:store_finish\"\n",
			stack->id, mod->name);
		mem_trace("mem.end_access name=\"A-%lld\"\n",
			stack->id);

		/* Return event queue element into event queue */
		if (mod->level == 1 && stack->event_queue && stack->event_queue_item)
			linked_list_add(stack->event_queue, stack->event_queue_item);

		/* Free the mod_client_info object, if any */
		if (mod->level == 1 && stack->client_info)
			mod_client_info_free(mod, stack->client_info);

		if (!stack->coalesced)
		{
			accu_retry_time_lost(stack);
			if(mod->level == 1 && stack->client_info && stack->client_info->arch){
				long long ciclo = stack->client_info->arch->timing->cycle;
				mem_load_finish(ciclo - stack->tiempo_acceso);

				stack->latencias.finish = stack->client_info->arch->timing->cycle - stack->latencias.start - stack->latencias.queue - stack->latencias.lock_mshr - stack->latencias.lock_dir - stack->latencias.eviction - stack->latencias.miss;
			}
			if(stack->retry && !stack->hit)
				add_latencias_nc_write(&(stack->latencias));
		}

		/*if(!stack->coalesced)
		{
	       	mod->mshr_count--;
			//mod_stack_wakeup_mod_head(mod);
		}*/

		/* Finish access */
		mod_access_finish(mod, stack);

		/* Return */
		mod_stack_return(stack);
		return;
	}

	abort();
}


void mod_handler_vi_find_and_lock(int event, void *data)
{
	struct mod_stack_t *stack = data;
	//, *oldest_stack=NULL;
	//struct mod_stack_t *ret = stack->ret_stack;
	struct mod_stack_t *ret = stack;
	//struct mod_stack_t *new_stack;

	struct mod_t *mod = stack->target_mod;


	if (event == EV_MOD_VI_FIND_AND_LOCK)
	{
		mem_debug("  %lld %lld 0x%x %s find and lock (blocking=%d)\n",
			esim_time, stack->id, stack->addr, mod->name, stack->blocking);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:find_and_lock\"\n",
			stack->id, mod->name);

		/* Default return values */
		ret->err = 0;
		ret->find_and_lock_stack = stack;
		assert(ret->id == stack->id);

		/* If this stack has already been assigned a way, keep using it */
		stack->way = ret->way;

		/* Get a port */
		mod_lock_port(mod, stack, EV_MOD_VI_FIND_AND_LOCK_PORT);
		return;
	}

	if (event == EV_MOD_VI_FIND_AND_LOCK_PORT)
	{
		struct mod_port_t *port = stack->port;
		struct dir_lock_t *dir_lock;

		assert(stack->port);
		mem_debug("  %lld %lld 0x%x %s find and lock port\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:find_and_lock_port\"\n",
			stack->id, mod->name);

		/* Set parent stack flag expressing that port has already been locked.
		 * This flag is checked by new writes to find out if it is already too
		 * late to coalesce. */
		ret->port_locked = 1;

		/* Look for block. */
		//stack->hit = mod_find_block(mod, stack->addr, &stack->set,
		//	&stack->way, &stack->tag, &stack->state);
                stack->hit = mod_find_block_new(mod, stack);

		//fran
		add_cache_states(stack->dir_entry->state, mod->level);

		//implementacion de valid mask
		if(stack->hit && stack->read)
		{
			unsigned int valid_mask = mod_get_valid_mask(stack->target_mod, stack->set, stack->way);
			if ((~valid_mask) & stack->valid_mask)
				stack->hit = 0;
		}

		/* Debug */
		if (stack->hit)
			mem_debug("    %lld 0x%x %s hit: set=%d, way=%d, state=%s\n", stack->id,
				stack->tag, mod->name, stack->set, stack->way,
				str_map_value(&cache_block_state_map, stack->dir_entry->state));

		/* Statistics */
		mod->accesses++;
		//hrl2(stack->hit, mod, stack->ret_stack->from_CU);
		if (stack->hit){
			mod->hits++;
			//sumamos acceso y hit
			//estadisticas(1, mod->level);
		}else{
			//sumamos acceso
			//estadisticas(0, mod->level);
		}
		if (stack->read)
		{
			mod->reads++;
			mod->effective_reads++;
			stack->blocking ? mod->blocking_reads++ : mod->non_blocking_reads++;
			if (stack->hit)
				mod->read_hits++;
		}
		else if (stack->nc_write)  /* Must go after read */
		{
			mod->nc_writes++;
			mod->effective_nc_writes++;
			stack->blocking ? mod->blocking_nc_writes++ : mod->non_blocking_nc_writes++;
			/* Increment witness variable when port is locked */
			if (stack->witness_ptr)
			{
				(*stack->witness_ptr)++;
				stack->witness_ptr = NULL;
			}
			if (stack->hit)
				mod->nc_write_hits++;
		}
		else if (stack->write)
		{
			mod->writes++;
			mod->effective_writes++;
			stack->blocking ? mod->blocking_writes++ : mod->non_blocking_writes++;

			/* Increment witness variable when port is locked */
			if (stack->witness_ptr)
			{
				(*stack->witness_ptr)++;
				stack->witness_ptr = NULL;
			}

			if (stack->hit)
				mod->write_hits++;
		}
		else
		{
			fatal("Unknown memory operation type");
		}

		if (!stack->retry)
		{
			mod->no_retry_accesses++;
			if (stack->hit)
				mod->no_retry_hits++;

			if (stack->read)
			{
				mod->no_retry_reads++;
				if (stack->hit)
					mod->no_retry_read_hits++;
			}
			else if (stack->nc_write)  /* Must go after read */
			{
				mod->no_retry_nc_writes++;
				if (stack->hit)
					mod->no_retry_nc_write_hits++;
			}
			else if (stack->write)
			{
				mod->no_retry_writes++;
				if (stack->hit)
					mod->no_retry_write_hits++;
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
			/*oldest_stack = mshr_block_in_flight(mod->mshr, stack);
			if(oldest_stack && flag_mshr_enabled && stack->read && stack->mshr_locked == 0 && mod->kind != mod_kind_main_memory)
			{
				mod_stack_wait_in_stack(stack, oldest_stack, EV_MOD_VI_FIND_AND_LOCK);
				mod_unlock_port(mod, port, stack);
				ret->port_locked = 0;
				return;
			}*/

			if (stack->way < 0)
			{
				stack->way = cache_replace_block(mod->cache, stack->set);
			}

			/* proba para accesosos*/

			/*if(mod->level == 1 && stack->read != 0 && stack->ret_stack->wavefront->wavefront_pool_entry->id_in_wavefront_pool != (asTiming(si_gpu)->cycle/ 10)%10){
				long long wait = ((asTiming(si_gpu)->cycle - asTiming(si_gpu)->cycle%100)  +(stack->ret_stack->wavefront->wavefront_pool_entry->id_in_wavefront_pool * 10)) - asTiming(si_gpu)->cycle;
				if(wait < 0)
					wait += 100;

				stack->event = EV_MOD_VI_FIND_AND_LOCK_PORT;
				esim_schedule_mod_stack_event(stack, wait);
				return;
			}*/

			if(flag_mshr_enabled && stack->read && stack->mshr_locked == 0 && mod->kind != mod_kind_main_memory)
			{
				/*if(mshr_pre_lock(mod->mshr, stack->ret_stack))
				{
					mod_unlock_port(mod, port, stack);
					ret->port_locked = 0;
					ret->mshr_locked = 0;
					//if(stack->dir_lock && stack->dir_lock->lock_queue && stack->dir_lock->lock == 0 )
					//	dir_entry_unlock(mod->dir, stack->set, stack->way);

					mshr_delay(mod->mshr,stack, EV_MOD_VI_FIND_AND_LOCK);
					return;
				}*/
				if(!mshr_lock(mod->mshr, stack))
				{
					mod_unlock_port(mod, port, stack);
					ret->port_locked = 0;
					ret->mshr_locked = 0;
					if(stack->dir_lock && list_count(stack->dir_lock->lock_list_up_down) && list_count(stack->dir_lock->lock_list_down_up) && stack->dir_lock->lock == 0 )
						dir_entry_unlock(stack->dir_entry);

					mshr_enqueue(mod->mshr,stack, EV_MOD_VI_FIND_AND_LOCK);
					return;
				}

				if(mod->level == 1 && stack->client_info && stack->client_info->arch){
					ret->latencias.lock_mshr = stack->client_info->arch->timing->cycle - ret->latencias.start - ret->latencias.queue;
				}
				stack->mshr_locked = 1;
			}
		}

		assert(stack->way >= 0);

		/* If directory entry is locked and the call to FIND_AND_LOCK is not
		 * blocking, release port and return error. */
		dir_lock = dir_lock_get(mod->dir, stack->dir_entry->set, stack->dir_entry->way, stack->dir_entry->w);
		/*if (dir_lock->lock && !stack->blocking)
		{
			mem_debug("    %lld 0x%x %s block locked at set=%d, way=%d by A-%lld - aborting\n",
				stack->id, stack->tag, mod->name, stack->set, stack->way, dir_lock->stack_id);
			ret->err = 1;
			mod_unlock_port(mod, port, stack);
			ret->port_locked = 0;
			mod_stack_return(stack);
			return;
		}*/

		/* Lock directory entry. If lock fails, port needs to be released to prevent
		 * deadlock.  When the directory entry is released, locking port and
		 * directory entry will be retried. */

		//if(stack->dir_lock && stack->dir_lock->lock == 0 && stack->dir_lock != dir_lock)
		//{
			//while(stack->dir_lock->lock_queue)
			//{
				/* Wake up access */
				//esim_schedule_event(stack->dir_lock->lock_queue->dir_lock_event, stack->dir_lock->lock_queue, 0);
				//stack->dir_lock->lock_queue = stack->dir_lock->lock_queue->dir_lock_next;
			//}
		//}

		stack->dir_lock = dir_lock;

		if (!dir_entry_lock(stack->dir_entry, EV_MOD_VI_FIND_AND_LOCK, stack))
		{
			mem_debug("    %lld 0x%x %s block locked at set=%d, way=%d by A-%lld - waiting\n",
				stack->id, stack->tag, mod->name, stack->set, stack->way, dir_lock->stack->id);

			mod_unlock_port(mod, port, stack);
			ret->port_locked = 0;

			return;
		}

		/* Miss */
		if (!stack->hit)
		{
			/* Find victim */
			//cache_get_block(mod->cache, stack->set, stack->way, NULL, &stack->state);
			if(stack->dir_entry->state)
			{
				if(stack->read)
					add_load_invalidation(mod->level);
				else
					add_store_invalidation(mod->level);
			}

			//cache_set_block(mod->cache, stack->set, stack->way,	0, cache_block_invalid);

			mem_debug("    %lld 0x%x %s miss -> lru: set=%d, way=%d, state=%s\n",
				stack->id, stack->tag, mod->name, stack->set, stack->way,
				str_map_value(&cache_block_state_map, stack->dir_entry->state));
		}


		/* Entry is locked. Record the transient tag so that a subsequent lookup
		 * detects that the block is being brought.
		 * Also, update LRU counters here. */
		cache_set_transient_tag(mod->cache, stack->set, stack->way, stack->tag);

		cache_access_block(mod->cache, stack->set, stack->way);

		/* Access latency */
		stack->event = EV_MOD_VI_FIND_AND_LOCK_ACTION;
		esim_schedule_mod_stack_event(stack, mod->dir_latency);
		//esim_schedule_event(EV_MOD_VI_FIND_AND_LOCK_ACTION, stack, mod->dir_latency);
		return;
	}
   	if ( event == EV_MOD_VI_FIND_AND_LOCK_ACTION)
	{
		struct mod_port_t *port = stack->port;

		assert(port);
		mem_debug("  %lld %lld 0x%x %s find and lock action\n", esim_time, stack->id,
			stack->tag, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:find_and_lock_action\"\n",
			stack->id, mod->name);

		/* Release port */
		mod_unlock_port(mod, port, stack);
		ret->port_locked = 0;
		if(mod->level == 1 && ret->client_info && ret->client_info->arch){
			ret->latencias.lock_dir = ret->client_info->arch->timing->cycle - ret->latencias.start - ret->latencias.queue - ret->latencias.lock_mshr;
		}

		/* On miss, evict if victim is a valid block. */
		/*if (!stack->hit && stack->state && cache_get_block_dirty_mask(mod->cache, stack->set, stack->way))
		{
			stack->eviction = 1;
			//cache_set_block(mod->cache, stack->src_set, stack->src_way,	0, cache_block_invalid);
		}*/

		/* Continue */
		stack->event = EV_MOD_VI_FIND_AND_LOCK_FINISH;
		esim_schedule_mod_stack_event(stack, 0);
		//esim_schedule_event(EV_MOD_VI_FIND_AND_LOCK_FINISH, stack, 0);
		return;
	}

	if (event == EV_MOD_VI_FIND_AND_LOCK_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s find and lock finish (err=%d)\n", esim_time, stack->id,
			stack->tag, mod->name, stack->err);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:find_and_lock_finish\"\n",
			stack->id, mod->name);

		/* If evict produced err, return err */
		assert(!stack->err);
		/*if (stack->err)
		{
			cache_get_block(mod->cache, stack->set, stack->way, NULL, &stack->state);
			assert(stack->state);
			assert(stack->eviction);
			ret->err = 1;
			dir_entry_unlock(mod->dir, stack->set, stack->way);
			mod_stack_return(stack);
			return;
		}*/

		/* If this is a main memory, the block is here. A previous miss was just a miss
		 * in the directory. */
		if (mod->kind == mod_kind_main_memory)
		{
			//stack->state = cache_block_valid;
			stack->hit = 1;
			//cache_set_block_new(mod->cache, stack->set, stack->way,
			//	stack->tag, stack->state);
                        cache_set_block_new(mod->cache, stack, cache_block_valid);
		}

		if (!stack->hit && stack->dir_entry->state && cache_get_block_dirty_mask(mod->cache, stack->set, stack->way))
		{
			stack->eviction = 1;
		}

		/* Eviction */
		if (stack->eviction)
		{
			mod->evictions++;
			//cache_get_block(mod->cache, stack->set, stack->way, NULL, &stack->state);
			//assert(!stack->state);
		}

		if(!stack->retry && (ret->origin || !stack->blocking))
		{
			if(stack->hit)
			{
				add_hit(mod->level);
			}else{
				add_miss(mod->level);
			}
		}

		/* Return */
		ret->err = 0;
		ret->set = stack->set;
		ret->eviction = stack->eviction;
		ret->way = stack->way;
		ret->hit = stack->hit;
		//ret->state = stack->state;
		ret->tag = stack->tag;
		ret->mshr_locked = stack->mshr_locked;
		ret->find_and_lock_stack = NULL;
		stack->event = stack->find_and_lock_return_event;
		stack->find_and_lock_return_event = 0;
		esim_schedule_mod_stack_event(stack, 0);
		//mod_stack_return(stack);
		return;
	}

	abort();
}
