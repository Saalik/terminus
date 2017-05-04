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

	fd = open(PATH, O_RDWR);

	if (fd == -1) {
		print("no fd");
		exit(1);
	}

	ptr = (int *) malloc(T_BUF_SIZE * sizeof(char));

	if (ptr == NULL) {
		printf("no malloc");
		exit(1);
	}

	if (argc <= 1) {
		printf("usage wesh\n");
		return 1;
	}

	if (strcmp(argv[1], "lsmod") == 0) {
		printf("lsmod\n");
		if (ioctl(fd, T_LSMOD, ptr) == 0) {
			printf("%s", ptr);
		}
	}

	close(fd);



	return 0;

}
