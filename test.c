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
	int fd = open("./cc.DATA", O_RDWR | O_CREAT, 7777);
	// char buf[] = "123456789";
	// int res = write(fd, buf, strlen(buf));
	// if (res != strlen(buf))
	// 	perror("write error");
	// off_t pos = lseek(fd, 10, SEEK_CUR);
	// // char buf1[] = "abcdefghi";
	// char buf1[] = "我是大帅哥";
	// res = write(fd, buf1, strlen(buf1));
	// if (res != strlen(buf1))
	// 	perror("write error");


	// char buf2[256] = {'\0'};
	// read(fd, buf2, 9);
	// PRINTF_STRING(buf2);
	// // lseek(fd, 10, SEEK_CUR);
	// memset(buf2, 0, 256);
	// int res = read(fd, buf2, 10);
	// if (res != 10){
	// 	PRINTF_INT(res);
	// 	perror("read error");
	// }
	// PRINTF_STRING(buf2);
	// memset(buf2, 0, 256);
	// read(fd, buf2, 15);
	// PRINTF_STRING(buf2);

	PRINTF_INT(SEEK_HOLE);


	// char buf1[] = "abcdefghijklm";
	// off_t pos = lseek(fd, 9, SEEK_CUR);
	// int res = write(fd, buf1, strlen(buf1));
	// if (res != strlen(buf1))
	// 	perror("write error");

	fsync(fd);
	close(fd);
	return 0;
}