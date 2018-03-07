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

#include <assert.h>

#include <lib/esim/esim.h>
#include <lib/esim/trace.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/misc.h>
#include <lib/util/debug.h>

#include "directory.h"
#include "mem-system.h"
#include "mod-stack.h"
#include "mshr.h"


//#define DIR_ENTRY_SHARERS_SIZE ((dir->num_nodes + 7) / 8)
#define DIR_ENTRY_SIZE (sizeof(struct dir_entry_t) + DIR_ENTRY_SHARERS_SIZE)
#define DIR_ENTRY(X, Y, Z) ((struct dir_entry_t *) (((void *) &dir->data) + DIR_ENTRY_SIZE * \
	((X) * dir->ysize * dir->zsize + (Y) * dir->zsize + (Z))))


struct dir_t *dir_create(char *name, int xsize, int ysize, int zsize, int num_nodes, struct mod_t *mod)
{
	struct dir_t *dir;
	struct dir_entry_t *dir_entry;
        struct dir_lock_t *dir_lock;
        struct cache_block_t *cache_block;

	//int dir_size;
	//int dir_entry_size;
        int sharer_size;
        
	int x;
	int y;
	int z;
        int w;
        int wsize = mod->cache->dir_entry_per_line;

	/* Calculate sizes */
	assert(num_nodes > 0);
	//dir_entry_size = sizeof(struct dir_entry_t) + (num_nodes + 7) / 8;
	sharer_size = (num_nodes + 7) / 8;
        //dir_size = sizeof(struct dir_t) + dir_entry_size * xsize * ysize * zsize;

	/* Initialize */
	//dir = xcalloc(1, dir_size);
	dir = xcalloc(1,sizeof(struct dir_t));
        dir->name = xstrdup(name);
	dir->dir_lock_file = xcalloc(xsize * ysize * wsize, sizeof(struct dir_lock_t));
        dir->dir_entry_file = xcalloc(xsize * ysize * zsize * wsize, sizeof(struct dir_entry_t));
	dir->num_nodes = num_nodes;
	dir->xsize = xsize;
	dir->ysize = ysize;
	dir->zsize = zsize;
        dir->wsize = wsize;
        dir->mod = mod;
        dir->dir_entry_sharers_size = sharer_size;
        dir->extra_dir_structure_type = mod->cache->extra_dir_structure_type;
        //dir->sets_extra_dir_used = xcalloc(dir->extra_dir_sets,sizeof(int));
        //dir->extra_dir_max = 512;
        
        if(dir->extra_dir_structure_type == extra_dir_per_cache_line)
        {
            /* Reset all owners */
            for (x = 0; x < xsize; x++)
            {
                    for (y = 0; y < ysize; y++)
                    {
                            struct list_t *lock_queue_up_down = list_create();
                            struct list_t *lock_queue_down_up = list_create();
                            for (w = 0; w < wsize ; w++)
                            {
                                    dir_lock = dir_lock_get(dir,x,y,w);
                                    dir_lock->dir_entry_list = list_create();
                                    dir_lock->dir = dir;
                                    dir_lock->lock_list_up_down = lock_queue_up_down;
                                    dir_lock->lock_list_down_up = lock_queue_down_up;
                                    assert(zsize == 1);
                                    for (z = 0; z < zsize; z++)
                                    {
                                            dir_entry = dir_entry_get(dir, x, y, z, w);
                                            dir_entry->owner = DIR_ENTRY_OWNER_NONE;
                                            dir_entry->sharer = xcalloc(sharer_size,sizeof(unsigned char));
                                            dir_entry->dir_lock = dir_lock;
                                            dir_entry->set = x;
                                            dir_entry->way = y;
                                            dir_entry->w = w;
                                            dir_entry->x = x;
                                            dir_entry->y = y;
                                            dir_entry->z = z;
                                            dir_entry->tag = -1;
                                            dir_entry->transient_tag = -1;
                                            list_add(dir_lock->dir_entry_list, dir_entry);
                                            if(z == 0)
                                            {
                                                cache_block = cache_get_block_new(mod->cache,x,y);
                                                dir_entry->cache_block = cache_block;
                                                //cache_block->dir_entries = dir_entry_get(dir, x, y, 0, w);
                                                if(w == 0)
                                                    cache_block->dir_entry_selected = dir_entry;
                                            }
                                    }
                            }
                    }
            }
        }else if(dir->extra_dir_structure_type == extra_dir_per_cache){
            /* Reset all owners */
            struct list_t *lock_queue_up_down = list_create();
            struct list_t *lock_queue_down_up = list_create();
            dir->extra_dir_entries = xcalloc(zsize * wsize, sizeof(struct dir_entry_t));
            for (int w = 0; w < wsize; w++)
            {
                dir_lock = xcalloc(1, sizeof(struct dir_lock_t));
                dir_lock->dir_entry_list = list_create();
                dir_lock->dir = dir;
                dir_lock->lock_list_up_down = lock_queue_up_down;
                dir_lock->lock_list_down_up = lock_queue_down_up;
                assert(zsize == 1);
                for (z = 0; z < zsize; z++)
                {
                    dir_entry = dir->extra_dir_entries + (w * zsize) + z;
                    dir_entry->owner = DIR_ENTRY_OWNER_NONE;
                    dir_entry->sharer = xcalloc(sharer_size,sizeof(unsigned char));
                    dir_entry->dir_lock = dir_lock;
                    dir_entry->set = -1;
                    dir_entry->way = -1;
                    dir_entry->w = w;
                    dir_entry->x = -1;
                    dir_entry->y = -1;
                    dir_entry->z = z;
                    dir_entry->tag = -1;
                    dir_entry->transient_tag = -1;
                    list_add(dir_lock->dir_entry_list, dir_entry);
                }
            }
        }
	/* Return */
	return dir;
}


