#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "sys/ioctl.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include <poll.h>
#include <sys/select.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>

struct display_stru {
	int x;
	int y;
	char *buf;
};

int main(int argc, char *argv[])
{
	int i;
	int fd;
	char *filename;
	struct display_stru dis_format;
	filename = argv[1];
	fd = open(filename, O_RDWR);
	if(fd < 0){
		printf("can't open file %s\r\n", filename);
		return -1;
	}

	dis_format.x = 0;
	dis_format.y = 0;
	dis_format.buf = "zyy welcome anhui hefi !";

	printf("%d \r\n", sizeof(int));
	printf("%d \r\n", sizeof(char *));
	printf("%d \r\n", sizeof(dis_format));
	while (1){
		usleep(200000);
		write(fd, &dis_format, sizeof(struct display_stru));
	}
	close(fd);

	return 0;
}
