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

#include "cache.h"
#include "directory.h"
#include "mem-system.h"
#include "mod-stack.h"
#include "prefetcher.h"
//fran
#include <lib/util/misc.h>
#include <lib/util/estadisticas.h>
#include <arch/southern-islands/timing/gpu.h>
#include <lib/util/class.h>
/* Events */

int EV_MOD_VI_LOAD;
int EV_MOD_VI_LOAD_SEND;
int EV_MOD_VI_LOAD_RECEIVE;
int EV_MOD_VI_LOAD_LOCK;
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
	struct mod_stack_t *new_stack, *older_stack;

	struct mod_t *mod = stack->mod;
	struct net_t *net;
	struct net_node_t *src_node;
	struct net_node_t *dst_node;
	int return_event;
int retry_lat;


	if (event == EV_MOD_VI_LOAD || event == EV_MOD_VI_NC_LOAD)
	{
		struct mod_stack_t *master_stack;

		mem_debug("%lld %lld 0x%x %s load\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.new_access name=\"A-%lld\" type=\"load\" "
			"state=\"%s:load\" addr=0x%x\n",
			stack->id, mod->name, stack->addr);
			
		//if(event == EV_MOD_VI_LOAD)
		//	stack->glc = 1;
		
		/* Record access */
		//mod_access_start(mod, stack, mod_access_load);
		//FRAN
		mod->loads++;
							/* Increment witness variable when port is locked */
		/*	if (stack->witness_ptr)
			{
				(*stack->witness_ptr)++;
				stack->witness_ptr = NULL;
			}*/
		
		/* Coalesce access */
		
		master_stack = mod_can_coalesce(mod, mod_access_load, stack->addr, NULL);
		if (master_stack)
		{
			mod_access_start(mod, stack, mod_access_load);
			mod->hits_aux++;
			mod->delayed_read_hit++;
			add_access(mod->level);
			add_coalesce(mod->level);

			mod_coalesce(mod, master_stack, stack);
			mod_stack_wait_in_stack(stack, master_stack, EV_MOD_VI_LOAD_FINISH);
			return;
		}
		
	/*	if (mod->mshr_size && mod->mshr_count >= mod->mshr_size)
		{
			mod_stack_wait_in_mod(stack, mod, EV_MOD_VI_LOAD);
			return;
		}
		mod->mshr_count++;*/
		
		mod_access_start(mod, stack, mod_access_load);
		add_access(mod->level);

		/* Next event */
		esim_schedule_event(EV_MOD_VI_LOAD_LOCK, stack, 0);
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
			
			stack->request_dir = mod_request_up_down;
					
			net = mod->high_net;
			src_node = stack->ret_stack->mod->low_net_node;
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
			esim_schedule_event(EV_MOD_VI_LOAD, stack, 0);
			
        }
        else if(stack->request_dir == mod_request_down_up)
        {
			net_receive(mod->low_net, mod->low_net_node, stack->msg);
			if(SALTAR_L1 && mod->level == 1)
				esim_schedule_event(EV_MOD_VI_LOAD_UNLOCK, stack, 0);	
			else
				esim_schedule_event(EV_MOD_VI_LOAD_MISS, stack, 0);					
		}
		else
        {
			fatal("Invalid request_dir in EV_MOD_VI_LOAD_RECEIVE");
        }
		return;
	}
