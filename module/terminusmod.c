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
#include "../inc/terminux.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Oskar Viljasaar, Saalik Hatia");
MODULE_DESCRIPTION("PNL Project UPMC - Terminus");

/* As named device number */
int dev_num;

long iohandler (struct file *filp,
	      unsigned int cmd, 
	      unsigned long arg);

static const struct file_operations fops = {
	.unlocked_ioctl = unioctl,
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

static void print_meminfo(void *arg_p)
{
	struct sysinfo values;

	memset(&values, 0, sizeof(struct sysinfo));
	si_meminfo(&values);
	copy_to_user((void *)arg_p, &values, sizeof(struct my_infos));

	pr_debug("Given value given to ya!\n");
}


static void a_print_meminfo(void *arg_p)
{
  	struct sysinfo values;
	si_meminfo(&values);
	copy_to_user((void *)arg_p, &values, sizeof(struct my_infos));
	wake_up(&waiter);
}

static void kill_signal(void *arg_p)
{
	struct signal_s s;
	struct pid *pid_target;

	copy_from_user(&s, arg_p, sizeof(struct signal_s));
	pid_target = find_get_pid(s.pid);
	
	/* Si on a bien trouvé un processus correspondant. */
	if (pid_target)
		kill_pid(pid_target, s.sig, 1);
}


static void a_kill_signal(struct work_struct *work)
{
	struct work_killer *wk;
	struct pid *target;

	wk = container_of(work, struct work_killer, wk_ws);
	
	target = find_get_pid(wk->wk_pid);
	/* Si on a bien trouvé un processus correspondant. */
	if (target)
		kill_pid(target, work->wk_sig, 1);

	/* On libère la structure qui contenait nos données */
	kfree(syn);
}
