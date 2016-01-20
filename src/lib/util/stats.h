#ifndef STATS_H
#define STATS_H

#include <lib/util/string.h>


/*
 * Global Variables
 */


extern int EV_INTERVAL_REPORT;

extern long long epoch_length; /* In cycles @ esim_frequency */

extern char *reports_dir;

extern char interval_reports_dir[MAX_PATH_SIZE];

extern char mod_interval_reports_dir[MAX_PATH_SIZE];
extern char dram_interval_reports_dir[MAX_PATH_SIZE];
extern char x86_ctx_interval_reports_dir[MAX_PATH_SIZE];
extern char x86_thread_interval_reports_dir[MAX_PATH_SIZE];

extern char x86_thread_mappings_reports_dir[MAX_PATH_SIZE];
extern char x86_ctx_mappings_reports_dir[MAX_PATH_SIZE];

extern char global_reports_dir[MAX_PATH_SIZE];


/*
 * Public Functions
 */


void m2s_interval_report_handler(int event, void *data);
void m2s_interval_report_schedule(void);

#endif /* STATS_H */
