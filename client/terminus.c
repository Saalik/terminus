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

#define T_PATH "/tmp/terminus"

int main(int argc, char ** argv)
{
	int fd = 0;

	char *ptr = NULL;
	struct my_infos infos;

	if (argc == 1) {
		perror("arguments");
		exit(EXIT_FAILURE);
	}



	fd = open(T_PATH, O_RDWR);

	if (fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	ptr = (char *) malloc(T_BUF_SIZE * sizeof(char));

	if (ptr == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	if (argc <= 1) {
		printf("usage: %s commande [args]\n", argv[0]);
		printf("lsmod: list all modules\n");
		return 1;
	}

	if (strcmp(argv[1], "meminfo") == 0) {
		printf("meminfo\n");
		if (ioctl(fd, T_MEMINFO, &infos) == 0) {
			printf("meminfo done\n");
			printf("%llu uptime\n", infos.uptime);
			printf("loads\n%llu\n%llu\n%llu\n", infos.loads[0], infos.loads[1], infos.loads[2]);
			printf("totalram %llu\n", infos.totalram);
		}
	}

	free(ptr);
	close(fd);

	return EXIT_SUCCESS;
}