void dir_free(struct dir_t *dir)
{
        struct dir_entry_t *dir_entry;
   
    	for (int x = 0; x < dir->xsize; x++)
	{
		for (int y = 0; y < dir->ysize; y++)
		{
                        for (int w = 0; w < dir->wsize ; w++)
                        {
                                for (int z = 0; z < dir->zsize; z++)
                                {
                                        dir_entry = dir_entry_get(dir, x, y, z, w);
                                        list_free(dir_entry->dir_lock->dir_entry_list);
                                        if(w == 0)
                                        {
                                            list_free(dir_entry->dir_lock->lock_list_up_down);  
                                            list_free(dir_entry->dir_lock->lock_list_down_up);
                                        }
                                        free(dir_entry->sharer);
                                }
                        }
                }
        }
        
        if(dir->extra_dir_structure_type == extra_dir_per_cache)
        {
            struct dir_entry_t *dir_entry;
            for (int w = 0; w < dir->wsize; w++)
            {
                for (int z = 0; z < dir->zsize; z++)
                {
                    dir_entry = dir->extra_dir_entries + (w * dir->zsize) + z;
                    free(dir_entry->sharer);                  
                }
                free(dir_entry->dir_lock->dir_entry_list);
                free(dir_entry->dir_lock->lock_list_up_down);
                free(dir_entry->dir_lock->lock_list_down_up);
                free(dir_entry->dir_lock);
            }
            free(dir->extra_dir_entries);
        }
        
	free(dir->name);
	free(dir->dir_lock_file);
        free(dir->dir_entry_file);
	free(dir);
}


struct dir_entry_t *dir_entry_get(struct dir_t *dir, int x, int y, int z, int w)
{
	assert(IN_RANGE(x, 0, dir->xsize - 1) || (x == -1 && y == -1 && dir->extra_dir_structure_type == extra_dir_per_cache));
	assert(IN_RANGE(y, 0, dir->ysize - 1) || (x == -1 && y == -1 && dir->extra_dir_structure_type == extra_dir_per_cache));
	assert(IN_RANGE(z, 0, dir->zsize - 1));
        assert(IN_RANGE(w, 0, dir->mod->cache->dir_entry_per_line - 1));
        if(x == -1 && y == -1)
        {
            return dir->extra_dir_entries + w * dir->zsize + z; 
        }else{
            return dir->dir_entry_file + (x * dir->ysize * dir->zsize * dir->mod->cache->dir_entry_per_line + y * dir->zsize * dir->mod->cache->dir_entry_per_line + w * dir->zsize + z);
        }
}


void dir_entry_dump_sharers(struct dir_entry_t *dir_entry)
{
	int i;
	mem_debug("  %d sharers: { ", dir_entry->num_sharers);
	for (i = 0; i < dir_entry->dir_lock->dir->num_nodes; i++)
		if (dir_entry_is_sharer(dir_entry, i))
			mem_debug("%d ", i);
	mem_debug("}\n");
}


void dir_entry_set_owner(struct dir_entry_t *dir_entry, int node)
{
	/* Set owner */
	assert(node == DIR_ENTRY_OWNER_NONE || IN_RANGE(node, 0, dir_entry->dir_lock->dir->num_nodes - 1));
	dir_entry->owner = node;

	/* Trace */
	mem_trace("mem.set_owner dir=\"%s\" x=%d y=%d z=%d w=%d owner=%d\n",
		dir_entry->dir_lock->dir->name, dir_entry->x, dir_entry->y, dir_entry->z, dir_entry->w, node);
}


