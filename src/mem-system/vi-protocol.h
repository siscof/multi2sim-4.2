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

#ifndef MEM_SYSTEM_VI_PROTOCOL_H
#define MEM_SYSTEM_VI_PROTOCOL_H


/* VI Event-Driven Simulation */

extern int EV_MOD_VI_LOAD;
extern int EV_MOD_VI_LOAD_SEND;
extern int EV_MOD_VI_LOAD_RECEIVE;
extern int EV_MOD_VI_LOAD_LOCK;
extern int EV_MOD_VI_LOAD_LOCK2;
extern int EV_MOD_VI_LOAD_ACTION;
extern int EV_MOD_VI_LOAD_MISS;
extern int EV_MOD_VI_LOAD_UNLOCK;
extern int EV_MOD_VI_LOAD_FINISH;

extern int EV_MOD_VI_NC_LOAD;
extern int EV_MOD_VI_NC_STORE;

extern int EV_MOD_VI_STORE;
extern int EV_MOD_VI_STORE_SEND;
extern int EV_MOD_VI_STORE_RECEIVE;
extern int EV_MOD_VI_STORE_LOCK;
extern int EV_MOD_VI_STORE_ACTION;
extern int EV_MOD_VI_STORE_UNLOCK;
extern int EV_MOD_VI_STORE_FINISH;


extern int EV_MOD_VI_FIND_AND_LOCK;
extern int EV_MOD_VI_FIND_AND_LOCK_PORT;
extern int EV_MOD_VI_FIND_AND_LOCK_ACTION;
extern int EV_MOD_VI_FIND_AND_LOCK_WAIT_MSHR;
extern int EV_MOD_VI_FIND_AND_LOCK_FINISH;


void mod_handler_vi_find_and_lock(int event, void *data);
void mod_handler_vi_load(int event, void *data);
void mod_handler_vi_nc_load(int event, void *data);
void mod_handler_vi_store(int event, void *data);


#endif
