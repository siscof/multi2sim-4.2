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

#ifndef MEM_SYSTEM_CACHE_H
#define MEM_SYSTEM_CACHE_H

#include "mod-stack.h"
#include "directory.h"

extern struct str_map_t cache_policy_map;
extern struct str_map_t cache_block_state_map;

enum cache_policy_t
{
	cache_policy_invalid = 0,
	cache_policy_lru,
        cache_policy_lru_ext,
        cache_policy_lru_base,
	cache_policy_fifo,
	cache_policy_random
};

struct cache_block_t
{
	struct cache_block_t *way_next;
	struct cache_block_t *way_prev;

	//int tag;
	//int transient_tag;
	int way;
	int prefetched;
	
        unsigned int dirty_mask;
	unsigned int valid_mask;
        struct dir_entry_t *dir_entry_selected;
        struct dir_entry_t *dir_entries;
        //struct dir_entry_t *extra_dir_entry;
        int dir_entry_size;

	//enum cache_block_state_t state;
};

struct cache_set_t
{
	struct cache_block_t *way_head;
	struct cache_block_t *way_tail;
	struct cache_block_t *blocks;
};

struct cache_t
{
	char *name;

	unsigned int num_sets;
	unsigned int block_size;
	unsigned int assoc;
	enum cache_policy_t policy;
        struct mod_t *mod;

	struct cache_set_t *sets;
	unsigned int block_mask;
	int log_block_size;
        int dir_entry_per_line;
        int extra_dir_sets;
        int dir_entry_per_line_max;
        int extra_dir_structure_type;

	struct prefetcher_t *prefetcher;
};

enum cache_waylist_enum
{
	cache_waylist_head,
	cache_waylist_tail
};

struct cache_t *cache_create(char *name, unsigned int num_sets, unsigned int block_size,
	unsigned int assoc, enum cache_policy_t policy, int dir_entry_per_block, int extra_dir_structure_type);
void cache_free(struct cache_t *cache);

void cache_decode_address(struct cache_t *cache, unsigned int addr,
	int *set_ptr, int *tag_ptr, unsigned int *offset_ptr);
int cache_find_block(struct cache_t *cache, unsigned int addr, int *set_ptr, int *pway, 
	int *state_ptr);
void cache_set_block(struct cache_t *cache, int set, int way, int tag, int state);
void cache_set_block_new(struct cache_t *cache, struct mod_stack_t *stack, int state);
void cache_get_block(struct cache_t *cache, int set, int way, int *tag_ptr, int *state_ptr);
struct cache_block_t *cache_get_block_new(struct cache_t *cache, int set, int way);

void cache_access_block(struct cache_t *cache, int set, int way);
int cache_replace_block(struct cache_t *cache, int set);
void cache_set_transient_tag(struct cache_t *cache, int set, int way, int tag);
void cache_update_waylist(struct cache_set_t *set, struct cache_block_t *blk, enum cache_waylist_enum where);

//fran 
void cache_write_block_dirty_mask(struct cache_t *cache, int set, int way, unsigned int dirty_mask);
void cache_write_block_valid_mask(struct cache_t *cache, int set, int way, unsigned int mask);
unsigned int cache_get_block_dirty_mask(struct cache_t *cache, int set, int way);
void cache_clean_block_dirty(struct cache_t *cache, int set, int way);
unsigned int cache_clean_word_dirty(struct cache_t *cache, int set, int way);
void cache_clean_block_valid(struct cache_t *cache, int set, int way);



#endif

