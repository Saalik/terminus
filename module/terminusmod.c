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

int dev_num;

long iohandler (struct file *filp,
	      unsigned int cmd, 
	      unsigned long arg);

static const struct file_operations fops = {
	.unlocked_ioctl = unioctl,
};

static struct workqueue_struct workst;

DECLARE_WAIT_QUEUE_HEAD(cond_wait_queue);
static bool cond;




static int __init init (void)
{
	
}

static void __exit end (void)
{

}

module_init(init);
module_exit(end);
