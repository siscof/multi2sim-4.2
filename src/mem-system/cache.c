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

#include <lib/esim/trace.h>
#include <lib/esim/esim.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "cache.h"
#include "mem-system.h"
#include "prefetcher.h"
#include "nmoesi-protocol.h"

/*
 * Public Variables
 */

struct str_map_t cache_policy_map =
{
	5, {
		{ "LRU", cache_policy_lru },
                { "LRU_EXT", cache_policy_lru_ext }, 
                { "LRU_BASE", cache_policy_lru_base },
		{ "FIFO", cache_policy_fifo },
		{ "Random", cache_policy_random }
	}
};

struct str_map_t cache_block_state_map =
{
	6, {
		{ "N", cache_block_noncoherent },
		{ "M", cache_block_modified },
		{ "O", cache_block_owned },
		{ "E", cache_block_exclusive },
		{ "S", cache_block_shared },
		{ "I", cache_block_invalid }
	}
};




/*
 * Private Functions
 */

/*enum cache_waylist_enum
{
	cache_waylist_head,
	cache_waylist_tail
};*/

void cache_update_waylist(struct cache_set_t *set,
	struct cache_block_t *blk, enum cache_waylist_enum where)
{
	if (!blk->way_prev && !blk->way_next)
	{
		assert(set->way_head == blk && set->way_tail == blk);
		return;

	}
	else if (!blk->way_prev)
	{
		assert(set->way_head == blk && set->way_tail != blk);
		if (where == cache_waylist_head)
			return;
		set->way_head = blk->way_next;
		blk->way_next->way_prev = NULL;

	}
	else if (!blk->way_next)
	{
		assert(set->way_head != blk && set->way_tail == blk);
		if (where == cache_waylist_tail)
			return;
		set->way_tail = blk->way_prev;
		blk->way_prev->way_next = NULL;

	}
	else
	{
		assert(set->way_head != blk && set->way_tail != blk);
		blk->way_prev->way_next = blk->way_next;
		blk->way_next->way_prev = blk->way_prev;
	}

	if (where == cache_waylist_head)
	{
		blk->way_next = set->way_head;
		blk->way_prev = NULL;
		set->way_head->way_prev = blk;
		set->way_head = blk;
	}
	else
	{
		blk->way_prev = set->way_tail;
		blk->way_next = NULL;
		set->way_tail->way_next = blk;
		set->way_tail = blk;
	}
}





/*
 * Public Functions
 */


struct cache_t *cache_create(char *name, unsigned int num_sets, unsigned int block_size,
	unsigned int assoc, enum cache_policy_t policy, int dir_entry_per_block, int extra_dir_structure_type)
{
	struct cache_t *cache;
	struct cache_block_t *block;
	unsigned int set, way;

	/* Initialize */
	cache = xcalloc(1, sizeof(struct cache_t));
	cache->name = xstrdup(name);
	cache->num_sets = num_sets;
	cache->block_size = block_size;
	cache->assoc = assoc;
	cache->policy = policy;
        cache->extra_dir_structure_type = extra_dir_structure_type;

	/* Derived fields */
	assert(!(num_sets & (num_sets - 1)));
	assert(!(block_size & (block_size - 1)));
	//assert(!(assoc & (assoc - 1)));
	cache->log_block_size = log_base2(block_size);
	cache->block_mask = block_size - 1;
        cache->dir_entry_per_line = dir_entry_per_block;

	/* Initialize array of sets */
	cache->sets = xcalloc(num_sets, sizeof(struct cache_set_t));
	for (set = 0; set < num_sets; set++)
	{
		/* Initialize array of blocks */
		cache->sets[set].blocks = xcalloc(assoc, sizeof(struct cache_block_t));
		cache->sets[set].way_head = &cache->sets[set].blocks[0];
		cache->sets[set].way_tail = &cache->sets[set].blocks[assoc - 1];
                
		for (way = 0; way < assoc; way++)
		{
			block = &cache->sets[set].blocks[way];
                        block->dir_entry_size = dir_entry_per_block;
			block->way = way;
                        //block->dir_entry = xcalloc(block->dir_entry_size, sizeof(struct dir_entry_t));
                        /*for(int i = 0; i > block->dir_entry_size; i++)
                        {
                            block->dir_entry[i].owner = DIR_ENTRY_OWNER_NONE;
                            block->dir_entry[i].sharer = xcalloc(2,sizeof(unsigned char));
                            block->dir_entry[i].dir_lock = xcalloc(1, sizeof(struct dir_lock_t));;
                            block->dir_entry[i].cache_block = block;
                            block->dir_entry[i].set = set;
                            block->dir_entry[i].way = way;
                            block->dir_entry[i].dir_lock->dir_entry = &block->dir_entry[i];
                        }*/
			//block->transient_tag = -1;
			block->way_prev = way ? &cache->sets[set].blocks[way - 1] : NULL;
			block->way_next = way < assoc - 1 ? &cache->sets[set].blocks[way + 1] : NULL;
		}
	}

	/* Return it */
	return cache;
}


