#ifndef TERMINUS
#define TERMINUS




struct pid_list {
	int size;
	int *first;
	int ret;
};

struct workkiller {
	struct work_struct wk_ws;
	int wk_pid;
	int wk_sig;
};


struct waiter {
	struct delayed_work wa_checker;
	int wa_fin;
	struct task_struct **wa_pids;
	int wa_pids_size;
};



struct lsmod_work {
	struct work_struct lw_ws;
	char *lw_data;
};

struct meminfo_work {
	struct work_struct mw_ws;
	struct sysinfo mw_values;
};


struct my_infos {
	long long uptime;		/* Seconds since boot */
	unsigned long long loads[3];	/* 1, 5, and 15 minute load averages */
	unsigned long long totalram;	/* Total usable main memory size */
	unsigned long long freeram;	/* Available memory size */
	unsigned long long sharedram;	/* Amount of shared memory */
	unsigned long long bufferram;	/* Memory used by buffers */
	unsigned long long totalswap;	/* Total swap space size */
	unsigned long long freeswap;	/* swap space still available */
	unsigned short procs;           /* Number of current processes */
	unsigned short pad;	        /* Explicit padding for m68k */
	unsigned long long totalhigh;	/* Total high memory size */
	unsigned long long freehigh;	/* Available high memory size */
	unsigned long mem_unit;	        /* Memory unit size in bytes */
};
