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

/* struct listfg { */
/*   union { */

struct workkiller {
	struct work_struct wk_ws;
	struct signal_s signal;
	int async;
	int sleep;
};

struct waiter {
	struct delayed_work wa_checker;
	int wa_fin;
	struct task_struct **wa_pids;
	int wa_pids_size;
	int async;
	int sleep;
};

struct meminfo_waiter {
	struct work_struct ws;
	struct sysinfo values;
	int async;
	int sleep;
};

struct modinfo_waiter {
	struct work_struct ws;
	struct infomod im;
	char *arg;
	int async;
	int sleep;
};

static struct listing *listcmd;
static int nbcmd;
static int flags[10];
static char* ret_async;



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
	destroy_workqueue(station);
	pr_alert("Terminus");
	device_destroy(class, dev_number);
	class_destroy(class);
	cdev_del(&c_dev);
	unregister_chrdev_region(dev_number, 1);
}

module_init(start);
module_exit(end);


static void t_list(struct work_struct *work)
{

}

static void t_fg(struct work_struct *work)
{

}

static void t_kill(struct work_struct *work)
{
	struct workkiller *wk;
	struct pid *pid_target;

	wk= container_of(work, struct workkiller, wk_ws);
	pid_target = find_get_pid(wk->signal.pid);

	/* Si on a bien trouvé un processus correspondant. */
	if (pid_target){
		wk->signal.state = 1;
		kill_pid(pid_target, wk->signal.sig, 1);
	}else{
		wk->signal.state = 0;
	}
	wk->sleep = 1;
	wake_up(&cond_wait_queue);
}

static void t_wait(void *arg, int once)
{
	struct waiter *wtr;
	int i, killed;
	struct pid_list pidlist;
	int *tab;
	struct pid *p;

	wtr = kmalloc(sizeof(struct waiter), GFP_KERNEL);
	INIT_DELAYED_WORK(&(wtr->wa_checker), t_wait_slow);
	copy_from_user(&pidlist, arg, sizeof(struct pid_list));
	/* Récupération de la taille de l'array */
	tab = kmalloc_array(pidlist.size, sizeof(int), GFP_KERNEL);
	if (tab == NULL)
		return;
	/* récup le tab en lui même */
	copy_from_user(tab, pidlist.first, sizeof(int) * pidlist.size);

	wtr->wa_pids =
	    kzalloc(sizeof(struct task_struct *) * pidlist.size, GFP_KERNEL);

	wtr->wa_pids_size = pidlist.size;

	for (i = 0; i < pidlist.size; i++) {
		p = find_get_pid(tab[i]);
		if (!p)
			goto nope_pid;
		wtr->wa_pids[i] = get_pid_task(p, PIDTYPE_PID);
		if (!wtr->wa_pids[i])
			goto nope_pid;
		put_pid(p);
	}
	pr_info("got some pids");
	/* l'incrémentation est faite dans la boucle et non dans le corps */
	for (killed = 0; killed < wtr->wa_pids_size;) {
		pr_info("%d processes are gone so far\n", killed);
		for (i = 0; i < wtr->wa_pids_size; i++) {
			if (wtr->wa_pids[i] != NULL) {
				if (!pid_alive(wtr->wa_pids[i])) {
					killed++;
					put_task_struct(wtr->wa_pids[i]);
					wtr->wa_pids[i] = NULL;
				}
			}
		}

		if (killed == wtr->wa_pids_size) break;

		/* Attend-on la fin d'un seul processus? */
		if (killed && once) break;

		if ((queue_delayed_work
		     (station, &(wtr->wa_checker), DELAY)) == 0) {
			/*
			 * Ralentir la boucle
			 * t_wait_slow(&condition) en Asynchrone.
			 */
			/*Appel de wait_slow */
			wtr->wa_fin = 0;
			pr_info("Avant wait interrupt");
			wait_event_interruptible(cond_wait_queue,
						 wtr->wa_fin != 0);
		}
	}

	pr_info("delayed work canceled w/ value %d\n", cancel_delayed_work(&(wtr->wa_checker)));

 nope_pid:
	kfree(wtr->wa_pids);
	kfree(wtr);
	kfree(tab);
}

