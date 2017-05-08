#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

/* Include for later .h empty for now */
#include <terminus.h>

#define T_PATH "/dev/terminus"

char *user_strings[T_BUF_STR];

ssize_t prompt_user(char *string, size_t count)
{
	printf("> ");
	fflush(stdout);

	return read(STDIN_FILENO, string, count);
}

void list_commandes()
{
	printf("modinfo [module]: infos sur un module noyau\n"
	       "meminfo: infos sur la memoire\n"
	       "kill [pid] [signal]: envoyer un signal a un processus\n"
	       "akill [pid] [signal]: envoyer un signal a un processus mais plus tard\n"
	       "wait [pid] ([pid2], ...): attendre la fin d'un des pid donnes\n"
	       "waitall [pid] ([pid2], ...): attendre la fin de tous les pid\n"
	       );
}

/* s2 est supposé directement être un string */
size_t lazy_cmp(char *s1, char *s2) {
	return strncmp(s1, s2, strlen(s2));
}

void meminfo(int fd, int async)
{
	struct module_argument arg;
	arg.arg_type = meminfo_t;
	arg.async = async;

	if (ioctl(fd, T_MEMINFO, &arg) == 0) {
		printf("TotalRam\t%llu pages\n", arg.meminfo_a.totalram);
		printf("SharedRam\t%llu pages\n", arg.meminfo_a.sharedram);
		printf("FreeRam\t\t%llu pages\n", arg.meminfo_a.freeram);
		printf("BufferRam\t%llu pages\n", arg.meminfo_a.bufferram);
		printf("TotalHigh\t%llu pages\n", arg.meminfo_a.totalhigh);
		printf("FreeHigh\t%llu pages\n", arg.meminfo_a.freehigh);
		printf("Memory unit\t%lu bytes\n", arg.meminfo_a.mem_unit);
	} else perror("ioctl");
}

void modinfo(int fd, char* module_name)
{
	char *tmp_ptr = NULL;
	struct module_argument arg;
	arg.arg_type = meminfo_t;
	arg.async = 0;


	memset(&arg.modinfo_a, 0, sizeof(union arg_infomod));
	arg.modinfo_a.arg = (char *) malloc(T_BUF_STR * sizeof(char));
	tmp_ptr = arg.modinfo_a.arg;
	memset(arg.modinfo_a.arg, 0, T_BUF_STR);

	if (module_name == NULL) {
		printf("Il faut fournir un nom de module.\n");
		free(tmp_ptr);
		return;
	}
	strcpy(arg.modinfo_a.arg, module_name);

	if (ioctl(fd, T_MODINFO, &arg) == 0) {
		if (arg.modinfo_a.data.module_core == NULL) {
			printf("Module %s pas trouvé.\n", module_name);
			free(tmp_ptr);
			return;
		}
		else {
			printf("%s\n%s\n%p\n%d arguments",
			       arg.modinfo_a.data.name,
			       arg.modinfo_a.data.version,
			       arg.modinfo_a.data.module_core,
			       arg.modinfo_a.data.num_kp);
			if (arg.modinfo_a.data.num_kp) {
				printf(":\n%s", arg.modinfo_a.data.args);
			}
			free(tmp_ptr);
			printf("\n");
			return;
		}
	} else perror("ioctl");

	free(tmp_ptr);
}

void kill(int fd, char* pid, char* sig, int async)
{
	struct signal_s signal;
	if ((pid == NULL) || (sig == NULL)) {
		printf("Il faut fournir un pid et un signal\n");
		return;
	}

	signal.pid = atoi(pid);
	signal.sig = atoi(sig);
	if (!async) {
		if (ioctl(fd, T_KILL, &signal) != 0)
			perror("ioctl");
	} /*else {
	    if (ioctl(fd, T_A_KILL, &signal) != 0)
	    perror("ioctl");
	    } */
}

void t_wait(int fd, int wait_all)
{
	struct pid_list list;
	int i;

	list.size = 0;
	list.first = NULL;

	for (i=1; user_strings[i] != NULL; i++)
		list.size++;

	if (list.size == 0) {
		printf("Il faut au moins un pid.\n");
		return;
	}

	list.first = (int *) malloc(list.size * sizeof(int));

	for (i=0; i < list.size; i++)
		list.first[i] = atoi(user_strings[i+1]);
	if (wait_all) {
		if (ioctl(fd, T_WAIT_ALL, &list) != 0) {
			perror("ioctl");
			return;
		}
	} else {
		if (ioctl(fd, T_WAIT, &list) != 0) {
			perror("ioctl");
			return;
		}
	}
}

int main(int argc, char ** argv)
{
	int fd = 0;
	int i, async = 0;
	char user_string[T_BUF_STR];

	fd = open(T_PATH, O_RDWR);

	if (fd == -1) {
		perror("open");
		if (errno == ENOENT)
			printf("Il faudrait charger le module avant.\n");
		exit(EXIT_FAILURE);
	}

	memset(user_string, 0, T_BUF_STR);
	memset(user_strings, 0, T_BUF_STR);

	while (prompt_user(user_string, T_BUF_STR) > 0) {
		user_strings[0] = strtok(user_string, " ");

		for (i=1; (user_strings[i] = strtok(NULL, " ")) != NULL; i++);

		if ((user_strings[i-1]) && (*user_strings[i-1] == '&'))
			async = 1;

		if (lazy_cmp(user_strings[0], "help") == 0) {
			list_commandes();
			goto cleanup;
		}

		if (lazy_cmp(user_strings[0], "meminfo") == 0) {
			meminfo(fd, async);
			goto cleanup;
		}

		if (lazy_cmp(user_strings[0], "modinfo") == 0) {
			modinfo(fd, user_strings[1]);
			goto cleanup;
		}

		if (lazy_cmp(user_strings[0], "kill") == 0) {
			kill(fd, user_strings[1], user_strings[2], async);
			goto cleanup;
		}

		if (lazy_cmp(user_strings[0], "waitall") == 0) {
			printf("sending waitall\n");
			t_wait(fd, 1);
			goto cleanup;
		}

		if (lazy_cmp(user_strings[0], "wait") == 0) {
			printf("sending wait\n");
			t_wait(fd, 0);
			goto cleanup;
		}

		printf("usage: %s commande [args]\n", argv[0]);
		printf("help pour la liste des commandes\n");

	cleanup:
		memset(user_string, 0, T_BUF_STR);
		memset(user_strings, 0, T_BUF_STR);
		async = 0;

	}
	printf("\n");

	close(fd);

	return EXIT_SUCCESS;
}