if (event == EV_MOD_VI_LOAD_LOCK)
{
	//struct mod_stack_t *older_stack;

	mem_debug("  %lld %lld 0x%x %s load lock\n", esim_time, stack->id,
		stack->addr, mod->name);
	mem_trace("mem.access name=\"A-%lld\" state=\"%s:load_lock\"\n",
		stack->id, mod->name);

	/* If there is any older access to the same address that this access could not
	 * be coalesced with, wait for it. */
	older_stack = mod_in_flight_write_fran(mod, stack);
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
			assert(!older_stack->waiting_list_event);
		mod_stack_wait_in_stack(stack, older_stack, EV_MOD_VI_LOAD_LOCK);
		return;
	}
	
	if (mod->mshr_size && mod->mshr_count >= mod->mshr_size)
    {
        mod->read_retries++;
        retry_lat = mod_get_retry_latency(mod);
       	mem_debug("mshr full, retrying in %d cycles\n", retry_lat);
       	stack->retry = 1;
       	esim_schedule_event(EV_MOD_VI_LOAD_LOCK, stack, retry_lat);
      	return;
    }
    mod->mshr_count++;
   
    if(SALTAR_L1 && mod->level == 1)
	{
		stack->request_dir = mod_request_down_up;
		new_stack = mod_stack_create(stack->id, mod_get_low_mod(mod, stack->addr), stack->addr, EV_MOD_VI_LOAD_SEND, stack);
       	new_stack->reply_size = 8;
       	new_stack->request_dir= mod_request_up_down;
       	new_stack->valid_mask = stack->valid_mask;
       	
		stack->reply_size = 8 + mod->block_size;
       	esim_schedule_event(EV_MOD_VI_LOAD_SEND, new_stack, 0);
		return;
	}

	
	/* Call find and lock */
	new_stack = mod_stack_create(stack->id, mod, stack->addr,
		EV_MOD_VI_LOAD_ACTION, stack);
	new_stack->blocking = 1;
	new_stack->read = 1;
	new_stack->valid_mask = stack->valid_mask;
	//new_stack->tiempo_acceso = stack->tiempo_acceso;
	new_stack->retry = stack->retry;
	esim_schedule_event(EV_MOD_VI_FIND_AND_LOCK, new_stack, 0);
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
		retry_lat = mod_get_retry_latency(mod);
		mem_debug("    lock error, retrying in %d cycles\n", retry_lat);
		stack->retry = 1;
		esim_schedule_event(EV_MOD_VI_LOAD_LOCK, stack, retry_lat);
		return;
	}

	mem_stats.mod_level[mod->level].entradas_bloqueadas++;

	if(mod->level == 1)
	{
			/* Hit */
		if (stack->hit && (stack->glc == 0))
		{
			esim_schedule_event(EV_MOD_VI_LOAD_UNLOCK, stack, 0);
			//estadisticas(1, 0);

			add_hit(mod->level);
			mod->hits_aux++;

			return;
		}	
		
		/* MISS */
//		cache_set_block(mod->cache, stack->set, stack->way, stack->tag, cache_block_invalid);
		stack->state = 0;
		
		add_miss(mod->level);
		
		new_stack = mod_stack_create(stack->id, mod_get_low_mod(mod, stack->addr), stack->addr,
			EV_MOD_VI_LOAD_SEND, stack);
		new_stack->reply_size = 8;
		new_stack->valid_mask = stack->valid_mask;
		stack->reply_size = 8 + mod->block_size;
		stack->request_dir = mod_request_down_up;
		new_stack->request_dir = mod_request_up_down;
		esim_schedule_event(EV_MOD_VI_LOAD_SEND, new_stack, 0);
		
		return;
	}
	else
	{
			/* Hit */
		if (stack->hit)
		{
			
			//estadisticas(1, 0);
			stack->ret_stack->valid_mask = mod_get_valid_mask(mod, stack->set, stack->way);
			add_hit(mod->level);
			mod->hits_aux++;
			esim_schedule_event(EV_MOD_VI_LOAD_UNLOCK, stack, 0);
			return;
		}
		
		/* MISS */
		add_miss(mod->level);

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
			esim_schedule_event(EV_MOD_VI_STORE_SEND, new_stack, 0);
		}

		
		
		new_stack = mod_stack_create(stack->id, mod_get_low_mod(mod, stack->addr), stack->addr,
			EV_MOD_VI_LOAD_SEND, stack);
		new_stack->reply_size = 8;
		stack->reply_size = 8 + mod->block_size;
		stack->request_dir = mod_request_down_up;
		new_stack->request_dir = mod_request_up_down;
		esim_schedule_event(EV_MOD_VI_LOAD_SEND, new_stack, 0);
		
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
		/*if (stack->err)
		{
			mod->read_retries++;
			retry_lat = mod_get_retry_latency(mod);
			dir_entry_unlock(mod->dir, stack->set, stack->way);
			mem_debug("    lock error, retrying in %d cycles\n", retry_lat);
			stack->retry = 1;
			esim_schedule_event(EV_MOD_VI_LOAD_LOCK, stack, retry_lat);
			return;
		}*/

		/* Set block state to excl/shared depending on return var 'shared'.
		 * Also set the tag of the block. */
		cache_set_block(mod->cache, stack->set, stack->way, stack->tag, cache_block_valid);
		cache_write_block_valid_mask(mod->cache, stack->set, stack->way, stack->valid_mask);
		/* Continue */
		esim_schedule_event(EV_MOD_VI_LOAD_UNLOCK, stack, 0);
		return;
	}

	if (event == EV_MOD_VI_LOAD_UNLOCK)
	{
		mem_debug("  %lld %lld 0x%x %s load unlock\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:load_unlock\"\n",
			stack->id, mod->name);
		assert(!stack->err);
		mem_stats.mod_level[mod->level].entradas_bloqueadas--;

		/* Unlock directory entry */
		dir_entry_unlock(mod->dir, stack->set, stack->way);
		
		/* Impose the access latency before continuing */

		esim_schedule_event(EV_MOD_VI_LOAD_FINISH, stack,
				mod->latency);

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
			
		//fran_debug("%lld %lld\n",esim_time, stack->id);
	 	if(mod->level == 1)
		{
			long long ciclo = asTiming(si_gpu)->cycle;
			if(esim_time > stack->tiempo_acceso){
				
				mem_load_finish(ciclo - stack->tiempo_acceso);			
				estadis[0].media_latencia += (long long) (ciclo - stack->tiempo_acceso);
				estadis[0].media_latencia_contador++;
			}
		}		
		
		if(!stack->coalesced)
		{	
	       	mod->mshr_count--;
			//mod_stack_wakeup_mod_head(mod);
		}
		

		if(stack->hit)
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
		if (stack->witness_ptr)
			(*stack->witness_ptr)++;

		/* Return event queue element into event queue */
		if (stack->event_queue && stack->event_queue_item)
			linked_list_add(stack->event_queue, stack->event_queue_item);

		/* Free the mod_client_info object, if any */
		if (stack->client_info)
			mod_client_info_free(mod, stack->client_info);

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
	struct mod_stack_t *new_stack, *older_stack, *master_stack;

	struct mod_t *mod = stack->mod;

        struct net_t *net;
        struct net_node_t *src_node;
        struct net_node_t *dst_node;
        int return_event;
int retry_lat;

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

		master_stack = mod_can_coalesce(mod, mod_access_store, stack->addr, stack);		
		//mod_access_start(mod, stack, mod_access_store);
		if (master_stack)
		{
			mod_access_start(mod, stack, mod_access_store);
			assert(master_stack->addr == stack->addr);
			mod->writes++;
			mod_stack_merge_dirty_mask(master_stack, stack->dirty_mask);
			mod_coalesce(mod, master_stack, stack);
			mod_stack_wait_in_stack(stack, master_stack, EV_MOD_VI_STORE_FINISH);

			// Increment witness variable 
			if (stack->witness_ptr)
				(*stack->witness_ptr)++;
			stack->witness_ptr = NULL;
			return;
		}
		
		/*if (mod->mshr_size && mod->mshr_count >= mod->mshr_size)
		{
			mod_stack_wait_in_mod(stack, mod, EV_MOD_VI_STORE);
			return;
		}
		mod->mshr_count++;*/
		
		mod_access_start(mod, stack, mod_access_store);
		/* Continue */
		esim_schedule_event(EV_MOD_VI_STORE_LOCK, stack, 0);
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
			esim_schedule_event(EV_MOD_VI_STORE, stack, 0);
        }
        else if(stack->request_dir == mod_request_down_up)
        {
			net_receive(mod->low_net, mod->low_net_node, stack->msg);
			esim_schedule_event(EV_MOD_VI_STORE_UNLOCK, stack, 0);
        }
        else
        {
			fatal("Invalid request_dir in EV_MOD_VI_STORE_RECEIVE");
        }

        return;
    }

	if (event == EV_MOD_VI_STORE_LOCK)
	{
		//struct mod_stack_t *older_stack;

		
		mem_debug("  %lld %lld 0x%x %s store lock\n", esim_time, stack->id,
			stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:store_lock\"\n",
			stack->id, mod->name);

		older_stack = mod_in_flight_write_fran(mod, stack);
                
        if (mod->level == 1 && older_stack)
        {
			assert(!older_stack->waiting_list_event);
            mem_debug("    %lld wait for write %lld\n", stack->id, older_stack->id);
            mod_stack_wait_in_stack(stack, older_stack, EV_MOD_VI_STORE_LOCK);
			return;
        }
                
        if (mod->mshr_size && mod->mshr_count >= mod->mshr_size)
		{
			mod->read_retries++;
			retry_lat = mod_get_retry_latency(mod);
			mem_debug("mshr full, retrying in %d cycles\n", retry_lat);
			stack->retry = 1;
			esim_schedule_event(EV_MOD_VI_STORE_LOCK, stack, retry_lat);
			return;
		}
		mod->mshr_count++;
	
		if(SALTAR_L1 && mod->level == 1)
		{
			stack->request_dir = mod_request_down_up;
			new_stack = mod_stack_create(stack->id, mod_get_low_mod(mod, stack->addr), stack->addr, EV_MOD_VI_STORE_SEND, stack);
			new_stack->request_dir= mod_request_up_down;
			new_stack->src_mod = mod;
			new_stack->dirty_mask = stack->dirty_mask;
			
			// Increment witness variable 
			if (stack->witness_ptr)
				(*stack->witness_ptr)++;
			stack->witness_ptr = NULL;
			
			esim_schedule_event(EV_MOD_VI_STORE_SEND, new_stack, 0);
			return;

		}
		

		/* Call find and lock */
		new_stack = mod_stack_create(stack->id, mod, stack->addr,
			EV_MOD_VI_STORE_ACTION, stack);
		new_stack->blocking = 1;
		new_stack->write = 1;
		new_stack->retry = stack->retry;
		new_stack->witness_ptr = stack->witness_ptr;
		stack->witness_ptr = NULL;
		esim_schedule_event(EV_MOD_VI_FIND_AND_LOCK, new_stack, 0);

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
			// store al level 2
			struct mod_t *target_mod = mod_get_low_mod(mod, stack->tag);
			new_stack = mod_stack_create(stack->id, target_mod, stack->tag, ESIM_EV_NONE, NULL);
			new_stack->src_mod = mod;
			new_stack->dirty_mask = stack->dirty_mask;
			new_stack->request_dir = mod_request_up_down;
			esim_schedule_event(EV_MOD_VI_STORE_SEND, new_stack, 0);
			
			
			//da igual si es un hit o un miss
			
			//cache_clean_block_dirty(mod->cache, stack->set, stack->way);
/*			if(stack->glc == 0 && (~stack->dirty_mask) == 0)
			{
				cache_set_block(mod->cache, stack->set, stack->way, stack->tag, cache_block_valid);
				cache_write_block_valid_mask(mod->cache, stack->set, stack->way, stack->dirty_mask);
			}
			else if(stack->hit)
			{
				cache_set_block(mod->cache, stack->set, stack->way, stack->tag, cache_block_invalid);
			}*/
		
			esim_schedule_event(EV_MOD_VI_STORE_UNLOCK, stack, 0);
		}
		else
		{
			/* Hit - state=V*/
			if (stack->hit)
			{
				if(mod->kind != mod_kind_main_memory)
				{
					cache_write_block_dirty_mask(mod->cache, stack->set, stack->way, stack->dirty_mask);
					cache_write_block_valid_mask(mod->cache, stack->set, stack->way, stack->dirty_mask);
				}
				//esim_schedule_event(EV_MOD_VI_STORE_UNLOCK, stack, 0);	
			}
			else
			{
			/* Miss - state=I */
			
				if(stack->eviction)
				{
					int tag = 0;
					cache_get_block(mod->cache, stack->set, stack->way, &tag, NULL);
					new_stack = mod_stack_create(stack->id, mod_get_low_mod(mod, tag), tag, ESIM_EV_NONE, NULL);
					new_stack->src_mod = mod;
					new_stack->dirty_mask = cache_get_block_dirty_mask(mod->cache, stack->set, stack->way);
					new_stack->reply_size = 8 + mod->block_size;
					new_stack->request_dir = mod_request_up_down;
					esim_schedule_event(EV_MOD_VI_STORE_SEND, new_stack, 0);			
					//cache_set_block(mod->cache, stack->set, stack->way, 0, cache_block_invalid);
				}
				
				new_stack = mod_stack_create(stack->id, mod_get_low_mod(mod, stack->addr), stack->addr,  EV_MOD_VI_STORE_SEND, stack);
				new_stack->src_mod = mod;
				new_stack->dirty_mask = cache_get_block_dirty_mask(mod->cache, stack->set, stack->way);
				new_stack->reply_size = 8 + mod->block_size;
				new_stack->request_dir = mod_request_up_down;
				stack->request_dir = mod_request_down_up;
				stack->reply_size = 8 + mod->block_size;
				esim_schedule_event(EV_MOD_VI_STORE_SEND, new_stack, 0);
				/*
				//resetearla mascara de dirty y añadir bit
				cache_set_block(mod->cache, stack->set, stack->way, stack->tag, cache_block_valid);
				//cache_clean_block_dirty(mod->cache, stack->set, stack->way);
				cache_write_block_dirty_mask(mod->cache, stack->set, stack->way, stack->dirty_mask);
				cache_write_block_valid_mask(mod->cache, stack->set, stack->way, stack->dirty_mask);
				// si dirty_mask esta completa deberiamos hacer evict? para limpiarla manteniendo el bloque valio? 
				dirty_mask = cache_get_block_dirty_mask(mod->cache, stack->set, stack->way);
				*/
				return;
			}
			esim_schedule_event(EV_MOD_VI_STORE_UNLOCK, stack, 0);
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
			cache_set_block(mod->cache, stack->set, stack->way, stack->tag, cache_block_valid);
			
		dir_entry_unlock(mod->dir, stack->set, stack->way);

		/* Impose the access latency before continuing */
		esim_schedule_event(EV_MOD_VI_STORE_FINISH, stack, 
			mod->latency);
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
		if (stack->event_queue && stack->event_queue_item)
			linked_list_add(stack->event_queue, stack->event_queue_item);

		/* Free the mod_client_info object, if any */
		if (stack->client_info)
			mod_client_info_free(mod, stack->client_info);

		if(!stack->coalesced)
		{	
	       	mod->mshr_count--;
			//mod_stack_wakeup_mod_head(mod);
		}

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
	struct mod_stack_t *ret = stack->ret_stack;
	//struct mod_stack_t *new_stack;

	struct mod_t *mod = stack->mod;


	if (event == EV_MOD_VI_FIND_AND_LOCK)
	{
		mem_debug("  %lld %lld 0x%x %s find and lock (blocking=%d)\n",
			esim_time, stack->id, stack->addr, mod->name, stack->blocking);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:find_and_lock\"\n",
			stack->id, mod->name);

		/* Default return values */
		ret->err = 0;

		/* If this stack has already been assigned a way, keep using it */
		//stack->way = ret->way;

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
		stack->hit = mod_find_block(mod, stack->addr, &stack->set,
			&stack->way, &stack->tag, &stack->state);
		
		//implementacion de valid mask
		if(stack->hit && stack->read)
		{
			unsigned int valid_mask = mod_get_valid_mask(stack->mod, stack->set, stack->way);
			if ((~valid_mask) & stack->valid_mask)
				stack->hit = 0;
		}

		/* Debug */
		if (stack->hit)
			mem_debug("    %lld 0x%x %s hit: set=%d, way=%d, state=%s\n", stack->id,
				stack->tag, mod->name, stack->set, stack->way,
				str_map_value(&cache_block_state_map, stack->state));
		
		/* Statistics */
		mod->accesses++;
		//hrl2(stack->hit, mod, stack->ret_stack->from_CU);
		if (stack->hit){
			mod->hits++;
			//sumamos acceso y hit
			estadisticas(1, mod->level);
		}else{
			//sumamos acceso
			estadisticas(0, mod->level);
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
			if (stack->way < 0) 
			{
				stack->way = cache_replace_block(mod->cache, stack->set);
			}
		}
		assert(stack->way >= 0);

		/* If directory entry is locked and the call to FIND_AND_LOCK is not
		 * blocking, release port and return error. */
		dir_lock = dir_lock_get(mod->dir, stack->set, stack->way);
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
		
		if(stack->dir_lock && stack->dir_lock->lock == 0 && stack->dir_lock != dir_lock)
		{
			while(stack->dir_lock->lock_queue)
			{
				/* Wake up access */
				esim_schedule_event(stack->dir_lock->lock_queue->dir_lock_event, stack->dir_lock->lock_queue, 0);
				stack->dir_lock->lock_queue = stack->dir_lock->lock_queue->dir_lock_next;
			}
		}
			
		stack->dir_lock = dir_lock;
		
		if (!dir_entry_lock(mod->dir, stack->set, stack->way, EV_MOD_VI_FIND_AND_LOCK, 
			stack))
		{
			mem_debug("    %lld 0x%x %s block locked at set=%d, way=%d by A-%lld - waiting\n",
				stack->id, stack->tag, mod->name, stack->set, stack->way, dir_lock->stack_id);
			
			mod_unlock_port(mod, port, stack);
			ret->port_locked = 0;
			
			return;
		}

		/* Miss */
		if (!stack->hit)
		{
			/* Find victim */
			cache_get_block(mod->cache, stack->set, stack->way, NULL, &stack->state);
			
			
			//cache_set_block(mod->cache, stack->set, stack->way,	0, cache_block_invalid);
			
			mem_debug("    %lld 0x%x %s miss -> lru: set=%d, way=%d, state=%s\n",
				stack->id, stack->tag, mod->name, stack->set, stack->way,
				str_map_value(&cache_block_state_map, stack->state));
		}


		/* Entry is locked. Record the transient tag so that a subsequent lookup
		 * detects that the block is being brought.
		 * Also, update LRU counters here. */
		//cache_set_transient_tag(mod->cache, stack->set, stack->way, stack->tag);
		
		cache_access_block(mod->cache, stack->set, stack->way);

		/* Access latency */
		esim_schedule_event(EV_MOD_VI_FIND_AND_LOCK_ACTION, stack, mod->dir_latency);
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


		/* On miss, evict if victim is a valid block. */
		/*if (!stack->hit && stack->state && cache_get_block_dirty_mask(mod->cache, stack->set, stack->way))
		{
			stack->eviction = 1;
			//cache_set_block(mod->cache, stack->src_set, stack->src_way,	0, cache_block_invalid);
		}*/

		/* Continue */
		esim_schedule_event(EV_MOD_VI_FIND_AND_LOCK_FINISH, stack, 0);
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
		if (mod->kind == mod_kind_main_memory/* && !stack->state*/)
		{
			stack->state = cache_block_valid;
			stack->hit = 1;		
			cache_set_block(mod->cache, stack->set, stack->way,
				stack->tag, stack->state);
		}
		
		if (!stack->hit && stack->state && cache_get_block_dirty_mask(mod->cache, stack->set, stack->way))
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

		/* Return */
		ret->err = 0;
		ret->set = stack->set;
		ret->eviction = stack->eviction;
		ret->way = stack->way;
		ret->hit = stack->hit;
		ret->state = stack->state;
		ret->tag = stack->tag;
		mod_stack_return(stack);
		return;
	}

	abort();
}

