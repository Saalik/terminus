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

ssize_t prompt_user(char *string, size_t count)
{
	printf("> ");
	fflush(stdout);

	return read(STDIN_FILENO, string, count);
}

void list_commandes()
{
	printf("modinfo [module]: infos sur un module noyau\n"
	       "meminfo: infos sur la mémoire\n"
	       );
}

/* s2 est supposé directement être un string */
size_t lazy_cmp(char *s1, char *s2) {
	return strncmp(s1, s2, strlen(s2));
}


int main(int argc, char ** argv)
{
	int fd = 0;
	int i;
	int nb_read = 0;
	char user_string[T_BUF_STR];
	char *user_strings[T_BUF_STR];
	char *ptr = NULL;
	struct my_infos infos;
	union arg_infomod info_module;
	struct signal_s sig;
	info_module.arg = (char *) malloc(T_BUF_STR * sizeof(char));

	ptr = info_module.arg;

	memset(info_module.arg, 0, T_BUF_STR);


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

		if (lazy_cmp(user_strings[0], "help") == 0) {
			list_commandes();
			goto cleanup;
		}

		if (strncmp(user_strings[0], "meminfo", strlen("meminfo")) == 0) {
			if (ioctl(fd, T_MEMINFO, &infos) == 0) {
				printf("TotalRam\t%llu pages\n", infos.totalram);
				printf("SharedRam\t%llu pages\n", infos.sharedram);
				printf("FreeRam\t\t%llu pages\n", infos.freeram);
				printf("BufferRam\t%llu pages\n", infos.bufferram);
				printf("TotalHigh\t%llu pages\n", infos.totalhigh);
				printf("FreeHigh\t%llu pages\n", infos.freehigh);
				printf("Memory unit\t%llu bytes\n", infos.mem_unit);
				continue;
			}
			else {
				perror("ioctl");
				exit(EXIT_FAILURE);
			}
		}

		if (lazy_cmp(user_strings[0], "modinfo") == 0) {
			if (user_strings[1] == NULL) {
				printf("Il faut fournir un nom de module\n");
				goto cleanup;
			}

			strcpy(info_module.arg, argv[2]);
			printf("info_module.arg = %s\n", info_module.arg);
			if (ioctl(fd, T_MODINFO, &info_module) == 0) {
				if (info_module.data.module_core == NULL) {
					printf("Module pas trouvé\n");
				}
				else {
					printf("%s\n%s\n%p\n%d arguments:\n%s\n",
					       info_module.data.name,
					       info_module.data.version,
					       info_module.data.module_core,
					       info_module.data.num_kp,
					       info_module.data.args);
				}

			}

			else {
				printf("ioctl modinfo setjdrhgs\n");
			}
			goto cleanup;
		}

		if (lazy_cmp(user_strings[0], "kill") == 0) {
			if (user_strings[2] == NULL) {
				printf("Il faut fournir un pid et un signal\n");
				goto cleanup;
			}

			sig.pid = atoi(user_strings[1]);
			sig.sig = atoi(user_strings[2]);

			if (ioctl(fd, T_KILL, &sig) != 0) {
				perror("ioctl");
			}

			goto cleanup;
		}

		printf("usage: %s commande [args]\n", argv[0]);
		printf("help pour la liste des commandes\n");
		goto cleanup;
	cleanup:
		memset(user_string, 0, T_BUF_STR);
		memset(user_strings, 0, T_BUF_STR);

	}
	printf("\n");
	free(ptr);
	close(fd);

	return EXIT_SUCCESS;
}
