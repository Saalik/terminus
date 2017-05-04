#include <linux/workqueue.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/pid.h>
#include <linux/fs.h>
#include <linux/mm.h>

/* Still useless for now */
#include "../inc/terminus.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Oskar Viljasaar, Saalik Hatia");
MODULE_DESCRIPTION("PNL Project UPMC - Terminus");

/* As named device number */
int dev_num;

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


long iohandler (struct file *filp,
		unsigned int cmd, 
		unsigned long arg);

static const struct file_operations fops = {
	.unlocked_ioctl = iohandler,
};

/*
  
  Structure pour la création de la workqueue
  struct workqueue_struct *create_workqueue(const char *name);

*/

static struct workqueue_struct station;

DECLARE_WAIT_QUEUE_HEAD(cond_wait_queue);
static bool cond;


static int __init start (void)
{

	dev_num = register_chrdev(0, "terminus", &fops);
	station = create_workqueue("workstation");

	
	if ( station == NULL ) {
		pr_alert("Workqueue station creation failed in init");
		return -1;
	}
	
	
	pr_alert("Start to Terminus");
	return 0;
}



static void __exit end (void)
{
	
	destroy_workqueue("workstation");
	pr_alert("Terminus");
	return 0;
}

module_init(start);
module_exit(end);



//static void t_list(void *arg) {}

static void t_meminfo(void *arg_p)
{
	struct sysinfo values;

	memset(&values, 0, sizeof(struct sysinfo));
	si_meminfo(&values);
	copy_to_user((void *)arg_p, &values, sizeof(struct my_infos));

	pr_debug("Given value given to ya!\n");
}

static void t_kill(void *arg_p)
{
	struct signal_s s;
	struct pid *pid_target;
	
	copy_from_user(&s, arg_p, sizeof(struct signal_s));
	pid_target = find_get_pid(s.pid);
	
	/* Si on a bien trouvé un processus correspondant. */
	if (pid_target)
		kill_pid(pid_target, s.sig, 1);
}




/* static void a_print_meminfo(void *arg_p) */
/* { */
/* 	struct sysinfo values; */
/* 	si_meminfo(&values); */
/* 	copy_to_user((void *)arg_p, &values, sizeof(struct my_infos)); */
/* 	wake_up(&waiter); */
/* } */




/* static void t_a_kill(struct work_struct *work) */
/* { */
/* 	struct work_killer *wk; */
/* 	struct pid *target; */

/* 	/\*REMEMBER*\/ */
/* 	wk = container_of(work, struct work_killer, wk_ws); */
/* 	/\* On cherche la cible *\/ */
/* 	target = find_get_pid(wk->wk_pid); */
/* 	/\* Si une cible a été trouvé*\/ */
/* 	if (target) */
/* 		kill_pid(target, work->wk_sig, 1); */
/* 	/\* Free*\/ */
/* 	kfree(wk); */
/* } */

/* static void t_lsmod(void *arg) { */
/* 	struct module *mod; */
/* 	char *sret, buf_str[T_BUF_STR]; */
/* 	int nbref, buf_size = T_BUF_SIZE; */

/* 	sret= kzalloc(T_BUF_SIZE, GFP_KERNEL); */

/* 	mod = THIS_MODULE; */
/* 	nbref = atomic_read(&(mod->mkobj.kobj.kref.refcount)); */

/* 	//buf_size = buf_size - */
/* } */

		
		
long iohandler (struct file *filp,unsigned int cmd, unsigned long arg)
{
	/* All the structs*/
	struct workkiller wk;
	struct lsmod_work lsw;
	struct meminfo_work miw;

	switch (cmd) {

	case T_MEMINFO:
		t_meminfo((void*)arg);
		break;
		
	/* case T_LIST: */
	/* 	t_list((void*)arg); */
	/* 	break; */
	/* case T_FG: */
	/* 	t_fg(); */
	/* 	break; */
	case T_KILL:
		t_kill((void*)arg);
		break;
	/* case T_A_KILL: */
	/* 	t_a_kill(); */
	/* 	break; */
	/* case T_WAIT: */
	/* 	t_wait(); */
	/* 	break; */
	
	/* case T_LSMOD: */
	/* 	t_lsmod((void *)arg); */
	/* 	break; */
		
	default:
		pr_alert("No station found");
		return -1;
	}
	
	return 0;
}