static void t_wait_slow(struct work_struct *work)
{
	struct waiter *wtr;
	struct delayed_work *dw;

	pr_info("t_wait_slow");
	dw = to_delayed_work(work);
	wtr = container_of(dw, struct waiter, wa_checker);

	wtr->wa_fin = 1;
	wake_up_interruptible(&cond_wait_queue);
}

static void t_meminfo(struct work_struct *work)
{
	struct meminfo_waiter *mew;

	mew = container_of(work, struct meminfo_waiter, ws);
	si_meminfo(&(mew->values));
	mew->sleep = 1;
	wake_up(&cond_wait_queue);
}

static void t_modinfo(struct work_struct *work)
{
	struct module *mod;
	struct modinfo_waiter *mow;
	char *mod_name = NULL;
	int i = 0;
	mow = container_of(work, struct modinfo_waiter, ws);
	mod_name = mow->arg;
	pr_info("module name %s\n", mod_name);
	mod = find_module(mod_name);
	if (mod != NULL) {
		scnprintf(mow->im.name, T_BUF_STR, "%s", mod->name);
		scnprintf(mow->im.version, T_BUF_STR,
			  "%s", mod->version);
		mow->im.module_core = mod->module_core;
		mow->im.num_kp = mod->num_kp;
		while (i < mod->num_kp) {
			/*kernel paramkp */
			scnprintf(mow->im.args, T_BUF_STR,
				  "%s ", mod->kp[i].name);
			i++;
		}
	} else {
		mow->im.module_core = NULL;
	}
	mow->sleep=1;
	wake_up(&cond_wait_queue);
}


long iohandler(struct file *filp, unsigned int cmd, unsigned long arg)
{
	/* Reborn of the project */
	/* Used for kill */
	struct workkiller *wk;
	/* Used for wait */
	/* struct waiter *wtr; */
	/* Used for meminfo */
	struct meminfo_waiter *mew;
	/* Used for modinfo */
	struct modinfo_waiter *mow;

	switch(cmd) {

	case T_LIST:
		break;
	case T_FG:
		break;
	case T_KILL:
		wk = kmalloc(sizeof(struct workkiller), GFP_KERNEL);
		wk->sleep = 0;
		copy_from_user(&(wk->signal), (void *)arg,
			       sizeof(struct signal_s));
		INIT_WORK(&(wk->wk_ws), t_kill);
		schedule_work(&(wk->wk_ws));
		wait_event(cond_wait_queue, wk->sleep!=0);
		copy_to_user((void *)arg, (void*)&(wk->signal),
		 	     sizeof(struct signal_s));
		kfree(wk);
		break;
	case T_WAIT:
		t_wait((void *)arg, 0);
		break;
	case T_WAIT_ALL:
		t_wait((void *)arg, 1);
		break;
	case T_MEMINFO:
		mew = kzalloc(sizeof(struct meminfo_waiter), GFP_KERNEL);
		mew->sleep = 0;
		mew->async= 0;
		INIT_WORK(&(mew->ws), t_meminfo);
		schedule_work(&(mew->ws));
		wait_event(cond_wait_queue, mew->sleep!=0);
		copy_to_user((void *)arg, (void*)&(mew->values),
		 	     sizeof(struct my_infos));
		kfree(mew);
		break;
	case T_MODINFO:
		mow = kzalloc(sizeof(struct modinfo_waiter), GFP_KERNEL);
		mow->sleep = 0;
		copy_from_user(&mow->arg, (void *) arg, sizeof(char) * T_BUF_STR);
		mow->async=0;
		INIT_WORK(&(mow->ws), t_modinfo);
		schedule_work(&(mow->ws));
		wait_event(cond_wait_queue, mow->sleep!=0);
		copy_to_user((void*)arg, (void*)&mow->im,
			     sizeof(struct infomod));
		kfree(mow);
		break;
	default:
		pr_alert("Unkown command");
		return-1;
	}
	return 0;
}