void cache_free(struct cache_t *cache)
{
	unsigned int set;

	for (set = 0; set < cache->num_sets; set++)
		free(cache->sets[set].blocks);
	free(cache->sets);
	free(cache->name);
	if (cache->prefetcher)
		prefetcher_free(cache->prefetcher);
	free(cache);
}


/* Return {set, tag, offset} for a given address */
void cache_decode_address(struct cache_t *cache, unsigned int addr, int *set_ptr, int *tag_ptr,
	unsigned int *offset_ptr)
{
	PTR_ASSIGN(set_ptr, (addr >> cache->log_block_size) % cache->num_sets);
	PTR_ASSIGN(tag_ptr, addr & ~cache->block_mask);
	PTR_ASSIGN(offset_ptr, addr & cache->block_mask);
}


/* Look for a block in the cache. If it is found and its state is other than 0,
 * the function returns 1 and the state and way of the block are also returned.
 * The set where the address would belong is returned anyways. */
int cache_find_block(struct cache_t *cache, unsigned int addr, int *set_ptr, int *way_ptr,
	int *state_ptr)
{
	int set, tag, way;

	/* Locate block */
	tag = addr & ~cache->block_mask;
	set = (addr >> cache->log_block_size) % cache->num_sets;
	PTR_ASSIGN(set_ptr, set);
	PTR_ASSIGN(state_ptr, 0);  /* Invalid */
	for (way = 0; way < cache->assoc; way++)
		if (cache->sets[set].blocks[way].dir_entry_selected->tag == tag && cache->sets[set].blocks[way].dir_entry_selected->state)
			break;

        if(cache->mod->dir->extra_dir_structure_type == extra_dir_per_cache )
        {
            for (int w = 0; w < cache->mod->dir->wsize ; w++ )
            {
               if (cache->mod->dir->extra_dir_entries[w].tag == tag && cache->mod->dir->extra_dir_entries[w].state)
               {
                    PTR_ASSIGN(set_ptr, -1);
                    PTR_ASSIGN(way_ptr, -1);
                    PTR_ASSIGN(state_ptr, cache->sets[set].blocks[way].dir_entry_selected->state);
                    return 1;
               }
            }
        }

	/* Block not found */
	if (way == cache->assoc)
		return 0;

	/* Block found */
	PTR_ASSIGN(way_ptr, way);
	PTR_ASSIGN(state_ptr, cache->sets[set].blocks[way].dir_entry_selected->state);
	return 1;
}

