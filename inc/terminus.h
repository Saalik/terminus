#ifndef TERMINUS
#define TERMINUS

#define TERMINUS_MAGIC 'N'

#define T_BUF_SIZE 4096
#define T_BUF_STR 256

#define T_LIST _IOR(TERMINUS_MAGIC, 1, int)
#define T_FG _IOWR(TERMINUS_MAGIC, 2, int)
#define T_KILL _IOR(TERMINUS_MAGIC, 3, int)
#define T_WAIT _IOR(TERMINUS_MAGIC, 4, int)


struct pid_list {
	int size;
	int *first;
	int ret;
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
