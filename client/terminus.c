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


int main(int argc, char ** argv)
{
	if (argc <= 1) {
		printf("usage wesh\n");
		return 1;
	}

	if (strcmp(argv[1], "lsmod") == 0) {
		printf("lsmod\n");
	}



	return 0;

}