void cache_set_block_new(struct cache_t *cache, struct mod_stack_t *stack, int state)
{
    
        int set = stack->dir_entry->set;
        int way = stack->dir_entry->way;
        int tag = stack->addr & ~cache->block_mask;
        
        //assert(stack->dir_entry->set == stack->set && stack->dir_entry->way == stack->way);     
        
        if(cache->mod->dir->extra_dir_structure_type == extra_dir_per_cache_line)
        {
            assert(set >= 0 && set < cache->num_sets);
            assert(way >= 0 && way < cache->assoc);
            //if(set != -1){
            mem_debug("    %lld 0x%x %s hit: set=%d, way=%d, w=%d, state=%s replacing tag=0x%x, w=%d, state=%s\n", stack->id,
                            stack->tag, cache->name, stack->dir_entry->set, stack->dir_entry->way, stack->dir_entry->w,
                            str_map_value(&cache_block_state_map, stack->dir_entry->state), cache->sets[set].blocks[way].dir_entry_selected->tag, 
                            cache->sets[set].blocks[way].dir_entry_selected->w, str_map_value(&cache_block_state_map, 
                            cache->sets[set].blocks[way].dir_entry_selected->state));

            mem_trace("mem.set_block cache=\"%s\" set=%d way=%d tag=0x%x state=\"%s\"\n",
                            cache->name, set, way, tag,
                            str_map_value(&cache_block_state_map, state));
            //}else{
            /*mem_debug("    %lld 0x%x %s hit: set=%d, way=%d, w=%d, state=%s replacing tag=0x%x, w=%d, state=%s\n", stack->id,
                stack->tag, cache->name, stack->dir_entry->set, stack->dir_entry->way, stack->dir_entry->w,
                str_map_value(&cache_block_state_map, stack->dir_entry->state), cache->sets[set].blocks[way].dir_entry_selected->tag, 
                cache->sets[set].blocks[way].dir_entry_selected->w, str_map_value(&cache_block_state_map, 
                cache->sets[set].blocks[way].dir_entry_selected->state));

            mem_trace("mem.set_block cache=\"%s\" set=%d way=%d tag=0x%x state=\"%s\"\n",
                            cache->name, set, way, tag,
                            str_map_value(&cache_block_state_map, state));*/
            //}
       if (cache->policy == cache_policy_fifo && cache->sets[set].blocks[way].dir_entry_selected->tag != tag)
                    cache_update_waylist(&cache->sets[set], &cache->sets[set].blocks[way], cache_waylist_head);

            if(state == cache_block_invalid || stack->dir_entry->tag != tag)
            {
                cache->sets[set].blocks[way].dirty_mask = 0;
                cache->sets[set].blocks[way].valid_mask = 0;
            }
            
            if(tag == stack->dir_entry->transient_tag)
                    stack->dir_entry->transient_tag = -1;
            
            if(state != cache_block_invalid)
            {
                stack->dir_entry->tag = tag;
            }
            else
                stack->dir_entry->tag = -1;

            stack->dir_entry->state = state;

            //hacer evict!!! de dir_entry_selected
            if(state != cache_block_invalid && cache->sets[set].blocks[way].dir_entry_selected != stack->dir_entry ) 
                //&& cache->sets[set].blocks[way].dir_entry_selected->state != cache_block_invalid)
            {
                if(cache->sets[set].blocks[way].dir_entry_selected->state != cache_block_invalid)
                {
                    int addr = cache->sets[set].blocks[way].dir_entry_selected->tag;
                    struct mod_stack_t *new_stack1 = mod_stack_create(stack->id, stack->target_mod, addr, 0, NULL);
                    new_stack1->dir_entry = cache->sets[set].blocks[way].dir_entry_selected;  

                    struct mod_stack_t *new_stack2 = mod_stack_create(stack->id, 
                            mod_get_low_mod(stack->target_mod, addr), addr,
                            EV_MOD_NMOESI_EVICT_CHECK, new_stack1);
                    new_stack2->return_mod = stack->target_mod;
                    new_stack2->dir_entry = cache->sets[set].blocks[way].dir_entry_selected;

                    new_stack2->event = EV_MOD_NMOESI_EVICT_LOCK_DIR;
                    esim_schedule_mod_stack_event(new_stack2, 0);
                }
                cache->sets[set].blocks[way].dir_entry_selected = stack->dir_entry; 
            }
        }else if(cache->mod->dir->extra_dir_structure_type == extra_dir_per_cache){
            
            if (cache->policy == cache_policy_fifo && cache->sets[set].blocks[way].dir_entry_selected->tag != tag)
                    cache_update_waylist(&cache->sets[set], &cache->sets[set].blocks[way], cache_waylist_head);

            if(state == cache_block_invalid || stack->dir_entry->tag != tag)
            {
                cache->sets[set].blocks[way].dirty_mask = 0;
                cache->sets[set].blocks[way].valid_mask = 0;
            }
            
            if(tag == stack->dir_entry->transient_tag)
                    stack->dir_entry->transient_tag = -1;
            
            if(state != cache_block_invalid)
            {
                stack->dir_entry->tag = tag;
            }
            else
                stack->dir_entry->tag = -1;

            stack->dir_entry->state = state;
            
            if(state != cache_block_invalid && cache->sets[set].blocks[way].dir_entry_selected != stack->dir_entry ) 
                //&& cache->sets[set].blocks[way].dir_entry_selected->state != cache_block_invalid)
            {
                assert(!cache->sets[set].blocks[way].dir_entry_selected->dir_lock->lock);
                dir_entry_swap(cache->sets[set].blocks[way].dir_entry_selected, stack->dir_entry); 
                stack->dir_lock = NULL;
                if(stack->dir_entry->state != cache_block_invalid)
                {
                    int addr = stack->dir_entry->tag;
                    struct mod_stack_t *new_stack1 = mod_stack_create(stack->id, stack->target_mod, addr, 0, NULL);
                    new_stack1->dir_entry = stack->dir_entry;  

                    struct mod_stack_t *new_stack2 = mod_stack_create(stack->id, 
                            mod_get_low_mod(stack->target_mod, addr), addr,
                            EV_MOD_NMOESI_EVICT_CHECK, new_stack1);
                    new_stack2->return_mod = stack->target_mod;
                    new_stack2->dir_entry = stack->dir_entry;
                    new_stack2->dir_lock = stack->dir_entry->dir_lock;
                    new_stack2->event = EV_MOD_NMOESI_EVICT;
                    esim_schedule_mod_stack_event(new_stack2, 0);
                }
            }
        }     
}


