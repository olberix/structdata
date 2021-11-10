#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#define PRINTF_INT(X) printf("%d\n", (X));
#define PRINTF_INT64(X) printf("%x\n", (X));
#define PRINTF_FLOAT(X) printf("%f\n", (X));
#define PRINTF_STRING(X) printf("%s\n", (X));

extern int fsync(int);
int main(int argc, char const *argv[])
{
	int fd = open("./cc.DATA", O_RDWR | O_CREAT, 777);
	char buf[] = "12345678";
	for (int i = 0; i < 4096 / 8; i++){
		write(fd, buf, strlen(buf));
	}
	for (int i = 0; i < 4096 / 8; i++){
		write(fd, buf, strlen(buf));
	}
	lseek(fd, 2048, SEEK_CUR);
	for (int i = 0; i < 4096 / 8; i++){
		write(fd, buf, strlen(buf));
	}
	for (int i = 0; i < 4096 / 8; i++){
		write(fd, buf, strlen(buf));
	}

	// fallocate(fd, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE, 0, 4096);
	// fallocate(fd, FALLOC_FL_PUNCH_HOLE, 4096, 4096);
	off_t pos = lseek(fd, 0, SEEK_HOLE);
	PRINTF_INT(pos);

	// fallocate  释放小于4KB空间

	fsync(fd);
	close(fd);
	return 0;
}