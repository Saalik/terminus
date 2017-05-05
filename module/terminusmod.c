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

/* As named device number */
static dev_t dev_number;
static struct cdev c_dev;
static struct class *class;

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

static int t_open(struct inode *i, struct file *f)
{
	return 0;
}

static int t_close(struct inode *i, struct file *f)
{
	return 0;
}

long iohandler (struct file *filp,
		unsigned int cmd,
		unsigned long arg);

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
static bool cond;


static int __init start (void)
{
	int result = 0;
	struct device *dev_return;

	result = alloc_chrdev_region(&dev_number, 0, 1, "terminus");

	if (result < 0) goto fail;

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

	//	dev_num = register_chrdev(0, "terminus", &fops);
	station = create_workqueue("workstation");


	if ( station == NULL ) {
		pr_alert("Workqueue station creation failed in init");
		return -1;
	}
	pr_info("Terminus created w/devnum %d", MAJOR(dev_number));

	pr_alert("Start to Terminus\n");
	return 0;
 device_fail:
	//	device_destroy(class, dev_number);
	class_destroy(class);
 fail_class:
	cdev_del(&c_dev);
	unregister_chrdev_region(dev_number, 1);

 fail:
	return result;
}



static void __exit end (void)
{

	destroy_workqueue(station);
	pr_alert("Terminus");
	device_destroy(class, dev_number);
	class_destroy(class);
	cdev_del(&c_dev);
	unregister_chrdev_region(dev_number, 1);
	return;
}

module_init(start);
module_exit(end);



//static void t_list(void *arg) {}

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

static void t_modinfo (void *arg)
{
	struct module *mod;
	struct infomod im;
	union arg_infomod info_mod;
	char *mod_name = NULL;
	int i = 0;

	copy_from_user(&info_mod, arg, sizeof(char)*T_BUF_STR);
	mod_name = info_mod.arg;
	pr_info("module name %s\n", mod_name);
	mod = find_module(mod_name);

	if (mod != NULL){
		scnprintf(im.name,T_BUF_STR,"%s",mod->name);
		scnprintf(im.version,T_BUF_STR,"%s",mod->version);
		im.module_core = mod->module_core;
		im.num_kp = mod->num_kp;
		while (i < mod->num_kp ) {
			/*kernel paramkp*/
			scnprintf(im.args,T_BUF_STR,"%s ",mod->kp[i].name);
			i++;
		}

	}else{
		im.module_core = NULL;
	}

	copy_to_user(arg, (void *) &im, sizeof(struct infomod));
}


static void t_wait(void *arg)
{
	struct delayed_work *dw;
	struct waiter wtr;
	int i, left = 0;
	struct pid_list pidlist;
	int *tab;
	INIT_DELAYED_WORK(&(wtr->wa_checker));
	copy_from_user(&pidlist, arg, sizeof(struct pid_list));
	tab = kmalloc_array(pidlist.size, sizeof(int), GFP_KERNEL);
	if(tab == NULL){
		pr_alert("Failed malloc in t_wait");
		return -1;
	}

	copy_from_user(tab,pidlist.first, sizeof(int) * pidlist.size);

	wtr = kzalloc(sizeof(struct waiter), GFP_KERNEL);
	wtr->wa_pids= kzalloc(sizeof(struct task_struct*) * pidlist.size, GFP_KERNEL);
	wtr->left = 1;

	wtr->wa_pids_size = pidlist.size;
	while(left){
		left=0
		for (i=0 ; i < wtr->wa_pids_size; i++){
			if(wtr->wa_pids[i] != NULL){
				left=1;
				if (!pid_alive(wtr->wa_pids[i])) {
					put_task_struct(wtr->wa_pids[i]);
					wtr->wa_pids[i] = NULL;
				}
			}
		}
		if(left)
			queue_delayed_work(station, &(wtr->wa_checker),DELAY);
	}



}




/* static void t_fg (struct workkiller *wk) { */
/* 	struct waiter wtr; */
/* 	int i;   */
/* 	/\* struct delayed_work { *\/ */
/* 	/\* struct work_struct work; *\/ */
/* 	/\* struct timer_list timer; *\/ */

/* 	/\* /\\* target workqueue and CPU ->timer uses to queue ->work *\\/ *\/ */
/* 	/\* struct workqueue_struct *wq; *\/ */
/* 	/\* int cpu; *\/ */
/* 	/\* }; *\/ */

/* 	struct delayed_work dw; */

/* 	dw = to_delayed_work(wk); */
/* 	wtr = container_of(dw, struct waiter, wa_checker); */
/* 	wake_up_interruptible(&cond_wait_queue); */

/* 	while(i<wtr->wa_pids_size){ */
/* 		if (!pid_alive(wtr->wa_pids[i])) { */
/* 			wtr->wa_fin = i; */
/* 			wake_up_interruptible(&station); */
/* 			return; */
/* 		} */
/* 		i++; */
/* 	} */
/* 	queue_delayed_work(station, &(wtr->wa_checker),DELAY); */
/* } */


long iohandler (struct file *filp,unsigned int cmd, unsigned long arg)
{
	/* All the structs*/
	struct workkiller wk;
	struct lsmod_work lsw;
	struct meminfo_work miw;

	switch (cmd) {

	case T_MEMINFO:
		pr_info("meminfo");
		t_meminfo((void*)arg);
		break;
	case T_KILL:
		t_kill((void*)arg);
		break;
	case T_MODINFO:
		t_modinfo((void *)arg);
		break;
	/* case T_FG:  */
	/* 	t_fg((void *) arg); */
	/* 	break; */
	/* case T_LIST: */
	/* 	t_list((void*)arg); */
	/* 	break; */
	/*
	/* case T_A_KILL: */
	/* 	t_a_kill(); */
	/* 	break; */
	/* case T_WAIT: */
	/* 	t_wait(); */
	/* 	break; */


	default:
		pr_alert("No station found");
		return -1;
	}

	return 0;
}