void dir_entry_set_sharer(struct dir_entry_t *dir_entry, int node)
{
	/* Nothing if sharer was already set */
	assert(IN_RANGE(node, 0, dir_entry->dir_lock->dir->num_nodes - 1));
	if (dir_entry->sharer[node / 8] & (1 << (node % 8)))
		return;

	/* Set sharer */
	dir_entry->sharer[node / 8] |= 1 << (node % 8);
	dir_entry->num_sharers++;
	assert(dir_entry->num_sharers <= dir_entry->dir_lock->dir->num_nodes);

	/* Debug */
	mem_trace("mem.set_sharer dir=\"%s\" x=%d y=%d z=%d w=%d sharer=%d\n",
		dir_entry->dir_lock->dir->name, dir_entry->x, dir_entry->y, dir_entry->z, dir_entry->w, node);
}


void dir_entry_clear_sharer(struct dir_entry_t *dir_entry, int node)
{
	/* Nothing if sharer is not set */
	assert(IN_RANGE(node, 0, dir_entry->dir_lock->dir->num_nodes - 1));
	if (!(dir_entry->sharer[node / 8] & (1 << (node % 8))))
		return;

	/* Clear sharer */
	dir_entry->sharer[node / 8] &= ~(1 << (node % 8));
	assert(dir_entry->num_sharers > 0);
	dir_entry->num_sharers--;

	/* Debug */
	mem_trace("mem.clear_sharer dir=\"%s\" x=%d y=%d z=%d w=%d sharer=%d\n",
		dir_entry->dir_lock->dir->name, dir_entry->x, dir_entry->y, dir_entry->z, dir_entry->w, node);
}


void dir_entry_clear_all_sharers(struct dir_entry_t *dir_entry)
{
	int i;

	/* Clear sharers */
	dir_entry->num_sharers = 0;
	for (i = 0; i < dir_entry->dir_lock->dir->dir_entry_sharers_size; i++)
		dir_entry->sharer[i] = 0;

	/* Debug */
	mem_trace("mem.clear_all_sharers dir=\"%s\" x=%d y=%d z=%d w=%d\n",
		dir_entry->dir_lock->dir->name, dir_entry->x, dir_entry->y, dir_entry->z, dir_entry->w);
}


int dir_entry_is_sharer(struct dir_entry_t *dir_entry, int node)
{
	assert(IN_RANGE(node, 0, dir_entry->dir_lock->dir->num_nodes - 1));
	return (dir_entry->sharer[node / 8] & (1 << (node % 8))) > 0;
}


int dir_entry_group_shared_or_owned(struct dir_t *dir, int x, int y, int w)
{
	struct dir_entry_t *dir_entry;
	int z;
        
	for (z = 0; z < dir->zsize; z++)
	{
		dir_entry = dir_entry_get(dir, x, y, z, w);
		if (dir_entry->num_sharers || DIR_ENTRY_VALID_OWNER(dir_entry))
			return 1;
	}
	return 0;
}


struct dir_lock_t *dir_lock_get(struct dir_t *dir, int x, int y, int w)
{
        assert(IN_RANGE(x, 0, dir->xsize - 1));
	assert(IN_RANGE(y, 0, dir->ysize - 1));
	assert(IN_RANGE(w, 0, dir->mod->cache->dir_entry_per_line - 1));
	struct dir_lock_t *dir_lock;

	dir_lock = &dir->dir_lock_file[x * dir->ysize * dir->mod->cache->dir_entry_per_line + y * dir->mod->cache->dir_entry_per_line + w];
	return dir_lock;
}

int dir_entry_lock(struct dir_entry_t *dir_entry, int event, struct mod_stack_t *stack)
{
	struct dir_lock_t *dir_lock;
	//struct mod_stack_t *lock_queue_iter;
        struct list_t *lock_queue;
        
	/* Get lock */
	//assert(x >= 0 && x < dir->xsize && y >= 0 && y < dir->ysize);
	//dir_lock = &dir->dir_lock_file[x * dir->ysize + y];
        dir_lock = dir_entry->dir_lock;
        
        
 
        
        if(stack->request_dir == mod_request_down_up)
        {
            lock_queue = dir_lock->lock_list_down_up;
        }else{
            lock_queue = dir_lock->lock_list_up_down;
        }
        
	/* If the entry is already locked, enqueue a new waiter and
	 * return failure to lock. */
	if (dir_lock->lock /*|| (dir_entry->cache_block->dir_entry_selected != dir_entry && dir_lock->dir->extra_dir_used >= dir_lock->dir->extra_dir_max && stack->hit)*/)
	{
		/* Enqueue the stack to the end of the lock queue */
		//stack->dir_lock_next = NULL;
		stack->dir_lock_event = event;
                list_add(lock_queue,stack);
                mem_debug("    0x%x access suspended\n", stack->addr);
		return 0;
	}
        
        //dir_lock extra?
        /*assert(dir_lock->dir->extra_dir_used >= 0);
        if(dir_entry->cache_block->dir_entry_selected != dir_entry && stack->hit == 0)
        {
            dir_entry->is_extra = true;
            dir_lock->dir->extra_dir_used++;
            printf("%d\n",dir_lock->dir->extra_dir_used);
        }*/
       
	/* Trace */
	mem_trace("mem.new_access_block cache=\"%s\" access=\"A-%lld\" set=%d way=%d  w=%d\n",
		dir_lock->dir->name, stack->id, dir_entry->x, dir_entry->y, dir_entry->w);

	/* Lock entry */
	dir_lock->lock = 1;
        dir_entry->lock_cycle = asTiming(si_gpu)->cycle;
	//dir_lock->stack_id = stack->id;
	//dir_lock->stack = stack->ret_stack;
	//stack->ret_stack->dir_lock = dir_lock;
	dir_lock->stack = stack;
	stack->dir_lock = dir_lock;
	return 1;
}


