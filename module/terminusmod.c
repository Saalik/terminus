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
	/* Creation de la workqueue */
	station = create_workqueue("workstation");

	if ( station == NULL ) {
		pr_alert("Workqueue station creation failed in init");
		return -1;
	}

	dev_num = register_chrdev(0, "terminus", &fops);
	
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
