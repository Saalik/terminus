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
		if (ioctl(fd, T_MEMINFO, &infos) == 0) {
			printf("TotalRam\t%llu pages\n", infos.totalram);
			printf("SharedRam\t%llu pages\n", infos.sharedram);
			printf("FreeRam\t\t%llu pages\n", infos.freeram);
			printf("BufferRam\t%llu pages\n", infos.bufferram);
			printf("TotalHigh\t%llu pages\n", infos.totalhigh);
			printf("FreeHigh\t%llu pages\n", infos.freehigh);
			printf("Memory unit\t%llu bytes\n", infos.mem_unit);
		}
		else {
			perror("ioctl");
			exit(EXIT_FAILURE);
		}
	}

	if (strcmp(argv[1], "modinfo") == 0) {
		if (argc != 3) {
			printf("Il faut fournir un nom de module\n");
			exit(EXIT_FAILURE);
		}
	}

	free(ptr);
	close(fd);

	return EXIT_SUCCESS;
}
