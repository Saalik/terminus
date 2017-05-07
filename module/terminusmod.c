#include <linux/workqueue.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/pid.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/mm.h>

/* Still useless for now */
#include "../inc/terminus.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Oskar Viljasaar, Saalik Hatia");
MODULE_DESCRIPTION("PNL Project UPMC - Terminus");

/*For the purposes of the waitctl */
int mem_info_wait;

/* As named device number */

static dev_t dev_number;
static struct cdev c_dev;
static struct class *class;

//struct listfg {
	

struct workkiller {
	struct work_struct wk_ws;
	struct signal_s signal;
	int async;
};

struct waiter {
	struct delayed_work wa_checker;
	int wa_fin;
	struct task_struct **wa_pids;
	int wa_pids_size;
	int async;
};

struct meminfo_waiter {
	struct work_struct ws;
	struct sysinfo values;
	int async;
};

struct modinfo_waiter {
	struct infomod im;
	int async;
}


static int t_open(struct inode *i, struct file *f)
{
	return 0;
}

static int t_close(struct inode *i, struct file *f)
{
	return 0;
}

static long iohandler(struct file *filp, unsigned int cmd, unsigned long arg);

static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = t_open,
	.release = t_close,
	.unlocked_ioctl = iohandler,
};

/*

  Structure pour la création de la workqueue
  struct workqueue_struct *create_workqueue(const char *name);

*/

static struct workqueue_struct *station;

DECLARE_WAIT_QUEUE_HEAD(cond_wait_queue);
static void t_wait_slow(struct work_struct *work);

static int __init start(void)
{
	int result = 0;
	struct device *dev_return;

	result = alloc_chrdev_region(&dev_number, 0, 1, "terminus");

	if (result < 0)
		goto fail;

	cdev_init(&c_dev, &fops);
	result = cdev_add(&c_dev, dev_number, 1);

	if (result < 0) {
		pr_alert("cdev_add\n");
		goto fail;
	}

	class = class_create(THIS_MODULE, "char");

	if (IS_ERR(class)) {
		result = PTR_ERR(class);
		pr_alert("class_create\n");
		goto fail_class;
	}

	dev_return = device_create(class, NULL, dev_number, NULL, "terminus");

	if (IS_ERR(dev_return)) {
		result = PTR_ERR(dev_return);
		pr_alert("device_create\n");
		goto device_fail;
	}
	/*dev_num = register_chrdev(0, "terminus", &fops); */
	station = create_workqueue("workstation");

	if (station == NULL) {
		pr_alert("Workqueue station creation failed in init");
		return -1;
	}
	pr_info("Terminus created w/devnum %d", MAJOR(dev_number));

	pr_alert("Start to Terminus\n");
	return 0;
device_fail:
	/*device_destroy(class, dev_number); */
	class_destroy(class);
fail_class:
	cdev_del(&c_dev);
	unregister_chrdev_region(dev_number, 1);
fail:
	return result;
}

static void __exit end(void)
{
	int i;
	destroy_workqueue(station);
	pr_alert("Terminus");
	device_destroy(class, dev_number);
	class_destroy(class);
	cdev_del(&c_dev);
	unregister_chrdev_region(dev_number, 1);
}

module_init(start);
module_exit(end);



long iohandler(struct file *filp, unsigned int cmd, unsigned long arg)
{
	/* Reborn of the project */
	/* Used for kill */
	struct workkiller *wk;
	/* Used for wait */
	struct waiter *wtr;
	/* Used for meminfo */
	struct meminfo_waiter mew;
	/* Used for modinfo */
	struct modinfo_waiter mow;

	


	
	switch(cmd) {
		
	case T_LIST:
		break;
	case T_FG:
		break;
	case T_KILL:
		wk = kmalloc(sizeof(struct workkiller), GFP_KERNEL);
		wk->async = 0;
		INIT_WORK(&(wk->wk_ws), t_async_kill);
		copy_from_user(&(wk->signal), (void *)arg,
			       sizeof(struct signal_s));
		queue_work(station, &(wk->wk_ws));
	case T_WAIT:
		break;
	case T_MEMINFO:
		break;
	case T_MODINFO:
		break;
	default:
		pr_alert("Unkown command");
		return-1;
	}
	return 0;
}