void dir_entry_unlock(struct dir_entry_t *dir_entry)
{
	struct dir_lock_t *dir_lock;
	struct mod_stack_t *stack = NULL;
	FILE *f;

	/* Get lock */
	//assert(x >= 0 && x < dir->xsize && y >= 0 && y < dir->ysize);
	//dir_lock = &dir->dir_lock_file[x * dir->ysize + y];
        dir_lock = dir_entry->dir_lock;
        if(dir_entry->is_extra){
            dir_entry->is_extra = false;
            dir_lock->dir->extra_dir_used--;
            if(dir_lock->dir->frc_extended_set != 0 && dir_lock->dir->extra_dir_used == 0)
            {
                dir_lock->dir->frc_extended_set = 0;
            }
            //int conversion_sets_dir_to_cache = dir_lock->dir->mod->cache->num_sets/ dir_lock->dir->mod->dir->extra_dir_sets;
            /*int mods;
            if(dir_lock->dir->mod->range_kind == mod_range_bounds)
            {
                mods = 1;
            }else if(dir_lock->dir->mod->range_kind == mod_range_interleaved){
                mods = dir_lock->dir->mod->range.interleaved.mod;
            }else{
                fatal("target_mod->range_kind invalid");
            }
            int set = ((dir_lock->stack->tag >> dir_lock->dir->mod->cache->log_block_size) / mods) % dir_lock->dir->mod->dir->extra_dir_sets;*/
            dir_lock->dir->mod->dir->extra_dir_set_entries_used[dir_entry->frc_set]--;
        }
        
	/* Wake up first waiter */
        if(list_count(dir_lock->lock_list_down_up) > 0)
        {
            stack = list_pop(dir_lock->lock_list_down_up);
        }
        else if(list_count(dir_lock->lock_list_up_down) > 0)
        {
            stack = list_pop(dir_lock->lock_list_up_down); 
        }
        
        
	if (stack)
	{
		/* Debug */
		f = debug_file(mem_debug_category);
		if (f)
		{
                    struct mod_stack_t *stack_aux;
			mem_debug("    A-%lld resumed", stack->id);
			if (list_count(dir_lock->lock_list_down_up) > 0 || list_count(dir_lock->lock_list_up_down) > 0)
			{
				mem_debug(" - {");
				for (int i = 0; i < list_count(dir_lock->lock_list_down_up); i++)
                                {
                                        stack_aux = list_get(dir_lock->lock_list_down_up,i);
					mem_debug(" A-%lld", stack_aux->id);
                                }
                                for (int i = 0; i < list_count(dir_lock->lock_list_up_down); i++)
                                {
                                        stack_aux = list_get(dir_lock->lock_list_up_down,i);
					mem_debug(" A-%lld", stack_aux->id);
                                }
				mem_debug(" } still waiting");
			}
			mem_debug("\n");
		}

		/* Wake up access */
		stack->event = stack->dir_lock_event;
		stack->dir_lock_event = 0;
		esim_schedule_mod_stack_event(stack, 1);
		//esim_schedule_event(dir_lock->lock_queue->dir_lock_event, dir_lock->lock_queue, 1);
	}

	/* Trace */
	mem_trace("mem.end_access_block cache=\"%s\" access=\"A-%lld\" set=%d way=%d  w=%d\n",
		dir_lock->dir->name, dir_lock->stack->id, dir_entry->x, dir_entry->y, dir_entry->w);

	/* Unlock entry */
	dir_lock->lock = 0;
	//dir_lock->stack_id = 0;
	//if(dir_lock->stack)
        if(dir_entry->cache_block->dir_entry_selected == dir_entry)
        {
            assert(dir_entry->lock_cycle);
            add_ics_dir_lock_cycles(dir_entry->dir_lock->stack->target_mod->level,asTiming(si_gpu)->cycle - dir_entry->lock_cycle);
        }
        dir_entry->lock_cycle = 0;
	dir_lock->stack->dir_lock = NULL;
	dir_lock->stack = NULL;
}

