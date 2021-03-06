#ifndef TERMINUS
#define TERMINUS

#define TERMINUS_MAGIC 'N'

#define T_BUF_SIZE 4096
#define T_BUF_STR 256
#define DELAY 100

#define T_LIST _IOR(TERMINUS_MAGIC, 1, int)
#define T_FG _IOWR(TERMINUS_MAGIC, 2, int)
#define T_KILL _IOR(TERMINUS_MAGIC, 3, int)
#define T_WAIT _IOR(TERMINUS_MAGIC, 4, int)
#define T_MODINFO _IOR(TERMINUS_MAGIC, 5, int)
#define T_MEMINFO _IOR(TERMINUS_MAGIC, 6, int)
#define T_A_KILL _IOR(TERMINUS_MAGIC, 7, int)
#define T_WAIT_ALL _IOR(TERMINUS_MAGIC, 8, int)
/* #define T_A_MEMINFO _IOR(TERMINUS_MAGIC, 9, int) */

struct pid_ret {
	int pid;
	int ret;
};

struct pid_list {
	int size;
	int *first;
	struct pid_ret *ret;
};

struct signal_s {
	int pid;
	int sig;
	int state;
};

/* Détails d'un module. */
struct infomod {
	char name[T_BUF_STR];	/* Nom du module */
	char version[T_BUF_STR];	/* Version */
	void *module_core;	/* Adresse de chargement */
	unsigned int num_kp;	/* Nombre d'arguments */
	char args[T_BUF_STR];	/* Arguments */
};

/* Argument passé à l'ioctl.
   arg représente le nom du module donné par l'utilisateur */
union arg_infomod {
	struct infomod data;
	char *arg;
};

struct fg_arguments {
	int id;			/* Identifiant d'un job */
	char *string;		/* Descriptif du job  */
};

struct my_infos {
	long long uptime;	/* Seconds since boot */
	unsigned long long loads[3];	/* 1, 5, and 15 minute load averages */
	unsigned long long totalram;	/* Total usable main memory size */
	unsigned long long freeram;	/* Available memory size */
	unsigned long long sharedram;	/* Amount of shared memory */
	unsigned long long bufferram;	/* Memory used by buffers */
	unsigned long long totalswap;	/* Total swap space size */
	unsigned long long freeswap;	/* swap space still available */
	unsigned short procs;	/* Number of current processes */
	unsigned short pad;	/* Explicit padding for m68k */
	unsigned long long totalhigh;	/* Total high memory size */
	unsigned long long freehigh;	/* Available high memory size */
	unsigned long mem_unit;	/* Memory unit size in bytes */
};

/* Structure utilisée pour énumérer les détails des jobs */
struct listing {
	struct module_argument *args;
	char *out;
	int size;
};

/* Type d'argument utilisée pour struct module_argument */
enum argument_type {
	pid_list_t = 1,
	fg_t = 2,
	kill_t = 3,
	wait_t = 4,
	modinfo_t = 5,
	meminfo_t = 6,
	wait_all_t = 7,
	args_end
};

/* Structure principale passée à l'ioctl englobant les structures ci-dessus */
struct module_argument {
	int async;
	enum argument_type arg_type;
	union {
		struct pid_list pid_list_a;
		struct signal_s kill_a;
		union arg_infomod modinfo_a;
		struct listing list_a;
		struct my_infos meminfo_a;
		struct fg_arguments fg_a;
	};
};

#endif				/* TERMINUS */