/* Set the tag and state of a block.
 * If replacement policy is FIFO, update linked list in case a new
 * block is brought to cache, i.e., a new tag is set. */
void cache_set_block(struct cache_t *cache, int set, int way, int tag, int state)
{
	assert(set >= 0 && set < cache->num_sets);
	assert(way >= 0 && way < cache->assoc);

	mem_trace("mem.set_block cache=\"%s\" set=%d way=%d tag=0x%x state=\"%s\"\n",
			cache->name, set, way, tag,
			str_map_value(&cache_block_state_map, state));

	if (cache->policy == cache_policy_fifo
		&& cache->sets[set].blocks[way].dir_entry_selected->tag != tag)
		cache_update_waylist(&cache->sets[set],
			&cache->sets[set].blocks[way],
			cache_waylist_head);

	if(tag == cache->sets[set].blocks[way].dir_entry_selected->transient_tag)
		cache->sets[set].blocks[way].dir_entry_selected->transient_tag = -1;

	cache->sets[set].blocks[way].dir_entry_selected->tag = tag;
	cache->sets[set].blocks[way].dir_entry_selected->state = state;
	cache->sets[set].blocks[way].dirty_mask = 0;
	cache->sets[set].blocks[way].valid_mask = 0;
}


void cache_get_block(struct cache_t *cache, int set, int way, int *tag_ptr, int *state_ptr)
{
	assert(set >= 0 && set < cache->num_sets);
	assert(way >= 0 && way < cache->assoc);
	PTR_ASSIGN(tag_ptr, cache->sets[set].blocks[way].dir_entry_selected->tag);
	PTR_ASSIGN(state_ptr, cache->sets[set].blocks[way].dir_entry_selected->state);
}

struct cache_block_t *cache_get_block_new(struct cache_t *cache, int set, int way)
{
	assert(set >= 0 && set < cache->num_sets);
	assert(way >= 0 && way < cache->assoc);
	return &cache->sets[set].blocks[way];
}

/* Update LRU counters, i.e., rearrange linked list in case
 * replacement policy is LRU. */
void cache_access_block(struct cache_t *cache, int set, int way)
{
	int move_to_head;

	assert(set >= 0 && set < cache->num_sets);
	assert(way >= 0 && way < cache->assoc);

	/* A block is moved to the head of the list for LRU policy.
	 * It will also be moved if it is its first access for FIFO policy, i.e., if the
	 * state of the block was invalid. */
	move_to_head = cache->policy == cache_policy_lru || cache_policy_lru_base || cache_policy_lru_ext ||
		(cache->policy == cache_policy_fifo && !cache->sets[set].blocks[way].dir_entry_selected->state);
	if (move_to_head && cache->sets[set].blocks[way].way_prev)
		cache_update_waylist(&cache->sets[set],
			&cache->sets[set].blocks[way],
			cache_waylist_head);
}


/* Return the way of the block to be replaced in a specific set,
 * depending on the replacement policy */
