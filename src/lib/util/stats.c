
#include <assert.h>
#include <stdbool.h>

#include <arch/common/arch.h>
#include <arch/x86/emu/emu.h>
#include <arch/x86/timing/cpu.h>
#include <lib/esim/esim.h>
#include <mem-system/mem-system.h>

#include "stats.h"


/*
 * Global Variables
 */


int EV_INTERVAL_REPORT;
int interval_report_domain_index;

long long epoch_length = 50000; /* Number of cycles in esim_frequency for the stats reporting interval */

char *reports_dir = "./";

char interval_reports_dir[MAX_PATH_SIZE];

char mod_interval_reports_dir[MAX_PATH_SIZE];
char dram_interval_reports_dir[MAX_PATH_SIZE];
char x86_ctx_interval_reports_dir[MAX_PATH_SIZE];
char x86_thread_interval_reports_dir[MAX_PATH_SIZE];

char x86_thread_mappings_reports_dir[MAX_PATH_SIZE];
char x86_ctx_mappings_reports_dir[MAX_PATH_SIZE];

char global_reports_dir[MAX_PATH_SIZE];


/*
 * Public Functions
 */


//void m2s_interval_report_handler(int event, void *data)
//{
//	assert(!data);
//	assert(event == EV_INTERVAL_REPORT);

//	x86_emu_interval_report();      /* x86 contexts */
//	x86_cpu_interval_report();      /* x86 cores and threads */
//	mem_system_interval_report();   /* Caches */

//	if (!esim_finish)
//		esim_schedule_event(EV_INTERVAL_REPORT, NULL, epoch_length);
//}


//void m2s_interval_report_schedule(void)
//{
//	bool any_detailed = false;

//	if (!epoch_length)
//		return;

//	if (arch_x86->sim_kind == arch_sim_kind_detailed)
//	{
		/* NOTE: x86 contexts interval reporting is initialized on creation */
//		x86_cpu_interval_report_init();      /* x86 cores and threads */
//		any_detailed = true;
//	}

//	if (any_detailed)
//	{
//		mem_system_interval_report_init();   /* Caches */

		/* New domain and event for interval reporting */
//		interval_report_domain_index = esim_new_domain(esim_frequency);
//		EV_INTERVAL_REPORT = esim_register_event_with_name(m2s_interval_report_handler, interval_report_domain_index, "m2s_interval_report");

//		assert(EV_INTERVAL_REPORT);
//		assert(interval_report_domain_index);
//		assert(epoch_length);
//		assert(reports_dir && reports_dir[0]);
//		assert(interval_reports_dir[0]);
//		assert(mod_interval_reports_dir[0]);
//	assert(x86_ctx_interval_reports_dir[0]);
//		assert(x86_thread_interval_reports_dir[0]);
//		assert(global_reports_dir[0]);

//		esim_schedule_event(EV_INTERVAL_REPORT, NULL, epoch_length);
//	}
//}
