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

#ifndef MEM_SYSTEM_DIRECTORY_H
#define MEM_SYSTEM_DIRECTORY_H

#include "module.h"
#include "cache.h"
#include "lib/util/list.h"

enum extra_dir_structure_t{
    extra_dir_per_cache_line = 0,
    extra_dir_per_cache_line_set,
    extra_dir_per_cache
};

struct dir_lock_t
{
	int lock;
	//long long stack_id;
	struct mod_stack_t *stack;
	//struct mod_stack_t *lock_queue;
        struct list_t *lock_list_up_down;
        struct list_t *lock_list_down_up;
        //struct dir_entry_t *dir_entry;
        struct list_t *dir_entry_list;
        struct dir_t *dir;
};

#define DIR_ENTRY_OWNER_NONE  (-1)
#define DIR_ENTRY_VALID_OWNER(dir_entry)  ((dir_entry)->owner >= 0)

struct dir_entry_t
{
        int tag;
        int transient_tag;
        enum cache_block_state_t state;
        int set; 
        int way; 
        int x;
        int y;
        int w;
        int z;
        bool is_extra;
        int owner;  /* Node owning the block (-1 = No owner)*/
	int num_sharers;  /* Number of 1s in next field */
	//unsigned char sharer[0];   /* Bitmap of sharers (must be last field) */
        unsigned char *sharer;
        struct dir_lock_t *dir_lock;
        long long lock_cycle;
        struct cache_block_t *cache_block;
        long long life_cycles;
        int frc_set;
};

struct dir_t
{
	char *name;

	/* Number of possible sharers for a block. This determines
	 * the size of the directory entry bitmap. */
	int num_nodes;

	/* Width, height and depth of the directory. For caches, it is
	 * useful to have a 3-dim directory. XSize is the number of
	 * sets, YSize is the number of ways of the cache, and ZSize
	 * is the number of sub-blocks of size 'cache_min_block_size'
	 * that fit within a block. */
	int xsize, ysize, zsize, wsize;
        int dir_entry_sharers_size;
        
        int extra_dir_used;
        int extra_dir_sets;
        int *extra_dir_set_entries_used;
        int extra_dir_max;
        int frc_extended_set;
        
        int extra_dir_structure_type;
        struct dir_entry_t *extra_dir_entries;
	/* Array of xsize * ysize locks. Each lock corresponds to a
	 * block, i.e. a set of zsize directory entries */
        struct dir_lock_t *dir_lock_file;

	/* Last field. This is an array of xsize*ysize*zsize elements of type
	 * dir_entry_t, which have likewise variable size. */
	//unsigned char data[0];
        struct dir_entry_t *dir_entry_file;
        struct mod_t *mod;
};

enum dir_type_t
{
	dir_type_nmoesi = 0,
	dir_type_vi
};

struct dir_t *dir_create(char *name, int xsize, int ysize, int zsize, int num_nodes, struct mod_t *mod);
void dir_free(struct dir_t *dir);

struct dir_entry_t *dir_entry_get(struct dir_t *dir, int x, int y, int z, int w);

void dir_entry_set_owner(struct dir_entry_t *dir_entry, int node);
void dir_entry_set_sharer(struct dir_entry_t *dir_entry, int node);
void dir_entry_clear_sharer(struct dir_entry_t *dir_entry, int node);
void dir_entry_clear_all_sharers(struct dir_entry_t *dir_entry);
int dir_entry_is_sharer(struct dir_entry_t *dir_entry, int node);
int dir_entry_group_shared_or_owned(struct dir_t *dir, int x, int y, int w);

void dir_entry_dump_sharers(struct dir_entry_t *dir_entry);

struct dir_lock_t *dir_lock_get(struct dir_t *dir, int x, int y, int w);
int dir_entry_lock(struct dir_entry_t *dir_entry, int event, struct mod_stack_t *stack);
void dir_entry_unlock(struct dir_entry_t *dir_entry);
//struct dir_lock_t *dir_lock_get_by_stack(struct dir_t *dir, int x, int y, struct mod_stack_t *stack);

#endif