int cache_replace_block(struct cache_t *cache, int set)
{
	struct cache_block_t *block;

	/* Try to find an invalid block. Do this in the LRU order, to avoid picking the
	 * MRU while its state has not changed to valid yet. */
	assert(set >= 0 && set < cache->num_sets);

	for (block = cache->sets[set].way_tail; block; block = block->way_prev)
	{
		if (!block->dir_entry_selected->state && block->dir_entry_selected->transient_tag == 0)
		{
			cache_update_waylist(&cache->sets[set], block,
				cache_waylist_head);
			return block->way;
		}
	}

	/* LRU and FIFO replacement: return block at the
	 * tail of the linked list */
	if(cache->policy == cache_policy_lru ||
		cache->policy == cache_policy_fifo)
	{
		int way = cache->sets[set].way_tail->way;
		cache_update_waylist(&cache->sets[set], cache->sets[set].way_tail,
			cache_waylist_head);

		return way;
	}
        
        if(cache->policy == cache_policy_lru_base)
	{
		int way = cache->sets[set].way_tail->way;
		return way;
	}
        
        if(cache->policy == cache_policy_lru_ext)
	{
                //struct dir_lock_t *dir_lock;
                
                block = cache->sets[set].way_tail;
                for(; block != NULL; block = block->way_prev)
                {
                    //dir_lock = dir_lock_get(cache->mod->dir, set, block->way);
                    if(!block->dir_entry_selected->dir_lock->lock)
                        break; 
		}
                
                
		/*int way = cache->sets[set].way_tail->way;
                for(; cache->sets[set].blocks[way]->way != null; way = cache->sets[set].blocks[way]->way)
                {
                    way = (way+i)%cache->assoc;
                    dir_lock = dir_lock_get(cache->mod->dir, set, way);
                    if(!dir_lock->lock)
                        break;
                    
		}*/
                if(block == NULL)
                {
                    block = cache->sets[set].way_tail;
                }
                
                cache_update_waylist(&cache->sets[set], block,
			cache_waylist_head);

		return block->way;
	}

	/* Random replacement */
	assert(cache->policy == cache_policy_random);
	return random() % cache->assoc;
}


void cache_set_transient_tag(struct cache_t *cache, int set, int way, int tag)
{
	struct cache_block_t *block;

	/* Set transient tag */
	block = &cache->sets[set].blocks[way];
	block->dir_entry_selected->transient_tag = tag;
}
void cache_clean_block_dirty(struct cache_t *cache, int set, int way)
{
	cache->sets[set].blocks[way].dirty_mask = 0;
}

void cache_clean_block_valid(struct cache_t *cache, int set, int way)
{
	cache->sets[set].blocks[way].valid_mask = 0;
}

unsigned int cache_clean_word_dirty(struct cache_t *cache, int set, int way)
{
	unsigned int mask = cache->sets[set].blocks[way].dirty_mask;
	unsigned int dirty = 0;
	unsigned int addr = -1;
	int words = 0;

	assert(cache->sets[set].blocks[way].dir_entry_selected->state);

	if(mask)
	{
		do
		{
			dirty = mask & (1 << words);
			words++;
		}
		while(!dirty);

		addr = cache->sets[set].blocks[way].dir_entry_selected->tag + ((words - 1) * 4);
		cache->sets[set].blocks[way].dirty_mask &= (~dirty);

	}
	return addr;
}

unsigned int cache_get_block_dirty_mask(struct cache_t *cache, int set, int way)
{
	return cache->sets[set].blocks[way].dirty_mask;
}

void cache_write_block_dirty_mask(struct cache_t *cache, int set, int way, unsigned int dirty_mask)
{
	/*unsigned int tag = stack->addr & ~cache->block_mask;
	unsigned int shift = (stack->addr - tag) >> 2;
	unsigned int mask = 0;
	if(words == 0)
		words = 1;

	assert((tag + cache->block_size) >= (addr + words * 4));

	for(;words > 0 ; words--)
	{
		mask |= 1 << (shift + words - 1);
	}*/
	//assert(!(cache->sets[set].blocks[way].dirty_mask & dirty_mask));
	cache->sets[set].blocks[way].dirty_mask |= dirty_mask;
}

void cache_write_block_valid_mask(struct cache_t *cache, int set, int way, unsigned int mask)
{
	cache->sets[set].blocks[way].valid_mask |= mask;
}
