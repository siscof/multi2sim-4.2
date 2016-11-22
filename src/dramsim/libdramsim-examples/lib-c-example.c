#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "bindings-c.h"

void power_callback(double a, double b, double c, double d)
{
	printf("power callback: %0.3f, %0.3f, %0.3f, %0.3f\n", a, b, c, d);
}

void read_callback(unsigned int id, uint64_t address, uint64_t clock_cycle)
{
	printf("[Callback] read complete: %d 0x%lx cycle=%lu\n", id, address, clock_cycle);
}

void write_callback(unsigned int id, uint64_t address, uint64_t clock_cycle)
{
	printf("[Callback] write complete: %d 0x%lx cycle=%lu\n", id, address, clock_cycle);
}

int add_one_and_run(struct dram_system_handler_t *ms, unsigned long long addr)
{
	// Create a transaction and add it
	dram_system_add_read_trans(ms, addr);

	// Send a read to channel 1 on the same cycle
	addr = 1LL<<33 | addr;
	dram_system_add_read_trans(ms, addr);

	// Advance some time in the future
	for (int i=0; i<5; i++)
		dram_system_update(ms);

	// Send a write to channel 0
	addr = 0x900012;
	dram_system_add_write_trans(ms, addr);

	// Do a bunch of updates (i.e. clocks) -- at some point the callback will fire
	for (int i=0; i<100; i++)
		dram_system_update(ms);

	// Get a nice summary of this epoch
	dram_system_print_stats(ms);

	return 0;
}


int main()
{
	// Pick a DRAM part to simulate
	struct dram_system_handler_t *ms1 = dram_system_create("../ini/DDR2_micron_16M_8b_x8_sg3E.ini", "./system.ini", 16384, "report1");
	dram_system_register_callbacks(ms1, read_callback, write_callback, power_callback);

	struct dram_system_handler_t *ms2 = dram_system_create("../ini/DDR2_micron_16M_8b_x8_sg3E.ini", "./system.ini", 16384, "report2");
	dram_system_register_callbacks(ms2, read_callback, write_callback, power_callback);

	printf("dramsim_test main()\n");
	printf("-----MEM1------\n");
	add_one_and_run(ms1, 0x100001UL);
	add_one_and_run(ms1, 0x200002UL);
	printf("-----MEM2------\n");
	add_one_and_run(ms2, 0x300002UL);

	dram_system_free(ms1);
	dram_system_free(ms2);

	return 0;
}

