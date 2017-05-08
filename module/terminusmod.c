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

struct waiter {
	struct delayed_work wa_checker;
	int wa_fin;
	struct task_struct **wa_pids;
	int wa_pids_size;
	int async;
	int sleep;
};

struct handler_struct {
	struct work_struct worker;
	struct mutex mut;
	struct list_head list;
	struct module_argument arg;
	int sleep;
	int id;
};


static struct listing *listcmd;
static int nbcmd;
static int flags[10];
static char* ret_async;
static int task_id = 0;

LIST_HEAD(tasks);


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

/* appelé à chaque fin de handler */
static void async_janitor(struct handler_struct *handler)
{
	if (handler->arg.async) {
		pr_info("first mutex lock, handler is %d @ %p\n", handler->arg.arg_type, handler);
		mutex_lock(&handler->mut);
		pr_info("unlocked all\n");
	}
}

static void t_list(struct work_struct *work)
{
	struct handler_struct *handler;
	struct list_head *head;

	pr_info("inside t_list\n");
	list_for_each(head,&tasks) {

		handler = list_entry(head, struct handler_struct, list);
		pr_info("task %d\n", handler->id);
	}

}

static void t_fg(struct work_struct *work)
{
	struct handler_struct *handler, *handler_done;
	pr_info("getting handler\n");
	handler = container_of(work, struct handler_struct, worker);
	pr_info("locking mutex\n");
	mutex_lock(&handler->mut);
	if (!list_empty(&tasks)) {
		pr_info("list not empty\n");
		handler_done = list_entry(&tasks, struct handler_struct, list);
		pr_info("got handler %d, sleep %d, copying now\n", handler_done->arg.arg_type, handler->sleep);
		pr_info("handler @ %p\n", handler_done);
		memcpy(handler, handler_done, sizeof(struct handler_struct));
		pr_info("deleting from list\n");
		list_del(&(handler_done->list));
		pr_info("freeing memory\n");
		/*		kfree(handler_done); */
	}
	pr_info("unlocking mutex\n");
	mutex_unlock(&handler->mut);

	handler->sleep = 1;
	wake_up(&cond_wait_queue);
}

static void t_kill(struct work_struct *work)
{
	struct handler_struct *handler;
	struct pid *pid_target;

	handler = container_of(work, struct handler_struct, worker);
	pid_target = find_get_pid(handler->arg.kill_a.pid);

	/* Si on a bien trouvé un processus correspondant. */
	if (pid_target){
		handler->arg.kill_a.state = 1;
		kill_pid(pid_target, handler->arg.kill_a.sig, 1);
	}else{
		handler->arg.kill_a.state = 0;
	}
	handler->sleep = 1;
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

		/* A-t-on attendu la fin de tous les processus? */
		if (killed == wtr->wa_pids_size) {
			break;
		}

		/* Attend-on la fin d'un seul processus? */
		if ((killed > 0) && (once == 1)) {
			break;
		}

		/* Ceci doit forcément être éxecuté après les break.
		   Sinon on break the world */
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
	struct handler_struct *handler;

	handler = container_of(work, struct handler_struct, worker);
	si_meminfo((struct sysinfo *) &(handler->arg.meminfo_a));
	handler->sleep = 1;
	wake_up(&cond_wait_queue);
}

static void t_modinfo(struct work_struct *work)
{
	struct handler_struct *handler;
	struct module *mod;
	int i = 0;
	char *mod_name;

	mod_name = kzalloc(T_BUF_STR * sizeof(char), GFP_KERNEL);
	handler = container_of(work, struct handler_struct, worker);
	pr_info("before copy_from %p\n", handler->arg.modinfo_a.arg);
	copy_from_user(mod_name, (void *) handler->arg.modinfo_a.arg, T_BUF_STR * sizeof(char));

	pr_info("module name %s + %d\n", mod_name, mod_name[0]);
	mod = find_module(mod_name);
	if (mod != NULL) {
		scnprintf(handler->arg.modinfo_a.data.name, T_BUF_STR, "%s", mod->name);
		scnprintf(handler->arg.modinfo_a.data.version, T_BUF_STR,
			  "%s", mod->version);
		handler->arg.modinfo_a.data.module_core = mod->module_core;
		handler->arg.modinfo_a.data.num_kp = mod->num_kp;
		while (i < mod->num_kp) {
			/*kernel paramkp */
			scnprintf(handler->arg.modinfo_a.data.args, T_BUF_STR,
				  "%s ", mod->kp[i].name);
			i++;
		}
	} else {
		handler->arg.modinfo_a.data.module_core = NULL;
	}

	kfree(mod_name);
	handler->sleep=1;

	pr_info("janiting start\n");
	async_janitor(handler);
	pr_info(" end\n");

	wake_up(&cond_wait_queue);
}

void do_it(struct module_argument *arg)
{
	struct handler_struct *handler;
	handler = kzalloc(sizeof(struct handler_struct), GFP_KERNEL);
	handler->sleep = 0;
	handler->id = task_id++;

	mutex_init(&(handler->mut));

	copy_from_user(&(handler->arg), arg, sizeof(struct module_argument));
	switch (arg->arg_type) {
	case meminfo_t:
		INIT_WORK(&(handler->worker), t_meminfo);
		break;
	case modinfo_t:
		INIT_WORK(&(handler->worker), t_modinfo);
		break;
	case kill_t:
		INIT_WORK(&(handler->worker), t_kill);
		break;
	case fg_t:
		INIT_WORK(&(handler->worker), t_fg);
		break;
	case pid_list_t:
		INIT_WORK(&(handler->worker), t_list);
		break;
	default:
		pr_info("default case\n");
		break;
	}
	mutex_lock(&handler->mut);
	schedule_work(&(handler->worker));
	/* fg is always synchronous. otherwise.. */
	if (handler->arg.async && (arg->arg_type != fg_t)) {
		list_add_tail(&(handler->list), &tasks);
		mutex_unlock(&handler->mut);
		return;
	}
	else {
		wait_event(cond_wait_queue, handler->sleep != 0);
		pr_info("copying to user\n");
		copy_to_user((void *) arg, (void *) &(handler->arg),
			     sizeof(struct module_argument));
		kfree(handler);
		mutex_unlock(&handler->mut);
	}
	return;
}


long iohandler(struct file *filp, unsigned int cmd, unsigned long arg)
{
	/* Reborn of the project */

	switch(cmd) {

	case T_WAIT:
		t_wait((void *)arg, 1);
		break;
	case T_WAIT_ALL:
		t_wait((void *)arg, 0);
		break;
	case T_KILL:
	case T_MEMINFO:
	case T_MODINFO:
	case T_FG:
	case T_LIST:
		do_it((struct module_argument *) arg);
		break;
	default:
		pr_alert("Unkown command");
		return-1;
	}
	return 0;
}
