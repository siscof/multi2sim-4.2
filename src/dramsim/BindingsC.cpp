/*
 *  DramSim C bindings
 *  Copyright (C) 2014 Vicent Selfa (viselol@disca.upv.es)
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

#include <cassert>
#include "MultiChannelMemorySystem.h"
#include "BindingsC.h"

extern "C"{

struct dram_system_handler_t* dram_system_create(const char *dev_desc_file, const char *sys_desc_file, unsigned int total_memory_megs, const char *vis_file)
{
	std::string dev(dev_desc_file);
	std::string sys(sys_desc_file);
	std::string vis(vis_file);
	return new MultiChannelMemorySystem(dev, sys, "", "", total_memory_megs, vis);
}


void dram_system_free(struct dram_system_handler_t *ds)
{
	ds->printStats(true);
	delete ds;
}


bool dram_system_add_read_trans(struct dram_system_handler_t *ds, unsigned long long addr, int core, int thread)
{
	return ds->addTransaction(false, (uint64_t) addr, core, thread);
}


bool dram_system_add_write_trans(struct dram_system_handler_t *ds, unsigned long long addr, int core, int thread)
{
	return ds->addTransaction(true, (uint64_t) addr, core, thread);
}


void dram_system_set_cpu_freq(struct dram_system_handler_t *ds, long long freq)
{
	ds->setCPUClockSpeed((uint64_t) freq);
}


void dram_system_cpu_tick(struct dram_system_handler_t *ds)
{
	ds->update();
}


void dram_system_dram_tick(struct dram_system_handler_t *ds)
{
	ds->actual_update();
}


void dram_system_set_epoch_length(struct dram_system_handler_t *ds, unsigned long long epoch_lenght)
{
	EPOCH_LENGTH = epoch_lenght;
}


void dram_system_print_stats(struct dram_system_handler_t *ds)
{
	ds->printStats(false);
}


void dram_system_print_final_stats(struct dram_system_handler_t *ds)
{
	ds->printStats(true);
}


bool dram_system_will_accept_trans(struct dram_system_handler_t *ds, unsigned long long addr)
{
	return 	ds->willAcceptTransaction(addr);
}


void dram_system_register_callbacks(
		struct dram_system_handler_t *ds,
		void(*read_done)(unsigned int, uint64_t, uint64_t),
		void(*write_done)(unsigned int, uint64_t, uint64_t),
		void(*report_power)(double bgpower, double burstpower, double refreshpower, double actprepower))
{
	typedef SimpleCallback<void, unsigned, uint64_t, uint64_t> SC;

	SC read_cb = SC(read_done);
	SC write_cb = SC(write_done);

	ds->RegisterCallbacks(read_cb, write_cb, report_power);
}


void dram_system_register_payloaded_callbacks(
		struct dram_system_handler_t *ds,
		void *payload,
		void(*read_done)(void*, unsigned int, uint64_t, uint64_t),
		void(*write_done)(void*, unsigned int, uint64_t, uint64_t),
		void(*report_power)(double bgpower, double burstpower, double refreshpower, double actprepower))
{
	typedef SimplePayloadedCallback<void*, void, unsigned, uint64_t, uint64_t> SPC;

	SPC read_cb = SPC(read_done, payload);
	SPC write_cb = SPC(write_done, payload);

	ds->RegisterCallbacks(read_cb, write_cb, report_power);
}


long long dram_system_get_dram_freq(struct dram_system_handler_t *ds)
{
	return ds->getDRAMClockSpeed();
}


long long dram_system_get_bwc(struct dram_system_handler_t *ds, int mc, int core)
{
	return ds->getMemoryController(mc)->getBWC(core);
}


long long dram_system_get_bwn(struct dram_system_handler_t *ds, int mc, int core)
{
	return ds->getMemoryController(mc)->getBWN(core);
}


long long dram_system_get_bwno(struct dram_system_handler_t *ds, int mc, int core)
{
	return ds->getMemoryController(mc)->getBWNO(core);
}


void dram_system_reset_bwc(struct dram_system_handler_t *ds, int mc, int core)
{
	ds->getMemoryController(mc)->setBWC(core, 0);
}


void dram_system_reset_bwn(struct dram_system_handler_t *ds, int mc, int core)
{
	ds->getMemoryController(mc)->setBWN(core, 0);
}


void dram_system_reset_bwno(struct dram_system_handler_t *ds, int mc, int core)
{
	ds->getMemoryController(mc)->setBWNO(core, 0);
}


int dram_system_get_num_mcs(struct dram_system_handler_t *ds)
{
	int num = ds->getNumMemoryControllers();
	assert(num >= 0); /* There is an implicit conversion from unsigned to signed that can potentially overflow and must be checked */
	return num;
}


} //extern "C"
