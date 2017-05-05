#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

/* Include for later .h empty for now */
#include <terminus.h>

#define T_PATH "/dev/terminus"

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
	info_module.arg = (char *) malloc(T_BUF_STR * sizeof(char));

	ptr = info_module.arg;


	memset(info_module.arg, 0, T_BUF_STR);


	fd = open(T_PATH, O_RDWR);

	if (fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	printf("> ");
	fflush(stdout);

	while ((nb_read = read(STDIN_FILENO, user_string, T_BUF_STR)) >= 0) {
		memset(user_string, 0, T_BUF_STR);
		memset(user_strings, 0, T_BUF_STR);



		user_strings[0] = strtok(user_string, " ");

		for (i=1; (user_strings[i] = strtok(NULL, " ")) != NULL; i++);


		if (argc <= 1) {
			printf("usage: %s commande [args]\n", argv[0]);
			printf("lsmod: list all modules\n");
			return 1;
		}

		if (strcmp(argv[1], "meminfo") == 0) {
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

		if (strcmp(argv[1], "modinfo") == 0) {
			if (argc <= 3) {
				printf("Il faut fournir un nom de module\n");
				exit(EXIT_FAILURE);
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
			free(ptr);
		}

		if (strcmp(argv[1], "kill") == 0) {
			if (argc < 4) {
				printf("Il faut fournir un pid et un signal\n");
			}
		}
		printf("> ");
		fflush(stdout);


	}

	close(fd);

	return EXIT_SUCCESS;
}
