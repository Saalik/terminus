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

/* As named device number */
static dev_t dev_number;
static struct cdev c_dev;
static struct class *class;

struct workkiller {
	struct work_struct wk_ws;
	struct signal_s signal;
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

	for(i = 0; i < 7
		    ; i++)
		pr_alert("\n");

}

module_init(start);
module_exit(end);

/* static void t_list(void *arg) {} */

static void t_meminfo(void *arg)
{
	struct sysinfo values;

	memset(&values, 0, sizeof(struct sysinfo));
	si_meminfo(&values);
	copy_to_user((void *)arg, &values, sizeof(struct my_infos));

	pr_debug("Given value given to ya!\n");
}

static void t_kill(void *arg)
{
	struct signal_s s;
	struct pid *pid_target;

	copy_from_user(&s, arg, sizeof(struct signal_s));
	pid_target = find_get_pid(s.pid);

	/* Si on a bien trouvé un processus correspondant. */
	if (pid_target)
		kill_pid(pid_target, s.sig, 1);
}

/*
En mode U le pointeur change
 */

static void t_modinfo(void *arg)
{
	struct module *mod;
	struct infomod im;
	union arg_infomod info_mod;
	char *mod_name = NULL;
	int i = 0;

	copy_from_user(&info_mod, arg, sizeof(char) * T_BUF_STR);
	mod_name = info_mod.arg;
	pr_info("module name %s\n", mod_name);
	mod = find_module(mod_name);

	if (mod != NULL) {
		scnprintf(im.name, T_BUF_STR, "%s", mod->name);
		scnprintf(im.version, T_BUF_STR, "%s", mod->version);
		im.module_core = mod->module_core;
		im.num_kp = mod->num_kp;
		while (i < mod->num_kp) {
			/*kernel paramkp */
			scnprintf(im.args, T_BUF_STR, "%s ", mod->kp[i].name);
			i++;
		}

	} else {
		im.module_core = NULL;
	}

	copy_to_user(arg, (void *)&im, sizeof(struct infomod));
}

static void t_wait(void *arg, int all)
{
	struct waiter *wtr;
	int i, left = 1;
	struct pid_list pidlist;
	int *tab;
	struct pid *p;


	wtr = kmalloc(sizeof(struct waiter), GFP_KERNEL);
	INIT_DELAYED_WORK(&(wtr->wa_checker), t_wait_slow);
	copy_from_user(&pidlist, arg, sizeof(struct pid_list));
	/* Récupération de la taille de l'array */
	tab = kmalloc_array(pidlist.size, sizeof(int), GFP_KERNEL);
	if (tab == NULL){
		pr_info("Salut je suis t_wait");
		return;
	}
	/* récup le tab en lui même */
	copy_from_user(tab, pidlist.first, sizeof(int) * pidlist.size);

	wtr->wa_pids =
	    kzalloc(sizeof(struct task_struct *) * pidlist.size, GFP_KERNEL);

	wtr->wa_pids_size = pidlist.size;

	for (i = 0; i < pidlist.size; i++) {
		p = find_get_pid(tab[i]);
		if (!p) {
			goto nope_pid;
		}
		wtr->wa_pids[i] = get_pid_task(p, PIDTYPE_PID);
		if (!wtr->wa_pids[i]) {
			goto nope_pid;
		}
		put_pid(p);
	}

	while (1) {
		left = 0;
		pr_info("je suis dans le while(left)");
		for (i = 0; i < wtr->wa_pids_size; i++) {
			if (wtr->wa_pids[i] != NULL) {
				pr_alert("There is such a thing as a PID\n");
				left++;
				if (!pid_alive(wtr->wa_pids[i])) {
					put_task_struct(wtr->wa_pids[i]);
					wtr->wa_pids[i] = NULL;
				}
			}else{
				if(all != 1)
					break;
			}
		}
		if (left){
			if((queue_delayed_work(station, &(wtr->wa_checker), DELAY)) == 0)
				pr_alert("Wesh, there is no such a thing as a slow wait\n");
			/*
			 * Ralentir la boucle
			 * t_wait_slow(&condition) en Asynchrone.
			 */
			/*Appel de wait_slow */
			wtr->wa_fin = 0;
			pr_info("Avant wait interrupt");
			wait_event_interruptible(cond_wait_queue, wtr->wa_fin != 0);
			pr_info("près wait interrupt");
		} else
			break;

	}

nope_pid:
	kfree(wtr->wa_pids);
	kfree(wtr);
	kfree(tab);


}

/*
 * Appelé au cours d'un wait et aprsès un nombre
 * donné de ticks.
 * Change la condition et wake le process
 * Wait_slow est appelée car elle est l'un des attributs d'une work_struct
 * qui contient un pointeur vers un delayed work.
 * Le delayed work est embedded dans une struct custom, qui contient
 * donc la condition.
 */
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

static void t_async_kill(struct work_struct *wurk)
{
	struct workkiller *w;
	struct pid *pid_tmp;

	w = container_of(wurk, struct workkiller, wk_ws);

	pid_tmp = find_get_pid(w->signal.pid);

	if (pid_tmp)
		kill_pid(pid_tmp, w->signal.sig, 1);

	pr_info("async killed some pid\n");

	kfree(w);
}

long iohandler(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct workkiller *wk;
	/* All the structs */
	/* struct workkiller wk; */
	/* struct lsmod_work lsw; */
	/* struct meminfo_work miw; */

	switch (cmd) {

	case T_MEMINFO:
		pr_info("meminfo");
		t_meminfo((void *)arg);
		break;
	case T_KILL:
		t_kill((void *)arg);
		break;
	case T_MODINFO:
		t_modinfo((void *)arg);
		break;
		/* case T_FG:  */
		/*      t_fg((void *) arg); */
		/*      break; */
		/* case T_LIST: */
		/*      t_list((void*)arg); */
		/*      break; */

		/* case T_A_KILL: */
		/*      t_a_kill(); */
		/*      break; */
	case T_WAIT:
		t_wait((void *)arg,0);
		break;
	case T_WAIT_ALL:
		t_wait((void *)arg,99);
		break;
	case T_A_KILL:
		wk = kmalloc(sizeof(struct workkiller), GFP_KERNEL);
		INIT_WORK(&(wk->wk_ws), t_async_kill);

		copy_from_user(&(wk->signal), (void *) arg, sizeof(struct signal_s));

		queue_work(station, &(wk->wk_ws));
	default:
		pr_alert("No station found");
		return -1;
	}

	return 0;
}
